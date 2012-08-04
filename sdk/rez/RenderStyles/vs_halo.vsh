; Vertex Data:
;   v0    -  Position
;   v1    -  Normal
;   v2    -  uv1
;
; Constant Data:
;   c0    -  Constant Vector (Scale, 0.0, 1.0, 2.0)
;   c1:c4 -  ModelViewProj Matrix
;   c5    -  Light direction
;   c6	  -  Light color

vs.1.1

// Scale normal by the requested scale (and add to new position)...
LT_MACRO_IFRIGID<mul r3,    v1,    c0.xxxy>
LT_MACRO_IFSKIN<mul  r3,    v3,    c0.xxxy>
add r3,    v0,    r3

// transform position (all the way)...
LT_MACRO_RIGIDTRANS4<oPos,r3,c6>			// Rigid Transform...
LT_MACRO_SKINBLENDTRANS4<r0,r3,v1,v2,r1,r2,c14>		// Skin Blended Transform...
LT_MACRO_SKINTRANS4<oPos,r0,c2>				// Skin Projection...

// Output UVs...
LT_MACRO_IFRIGID<mov oT0,   v2>
LT_MACRO_IFSKIN<mov  oT0,   v4>

// Output Color
mov oD0,   c1.xyz

// Figure out the vert alpha color (e dot n)
add r0,    v0,    -c10					// Compute the eye vector (in model space)...
dp3 r0.w,  r0,    r0					// Normalize it
rsq r0.w,  r0.w
mul r0,    r0,    r0.w
LT_MACRO_IFRIGID<dp3 r1.w,  r0,    v1>			// e dot n
LT_MACRO_IFSKIN<dp3  r1.w,  r0,    v3>
mul r1.w, r1.w, c1.w
mul oD0.w, r1.w,  c12.w





