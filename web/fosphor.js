/*
 * fosphor.js
 */

/* ------------------------------------------------------------------------ */
/* Resource loading                                                         */
/* ------------------------------------------------------------------------ */

var fosphorData = {};
var Module = {};

function fosphorInit()
{
	/* Load the resource descriptor file */
	var request = $.ajax( "fosphor_data", { dataType: "json" } );

	/* Chained with processing */
	return request.pipe(function (data) {
		/* List of async to wait for */
		var async = [];
		var d;

		/* Iterate through resources */
		for (var dn in data)
		{
			if (data[dn][0] == 'json') {
				/* Simple data, just set it */
				fosphorData[dn] = data[dn][1];

			} else if (data[dn][0] == 'image') {
				/* Image, do a load */
				var img = new Image();
				fosphorData[dn] = img;

				d = new $.Deferred();
				async.push(d);

				img.onload = d.resolve.bind(d);
				img.src = data[dn][1];

			} else if (data[dn][0] == 'emscripten') {
				/* Inject script */
				d = new $.Deferred();
				async.push(d);

				Module.onRuntimeInitialized = d.resolve.bind(d);

				$.getScript(data[dn][1]);
			}
		}

		return $.when.apply(null, async);
	});
}


/* ------------------------------------------------------------------------ */
/* fosphorSurface                                                           */
/* ------------------------------------------------------------------------ */

function fosphorSurface(canvas, fftBins, pwrBins, wfLines, wfEnabled)
{
	/* Store data */
	this.canvas    = canvas;
	this.fftBins   = fftBins;
	this.pwrBins   = pwrBins;
	this.wfLines   = wfLines;
	this.wfEnabled = true;

	this.histoCmap = 0
	this.wfCmap    = 0

	/* Waterfall init */
	this._wfDirty = true;
	this._wfPosition = 0;
	this._wfData = new Uint8Array(this.fftBins * this.wfLines);

	/* GL Init */
	this._initGL();

	/* Freq range */
	this.setFrequencyRange(0, 0);

	/* First layout */
	this.refreshLayout();
}


/* Initialization - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

fosphorSurface.prototype._getShader = function(name)
{
	var gl = this.gl;
	var shader;

	/* Check cache */
	if (this._shaders == undefined) {
		this._shaders = {};
	}

	if (name in this._shaders) {
		return this._shaders[name];
	}

	/* Check it exists */
	if (!(name in fosphorData)) {
		console.error("[fosphor] Request for non existent shader !");
		return null;
	}

	/* Compile it */
	if (name.startsWith("fs-")) {
		shader = gl.createShader(gl.FRAGMENT_SHADER);
	} else if (name.startsWith("vs-")) {
		shader = gl.createShader(gl.VERTEX_SHADER);
	} else {
		return null;
	}

	gl.shaderSource(shader, fosphorData[name]);
	gl.compileShader(shader);

	if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
		console.error("[fosphor] Failed to build shader '" + name + "' :" + gl.getShaderInfoLog(shader));
		return null;
	}

	/* Store in cache */
	this._shaders[name] = shader;

	return shader;

}

fosphorSurface.prototype._initShaders = function()
{
	var gl = this.gl;

	/* Shaders description */
	var desc = {
		prog_color: {
			shaders: [ 'vs-basic', 'fs-color' ],
			uniforms: [ 'xyMap', 'color' ],
			attributes: ['iVtxPos']
		},
		prog_tex: {
			shaders: [ 'vs-basic', 'fs-tex' ],
			uniforms: [ 'xyMap', 'uvMap', 'tex' ],
			attributes: ['iVtxPos']
		},
		prog_cmap: {
			shaders: [ 'vs-basic', 'fs-cmap' ],
			uniforms: [ 'xyMap', 'uvMap', 'tex', 'cmap', 'cmapConfig' ],
			attributes: ['iVtxPos']
		}
	};

	/* Process all programs */
	for (var prog_name in desc)
	{
		var prog = gl.createProgram();

		/* Attach the shaders */
		for (var i=0; i<desc[prog_name].shaders.length; i++)
		{
			gl.attachShader(prog, this._getShader(desc[prog_name].shaders[i]));
		}

		/* Link it */
		gl.linkProgram(prog);

		if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
			console.error("[fosphor] Failed to link program '" + prog_name + "'");
			return;
		}

		/* Store program */
		this[prog_name] = prog;

		/* Get Attributes */
		for (var i=0; i<desc[prog_name].attributes.length; i++)
		{
			var an = desc[prog_name].attributes[i];
			prog['a_'+an] = gl.getAttribLocation(prog, an);
		}

		/* Get Uniforms */
		for (var i=0; i<desc[prog_name].uniforms.length; i++)
		{
			var un = desc[prog_name].uniforms[i];
			prog['u_'+un] = gl.getUniformLocation(prog, un);
		}
	};
}

fosphorSurface.prototype._initVBOs = function()
{
	var gl = this.gl;

	/* Quad */
	this.vbo_quad = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo_quad);
	vertices = new Float32Array([
		0.0, 0.0,	/* BL */
		1.0, 0.0,	/* BR */
		0.0, 1.0,	/* TL */
		1.0, 1.0,	/* TR */
	]);

	gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

	this.vbo_quad.itemSize = 2;
	this.vbo_quad.numItems = 4;

	/* Grid */
	this.vbo_grid = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo_grid);
	vertices = new Float32Array(2 * 11 * 4);

	for (i=0; i<=10; i++) {
		vertices[(4*i) +  0] =  0;	/* Hor - X1 */
		vertices[(4*i) +  1] =  i;	/* Hor - Y1 */
		vertices[(4*i) +  2] = 10;	/* Hor - X2 */
		vertices[(4*i) +  3] =  i;	/* Hor - Y2 */
		vertices[(4*i) + 44] =  i;	/* Ver - X1 */
		vertices[(4*i) + 45] =  0;	/* Ver - Y1 */
		vertices[(4*i) + 46] =  i;	/* Ver - X2 */
		vertices[(4*i) + 47] = 10;	/* Ver - Y2 */
	}

	gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

	this.vbo_grid.itemSize = 2;
	this.vbo_grid.numItems = 11 * 4;

	/* Spectrum lines */
	this.vbo_spectrum = gl.createBuffer();

	this.vbo_spectrum.itemSize = 2;
	this.vbo_spectrum.numItems = this.fftBins * 2;
}

fosphorSurface.prototype._initTextures = function()
{
	var gl = this.gl;

	gl.activeTexture(gl.TEXTURE0);

	/* Color map texture */
	this.tex_cmap = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, this.tex_cmap);

	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,     gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,     gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);

	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, fosphorData.cmap);

	/* Histogram texture */
	this.tex_hist = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, this.tex_hist);

	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,     gl.REPEAT);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,     gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);

	gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, this.fft_bins, this.pwr_bin, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, null);

	/* Waterfall texture */
	this.tex_wf = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, this.tex_wf);

	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,     gl.REPEAT);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,     gl.REPEAT);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);

	gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, this.fft_bins, this.wf_lines, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, null);

	/* Frequency margin texture */
	this.tex_freq = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, this.tex_freq);

	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,     gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,     gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

	/* Power margin texture */
	this.tex_pwr = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, this.tex_pwr);

	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,     gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,     gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
}

fosphorSurface.prototype._initGL = function()
{
	/* Context */
	var gl = this.canvas.getContext('webgl');
	this.gl = gl;

	/* Shaders */
	this._initShaders();

	/* VBOs */
	this._initVBOs();

	/* Textures */
	this._initTextures();
}


/* Layout - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

fosphorSurface.prototype._refreshFrequencyAxis = function()
{
	var gl = this.gl;
	var ly = this.layout;

	/* Draw into a canvas */
	var i, n_div = 10;
	var x, y;

	var canvas = document.createElement('canvas');
	canvas.width  = ly.x(3) - ly.x(0);
	canvas.height = ly.y(3) - ly.y(2);

	var ctx = canvas.getContext('2d')

	ctx.font = "9pt monospace";
	ctx.fillStyle = "white";
	ctx.textBaseline = "middle";

	y = Math.round( canvas.height / 2.0 );

	for (i=0; i<=n_div; i++)
	{
		if (i == 0) {
			ctx.textAlign = "left";
			x = ly.x(1) - 7;
		} else if (i == n_div) {
			ctx.textAlign = "right";
			x = ly.x(2) + 7;
		} else {
			ctx.textAlign = "center";
			x = ly.x(1) + ly.x_div * i;
		}

		x -= ly.x(0);

		ctx.fillText(this.freqAxis.render(i-n_div/2), x, y);
	}

	/* Upload new texture */
	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, this.tex_freq);
	gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);
}

fosphorSurface.prototype._refreshPowerAxis = function()
{
	var gl = this.gl;
	var ly = this.layout;

	/* Draw into a canvas */
	var i, n_div = 10;
	var x, y;

	var canvas = document.createElement('canvas');
	canvas.width  = ly.x(1) - ly.x(0);
	canvas.height = ly.y(5) - ly.y(2);

	var ctx = canvas.getContext('2d')

	ctx.font = "9pt monospace";
	ctx.fillStyle = "white";
	ctx.textAlign = "right"
	ctx.textBaseline = "middle";

	x = canvas.width - 8;

	for (i=0; i<=n_div; i++)
	{
		y = ly.y(5) - ly.y(4) + ly.y_div * i;

		ctx.fillText(-10 * i, x, y);
	}

	/* Upload new texture */
	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, this.tex_pwr);
	gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);
}

fosphorSurface.prototype.refreshLayout = function()
{
	var gl = this.gl;

	/* New dimensions */
	gl.viewportWidth  = this.canvas.width;
	gl.viewportHeight = this.canvas.height;

	/* New view port */
	gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);

	/* Layout object */
	this.layout = new Module.fosphorLayout(gl.viewportWidth, gl.viewportHeight, this.wfEnabled);

	/* Refresh sub elements */
	this._refreshFrequencyAxis();
	this._refreshPowerAxis();
}


/* Drawing - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

fosphorSurface.prototype._drawQuad = function(prog, pos, texcoord)
{
	var xs, ys, xo, yo;
	var gl = this.gl;

	/* Select program */
	gl.useProgram(prog);
	gl.enableVertexAttribArray(prog.a_iVtxPos);

	/* Set uniforms */
		/* Position */
	xs = 2.0 * (pos[2] - pos[0]) / gl.viewportWidth;
	ys = 2.0 * (pos[3] - pos[1]) / gl.viewportHeight;
	xo = -1.0 + 2.0 * pos[0] / gl.viewportWidth;
	yo = -1.0 + 2.0 * pos[1] / gl.viewportHeight;

	gl.uniform4f(prog.u_xyMap, xs, ys, xo, yo);

		/* Tex Coord, if applicable */
	if (texcoord != null)
	{
		us = texcoord[2] - texcoord[0];
		vs = texcoord[3] - texcoord[1];
		uo = texcoord[0];
		vo = texcoord[1];

		gl.uniform4f(prog.u_uvMap, us, vs, uo, vo);
	}

	/* Setup VBO */
	gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo_quad);
	gl.vertexAttribPointer(prog.a_iVtxPos, this.vbo_quad.itemSize, gl.FLOAT, false, 0, 0);

	/* Draw */
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, this.vbo_quad.numItems);
}

fosphorSurface.prototype._drawTexturedQuad = function(tex, pos, texcoord)
{
	var gl = this.gl;
	var prog = this.prog_tex;

	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, tex);

	this._drawQuad(prog, pos, texcoord);
}

fosphorSurface.prototype._drawColorMappedQuad = function(tex, cmap_id, pos, texcoord)
{
	var gl = this.gl;
	var prog = this.prog_cmap;

	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, tex);

	gl.activeTexture(gl.TEXTURE1);
	gl.bindTexture(gl.TEXTURE_2D, this.tex_cmap);

	gl.useProgram(prog);
	gl.uniform1i(prog.u_tex, 0);
	gl.uniform1i(prog.u_cmap, 1);
	gl.uniform3f(prog.u_cmapConfig, 1.0, 0.0, (cmap_id + 0.5) / fosphorData.cmap.width);

	this._drawQuad(prog, pos, texcoord);
}

	/* FIXME: Need two draw calls with two different uniforms set for Hor / Ver lines ! */
	/* Also draw it once white, once black */
fosphorSurface.prototype._drawGrid = function()
{
	var xs, ys, xo, yo;
	var gl = this.gl;
	var ly = this.layout;

	/* Select program */
	var prog = this.prog_color;

	gl.useProgram(prog);
	gl.enableVertexAttribArray(prog.a_iVtxPos);

	/* Set uniforms */
		/* Position */
	xs =  2.0 * ly.x_div / gl.viewportWidth;
	ys =  2.0 * ly.y_div / gl.viewportHeight;
	xo = -1.0 + 2.0 * (ly.x(1) + 0.5) / gl.viewportWidth;
	yo = -1.0 + 2.0 * (ly.y(3) + 0.5) / gl.viewportHeight;

	gl.uniform4f(prog.u_xyMap, xs, ys, xo, yo);

		/* Color */
	gl.uniform4f(prog.u_color, 0.0, 0.0, 0.0, 0.5);

	/* Setup VBO */
	gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo_grid);
	gl.vertexAttribPointer(prog.a_iVtxPos, this.vbo_grid.itemSize, gl.FLOAT, false, 0, 0);

	/* Draw */
	gl.drawArrays(gl.LINES, 0, this.vbo_grid.numItems);
}

fosphorSurface.prototype._drawSpectrum = function()
{
	var gl = this.gl;
	var ly = this.layout;

	/* Select program */
	var prog = this.prog_color;

	gl.useProgram(prog);
	gl.enableVertexAttribArray(prog.a_iVtxPos);

	/* Set uniforms */
	var xt = new transform();
	var yt = new transform();

		/* Scaling */
	xt.translate(-0.5);
	xt.scale(1.0 / this.fftBins);

	yt.translate(9.4208);  /* (100 - 96.32) / 100 * 256 */
	yt.scale(0.9632 / 256.0);

		/* Map to the histogram position */
	xt.scale(ly.x(2) - ly.x(1));
	yt.scale(ly.y(4) - ly.y(3));
	xt.translate(ly.x(1))
	yt.translate(ly.y(3))

		/* Viewport mapping */
	xt.scale(2.0 / this.gl.viewportWidth);
	yt.scale(2.0 / this.gl.viewportHeight);
	xt.translate(-1.0);
	yt.translate(-1.0);

	gl.uniform4f(prog.u_xyMap, xt.factor, yt.factor, xt.offset, yt.offset);

	/* Setup VBO */
	gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo_spectrum);
	gl.vertexAttribPointer(prog.a_iVtxPos, this.vbo_spectrum.itemSize, gl.FLOAT, false, 0, 0);

	/* Draw */
		/* Max Hold */
	gl.uniform4f(prog.u_color, 1.0, 0.0, 0.0, 0.75);
	gl.drawArrays(gl.LINE_STRIP, 1, this.fftBins-1);

		/* Average */
	gl.uniform4f(prog.u_color, 0.75, 1.0, 0.75, 0.75);
	gl.drawArrays(gl.LINE_STRIP, this.fftBins+1, this.fftBins-1);
}

fosphorSurface.prototype.draw = function()
{
	var gl = this.gl;
	var ly = this.layout;
	var i;

	/* Upload new waterfall data if needed */
	if (this._wfDirty) {
		/* Upload */
		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, this.tex_wf);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, this.fftBins, this.wfLines, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, this._wfData);

		/* All clean now */
		this._wfDirty = false;
	}

	/* Clear buffer */
	gl.clearColor(0.0, 0.0, 0.0, 1.0);
	gl.clear(gl.COLOR_BUFFER_BIT);

	/* Blending */
	gl.enable(gl.BLEND);
	gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);

	/* Draw quads */

		/* Histogram */
	this._drawColorMappedQuad(
		this.tex_hist,
		this.histoCmap,
		[ ly.x(1), ly.y(3), ly.x(2), ly.y(4) ],
		[ 0.0, -0.038, 1.0, 1.0 ]
	);

		/* Waterfall */
	i = this._wfPosition / this.wfLines;
	this._drawColorMappedQuad(
		this.tex_wf,
		this.wfCmap,
		[ ly.x(1), ly.y(1), ly.x(2), ly.y(2) ],
		[ 0.0, i, 1.0, i+1.0 ]
	);

		/* Power Margin */
	this._drawTexturedQuad(
		this.tex_pwr,
		[ ly.x(0), ly.y(2), ly.x(1), ly.y(5) ],
		[ 0.0, 1.0, 1.0, 0.0 ]
	);

		/* Frequency Margin */
	this._drawTexturedQuad(
		this.tex_freq,
		[ ly.x(0), ly.y(2), ly.x(3), ly.y(3) ],
		[ 0.0, 1.0, 1.0, 0.0 ]
	);

		/* Histogram intensity scale */
	i = (this.histoCmap + 0.5) / fosphorData.cmap_desc.length;
	this._drawTexturedQuad(
		this.tex_cmap,
		[ ly.x(2) + 2.0, ly.y(3), ly.x(2) + 10.0, ly.y(4) ],
		[ i, 0.0, i, 1.0 ]
	);

		/* Waterfall intensity scale */
	i = (this.wfCmap + 0.5) / fosphorData.cmap_desc.length;
	this._drawTexturedQuad(
		this.tex_cmap,
		[ ly.x(2) + 2.0, ly.y(1), ly.x(2) + 10.0, ly.y(2) ],
		[ i, 0.0, i, 1.0 ]
	);

	/* Draw grid */
	this._drawGrid();

	/* Draw spectrum lines */
	this._drawSpectrum();

	/* Cleanup */
	gl.disable(gl.BLEND);
}


/* Data feed - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

fosphorSurface.prototype.feedHistoFrame = function(data, redraw)
{
	var blob = new Blob([data], {type:'image/png'})
	var url  = URL.createObjectURL(blob);
	var img  = new Image();

	img.onload = (function() {
		var gl = this.gl;

		/* Don't leak memory */
		URL.revokeObjectURL(url);

		/* Canvas to work with */
		var canvas = document.createElement('canvas');
		var ctx = canvas.getContext('2d');

		/* Draw histogram on a canvas */
		canvas.width  = this.fftBins;
		canvas.height = this.pwrBins;
		ctx.drawImage(img,
			0, 0, this.fftBins, this.pwrBins,
			0, 0, this.fftBins, this.pwrBins
		);

		/* Upload to texture */
		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, this.tex_hist);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, gl.LUMINANCE, gl.UNSIGNED_BYTE, canvas);

		/* Draw the MaxHold/Average */
		canvas.width  = this.fftBins;
		canvas.height = 2;
		ctx.drawImage(img,
			0, this.pwrBins, this.fftBins, 2,
			0, 0, this.fftBins, 2
		);

		/* Extract data / format it / upload to VBO */
		var slsd = ctx.getImageData(0, 0, this.fftBins, 2).data;
		var slfd = new Float32Array(this.fftBins * 4);

		for (var i=0; i<this.fftBins; i++)
		{
			var mi = 2 *  i;
			var li = 2 * (i + this.fftBins);

			slfd[mi  ] = i;
			slfd[mi+1] = slsd[2*mi];
			slfd[li  ] = i;
			slfd[li+1] = slsd[2*li];
		}

		gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo_spectrum);
		gl.bufferData(gl.ARRAY_BUFFER, slfd, gl.DYNAMIC_DRAW);

		/* Force a redraw */
		this.draw();
	}).bind(this);

	img.src = url
}

fosphorSurface.prototype.feedWaterfallLine = function (data, rep)
{
	for (var i=0; i<rep; i++)
	{
		/* Fill in */
		for (var j=0; j<this.fftBins; j++)
		{
			this._wfData[this.fftBins * this._wfPosition + j] = data[j];
		}

		/* Next position */
		this._wfPosition = (this._wfPosition + 1) % this.wfLines;
	}

	this._wfDirty = true;
}


/* Config - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

fosphorSurface.prototype.setFrequencyRange = function(center_freq, span)
{
	this.freqAxis = new Module.FrequencyAxis(center_freq, span, 10);
	this.refreshLayout();
}


/* Helpers - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

function transform()
{
	this.factor = 1.0;
	this.offset = 0.0;
}

transform.prototype.translate = function (d)
{
	this.offset += d;
}

transform.prototype.scale = function (s)
{
	this.factor *= s;
	this.offset *= s;
}
