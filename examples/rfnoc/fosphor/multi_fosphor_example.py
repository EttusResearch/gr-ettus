#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2021 Ettus Research
# SPDX-License-Identifier: GPL-3.0
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#
#
# Title: multi_fosphor_example
# GNU Radio version: 3.8.2.0
#
# This program illustrates the use of GNU Radio and gr-ettus's Fosphor RFNoC
# block and display support to show real-time spectra acquired from an RFNoC-
# enabled USRP.
#
# The program scans the RFNoC image of the USRP, looking for Fosphor-
# enabled radio channels consisting of a radio, optional DDC, FFT, and Fosphor
# RFNoC block connected together. For each one of these channels found, a
# GNU Radio flowgraph is created to connect the blocks to a Fosphor display
# block to show the spectrum. Each Fosphor channel also has an interactive
# GUI, allowing channel parameters and configuration to be modified in real
# time. The interactive GUI can be disabled, leaving more display real estate
# for the Fosphor spectrum display.

from distutils.version import StrictVersion

if __name__ == "__main__":
    import ctypes
    import sys

    if sys.platform.startswith("linux"):
        try:
            x11 = ctypes.cdll.LoadLibrary("libX11.so")
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")
import gnuradio.uhd
from gnuradio import gr
from gnuradio.filter import firdes
from gnuradio.eng_arg import eng_float, intx
from gnuradio.qtgui import Range, RangeWidget
from gnuradio import qtgui
from PyQt5 import Qt
from PyQt5.QtCore import QObject, pyqtSlot
from gnuradio import eng_notation
import sip
import ettus
import logging
import sys
import signal
import argparse
import re
import numpy as np
import uhd
from datetime import datetime, timedelta

class LogFormatter(logging.Formatter):
    """Log formatter which prints the timestamp with fractional seconds"""

    @staticmethod
    def pp_now():
        """Returns a formatted string containing the time of day"""
        now = datetime.now()
        return "{:%H:%M}:{:05.2f}".format(now, now.second + now.microsecond / 1e6)

    def formatTime(self, record, datefmt=None):
        converter = self.converter(record.created)
        if datefmt:
            formatted_date = converter.strftime(datefmt)
        else:
            formatted_date = LogFormatter.pp_now()
        return formatted_date


# Parses a number with an SI unit suffix, returning a float.  Supported suffixes
# are 'M' (mega, 10^6), 'K' or 'k' (kilo, 10^3), and 'G' (giga, 10^9).
def parse_si_value(value):
    multipliers = {
        'M': 1e6,
        'K': 1e3,
        'k': 1e3,
        'G': 1e9
    }
    for m in multipliers:
        if value.endswith(m):
            return float(value.rstrip(m)) * multipliers[m]
    return float(value)

# Parses an RFNoC block ID string, returning a dictionary containing the device
# number, block instance, name, and port number extracted from the string.
def parse_block_id(block_id):
    info = {}
    block_split = block_id.split(":")
    block = uhd.rfnoc.BlockID(block_split[0])
    info["device_num"] = block.get_device_no()
    info["block_count"] = block.get_block_count()
    info["block_name"] = block.get_block_name()
    info["port_num"] = int(block_split[1])
    return info


# This class encapsulates a complete set of objects that comprise a complete
# 'Fosphor display unit' for a single radio channel, including the GNU Radio
# blocks that make up the flowgraph, the Qt widgets that form the GUI for the
# Fosphor chain (the Fosphor spectrum display and the channel controls, if the
# channel controls are not disabled), and the parameters used to configure the
# chain (e.g., center frequency, channel gain, Fosphor display options, etc.).
#
# All enabled GUI elements associated with the Fosphor channel are aggregated
# together into a single Qt.QWidget from which this class derives. This allows
# multiple instances of this class to be created and added to a master layout
# for displaying multiple Fosphor channels simultaneously.
#
# Also, as this class derives from gr.top_block, each instance is its own
# top-level flowgraph, able to be started and stopped individually. Note,
# however, that the demo does not support this usage, but starts and stops all
# enabled Fosphor chains together.
class fosphor_creator(gr.top_block, Qt.QWidget):

    # The constructor takes the settings dictionary, containing the desired
    # configuration for the all of the Fosphor channels, a list of strings of
    # RFNoC block IDs that need to be connected together to build the RFNoC
    # portion of the Fosphor graph, and a number enumerating which Fosphor
    # channel this instance represents (0 for the first, 1 for the second etc.).
    def __init__(
        self, settings_dict, fosphor_chain_strings, fosphor_num
    ):
        gr.top_block.__init__(self, f"Fosphor display unit {fosphor_num}")
        Qt.QWidget.__init__(self)

        self.rfnoc_graph = settings_dict["global"]["rfnoc_graph"]
        self.fosphor_layout = Qt.QGridLayout(self)
        self.settings_dict = settings_dict
        self.fosphor_num = fosphor_num
        self.fosphor_position = 3

        # Creates, configures, and returns the radio RFNoC block from the
        # provided block ID string.
        def create_radio(block_id):
            block_info = parse_block_id(block_id)
            device = block_info["device_num"]
            instance = block_info["block_count"]
            self.radio_port = int(block_info["port_num"])
            radio = self.settings_dict["global"]["rfnoc_block_refs"].setdefault(
                block_id,
                ettus.rfnoc_rx_radio(
                    self.rfnoc_graph,
                    gnuradio.uhd.device_addr(
                        "spp={}".format(self.settings_dict["global"]["fft_size"])
                    ),
                    device,
                    instance
                )
            )
            radio.set_antenna(
                self.settings_dict["channels"][self.fosphor_num]["channel_type"],
                self.radio_port,
            )
            radio.set_frequency(
                self.settings_dict["channels"][fosphor_num]["freq"], self.radio_port
            )
            radio.set_gain(
                self.settings_dict["channels"][fosphor_num]["gain"], self.radio_port
            )
            radio.set_int_property(
                "spp", self.settings_dict["global"]["fft_size"], self.radio_port
            )
            try:
                radio.set_rx_dc_offset(True, self.radio_port)
            except:
                pass
            radio.set_iq_balance(False, self.radio_port)
            return radio

        # Creates, configures, and returns the DDC RFNoC block from the
        # provided block ID string.
        def create_ddc(block_id):
            block_info = parse_block_id(block_id)
            device = block_info["device_num"]
            instance = block_info["block_count"]
            port = int(block_info["port_num"])
            ddc_key = f"{device}/DDC#{instance}"
            if ddc_key in self.settings_dict["global"]["rfnoc_block_refs"]:
                ddc = self.settings_dict["global"]["rfnoc_block_refs"][ddc_key]
            else:
                ddc = ettus.rfnoc_ddc(
                    self.rfnoc_graph, gnuradio.uhd.device_addr(""), device, instance)
                self.settings_dict["global"]["rfnoc_block_refs"][ddc_key] = ddc
            ddc.set_output_rate(self.settings_dict["global"]["bandwidth"], port)
            ddc.set_freq(0, port)
            return ddc

        # Creates, configures, and returns the FFT RFNoC block from the
        # provided block ID string.
        def create_fft(block_id):
            block_info = parse_block_id(block_id)
            device = block_info["device_num"]
            instance = block_info["block_count"]
            port = block_info["port_num"]
            fft = self.settings_dict["global"]["rfnoc_block_refs"].setdefault(
                f"{device}/FFT#{instance}",
                ettus.rfnoc_block_generic(
                    self.rfnoc_graph,
                    gnuradio.uhd.device_addr(""),
                    "FFT",
                    device,
                    instance
                )
            )
            fft.set_int_property("length", self.settings_dict["global"]["fft_size"])
            fft.set_int_property("direction", 1)
            fft.set_int_property("magnitude", 0)
            fft.set_int_property("fft_scaling", 1706)
            fft.set_int_property("shift_config", 0)
            return fft

        # Creates, configures, and returns the Fosphor RFNoC block from the
        # provided block ID string.
        def create_fosphor(block_id):
            block_info = parse_block_id(block_id)
            device = block_info["device_num"]
            instance = block_info["block_count"]
            port = block_info["port_num"]
            fosphor = self.settings_dict["global"]["rfnoc_block_refs"].setdefault(
                f"{device}/Fosphor#{instance}",
                ettus.rfnoc_block_generic(
                    self.rfnoc_graph,
                    gnuradio.uhd.device_addr(""),
                    "Fosphor",
                    0,
                    instance
                )
            )

            # Determines the decimation rate to set on the Fosphor RFNoC block
            # using a similar formula to that in the Fosphor display block C++
            # code, but without needing to hook up a message port.
            def calculate_fosphor_decim():
                decim = self.settings_dict["global"]["bandwidth"] / \
                    (float(self.settings_dict["global"]["fft_size"]) *
                    float(self.settings_dict["global"]["power_bins"]) *
                    float(self.settings_dict["global"]["frame_rate"]))
                return decim

            fosphor.set_bool_property(
                "enable_dither", self.settings_dict["channels"][fosphor_num]["dither"]
            )
            fosphor.set_int_property("hist_decimation", calculate_fosphor_decim())
            fosphor.set_int_property(
                "offset", self.settings_dict["channels"][fosphor_num]["offset"]
            )
            fosphor.set_int_property(
                "scale", self.settings_dict["channels"][fosphor_num]["scale"]
            )
            fosphor.set_int_property(
                "trise", self.settings_dict["channels"][fosphor_num]["rise"]
            )
            fosphor.set_int_property(
                "tdecay", self.settings_dict["channels"][fosphor_num]["decay"]
            )
            fosphor.set_int_property(
                "alpha", self.settings_dict["channels"][fosphor_num]["alpha"]
            )
            fosphor.set_int_property(
                "epsilon", self.settings_dict["channels"][fosphor_num]["max_hold_decay"]
            )
            fosphor.set_int_property(
                "wf_mode", self.settings_dict["channels"][fosphor_num]["wf_mode"]
            )
            fosphor.set_bool_property(
                "enable_noise", self.settings_dict["channels"][fosphor_num]["randomization"]
            )
            fosphor.set_int_property("wf_decimation", calculate_fosphor_decim())
            fosphor.set_int_property("wf_predivision_ratio", 0)
            fosphor.set_bool_property("clear_history", True)
            fosphor.set_bool_property("enable_histogram", True)
            fosphor.set_bool_property("enable_waterfall", True)
            return fosphor

        # Creates, configures, and returns an RFNoC streamer object.
        def create_streamer():
            streamer = ettus.rfnoc_rx_streamer(
                self.rfnoc_graph,
                1,
                gnuradio.uhd.stream_args(
                    cpu_format="s8",
                    otw_format="s8",
                    channels=[],
                    args="spp={}".format(self.settings_dict["global"]["fft_size"]),
                ),
                self.settings_dict["global"]["fft_size"],
                True,
            )
            return streamer

        # Creates, configures, and returns the Fosphor Qt display GNU Radio
        # block and adds it to the layout for the widgets for the Fosphor chain.
        def create_fosphor_display():
            display = ettus.fosphor_display(
                self.settings_dict["global"]["fft_size"],
                self.settings_dict["global"]["power_bins"],
                self.settings_dict["global"]["fft_size"],
            )
            display.set_frame_rate(self.settings_dict["global"]["frame_rate"])
            display.set_frequency_range(
                self.settings_dict["channels"][self.fosphor_num]["freq"],
                self.settings_dict["global"]["bandwidth"],
            )
            display.set_palette(
                self.settings_dict["channels"][self.fosphor_num]["color"]
            )
            display.set_waterfall(True)
            display.set_grid(True)
            _fosphor_display_win = sip.wrapinstance(display.pyqwidget(), Qt.QWidget)
            self.fosphor_layout.addWidget(_fosphor_display_win, 1, 2, 7, 4)
            for r in range(1, 8):
                self.fosphor_layout.setRowStretch(r, 1)
            for c in range(2, 6):
                self.fosphor_layout.setColumnStretch(c, 1)
            return display

        # Calls the appropriate GNU Radio block creation and configuration
        # function above based on the name of the block in `block_id`.
        def create_block(block_id):
            if "Radio" in block_id:
                block = create_radio(block_id)
            elif "DDC" in block_id:
                block = create_ddc(block_id)
            elif "FFT" in block_id:
                block = create_fft(block_id)
            elif "Fosphor" in block_id:
                block = create_fosphor(block_id)
            else:
                assert False
            return block

        # For the Fosphor chain described in `fosphor_chain_strings`, creates
        # all GNU Radio blocks that are required for the chain, including the
        # RFNoC streamers and the Fosphor Qt display block (which are not part
        # of the chain string), returning the results in a list.
        def create_fosphor_chain(fosphor_chain_strings):
            gr_blocks = []
            for block in fosphor_chain_strings:
                gr_block = create_block(block)
                gr_blocks.append(gr_block)
            gr_blocks.append(create_streamer()) # hist
            gr_blocks.append(create_streamer()) # wf
            gr_blocks.append(create_fosphor_display())
            return gr_blocks

        # Connects all the GNU Radio blocks in the Fosphor chain together.
        # The expected blocks in `fosphor_chain`, in order, are:
        #
        # Radio -> DDC(Optional) -> FFT -> Fosphor -> Streamer -> Fosphor Display
        #                                         \-> Streamer ->/
        # ^____________ RFNoC domain ___________________^ ^___ Host domain ___^
        #
        # If the DDC is not detected, the function will shift all the indices
        # back to adjust for its absence. The function also takes the string
        # describing the Fosphor chain, using the port number in each block to
        # ensure the correct ports on the GNU Radio blocks are connected.
        def connect_blocks(fosphor_chain, fosphor_chains_strings):
            radio_info = parse_block_id(fosphor_chain_strings[0])
            second_block_info = parse_block_id(fosphor_chain_strings[1])
            third_block_info = parse_block_id(fosphor_chain_strings[2])
            fourth_block_info = parse_block_id(fosphor_chain_strings[3])
            self.rfnoc_graph.connect(
                fosphor_chain[0].get_unique_id(),
                radio_info["port_num"],
                fosphor_chain[1].get_unique_id(),
                second_block_info["port_num"],
                False,
            )
            self.rfnoc_graph.connect(
                fosphor_chain[1].get_unique_id(),
                second_block_info["port_num"],
                fosphor_chain[2].get_unique_id(),
                third_block_info["port_num"],
                False,
            )
            if "DDC" in second_block_info["block_name"]:
                self.rfnoc_graph.connect(
                    fosphor_chain[2].get_unique_id(),
                    third_block_info["port_num"],
                    fosphor_chain[3].get_unique_id(),
                    fourth_block_info["port_num"],
                    False,
                )
                self.fosphor_position = 3
            else:
                self.fosphor_position = 2
            self.rfnoc_graph.connect(
                fosphor_chain[self.fosphor_position].get_unique_id(),
                0,
                fosphor_chain[self.fosphor_position + 1].get_unique_id(),
                0,
                False,
            )
            self.rfnoc_graph.connect(
                fosphor_chain[self.fosphor_position].get_unique_id(),
                1,
                fosphor_chain[self.fosphor_position + 2].get_unique_id(),
                0,
                False,
            )
            self.connect(
                (fosphor_chain[self.fosphor_position + 1], 0),
                (fosphor_chain[self.fosphor_position + 3], 0),
            )
            self.connect(
                (fosphor_chain[self.fosphor_position + 2], 0),
                (fosphor_chain[self.fosphor_position + 3], 1),
            )

        # Creates and configures the Qt widgets that comprise the GUI controls
        # for a single Fosphor channel, and then adds them to the layout.
        def create_gui(fosphor_num):
            starting_row = 0
            starting_col = 0

            # Setup for Gain Widget
            gain_row = starting_row
            gain_col = starting_col + 2
            gain_height = 1
            gain_width = 4
            self.channel_gain = settings_dict["channels"][fosphor_num]["gain"]
            self._channel_gain_range = Range(settings_dict["channels"][self.fosphor_num]["min_gain"], settings_dict["channels"][self.fosphor_num]["max_gain"], 1.0, self.channel_gain, 200)
            self._channel_gain_win = RangeWidget(self._channel_gain_range, self.set_channel_gain, 'Gain', "counter_slider", float)
            self.fosphor_layout.addWidget(self._channel_gain_win, gain_row, gain_col, gain_height, gain_width)
            for r in range(gain_row, gain_row + gain_height):
                self.fosphor_layout.setRowStretch(r, 0)
            for c in range(gain_col, gain_col + gain_width):
                self.fosphor_layout.setColumnStretch(c, 0)

            # Setup for Frequency Widget
            freq_row = starting_row
            freq_col = starting_col
            freq_height = 1
            freq_width = 2
            self.channel_freq = settings_dict["channels"][fosphor_num]["freq"]
            self._freq_tool_bar = Qt.QToolBar(self)
            self._freq_tool_bar.addWidget(Qt.QLabel('Center Frequency' + ": "))
            self._freq_line_edit = Qt.QLineEdit(str(self.channel_freq))
            self._freq_tool_bar.addWidget(self._freq_line_edit)
            self._freq_change_button = Qt.QPushButton("Tune")
            self._freq_change_button.released.connect(lambda: self.channel_freq_control_callback(self._freq_line_edit.text()))
            self._freq_tool_bar.addWidget(self._freq_change_button)
            self.fosphor_layout.addWidget(self._freq_tool_bar, freq_row, freq_col, freq_height, freq_width)
            for r in range(freq_row, freq_row + freq_height):
                self.fosphor_layout.setRowStretch(r, 0)
            for c in range(freq_col, freq_col + freq_width):
                self.fosphor_layout.setColumnStretch(c, 0)

            # Setup for Waterfall Mode
            wf_row = starting_row + 4
            wf_col = starting_col
            wf_height = 1
            wf_width = 1

            self._waterfall_mode_options = [0, 1]
            self._waterfall_mode_labels = ['Max Hold', 'Average']
            self._waterfall_mode = settings_dict["channels"][fosphor_num]["wf_mode"]
            self._waterfall_mode_tool_bar = Qt.QToolBar(self)
            self._waterfall_mode_tool_bar.addWidget(Qt.QLabel('Waterfall Mode' + ": "))
            self._waterfall_mode_combo_box = Qt.QComboBox()
            self._waterfall_mode_tool_bar.addWidget(self._waterfall_mode_combo_box)
            for _label in self._waterfall_mode_labels: self._waterfall_mode_combo_box.addItem(_label)
            self._waterfall_mode_combo_box.currentIndexChanged.connect(self.waterfall_mode_control_callback)
            self.fosphor_layout.addWidget(self._waterfall_mode_tool_bar, wf_row, wf_col, wf_height, wf_width)
            for r in range(wf_row, wf_row + wf_height):
                self.fosphor_layout.setRowStretch(r, 0)
            for c in range(wf_col, wf_col + wf_width):
                self.fosphor_layout.setColumnStretch(c, 0)

            # Setup for Rise Time
            self._rise_time_range = Range(0, 65535, 1, 4096, 200)
            self._rise_time_win = RangeWidget(self._rise_time_range, self.set_rise_time, 'Rise Time', "slider", float)

            # Setup for Randomization
            rand_row = starting_row + 1
            rand_col = starting_col + 1
            rand_height = 1
            rand_width = 1
            self._randomization_enable_options =[False, True]
            self._randomization_enable_labels = ['Disabled', 'Enabled']
            self.randomization_enable = settings_dict["channels"][fosphor_num]["randomization"]
            self._randomization_enable_tool_bar = Qt.QToolBar(self)
            self._randomization_enable_tool_bar.addWidget(Qt.QLabel('Randomization' + ": "))
            self._randomization_enable_combo_box = Qt.QComboBox()
            self._randomization_enable_tool_bar.addWidget(self._randomization_enable_combo_box)
            for _label in self._randomization_enable_labels: self._randomization_enable_combo_box.addItem(_label)
            self._randomization_enable_combo_box.currentIndexChanged.connect(self.randomization_enable_control_callback)
            Qt.QMetaObject.invokeMethod(self._randomization_enable_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._randomization_enable_options.index(self.randomization_enable)))
            self.fosphor_layout.addWidget(self._randomization_enable_tool_bar, rand_row, rand_col, rand_height, rand_width)
            for r in range(rand_row, rand_row + rand_height):
                self.fosphor_layout.setRowStretch(r, 0)
            for c in range(rand_col, rand_col + rand_width):
                self.fosphor_layout.setColumnStretch(c, 0)

            # Setup for Max Hold Decay
            self._max_hold_decay_range = Range(0, 65535, 1, 1, 200)
            self._max_hold_decay_win = RangeWidget(self._max_hold_decay_range, self.set_max_hold, 'Max Hold Decay', "slider", int)

            # Setup for Histogram Scale
            self._hist_scale_range = Range(0, 65535, 1, 256, 200)
            self._hist_scale_win = RangeWidget(self._hist_scale_range, self.set_hist_scale, 'Scale', "slider", float)

            # Setup for Histogram Offset
            self._hist_offset_range = Range(0, 65535, 1, 0, 200)
            self._hist_offset_win = RangeWidget(self._hist_offset_range, self.set_hist_offset, 'Offset', "slider", float)

            # Setup for Dither
            dither_row = starting_row + 1
            dither_col = starting_col
            dither_height = 1
            dither_width = 1
            self._dither_enable_options = [False, True]
            self._dither_enable_labels = ['Disabled', 'Enabled']
            self.dither_enable = self.settings_dict["channels"][fosphor_num]["dither"]
            self._dither_enable_tool_bar = Qt.QToolBar(self)
            self._dither_enable_tool_bar.addWidget(Qt.QLabel('Dither' + ": "))
            self._dither_enable_combo_box = Qt.QComboBox()
            self._dither_enable_tool_bar.addWidget(self._dither_enable_combo_box)
            for _label in self._dither_enable_labels: self._dither_enable_combo_box.addItem(_label)
            Qt.QMetaObject.invokeMethod(self._dither_enable_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._dither_enable_options.index(self.dither_enable)))
            self._dither_enable_combo_box.currentIndexChanged.connect(self.dither_enable_control_callback)
            self.fosphor_layout.addWidget(self._dither_enable_tool_bar, dither_row, dither_col, dither_height, dither_width)
            for r in range(dither_row, dither_row + dither_height):
                self.fosphor_layout.setRowStretch(r, 0)
            for c in range(dither_col, dither_col + dither_width):
                self.fosphor_layout.setColumnStretch(c, 0)

            # Setup for Decay Time
            self._decay_time_range = Range(0, 65535, 1, 16384, 200)
            self._decay_time_win = RangeWidget(self._decay_time_range, self.set_decay_time, 'Decay Time', "slider", int)

            # Setup for Histogram Color
            color_row = starting_row + 4
            color_col = starting_col + 1
            color_height = 1
            color_width = 1
            self._color_options = ['iron', 'cubehelix', 'sdrangelove_histogram', 'rainbow', 'prog']
            self._color_labels = ['Iron', 'Cube Helix', 'SDRangeLove', 'Rainbow', "Prog's"]
            self.color = color = settings_dict["channels"][fosphor_num]["color"]
            self._color_tool_bar = Qt.QToolBar(self)
            self._color_tool_bar.addWidget(Qt.QLabel('Color' + ": "))
            self._color_combo_box = Qt.QComboBox()
            self._color_tool_bar.addWidget(self._color_combo_box)
            for _label in self._color_labels: self._color_combo_box.addItem(_label)
            self._color_combo_box.currentIndexChanged.connect(self.color_control_callback)
            Qt.QMetaObject.invokeMethod(self._color_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._color_options.index(self.color)))
            self.fosphor_layout.addWidget(self._color_tool_bar, color_row, color_col, color_height, color_width)
            for r in range(color_row, color_row + color_height):
                self.fosphor_layout.setRowStretch(r, 0)
            for c in range(color_col, color_col + color_width):
                self.fosphor_layout.setColumnStretch(c, 0)

            # Setup for Averaging Alpha
            self._averaging_alpha_range = Range(65200, 65360, 1, 65280, 200)
            self._averaging_alpha_win = RangeWidget(self._averaging_alpha_range, self.set_averaging_alpha, 'Averaging Alpha', "slider", int)

            # Organize Widgets with title boxes
            spectrum_row = starting_row + 2
            spectrum_col = starting_col
            spectrum_height = 2
            spectrum_width = 2
            self.spectrum_title_box = Qt.QGroupBox("Spectrum Settings")
            self.spectrum_box = Qt.QVBoxLayout()
            self.spectrum_box.addWidget(self._averaging_alpha_win)
            self.spectrum_box.addWidget(self._max_hold_decay_win)
            self.spectrum_title_box.setLayout(self.spectrum_box)
            self.fosphor_layout.addWidget(self.spectrum_title_box, spectrum_row, spectrum_col, spectrum_height, spectrum_width)

            histogram_row = starting_row + 5
            histogram_col = starting_col
            histogram_height = 4
            histogram_width = 2
            self.histogram_title_box = Qt.QGroupBox("Histogram Settings")
            self.histogram_box = Qt.QVBoxLayout()
            self.histogram_box.addWidget(self._rise_time_win)
            self.histogram_box.addWidget(self._decay_time_win)
            self.histogram_box.addWidget(self._hist_scale_win)
            self.histogram_box.addWidget(self._hist_offset_win)
            self.histogram_title_box.setLayout(self.histogram_box)
            self.fosphor_layout.addWidget(self.histogram_title_box, histogram_row, histogram_col, histogram_height, histogram_width)

        # Creates the Fosphor GNU Radio blocks, connects them, then creates the
        # GUI controls (if enabled)
        self.fosphor_chain = create_fosphor_chain(fosphor_chain_strings)
        connect_blocks(self.fosphor_chain, fosphor_chain_strings)
        if not self.settings_dict["global"]["no_gui"]:
            create_gui(fosphor_num)

    def get_fosphor_layout(self):
        return self

    def get_channel_gain(self):
        return self.channel_gain

    def set_channel_gain(self, gain):
        if int(gain) in range(int(self.settings_dict["channels"][self.fosphor_num]["min_gain"]), int(self.settings_dict["channels"][self.fosphor_num]["max_gain"])):
                self.fosphor_chain[0].set_gain(gain, self.radio_port)
                self.channel_gain = gain
        else:
            assert False

    def channel_gain_control_changed_callback(self, gain):
        self.set_channel_gain(float(gain))

    def get_channel_freq(self):
        return self.channel_freq

    def set_channel_freq(self, freq):
        if int(freq) in range(int(self.settings_dict["channels"][self.fosphor_num]["min_freq"]), int(self.settings_dict["channels"][self.fosphor_num]["max_freq"])):
            self.fosphor_chain[0].set_frequency(freq, self.radio_port)
            self.fosphor_chain[self.fosphor_position+3].set_frequency_range(freq, self.settings_dict["global"]["bandwidth"])
            self.channel_freq = freq
        else:
            assert False

    def channel_freq_control_callback(self, freq):
        self.set_channel_freq(parse_si_value(freq))

    def update_wf_mode_widget(self, mode):
        Qt.QMetaObject.invokeMethod(self._waterfall_mode_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._waterfall_mode_options.index(mode)))

    def get_waterfall_mode(self):
        return self.fosphor_chain[self.fosphor_position].get_int_property("wf_mode")

    def set_waterfall_mode(self, mode):
        self.fosphor_chain[self.fosphor_position].set_int_property("wf_mode", mode)
        actual_mode = self.get_waterfall_mode()
        self.update_wf_mode_widget(actual_mode)

    def waterfall_mode_control_callback(self, mode):
        self.set_waterfall_mode(mode)

    def get_rise_time(self):
        return self.fosphor_chain[self.fosphor_position].get_int_property("trise")

    def set_rise_time(self, time):
        self.fosphor_chain[self.fosphor_position].set_int_property("trise", time)

    def rise_time_control_callback(self, time):
        self.set_rise_time(time)

    def get_randomization_enable(self):
        return self.fosphor_chain[self.fosphor_position].get_bool_property("enable_noise")

    def set_randomization_enable(self, rand):
        self.fosphor_chain[self.fosphor_position].set_bool_property("enable_noise", bool(rand))

    def randomization_enable_control_callback(self, rand):
        self.set_randomization_enable(rand)

    def get_max_hold(self):
        return self.fosphor_chain[self.fosphor_position].get_int_property("epsilon")

    def set_max_hold(self, max_hold):
        self.fosphor_chain[self.fosphor_position].set_int_property("epsilon", max_hold)

    def max_hold_control_callback(self, max_hold):
        self.set_max_hold(max_hold)

    def get_hist_scale(self):
        return self.fosphor_chain[self.fosphor_position].get_int_property("scale")

    def set_hist_scale(self, scale):
        self.fosphor_chain[self.fosphor_position].set_int_property("scale", scale)

    def hist_scale_control_callback(self, scale):
        self.set_hist_scale(scale)

    def get_hist_offset(self):
        return self.fosphor_chain[self.fosphor_position].get_int_property("offset")

    def set_hist_offset(self, offset):
        self.fosphor_chain[self.fosphor_position].set_int_property("offset", offset)

    def hist_offset_control_callback(self, offset):
        self.set_hist_offset(offset)

    def get_dither_enable(self):
        return self.fosphor_chain[self.fosphor_position].get_bool_property("enable_dither")

    def set_dither_enable(self, dither):
        self.fosphor_chain[self.fosphor_position].set_bool_property("enable_dither", dither)

    def dither_enable_control_callback(self, dither):
        self.set_dither_enable(bool(dither))

    def get_decay_time(self):
        return self.fosphor_chain[self.fosphor_position].get_int_property("tdecay")

    def set_decay_time(self, time):
        self.fosphor_chain[self.fosphor_position].set_int_property("tdecay", time)

    def decay_time_control_callback(self, time):
        self.set_decay_time(time)

    def get_color(self):
        return self.fosphor_chain[6].get_palette()

    def set_color(self, color_index):
        self.fosphor_chain[self.fosphor_position + 3].set_palette(self._color_options[color_index])
        self.color = self._color_options[color_index]

    def color_control_callback(self, color):
        self.set_color(color)

    def get_averaging_alpha(self):
        return self.fosphor_chain[self.fosphor_position].get_int_property("alpha")

    def set_averaging_alpha(self, alpha):
        self.fosphor_chain[self.fosphor_position].set_int_property("alpha", alpha)

    def averaging_alpha_control_callback(self, alpha):
        self.set_averaging_alpha(alpha)


# This class aggregates all of the enabled Fosphor channels' display units,
# building them upon construction and adding them to a grid layout of 2x2
# display units for four Fosphor channels maximum or Nx1 for fewer than four
# channels.
class multi_fosphor_example(Qt.QWidget):
    def __init__(self, settings_dict, fosphor_chains_strings):
        Qt.QWidget.__init__(self)
        self.setWindowTitle("multi_fosphor_example")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme("gnuradio-grc"))
        except:
            pass
        self.top_layout = Qt.QGridLayout()
        self.setLayout(self.top_layout)
        self.display_settings = Qt.QSettings("GNU Radio", "multi_fosphor_example")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(
                    self.display_settings.value("geometry").toByteArray()
                )
            else:
                self.restoreGeometry(self.display_settings.value("geometry"))
        except:
            pass

        self.settings_dict = settings_dict
        self.rfnoc_graph = settings_dict["global"]["rfnoc_graph"]
        self.fosphor_chains = fosphor_chains_strings
        self.channel_select = self.settings_dict["global"]["channel_select"]
        self.fosphor_display_units = []

        if len(self.channel_select) == 4:
            for (fosphor_num, chain) in enumerate(fosphor_chains_strings):
                fosphor_display_unit = fosphor_creator(
                    settings_dict, chain, fosphor_num
                )
                grid_coords = {0: (0, 0), 1: (0, 1), 2: (1, 0), 3: (1, 1)}
                self.top_layout.addWidget(fosphor_display_unit.get_fosphor_layout(),
                    grid_coords[fosphor_num][0], grid_coords[fosphor_num][1], 1, 1)
                self.fosphor_display_units.append(fosphor_display_unit)
        else:
            for (fosphor_num, _) in enumerate(range(len(self.channel_select))):
                fosphor_display_unit = fosphor_creator(
                    settings_dict,
                    fosphor_chains_strings[self.channel_select[fosphor_num]],
                    fosphor_num
                )
                self.top_layout.addWidget(fosphor_display_unit.get_fosphor_layout(), 0, fosphor_num, 1, 1)
                self.fosphor_display_units.append(fosphor_display_unit)

    # Starts all the individual Fosphor channels' `gr::top_block`s (i.e., the
    # GNU Radio flowgraph associated with the Fosphor channel).
    def go(self):
        for display_unit in self.fosphor_display_units:
            display_unit.start(self.settings_dict["global"]["max_noutput_items"])

    # Stops all the Fosphor channels' flowgraphs.
    def stop(self):
        for display_unit in self.fosphor_display_units:
            display_unit.stop()

    # Waits for all the GNU Radio threads to terminate.
    def wait(self):
        for display_unit in self.fosphor_display_units:
            display_unit.wait()

    # Closes the GNU Radio instance.
    def close_event(self, event):
        self.display_settings = Qt.QSettings("GNU Radio", "multi_fosphor_example")
        self.display_settings.setValue("geometry", self.saveGeometry())
        event.accept()


# Uses the RFNoC graph API to enumerate connections between RFNoC blocks in the
# RFNoC image on the radio to find Fosphor-enabled channels. A Fosphor-enabled
# channel is found if a chain of the following RFNoC blocks is found:
#
#  Radio --> DDC (optional) --> FFT --> Fosphor --> SEP
#         \----------------/                    \-> SEP
#
# The function returns a list, with each item in the list being a list of
# strings of the RFNoC block IDs (minus the stream endpoints) that comprise
# a complete Fosphor-enabled channel, e.g.:
#   [['0/Radio#0:0', '0/DDC#0:0', '0/FFT#0:0', '0/Fosphor#0:0'],  # Fosphor channel 0
#    ['0/Radio#1:0', '0/DDC#1:0', '0/FFT#1:0', '0/Fosphor#1:0']]  # Fosphor channel 1
def find_fosphor_chains(settings_dict):
    graph = uhd.rfnoc.RfnocGraph(settings_dict["global"]["args"])
    static_connections = graph.enumerate_static_connections()
    radio_list = []
    connection_dict = {}
    for edge in static_connections:
        connection_dict[edge.src_blockid + ":" + str(edge.src_port)] = (
            edge.dst_blockid + ":" + str(edge.dst_port)
        )
        if "Radio" in edge.src_blockid:
            radio_list.append(edge.src_blockid + ":" + str(edge.src_port))

    error_message = ""
    fosphor_chains = []
    connection_search = ""
    for radio in radio_list:
        chain = []
        connection_search = radio
        chain.append(radio)
        if "DDC" in connection_dict.get(connection_search):  # DDC Found
            connection_search = connection_dict.get(connection_search)
            chain.append(connection_search)
        if "FFT" in connection_dict.get(connection_search):  # FFT Found
            connection_search = connection_dict.get(connection_search)
            chain.append(connection_search)
            # Don't add an SEP found between the FFT and Fosphor blocks to the chain
            if "SEP" in connection_dict.get(connection_search):
                connection_search = connection_dict.get(connection_search)
            if "Fosphor" in connection_dict.get(connection_search):  # Fosphor found
                connection_search = connection_dict.get(connection_search)
                chain.append(connection_search)  # Adds fosphor port connected to FFT
                connection_search = (
                    connection_search[: len(connection_search) - 1] + "0"
                )  # Ensures that both ports of fosphor output are connected to SEPs
                if "SEP" in connection_dict.get(connection_search):
                    connection_search = (
                        connection_search[: len(connection_search) - 1] + "1"
                    )
                    if "SEP" in connection_dict.get(connection_search):
                        fosphor_chains.append(chain)
                        logger.debug("Successful Fosphor chain found for " + radio)
                        radio_split = radio.split(":")
                        block_id = radio_split[0]
                        block_port = int(radio_split[1])
                        radio_control = uhd.rfnoc.RadioControl(graph.get_block(block_id))
                        gain_range = radio_control.get_rx_gain_range(block_port)
                        freq_range = radio_control.get_rx_frequency_range(block_port)
                        radio_control_dict = {
                            "min_gain" : gain_range.start(),
                            "max_gain" : gain_range.stop(),
                            "min_freq" : freq_range.start(),
                            "max_freq" : freq_range.stop(),
                        }
                        settings_dict["channels"].append(radio_control_dict)

    return fosphor_chains

# Parses the command-line arguments used to configure the settings for the
# application and each Fosphor channel.
def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--args",
        help="UHD device arguments",
        type=str,
    )
    parser.add_argument(
        "--fft_size",
        default=1024,
        choices=[16, 32, 64, 128, 256, 512, 1024, 2048, 4096],
        help="# of FFT bins, must be a power of 2",
        type=int,
    )
    parser.add_argument(
        "--channel_select",
        default=[0, 1, 2, 3],
        choices=[0, 1, 2, 3],
        nargs="+",
        type=int,
        help="channel select, 0-3",
    )
    parser.add_argument("--power_bins", default=64, help="# of power bins", type=int)
    parser.add_argument("--frame_rate", default=30, help="Update rate (frames/sec)", type=int)
    parser.add_argument("--bandwidth", default="10M", type=str)
    parser.add_argument("--max_noutput_items",
        default=10e6,
        help="max_noutput_items for channel top_blocks (advanced)",
        type=int)
    parser.add_argument("--no_gui",
        help = "Disable interactive controls for Fosphor channels (show Fosphor only)",
        action = "store_true")

    # Channel Settings
    parser.add_argument(
        "--channel_freq",
        nargs="+",
        help="Channel frequencies, separated by spaces",
        type=str,
    )
    parser.add_argument(
        "--channel_gain",
        nargs="+",
        default=[10.0],
        help="Channel gains, separated by spaces",
        type=float,
    )
    parser.add_argument("--max_hold_decay", nargs="+", default=[32], type=int)
    parser.add_argument("--randomization_enable", nargs="+", default=[True], type=bool)
    parser.add_argument("--dither_enable", nargs="+", default=[True], type=bool)
    parser.add_argument(
        "--wf_mode",
        nargs="+",
        default=["max_hold"],
        choices=["max_hold", "average"],
        help="wf mode, choose between max_hold or average",
    )
    parser.add_argument("--averaging_alpha", nargs="+", default=[65280], type=int)
    parser.add_argument(
        "--rise", nargs="+", default=[4096], type=int, help="Time rise of histogram"
    )
    parser.add_argument(
        "--decay", nargs="+", default=[4096], type=int, help="Time decay of histogram"
    )
    parser.add_argument("--offset", nargs="+", default=[0], type=int)
    parser.add_argument("--scale", nargs="+", default=[256], type=int)
    parser.add_argument("--color", nargs="+", default=["iron"], type=str)
    parser.add_argument("--channel_type", nargs="+", default=["RX2"], type=str)
    return parser.parse_args()

# Takes a list of values and the index within that list to return. If that index
# is out of range, the element at index 0 of the list is returned instead.
def index_or_default(parameter, index):
    assert len(parameter) > 0
    return parameter[index] if index < len(parameter) else parameter[0]

def wf_mode_check(wf_mode):
    return {"max_hold": 0, "average": 1}.get(wf_mode)

def main():

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string("qtgui", "style", "raster")
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    # Parse the command-line parameters and populate the dictionary holding
    # application-wide (i.e., non-channel-specific) settings.
    args = parse_args()
    if args.channel_freq is None:
        logger.error("Please specify --channel_freq")
        return False
    settings_dict = {
        "global": {
            "fft_size": args.fft_size,
            "bandwidth": parse_si_value(args.bandwidth),
            "args": args.args,
            "power_bins": args.power_bins,
            "frame_rate": args.frame_rate,
            "max_noutput_items": args.max_noutput_items,
            "channel_select": args.channel_select,
            "no_gui": args.no_gui
        },
        "channels": [],
    }

    # Attempt to locate Fosphor-enabled channels on the RFNoC image of the
    # radio
    fosphor_chains_strings = find_fosphor_chains(settings_dict)
    if len(fosphor_chains_strings) < 1:
        logger.error("No fosphor chains found")
        return False

    if (settings_dict["global"]["channel_select"] != [0, 1, 2, 3]) and (
        max(settings_dict["global"]["channel_select"]) >= len(fosphor_chains_strings)
    ):
        logger.error(
            "Channel "
            + str(max(settings_dict["global"]["channel_select"]))
            + " not detected"
        )
        return False

    # Create an RFNoC graph for use in connecting the RFNoC blocks on the
    # radio to form the Fosphor channels
    rfnoc_graph = ettus.rfnoc_graph(
        gnuradio.uhd.device_addr(settings_dict["global"]["args"])
    )
    settings_dict["global"]["rfnoc_graph"] = rfnoc_graph
    settings_dict["global"]["rfnoc_block_refs"] = dict()

    # For each detected Fosphor channels, populate the channel-specific settings
    # dictionary from the appropriate command-line options
    for (chain_index, _) in enumerate(fosphor_chains_strings):
        channel_settings = {
            "freq": parse_si_value(index_or_default(args.channel_freq, chain_index)),
            "gain": index_or_default(args.channel_gain, chain_index),
            "max_hold_decay": index_or_default(args.max_hold_decay, chain_index),
            "randomization": index_or_default(args.randomization_enable, chain_index),
            "dither": index_or_default(args.dither_enable, chain_index),
            "wf_mode": wf_mode_check(index_or_default(args.wf_mode, chain_index)),
            "alpha": index_or_default(args.averaging_alpha, chain_index),
            "rise": index_or_default(args.rise, chain_index),
            "decay": index_or_default(args.decay, chain_index),
            "offset": index_or_default(args.offset, chain_index),
            "scale": index_or_default(args.scale, chain_index),
            "color": index_or_default(args.color, chain_index),
            "channel_type": index_or_default(args.channel_type, chain_index),
        }
        settings_dict["channels"][chain_index].update(channel_settings)

    # Build all the GNU Radio flowgraphs and GUI elements for each Fosphor
    # channel
    mfe = multi_fosphor_example(settings_dict, fosphor_chains_strings)
    mfe.show()

    # Start the Fosphor channel flowgraphs
    mfe.go()

    def sig_handler(sig=None, frame=None):
        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    def quitting():
        mfe.stop()

    qapp.aboutToQuit.connect(quitting)
    qapp.exec_()

    return True


if __name__ == "__main__":
    # Setup the logger with our custom timestamp formatting
    global logger
    logger = logging.getLogger(__name__)
    logger.setLevel(logging.DEBUG)
    console = logging.StreamHandler()
    logger.addHandler(console)
    formatter = LogFormatter(
        fmt="[%(asctime)s] [%(levelname)s] (%(threadName)-10s) %(message)s"
    )
    console.setFormatter(formatter)

    sys.exit(not main())
