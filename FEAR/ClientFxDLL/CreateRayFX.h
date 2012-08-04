// ----------------------------------------------------------------------- //
//
// MODULE  : CreateRayFX.h
//
// PURPOSE : This FX is used to dynamicly create another FX within a Group
//
// CREATED : 7/27/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#ifndef	__CREATERAYFX_H__
#define __CREATERAYFX_H__

#ifndef __BASECREATEFX_H__
#	include "BaseCreateFX.h"
#endif

//-------------------------------------------
// CCreateProps
//-------------------------------------------
class CCreateRayProps : 
	public CBaseCreateProps
{
public:

	enum EAlignment
	{
		eAlign_ToSource,
		eAlign_Normal,
		eAlign_Outgoing,
		eAlign_ToViewer
	};

	CCreateRayProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//called after all the properties have been loaded to perform any custom initialization
	virtual bool PostLoadProperties();

	//the maximum distance that this ray can travel
	float		m_fMaxDist;

	//the minimum distance that this ray can travel
	float		m_fMinDist;

	//the angle of variance in the casting of rays in radians
	float		m_fRandomCone;

	//the bias that allows biasing towards the center or the edges of the cone
	float		m_fCenterBias;

	//the offset from the surface normal
	float		m_fOffset;

	//the alignment that the created effect should use
	EAlignment	m_eAlignment;

};

//-------------------------------------------
// CCreateRayFX
//-------------------------------------------
class CCreateRayFX : 
	public CBaseCreateFX
{
public:

	CCreateRayFX() :
		CBaseCreateFX( CBaseFX::eCreateRayFX )
	{
	}

	bool	Update(float tmElapsed);
	bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps );

private:

	//performs a ray intersection. This will return false if nothing is hit, or true if something
	//is. If something is hit, it will fill out the intersection property and the alignment
	//vector according to the properties that the user has setup. If an object is hit, it will
	//fill out the hit object, and specify the output transform relative to the hit object's space	
	bool	DetermineIntersection(	const LTVector& vObjPos, const LTVector& vObjForward, 
									HOBJECT& hOutObj, LTRigidTransform& tOutTrans);

	const CCreateRayProps*		GetProps()		{ return (const CCreateRayProps*)m_pProps; }

	PREVENT_OBJECT_COPYING(CCreateRayFX);
};

#endif 