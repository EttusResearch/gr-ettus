uniform vec4 xyMap;
uniform vec4 uvMap;

attribute vec2 iVtxPos;

varying vec2 vVtxTexCoord;

void main(void) {
	gl_Position  = vec4((iVtxPos.xy * xyMap.xy) + xyMap.zw, 0.0, 1.0);
	vVtxTexCoord =      (iVtxPos.xy * uvMap.xy) + uvMap.zw;
}
