precision mediump float;

uniform sampler2D tex;
uniform sampler2D cmap;

uniform vec3 cmapConfig;

varying vec2 vVtxTexCoord;

void main(void) {
	float i = texture2D(tex, vVtxTexCoord).x;
	float m = (i + cmapConfig.y) * cmapConfig.x;
	gl_FragColor = texture2D(cmap, vec2(cmapConfig.z, m));
}
