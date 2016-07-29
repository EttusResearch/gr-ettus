#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy as np
from gnuradio import gr
import pmt

class fosphor_histo_framer_bb(gr.basic_block):

	def __init__(self, fft_size=1024, histo_bins=64):
		gr.basic_block.__init__(self,
			name = 'fosphor_histo_framer',
			in_sig=[ (np.uint8, fft_size) ],
			out_sig = [ (np.uint8, fft_size * (histo_bins + 2)) ],
		)
		self.fft_size   = fft_size
		self.histo_bins = histo_bins
		self.frame_len  = self.histo_bins + 2
		self.frame = []

	def general_work(self, input_items, output_items):
		# Loop init
		consumed = 0
		produced = 0

		nR = self.nitems_read(0)
		tags = self.get_tags_in_range(0, nR, nR+len(input_items[0]))

		# Scan
		while True:
			# If we're at the end, quit
			if (consumed >= len(input_items[0])) or (produced >= len(output_items[0])):
				break

			# Store
			self.frame.append(np.array(input_items[0][consumed]))

			# Do we have a tag here ?
			if len(tags) and (tags[0].offset == (nR + consumed)):
				# Is it EOB ?
				if pmt.symbol_to_string(tags[0].key) == "rx_eob":
					# Is the frame complete ?
					if len(self.frame) == self.frame_len:
						output_items[0][produced] = np.concatenate(self.frame)
						produced += 1

					# Clear frame
					self.frame = []

				# Remove tag
				tags = tags[1:]

			# Consumed
			consumed += 1

		# Consume / Produce
		self.consume(0, consumed)
		return produced
