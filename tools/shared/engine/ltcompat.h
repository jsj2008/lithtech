
// This file defines old things for compatibility.
// Don't use anything in here cuz it's on its way out.

#ifndef __LT_COMPAT_H__
#define __LT_COMPAT_H__


	// These don't exist anymore.
	#define	bc_EngineMessageFn	LTNULL
	#define	bc_ObjectMessageFn	LTNULL

	#define g_pClientDE		g_pLTClient
	#define g_pServerDE		g_pLTServer

	// Old types
	#define ClientDE		ILTClient
	#define CClientDE		ILTClient
	#define ClientShellDE	IClientShell
	#define ServerDE		ILTServer
	#define LMessage		ILTMessage
	#define LVideoMgr		ILTVideoMgr
	#define MathLT			ILTMath
	#define DStream			ILTStream
	#define DECLARE_DLINK	DECLARE_LTLINK
	#define DLink			LTLink
	#define AnimTracker		LTAnimTracker
	#define DPlane			LTPlane
	#define CPlane			LTPlane
	#define DRotation		LTRotation
	#define CVector			LTVector
	#define DVector			LTVector
	#define CMatrix			LTMatrix
	#define DMatrix			LTMatrix
	#define DECommandStruct		LTCommandStruct
	#define DEEngineVar		LTEngineVar
	#define DRGBColor		LTRGBColor
	
	// Old constants
	#define DDWORD	uint32
	
	// Old defines
	#define DMIN	LTMIN
	#define DMAX	LTMAX
	#define DCLAMP	LTCLAMP
	
	#define DTOCVEC(_vec) 
	#define CTODVEC(_vec) 
	
	// Old result codes
	#define DRESULT							LTRESULT
	#define DE_OK							LT_OK
	
	// 
	// Old macros, these shouldn't be used in new code. Their
	// respective classes have functions to do the same thing.
	// 

	#define PLANE_COPY(dest, src) ((dest) = (src))

	
	// Get the distance from a point to a plane.
	#define DIST_TO_PLANE(vec, plane) ( VEC_DOT((plane).m_Normal, (vec)) - (plane).m_Dist )

#endif  // __LT_COMPAT_H__
