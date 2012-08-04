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
LT_MACRO_IFRIGID<mul r3, v1, c0.xxxy>
LT_MACRO_IFSKIN<mul  r3, v3, c0.xxxy>
add r3, v0, r3

// Transform position (all the way)...
LT_MACRO_RIGIDTRANS4<oPos,r3,c5>			// Rigid Transform...
LT_MACRO_SKINBLENDTRANS4<r0,r3,v1,v2,r1,r2,c10>		// Skin Blended Transform...
LT_MACRO_SKINTRANS4<oPos,r0,c1>				// Skin Projection...

LT_MACRO_SKINBLENDTRANS3<r6,v3,v1,v2,r1,r2,c10>		// Skin Blended Transform (Normal)...
LT_MACRO_IFSKIN<dp3 r6.w, r6,  r6>			// Normalize the normal...
LT_MACRO_IFSKIN<rsq r6.w, r6.w>
LT_MACRO_IFSKIN<mul r6,   r6,  r6.w>

// Set the vert alpha by (e dot n)...
LT_MACRO_IFRIGID<add r7,   -v0,   c9>
LT_MACRO_IFRIGID<dp3 r7.w,  r7,   r7>			// Normalize the eye vector (Model space)...
LT_MACRO_IFRIGID<rsq r7.w,  r7.w>
LT_MACRO_IFRIGID<mul r7,    r7,   r7.w>
LT_MACRO_IFRIGID<dp3 r7.w,  v1,   r7>
LT_MACRO_IFRIGID<add oD0.w, c0.w, -r7.w>

LT_MACRO_IFSKIN<mov r7,    -r0>
LT_MACRO_IFSKIN<dp3 r7.w,  r7,   r7>			// Normalize the eye vector (View space)...
LT_MACRO_IFSKIN<rsq r7.w,  r7.w>
LT_MACRO_IFSKIN<mul r7,    r7,   r7.w>
LT_MACRO_IFSKIN<dp3 r7.w,  r6,   r7>
LT_MACRO_IFSKIN<add oD0.w, c0.w, -r7.w>

// Output UVs...
LT_MACRO_IFRIGID<mov oT0, v2>
LT_MACRO_IFSKIN<mov  oT0, v4>

// Output Color
mov oD0.xyz, c0.zzz





