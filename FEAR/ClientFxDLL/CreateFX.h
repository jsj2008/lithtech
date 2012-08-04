// ----------------------------------------------------------------------- //
//
// MODULE  : CreateFX.h
//
// PURPOSE : This FX is used to dynamicly create another FX within a Group
//
// CREATED : 7/27/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#ifndef	__CREATEFX_H__
#define __CREATEFX_H__

#ifndef __BASECREATEFX_H__
#	include "BaseCreateFX.h"
#endif

//-------------------------------------------
// CCreateProps
//-------------------------------------------
class CCreateProps : 
	public CBaseCreateProps
{
public:

	CCreateProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);
};

//-------------------------------------------
// CCreateFX
//-------------------------------------------
class CCreateFX : 
	public CBaseCreateFX
{
public:

	CCreateFX() :
		CBaseCreateFX( CBaseFX::eCreateFX )
	{
	}

	bool	Update(float tmElapsed);
	bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps );

private:

	const CCreateProps*		GetProps()		{ return (const CCreateProps*)m_pProps; }

	PREVENT_OBJECT_COPYING(CCreateFX);
};

#endif 