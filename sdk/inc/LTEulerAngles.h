/**** EulerAngles.h - Support for 24 angle schemes      ****/
/**** by Ken Shoemake, shoemake@graphics.cis.upenn.edu  ****/
/**** in "Graphics Gems IV", Academic Press, 1994       ****/

// KEF 11/17/99 - Minor modifications to fit LT data types
//		EulerAngles are stored in CVector objects w/ .w being used for the angle order

#ifndef _H_EulerAngles
#define _H_EulerAngles
#include "ltbasetypes.h"

typedef struct EulerAngles_t {
	EulerAngles_t(float nx = 0.0f, float ny = 0.0f, float nz = 0.0f, float nw = 0.0f) :
		x(nx),
		y(ny),
		z(nz),
		w(nw)
	{
	};
	void Init(float nx, float ny, float nz, float nw)
	{
		x = nx; y = ny; z = nz; w = nw;
	};

	float x, y, z, w;
} EulerAngles;

/*** Order type constants, constructors, extractors ***/

    /* There are 24 possible conventions, designated by:    */
    /*	  o EulAxI = axis used initially		    */
    /*	  o EulPar = parity of axis permutation		    */
    /*	  o EulRep = repetition of initial axis as last	    */
    /*	  o EulFrm = frame from which axes are taken	    */
    /* Axes I,J,K will be a permutation of X,Y,Z.	    */
    /* Axis H will be either I or K, depending on EulRep.   */
    /* Frame S takes axes from initial static frame.	    */
    /* If ord = (AxI=X, Par=Even, Rep=No, Frm=S), then	    */
    /* {a,b,c,ord} means Rz(c)Ry(b)Rx(a), where Rz(c)v	    */
    /* rotates v around Z by c radians.			    */

#define EulFrmS	     0
#define EulFrmR	     1
#define EulFrm(ord)  ((unsigned)(ord)&1)
#define EulRepNo     0
#define EulRepYes    1
#define EulRep(ord)  (((unsigned)(ord)>>1)&1)
#define EulParEven   0
#define EulParOdd    1
#define EulPar(ord)  (((unsigned)(ord)>>2)&1)
#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulAxI(ord)  ((int)(EulSafe[(((unsigned)(ord)>>3)&3)]))
#define EulAxJ(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)==EulParOdd)]))
#define EulAxK(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)!=EulParOdd)]))
#define EulAxH(ord)  ((EulRep(ord)==EulRepNo)?EulAxK(ord):EulAxI(ord))
    /* EulOrd creates an order value between 0 and 23 from 4-tuple choices. */
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
	/* Euler index defines */
#define EulX 0
#define EulY 1
#define EulZ 2
#define EulW 3
    /* Static axes */
#define EulOrdXYZs    EulOrd(EulX,EulParEven,EulRepNo,EulFrmS)
#define EulOrdXYXs    EulOrd(EulX,EulParEven,EulRepYes,EulFrmS)
#define EulOrdXZYs    EulOrd(EulX,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdXZXs    EulOrd(EulX,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdYZXs    EulOrd(EulY,EulParEven,EulRepNo,EulFrmS)
#define EulOrdYZYs    EulOrd(EulY,EulParEven,EulRepYes,EulFrmS)
#define EulOrdYXZs    EulOrd(EulY,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdYXYs    EulOrd(EulY,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdZXYs    EulOrd(EulZ,EulParEven,EulRepNo,EulFrmS)
#define EulOrdZXZs    EulOrd(EulZ,EulParEven,EulRepYes,EulFrmS)
#define EulOrdZYXs    EulOrd(EulZ,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdZYZs    EulOrd(EulZ,EulParOdd,EulRepYes,EulFrmS)
    /* Rotating axes */
#define EulOrdZYXr    EulOrd(EulX,EulParEven,EulRepNo,EulFrmR)
#define EulOrdXYXr    EulOrd(EulX,EulParEven,EulRepYes,EulFrmR)
#define EulOrdYZXr    EulOrd(EulX,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdXZXr    EulOrd(EulX,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdXZYr    EulOrd(EulY,EulParEven,EulRepNo,EulFrmR)
#define EulOrdYZYr    EulOrd(EulY,EulParEven,EulRepYes,EulFrmR)
#define EulOrdZXYr    EulOrd(EulY,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdYXYr    EulOrd(EulY,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdYXZr    EulOrd(EulZ,EulParEven,EulRepNo,EulFrmR)
#define EulOrdZXZr    EulOrd(EulZ,EulParEven,EulRepYes,EulFrmR)
#define EulOrdXYZr    EulOrd(EulZ,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdZYZr    EulOrd(EulZ,EulParOdd,EulRepYes,EulFrmR)

/* EulGetOrd unpacks all useful information about order simultaneously. */
inline void EulGetOrd(int ord, int &i, int &j, int &k, int &h, int &n, int &s, int &f);

EulerAngles Eul_(float ai, float aj, float ah, int order);
LTRotation Eul_ToQuat(EulerAngles ea);
void Eul_ToMatrix(EulerAngles ea, LTMatrix &M);
EulerAngles Eul_FromMatrix(LTMatrix& M, int order);
EulerAngles Eul_FromQuat(LTRotation& q, int order);

#endif
