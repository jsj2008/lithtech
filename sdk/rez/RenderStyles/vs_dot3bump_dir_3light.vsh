; Vertex Data:
;   v0    -  Position
;   v1    -  Normal
;   v2    -  uv1
;
; Constant Data:
;   c0    -  Constant Vector (0.0, 0.5, 1.0, 2.0)
;   c1:c4 -  Projection Matrix
;   c5:c8 -  ModelViewProj Matrix
;   c5    -  Light direction
;   c6	  -  Light color

vs.1.1

// Transform position (all the way)...
LT_MACRO_RIGIDTRANS4<oPos,v0,c5>			// Rigid Transform...
LT_MACRO_SKINBLENDTRANS4<r0,v0,v1,v2,r1,r2,c18>		// Skin Blended Transform...
LT_MACRO_SKINTRANS4<oPos,r0,c1>				// Skin Projection...

// Figure out the Light vector...
LT_MACRO_IFRIGID<add r5, -v0, c9>			// UnNormalized L1 vector...
LT_MACRO_IFSKIN<add r5, -r0, c12>
dp3 r1.w,    r5,      r5
rsq r2.w,    r1.w
mul r5,      r5,      r2.w				// r5 = Normalized L1 vector...
dst r1,      r1.w,    r2.w
dp3 r1.w,    r1.xyz,  c15.xyz
rcp r2.w,    r1.w					// r2.w = Att1
mul r3,      r5,      r2.w				// r3 = L1 * Att1
LT_MACRO_IFRIGID<add r5, -v0, c10>			// UnNormalized L2 vector...
LT_MACRO_IFSKIN<add r5, -r0, c13>
dp3 r1.w,    r5,      r5
rsq r2.w,    r1.w
mul r5,      r5,      r2.w				// r5 = Normalized L2 vector...
dst r1,      r1.w,    r2.w
dp3 r1.w,    r1.xyz,  c16.xyz
rcp r2.w,    r1.w					// r2.w = Att2
mul r4,      r5,      r2.w
add r3,      r3,      r4				// r3 += L2 * Att2
LT_MACRO_IFRIGID<add r5, -v0, c11>			// UnNormalized L2 vector...
LT_MACRO_IFSKIN<add r5, -r0, c14>
dp3 r1.w,    r5,      r5
rsq r2.w,    r1.w
mul r5,      r5,      r2.w				// r5 = Normalized L3 vector...
dst r1,      r1.w,    r2.w
dp3 r1.w,    r1.xyz,  c17.xyz
rcp r2.w,    r1.w					// r2.w = Att3
mul r4,      r5,      r2.w
add r3,      r3,      r4				// r3 += L3 * Att3
;mul r3,      r3,      c0.wwww				// r3 *= BumpScale

// Skin Blend the normal and T (into viewspace)...
LT_MACRO_SKINBLENDTRANS3<r6,v3,v1,v2,r1,r2,c18>		// Skin Blended Transform (Normal)...
LT_MACRO_SKINBLENDTRANS3<r7,v5,v1,v2,r1,r2,c18>		// Skin Blended Transform (T)...
LT_MACRO_IFRIGID<mov r7, v3>

// Figure out T (by S cross the Normal)...
LT_MACRO_IFRIGID<mul r1,  r7.zxyw, v1.yzxw>
LT_MACRO_IFSKIN<mul r1,   r7.zxyw, r6.yzxw>
LT_MACRO_IFRIGID<mad r2, -r1,      r7.yzxw,   v1.zxyw>
LT_MACRO_IFSKIN<mad r2,  -r1,      r7.yzxw,   v3.zxyw>

// transform light by basis vectors to put it into texture space
LT_MACRO_IFRIGID<dp3 r4.x, r3, v3>
LT_MACRO_IFSKIN<dp3 r4.x, r3, r7>
dp3 r4.y,    r3,      r2 
LT_MACRO_IFRIGID<dp3 r4.z, r3, v1>
LT_MACRO_IFSKIN<dp3 r4.z, r3, r6>

// Normalize the light vector
dp3 r4.w,    r4,      r4
rsq r4.w,    r4.w
mul r4,      r4,      r4.w

// Scale to 0-1
add r4,      r4,      c0.zzzz
mul oD0,     r4,      c0.yyyy

// Set alpha to 1
;mov oD0.w,   c0.z

// Output UVs...
LT_MACRO_IFRIGID<mov oT0,     v2>
LT_MACRO_IFSKIN<mov oT0,     v4>
LT_MACRO_IFRIGID<mov oT1,     v2>
LT_MACRO_IFSKIN<mov oT1,     v4>