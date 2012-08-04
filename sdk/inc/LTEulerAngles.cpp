/**** EulerAngles.c - Convert Euler angles to/from matrix or quat ****/
/* Ken Shoemake, 1993 */

// KEF 11/17/99 - Minor modifications to fit LT data types

#include <math.h>
#include <float.h>
#include "lteulerangles.h"

inline void EulGetOrd(int ord, int &i, int &j, int &k, int &h, int &n, int &s, int &f)
{
	unsigned o=ord;
	f=o&1;
	o>>=1;
	s=o&1;
	o>>=1;
	n=o&1;
	o>>=1;
	i=EulSafe[o&3];
	j=EulNext[i+n];
	k=EulNext[i+1-n];
	h=s?k:i;
}

EulerAngles Eul_(float ai, float aj, float ah, int order)
{
    EulerAngles ea;
    ea.x = ai; ea.y = aj; ea.z = ah;
    ea.w = (float)order;
    return (ea);
}

/* Construct quaternion from Euler angles (in radians). */
LTRotation Eul_ToQuat(EulerAngles ea)
{
    LTRotation qu;
    float a[3], ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i,j,k,h,n,s,f;
    EulGetOrd((int)ea.w,i,j,k,h,n,s,f);
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    if (n==EulParOdd) ea.y = -ea.y;
    ti = ea.x*0.5f; tj = ea.y*0.5f; th = ea.z*0.5f;
    ci = (float)cos(ti);  cj = (float)cos(tj);  ch = (float)cos(th);
    si = (float)sin(ti);  sj = (float)sin(tj);  sh = (float)sin(th);
    cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
    if (s==EulRepYes) {
	a[i] = cj*(cs + sc);	/* Could speed up with */
	a[j] = sj*(cc + ss);	/* trig identities. */
	a[k] = sj*(cs - sc);
	qu[3] = cj*(cc - ss);
    } else {
	a[i] = cj*sc - sj*cs;
	a[j] = cj*ss + sj*cc;
	a[k] = cj*cs - sj*sc;
	qu[3] = cj*cc + sj*ss;
    }
    if (n==EulParOdd) a[j] = -a[j];
    qu[0] = a[EulX]; qu[1] = a[EulY]; qu[2] = a[EulZ];
    return (qu);
}

/* Construct matrix from Euler angles (in radians). */
void Eul_ToMatrix(EulerAngles ea, LTMatrix &M)
{
    float ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i,j,k,h,n,s,f;
    EulGetOrd((int)ea.w,i,j,k,h,n,s,f);
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    if (n==EulParOdd) {ea.x = -ea.x; ea.y = -ea.y; ea.z = -ea.z;}
    ti = ea.x;	  tj = ea.y;	th = ea.z;
    ci = (float)cos(ti); cj = (float)cos(tj); ch = (float)cos(th);
    si = (float)sin(ti); sj = (float)sin(tj); sh = (float)sin(th);
    cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
    if (s==EulRepYes) {
	M.El(i,i) = cj;	  M.El(i,j) =  sj*si;    M.El(i,k) =  sj*ci;
	M.El(j,i) = sj*sh;  M.El(j,j) = -cj*ss+cc; M.El(j,k) = -cj*cs-sc;
	M.El(k,i) = -sj*ch; M.El(k,j) =  cj*sc+cs; M.El(k,k) =  cj*cc-ss;
    } else {
	M.El(i,i) = cj*ch; M.El(i,j) = sj*sc-cs; M.El(i,k) = sj*cc+ss;
	M.El(j,i) = cj*sh; M.El(j,j) = sj*ss+cc; M.El(j,k) = sj*cs-sc;
	M.El(k,i) = -sj;	 M.El(k,j) = cj*si;    M.El(k,k) = cj*ci;
    }
    M.El(EulW,EulX)=M.El(EulW,EulY)=M.El(EulW,EulZ)=M.El(EulX,EulW)=M.El(EulY,EulW)=M.El(EulZ,EulW)=0.0; M.El(EulW,EulW)=1.0;
}

/* Convert matrix to Euler angles (in radians). */
EulerAngles Eul_FromMatrix(LTMatrix& M, int order)
{
    EulerAngles ea;
    int i,j,k,h,n,s,f;
    EulGetOrd(order,i,j,k,h,n,s,f);
    if (s==EulRepYes) {
	float sy = (float)sqrt(M.El(i,j)*M.El(i,j) + M.El(i,k)*M.El(i,k));
	if (sy > 16*FLT_EPSILON) {
	    ea.x = (float)atan2(M.El(i,j), M.El(i,k));
	    ea.y = (float)atan2(sy, M.El(i,i));
	    ea.z = (float)atan2(M.El(j,i), -M.El(k,i));
	} else {
	    ea.x = (float)atan2(-M.El(j,k), M.El(j,j));
	    ea.y = (float)atan2(sy, M.El(i,i));
		ea.z = 0;
	}
    } else {
	float cy = (float)sqrt(M.El(i,i)*M.El(i,i) + M.El(j,i)*M.El(j,i));
	if (cy > 16*FLT_EPSILON) {
	    ea.x = (float)atan2(M.El(k,j), M.El(k,k));
	    ea.y = (float)atan2(-M.El(k,i), cy);
	    ea.z = (float)atan2(M.El(j,i), M.El(i,i));
	} else {
	    ea.x = (float)atan2(-M.El(j,k), M.El(j,j));
	    ea.y = (float)atan2(-M.El(k,i), cy);
	    ea.z = 0;
	}
    }
    if (n==EulParOdd) {ea.x = -ea.x; ea.y = - ea.y; ea.z = -ea.z;}
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    ea.w = (float)order;
    return (ea);
}

/* Convert quaternion to Euler angles (in radians). */
EulerAngles Eul_FromQuat(LTRotation& q, int order)
{
    LTMatrix M;
    float Nq = q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3];
    float s = (Nq > 0.0f) ? (2.0f / Nq) : 0.0f;
    float xs = q[0]*s,	  ys = q[1]*s,	 zs = q[2]*s;
    float wx = q[3]*xs,	  wy = q[3]*ys,	 wz = q[3]*zs;
    float xx = q[0]*xs,	  xy = q[0]*ys,	 xz = q[0]*zs;
    float yy = q[1]*ys,	  yz = q[1]*zs,	 zz = q[2]*zs;
    M.El(EulX,EulX) = 1.0f - (yy + zz); M.El(EulX,EulY) = xy - wz; M.El(EulX,EulZ) = xz + wy;
    M.El(EulY,EulX) = xy + wz; M.El(EulY,EulY) = 1.0f - (xx + zz); M.El(EulY,EulZ) = yz - wx;
    M.El(EulZ,EulX) = xz - wy; M.El(EulZ,EulY) = yz + wx; M.El(EulZ,EulZ) = 1.0f - (xx + yy);
    M.El(EulW,EulX)=M.El(EulW,EulY)=M.El(EulW,EulZ)=M.El(EulX,EulW)=M.El(EulY,EulW)=M.El(EulZ,EulW)=0.0; M.El(EulW,EulW)=1.0f;
    return (Eul_FromMatrix(M, order));
}