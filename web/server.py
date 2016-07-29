#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# RFNoC fosphor - WebSocket server
#
# Copyright (C) 2016  Ettus Corporation LLC
#

import base64
import json
import os
import sys
import time

from PIL import Image
import cStringIO as StringIO

import eventlet
from eventlet import event
from eventlet import wsgi
from eventlet import websocket
from eventlet.green import zmq


# Globals
g_src_histo = None
g_src_wf = None


# ----------------------------------------------------------------------------
# Web Server
# ----------------------------------------------------------------------------

class WebServer(object):

	def __init__(self, path_map):
		self.path_map = path_map

	def _dispatch(self, environ, start_response):
		# Grab descriptor
		path = environ['PATH_INFO'][1:]
		desc = self.path_map.get(path, None)

		# Not found
		if desc is None:
			start_response('404 Not Found', [('content-type', 'text/plain')])
			return ['Not found']

		# Handle static files
		# (FIXME: A real web server would use caching / compression here)
		if not callable(desc):
			start_response('200 OK', desc[1].items())
			return [open(os.path.join(
				os.path.dirname(__file__),
				desc[0])).read()]

		# Defer
		return desc(environ, start_response)

	def start(self):
		listener = eventlet.listen(('0.0.0.0', 8080))
		wsgi.server(listener, self._dispatch)


@websocket.WebSocketWSGI
def stream_histogram(ws):
	# Init
	fr = 0.0
	pt = time.time()

	# Wait for a request
	m = ws.wait()

	# Infinite loop
	for frame in g_src_histo.frames():
		# Send frame data
		ws.send(frame)

		# Wait for a request
		m = ws.wait()
		if m is None:
			break

		# Frame rate math
		now = time.time()
		fr = 0.95 * fr + 0.05 * 1.0 / (now - pt)
		pt = now
		print "FPS: %5.2f - Size: %5.2f kB" % ( fr, len(frame) / 1024.0 )


@websocket.WebSocketWSGI
def stream_waterfall(ws):

	# First frame doesn't have a version
	m = ws.wait()
	ws.send( g_src_wf.next_frame() )

	# Then get version
	while True:
		# Get request
		m = ws.wait()
		if m is None:
			break

		# Send response
		ws.send( g_src_wf.next_frame(ord(m)) )


def content(fn):
	return open(fn).read()

def data(ct,fn):
	return "data:%s;base64,%s" % (ct, base64.b64encode(content(fn)) )

def jsonfile(fn):
	return json.loads(content(fn))

DATA = {
	'vs-basic':		[ 'json',  content('vs-basic.glsl') ],
	'fs-color':		[ 'json',  content('fs-color.glsl') ],
	'fs-tex':		[ 'json',  content('fs-tex.glsl')   ],
	'fs-cmap':		[ 'json',  content('fs-cmap.glsl')  ],
	'cmap':			[ 'image', data('image/png', 'cmap.png') ],
	'cmap_desc':	[ 'json',  jsonfile('cmap.json') ],
	'fosphor_em':	[ 'emscripten', 'fosphor_em.js' ],
}

def data_gen(environ, start_response):
	start_response('200 OK', [('content-type', 'application/json')])
	return json.dumps(DATA)



WEB_PATH_MAP = {
	# Static files
	'jquery.js':		( 'jquery-2.2.3.min.js',		{ 'content-type': 'text/javascript' } ),
	'fosphor.js':		( 'fosphor.js',					{ 'content-type': 'text/javascript' } ),
	'fosphor_em.js':	( 'fosphor_em.js',				{ 'content-type': 'text/javascript' } ),
	'fosphor_em.js.mem':( 'fosphor_em.js.mem',			{ 'content-type': 'application/octet-stream' } ),
	'fosphor_data.js':	( 'fosphor_data.js',			{ 'content-type': 'text/javascript' } ),
	'canvas-3d.html':	( 'canvas-3d.html',				{ 'content-type': 'text/html' } ),

	# Resource generator
	'fosphor_data': data_gen,

	# WebSocket endpoint
	'stream_histogram': stream_histogram,
	'stream_waterfall': stream_waterfall,
}


# ----------------------------------------------------------------------------
# Data source
# ----------------------------------------------------------------------------

class DataSource(object):

	def __init__(self, frame_len, filename=None, zmq_addr=None, fps=None):
		# Save values
		self.fps = fps
		self.frame_len = frame_len

		# Can't be both
		if (filename is not None) and (zmq_addr is not None):
			raise RuntimeError("Can't read both from file and from ZMQ !")

		# Init file
		if filename is not None:
			self._file = file(filename, 'rb')
			self._read = self._read_file

		# Init ZMQ
		else:
			self._zmq = zmq.Socket(zmq.Context(), zmq.SUB)
			self._zmq.connect(zmq_addr)
			self._zmq.setsockopt(zmq.SUBSCRIBE, "")
			self._read = self._read_zmq
			self._frame = ''

		# Start the thread
		eventlet.spawn(self._run)

	def _read_file(self):
		# Loop to read a full frame
		frame = ''
		r = False

		while len(frame) < self.frame_len:
			# Read what's missing
			nd = self._file.read( self.frame_len - len(frame) )

			# EOF reset ?
			if not nd:
				if r:
					raise RuntimeError("Source file too small, need at least one full frame (%d bytes)" % self.frame_len)
				else:
					self._file.seek(0)
					frame = ''
					r = True

			# Append
			frame += nd

		return frame

	def _read_zmq(self):
		# Read if no data pending
		if not len(self._frame):
			self._frame = self._zmq.recv()
			if len(self._frame) % self.frame_len:
				raise RuntimeError("Invalid frame received from ZMQ: (expected %d bytes, got %d)" % (
					self.frame_len, len(self._frame)
				))

		# Get some
		frame = self._frame[0:self.frame_len]
		self._frame = self._frame[self.frame_len:]

		# And return it
		return frame

	def _run(self):
		# Infinite loop
		while True:
			# Now
			start = time.time()

			# Grab a frame
			frame = self._read()

			# Processit
			self._process_frame(frame)

			# Framerate limit
			if self.fps:
				nfw = start + (1.0 / self.fps) - time.time()
				if nfw > 0.0:
					eventlet.sleep(nfw)


class HistogramSource(DataSource):

	def __init__(self, fft_bins, pwr_bins, **kwargs):
		# Save values
		self.fft_bins = fft_bins
		self.pwr_bins = pwr_bins

		# Init state
		self.hist = (0, None, event.Event())

		# Super init
		super(HistogramSource, self).__init__(
			fft_bins * (pwr_bins + 2),
			**kwargs
		)

	def _process_frame(self, frame):
		# Compress into PNG
		img = Image.frombytes('L', (self.fft_bins, self.pwr_bins+2), frame)
		ds = StringIO.StringIO()
		img.save(ds, 'png', optimize=True)
		ds.seek(0)
		data = ds.read()

		# Send to anyone waiting
		e = self.hist[2]
		self.hist = (self.hist[0]+1, data, event.Event())
		e.send(None)

	def frames(self):
		v = 0
		while True:
			# Check current
			hist = self.hist

			if hist[0] == v:
				hist[2].wait()
				continue

			# New version
			v = hist[0]
			yield hist[1]


def maxseq(a,b):
	return ''.join( chr(max(ord(x),ord(y))) for x,y in zip(a,b) )


class WaterfallSource(DataSource):

	def __init__(self, fft_bins, **kwargs):
		# Save values
		self.fft_bins = fft_bins
		self.ver = (0, event.Event())
		self.data = [
			'\x00' * 1024
				for v in range(256)
		]

		# Super init
		super(WaterfallSource, self).__init__(
			fft_bins,
			**kwargs
		)

	def _process_frame(self, frame):
		evt = self.ver[1]
		nv = (self.ver[0] + 1) & 255
		self.data[nv] = frame
		self.ver = (nv, event.Event())
		evt.send(None)

	def next_frame(self, ver=None):
		# Current version
		cv, evt = self.ver

		# Difference of version #
		if ver is None:
			d = 0
		else:
			d = ((cv - ver) & 255)

		if d > 32:
			print "WARNING: Slow client"

		# If difference is 0, wait for next version
		if d == 0:
			# Wait
			evt.wait()

			# Get update version and associate data and return frame
			cv = self.ver[0]
			return chr(cv) + '\x01' + self.data[cv]

		# Ok, there was some delay, we may need aggregation !
		data = self.data[cv]

		for i in range(1,d):
			data = maxseq(data, self.data[(cv-i)&255])

		return chr(cv) + chr(d) + data


# ----------------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------------

def main():
	global g_src_histo
	global g_src_wf

	# Histogram source
	#g_src_histo = HistogramSource(1024, 64, filename='/tmp/out_hist.dat', fps=15)
	g_src_histo = HistogramSource(1024, 64, zmq_addr='tcp://127.0.0.1:26223')

	# Waterfall source
	#g_src_wf = WaterfallSource(1024, filename='/tmp/out_wf.dat', fps=15)
	g_src_wf = WaterfallSource(1024, zmq_addr='tcp://127.0.0.1:26224')

	# Server
	srv = WebServer(WEB_PATH_MAP)
	srv.start()

	return 0

if __name__ == '__main__':
	sys.exit(main())
