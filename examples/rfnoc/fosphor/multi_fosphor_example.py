#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# Title: multi_fosphor_example
# GNU Radio version: 3.8.2.0
# multi_fosphor_example for all RFNoC-based NI and ettus branded radios
#
# This program is an example to run multiple fosphor_displays using GNU Radio and the RFNOC API
# Program detects number of full fosphor chains on the FPGA, which consists of a Radio, DDC, FFT, Fosphor, and SEPs all connected
# Program then creates the blocks with the channel settings specified in from the command line and connects them all
# Block creation and connection is repeated for the number of fosphor chains detected
# Fosphor displays are then added to the top block format and displayed


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

# Returns dictionary with device number, block count, name, and port number based on string block ID input
def read_id(block_id):
    info = {}
    block_split = block_id.split(":")
    block = uhd.rfnoc.BlockID(block_split[0])
    info["device_num"] = block.get_device_no()
    info["block_count"] = block.get_block_count()
    info["block_name"] = block.get_block_name()
    info["port_num"] = int(block_split[1])
    return info


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


# This class encapsulates one single set of GNU Radio blocks that form a complete fosphor chain for one radio channel
class fosphor_creator:

    def get_fosphor_layout(self):
        return self.fosphor_layout


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
        self.set_channel_freq(float(freq))


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



    # Constructor creates all the GNU Radio blocks for 1 chain and connects then
    # Uses the setting passed in through settings dictionary as well as the fosphor_chain_strings to determine the ports of the blocks
    def __init__(
        self, settings_dict, fosphor_chain_strings, fosphor_num, top_block_ref
    ):

        self.rfnoc_graph = settings_dict["global"]["rfnoc_graph"]
        self.fosphor_layout = Qt.QGridLayout()
        self.settings_dict = settings_dict
        self.fosphor_num = fosphor_num
        self.fosphor_position = 3

        # Creates and returns the radio RFNoC Block from the block_id string
        def create_radio(block_id):
            block_info = read_id(block_id)
            device = block_info["device_num"]
            instance = block_info["block_count"]
            self.radio_port = int(block_info["port_num"])
            radio = ettus.rfnoc_rx_radio(
                self.rfnoc_graph,
                gnuradio.uhd.device_addr(
                    "spp={}".format(self.settings_dict["global"]["fft_size"])
                ),
                device,
                instance,
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
            radio.set_bandwidth(
                self.settings_dict["global"]["bandwidth"], self.radio_port
            )
            radio.set_int_property(
                "spp", self.settings_dict["global"]["fft_size"], self.radio_port
            )
            radio.set_dc_offset(False, self.radio_port)
            radio.set_iq_balance(False, self.radio_port)
            return radio

        # Creates and returns the ddc RFNoC Block from the block_id string
        def create_ddc(block_id):
            block_info = read_id(block_id)
            device = block_info["device_num"]
            instance = block_info["block_count"]
            port = int(block_info["port_num"])
            ddc = ettus.rfnoc_ddc(
                self.rfnoc_graph, gnuradio.uhd.device_addr(""), device, instance
            )
            ddc.set_output_rate(self.settings_dict["global"]["bandwidth"], port)
            ddc.set_freq(0, port)
            return ddc

        # Creates and returns the fft RFNoC Block from the block_id string
        def create_fft(block_id):
            block_info = read_id(block_id)
            device = block_info["device_num"]
            instance = block_info["block_count"]
            port = block_info["port_num"]
            fft = ettus.rfnoc_block_generic(
                self.rfnoc_graph,
                gnuradio.uhd.device_addr(""),
                "FFT",
                device,
                instance,
            )
            fft.set_int_property("length", self.settings_dict["global"]["fft_size"])
            fft.set_int_property("direction", 1)
            fft.set_int_property("magnitude", 0)
            fft.set_int_property("fft_scaling", 1706)
            fft.set_int_property("shift_config", 0)
            return fft

        # Creates and returns the fosphor RFNoC Block from the block_id string
        def create_fosphor(block_id):
            block_info = read_id(block_id)
            device = block_info["device_num"]
            instance = block_info["block_count"]
            port = block_info["port_num"]
            fosphor = ettus.rfnoc_block_generic(
                self.rfnoc_graph,
                gnuradio.uhd.device_addr(""),
                "Fosphor",
                0,
                instance,
            )
            fosphor.set_bool_property(
                "enable_dither", self.settings_dict["channels"][fosphor_num]["dither"]
            )
            fosphor.set_int_property("hist_decimation", 16)
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
            fosphor.set_int_property("wf_predivision_ratio", 0)
            fosphor.set_int_property("wf_decimation", 32)
            fosphor.set_bool_property("clear_history", True)
            fosphor.set_bool_property("enable_histogram", True)
            fosphor.set_bool_property("enable_waterfall", True)
            return fosphor

        # Creates and returns RFNoC streamer
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

        # Creates and returns fosphor display and adds it to the fosphor layout for this fosphor chain
        def create_fosphor_display():
            display = ettus.fosphor_display(
                self.settings_dict["global"]["fft_size"],
                self.settings_dict["global"]["power_bins"],
                self.settings_dict["global"]["fft_size"],
            )
            display.set_frame_rate(30)
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

        # Calls create_... function based on the block_id
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

        # Creates all blocks required for fosphor chain by calling
        # create_block for all blocks in the fosphor chain string.
        # Also creates the fosphor display and 2 streamers needed
        # for the fosphor chain
        def create_fosphor_chain(fosphor_chain_strings):
            gr_blocks = []
            for block in fosphor_chain_strings:
                gr_block = create_block(block)
                gr_blocks.append(gr_block)
            gr_blocks.append(create_streamer())
            gr_blocks.append(create_streamer())
            gr_blocks.append(create_fosphor_display())
            return gr_blocks

        # Connects all the blocks in the fosphor chain together. Checks to see if there is a DDC block and connects the blocks accordingly
        # Expected blocks in fosphor_chain(in this order of indicies):
        # Radio->DDC(Optional)->FFT->Fosphor ->Streamer-> Fosphor Display
        #                                   \->Streamer->/
        # If DDC is not detected, the function will shift all the indicies back to adjust
        # Function also expects the chain used to create the blocks referenced above to determine port numbers of the blocks
        def connect_blocks(fosphor_chain, fosphor_chains_strings):
            radio_info = read_id(fosphor_chain_strings[0])
            second_block_info = read_id(fosphor_chain_strings[1])
            third_block_info = read_id(fosphor_chain_strings[2])
            fourth_block_info = read_id(fosphor_chain_strings[3])
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
            top_block_ref.connect(
                (fosphor_chain[self.fosphor_position + 1], 0),
                (fosphor_chain[self.fosphor_position + 3], 0),
            )
            top_block_ref.connect(
                (fosphor_chain[self.fosphor_position + 2], 0),
                (fosphor_chain[self.fosphor_position + 3], 1),
            )


        def create_gui(fosphor_num):
            starting_row = 0
            starting_col = 0

            #Setup for Gain Widget
            gain_row = starting_row
            gain_col = starting_col + 2
            gain_height = 1
            gain_width = 4
            self.channel_gain = settings_dict["channels"][fosphor_num]["gain"]
            self._channel_gain_range = Range(settings_dict["channels"][self.fosphor_num]["min_gain"], settings_dict["channels"][self.fosphor_num]["max_gain"], 1.0, self.channel_gain, 200)
            self._channel_gain_win = RangeWidget(self._channel_gain_range, self.set_channel_gain, 'Gain', "counter_slider", float)
            self.fosphor_layout.addWidget(self._channel_gain_win, gain_row, gain_col, gain_height, gain_width)
            for r in range(gain_row, gain_row + gain_height):
                self.fosphor_layout.setRowStretch(r, 1)
            for c in range(gain_col, gain_col + gain_width):
                self.fosphor_layout.setColumnStretch(c, 1)

            #Setup for Frequency Widget
            freq_row = starting_row
            freq_col = starting_col
            freq_height = 1
            freq_width = 2
            self.channel_freq = settings_dict["channels"][fosphor_num]["freq"]
            self._freq_tool_bar = Qt.QToolBar(top_block_ref)
            self._freq_tool_bar.addWidget(Qt.QLabel('Center Frequency' + ": "))
            self._freq_line_edit = Qt.QLineEdit(str(self.channel_freq))
            self._freq_tool_bar.addWidget(self._freq_line_edit)
            self._freq_line_edit.textChanged.connect(self.channel_freq_control_callback)
            self.fosphor_layout.addWidget(self._freq_tool_bar, freq_row, freq_col, freq_height, freq_width)
            for r in range(freq_row, freq_row + freq_height):
                self.fosphor_layout.setRowStretch(r, 1)
            for c in range(freq_col, freq_col + freq_width):
                self.fosphor_layout.setColumnStretch(c, 1)

            #Setup for Waterfall Mode
            wf_row = starting_row + 4
            wf_col = starting_col
            wf_height = 1
            wf_width = 1

            self._waterfall_mode_options = [0, 1]
            self._waterfall_mode_labels = ['Max Hold', 'Average']
            self._waterfall_mode = settings_dict["channels"][fosphor_num]["wf_mode"]
            self._waterfall_mode_tool_bar = Qt.QToolBar(top_block_ref)
            self._waterfall_mode_tool_bar.addWidget(Qt.QLabel('Waterfall Mode' + ": "))
            self._waterfall_mode_combo_box = Qt.QComboBox()
            self._waterfall_mode_tool_bar.addWidget(self._waterfall_mode_combo_box)
            for _label in self._waterfall_mode_labels: self._waterfall_mode_combo_box.addItem(_label)
            self._waterfall_mode_combo_box.currentIndexChanged.connect(self.waterfall_mode_control_callback)
            self.fosphor_layout.addWidget(self._waterfall_mode_tool_bar, wf_row, wf_col, wf_height, wf_width)
            for r in range(wf_row, wf_row + wf_height):
                self.fosphor_layout.setRowStretch(r, 1)
            for c in range(wf_col, wf_col + wf_width):
                self.fosphor_layout.setColumnStretch(c, 1)

            #Setup for Rise Time
            self._rise_time_range = Range(0, 65535, 1, 4096, 200)
            self._rise_time_win = RangeWidget(self._rise_time_range, self.set_rise_time, 'Rise Time', "slider", float)

            #Setup for Randomization
            rand_row = starting_row + 1
            rand_col = starting_col + 1
            rand_height = 1
            rand_width = 1
            self._randomization_enable_options =[False, True]
            self._randomization_enable_labels = ['Disabled', 'Enabled']
            self.randomization_enable = settings_dict["channels"][fosphor_num]["randomization"]
            self._randomization_enable_tool_bar = Qt.QToolBar(top_block_ref)
            self._randomization_enable_tool_bar.addWidget(Qt.QLabel('Randomization' + ": "))
            self._randomization_enable_combo_box = Qt.QComboBox()
            self._randomization_enable_tool_bar.addWidget(self._randomization_enable_combo_box)
            for _label in self._randomization_enable_labels: self._randomization_enable_combo_box.addItem(_label)
            self._randomization_enable_combo_box.currentIndexChanged.connect(self.randomization_enable_control_callback)
            Qt.QMetaObject.invokeMethod(self._randomization_enable_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._randomization_enable_options.index(self.randomization_enable)))
            self.fosphor_layout.addWidget(self._randomization_enable_tool_bar, rand_row, rand_col, rand_height, rand_width)
            for r in range(rand_row, rand_row + rand_height):
                self.fosphor_layout.setRowStretch(r, 1)
            for c in range(rand_col, rand_col + rand_width):
                self.fosphor_layout.setColumnStretch(c, 1)

            #Setup for Max Hold Decay
            self._max_hold_decay_range = Range(0, 65535, 1, 1, 200)
            self._max_hold_decay_win = RangeWidget(self._max_hold_decay_range, self.set_max_hold, 'Max Hold Decay', "slider", int)

            #Setup for Histogram Scale
            self._hist_scale_range = Range(0, 65535, 1, 256, 200)
            self._hist_scale_win = RangeWidget(self._hist_scale_range, self.set_hist_scale, 'Scale', "slider", float)

            #Setup for Histogram Offset
            self._hist_offset_range = Range(0, 65535, 1, 0, 200)
            self._hist_offset_win = RangeWidget(self._hist_offset_range, self.set_hist_offset, 'Offset', "slider", float)

            #Setup for Dither
            dither_row = starting_row + 1
            dither_col = starting_col
            dither_height = 1
            dither_width = 1
            self._dither_enable_options = [False, True]
            self._dither_enable_labels = ['Disabled', 'Enabled']
            self.dither_enable = self.settings_dict["channels"][fosphor_num]["dither"]
            self._dither_enable_tool_bar = Qt.QToolBar(top_block_ref)
            self._dither_enable_tool_bar.addWidget(Qt.QLabel('Dither' + ": "))
            self._dither_enable_combo_box = Qt.QComboBox()
            self._dither_enable_tool_bar.addWidget(self._dither_enable_combo_box)
            for _label in self._dither_enable_labels: self._dither_enable_combo_box.addItem(_label)
            Qt.QMetaObject.invokeMethod(self._dither_enable_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._dither_enable_options.index(self.dither_enable)))
            self._dither_enable_combo_box.currentIndexChanged.connect(self.dither_enable_control_callback)
            self.fosphor_layout.addWidget(self._dither_enable_tool_bar, dither_row, dither_col, dither_height, dither_width)
            for r in range(dither_row, dither_row + dither_height):
                self.fosphor_layout.setRowStretch(r, 1)
            for c in range(dither_col, dither_col + dither_width):
                self.fosphor_layout.setColumnStretch(c, 1)

            #Setup for Decay Time
            self._decay_time_range = Range(0, 65535, 1, 16384, 200)
            self._decay_time_win = RangeWidget(self._decay_time_range, self.set_decay_time, 'Decay TIme', "slider", int)

            #Setup for Histogram Color
            color_row = starting_row + 4
            color_col = starting_col + 1
            color_height = 1
            color_width = 1
            self._color_options = ['iron', 'cubehelix', 'sdrangelove_histogram', 'rainbow', 'prog']
            self._color_labels = ['Iron', 'Cube Helix', 'SDRangeLove', 'Rainbow', "Prog's"]
            self.color = color = settings_dict["channels"][fosphor_num]["color"]
            self._color_tool_bar = Qt.QToolBar(top_block_ref)
            self._color_tool_bar.addWidget(Qt.QLabel('Color' + ": "))
            self._color_combo_box = Qt.QComboBox()
            self._color_tool_bar.addWidget(self._color_combo_box)
            for _label in self._color_labels: self._color_combo_box.addItem(_label)
            self._color_combo_box.currentIndexChanged.connect(self.color_control_callback)
            Qt.QMetaObject.invokeMethod(self._color_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._color_options.index(self.color)))
            self.fosphor_layout.addWidget(self._color_tool_bar, color_row, color_col, color_height, color_width)
            for r in range(color_row, color_row + color_height):
                self.fosphor_layout.setRowStretch(r, 1)
            for c in range(color_col, color_col + color_width):
                self.fosphor_layout.setColumnStretch(c, 1)

            #Setup for Averaging Alpha
            self._averaging_alpha_range = Range(65200, 65360, 1, 65280, 200)
            self._averaging_alpha_win = RangeWidget(self._averaging_alpha_range, self.set_averaging_alpha, 'Averaging Alpha', "slider", int)


            #Organizing Widgets with title boxes
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

        # Function calls to create and connect blocks
        self.fosphor_chain = create_fosphor_chain(fosphor_chain_strings)
        connect_blocks(self.fosphor_chain, fosphor_chain_strings)
        create_gui(fosphor_num)

# This class creates and sets up GNURadio as well as creating the layoput for the fosphor displays to go once they are created
# Checks to see which channels are selected, then calls fosphor creator and adds the layouts that are returned to the top_layout to organize all of the displays
class multi_fosphor_example(gr.top_block, Qt.QWidget):
    def __init__(self, settings_dict, fosphor_chains_strings):
        gr.top_block.__init__(self, "multi_fosphor_example")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("multi_fosphor_example")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme("gnuradio-grc"))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
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
        fosphor_num = 0

        # Determines if displays will be created for all detected chains or specific channels
        if len(self.channel_select) == 4:
            for chain in fosphor_chains_strings:
                fosphor_display_unit = fosphor_creator(
                    settings_dict, chain, fosphor_num, self
                )
                self.top_layout.addLayout(fosphor_display_unit.get_fosphor_layout())
                fosphor_num = fosphor_num + 1
        else:
            while fosphor_num < len(self.channel_select):
                fosphor_display_unit = fosphor_creator(
                    settings_dict,
                    fosphor_chains_strings[self.channel_select[fosphor_num]],
                    fosphor_num,
                    self,
                )
                self.top_layout.addLayout(fosphor_display_unit.get_fosphor_layout())
                fosphor_num = fosphor_num + 1

    # Closes the GNURadio instance
    def close_event(self, event):
        self.display_settings = Qt.QSettings("GNU Radio", "multi_fosphor_example")
        self.display_settings.setValue("geometry", self.saveGeometry())
        event.accept()

# This function will find completete fosphor chains that begin with a radio block and follow the path: Radio->DDC->FFT->Fosphor(port0)->SEP and Fosphor(port1)->SEP
# Returns a list of lists that hold the strings for the fosphor chain blocks
# Requires a static chain of radio->ddc->fft->fosphor->sep

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
    parser.add_argument("--bandwidth", default=10e6, type=float)
    # Channel Settings
    parser.add_argument(
        "--channel_freq",
        nargs="+",
        help="Channel frequencies, separated by spaces",
        type=float,
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


# Allows for arguments to be UP TO the number of fosphor chains and will select
#default value if the specified channel setting was not included in the argument parsing
def index_or_default(parameter, index):
    if parameter is None:
        logger.error("Missing Argument")
    if index < len(parameter):
        return parameter[index]
    else:
        return parameter[0]


def wf_mode_check(wf_mode):
    if wf_mode == "max_hold":
        return 0
    if wf_mode == "average":
        return 1


def main(top_block_cls=multi_fosphor_example, options=None):

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string("qtgui", "style", "raster")
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    args = parse_args()
    if args.channel_freq is None:
        logger.error("Please specify --channel_freq")
        return False
    settings_dict = {
        "global": {
            "fft_size": args.fft_size,
            "bandwidth": args.bandwidth,
            "args": args.args,
            "power_bins": args.power_bins,
            "channel_select": args.channel_select,
        },
        "channels": [],
    }
    fosphor_chains_strings = find_fosphor_chains(settings_dict)
    if len(fosphor_chains_strings) < 1:
        logger.error("No fosphor chains found")
        return False
    # If the channel_select is not the default, it checks to make sure the highest channel selected is available
    if (settings_dict["global"]["channel_select"] != [0, 1, 2, 3]) and (
        max(settings_dict["global"]["channel_select"]) >= len(fosphor_chains_strings)
    ):
        logger.error(
            "Channel "
            + str(max(settings_dict["global"]["channel_select"]))
            + " not detected"
        )
        return False

    rfnoc_graph = ettus.rfnoc_graph(
        gnuradio.uhd.device_addr(settings_dict["global"]["args"])
    )
    settings_dict["global"]["rfnoc_graph"] = rfnoc_graph

    chain_index = 0
    for chain in fosphor_chains_strings:
        channel_settings = {
            "freq": index_or_default(args.channel_freq, chain_index),
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
        chain_index = chain_index + 1

    top_block = top_block_cls(settings_dict, fosphor_chains_strings)
    top_block.start()
    top_block.show()

    def sig_handler(sig=None, frame=None):
        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect

    def quitting():
        top_block.stop()
        top_block.wait()

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
