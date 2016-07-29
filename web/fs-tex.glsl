precision mediump float;

uniform sampler2D tex;

varying vec2 vVtxTexCoord;

void main(void) {
	gl_FragColor = texture2D(tex, vVtxTexCoord);
}
