// ----------------------------------------------------------------------- //
//
// MODULE  : BaseCreateFX.h
//
// PURPOSE : Provides a base class for all creation based effects to derive from
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef	__BASECREATEFX_H__
#define __BASECREATEFX_H__

#include "basefx.h"
#include "ClientFX.h"

//-------------------------------------------
// CBaseCreateProps
//-------------------------------------------
class CBaseCreateProps : 
	public CBaseFXProps
{
public:

	CBaseCreateProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	//the number of different effects that can be randomly chosen
	enum { knNumEffects = 16 };

	//the list of the different effects
	const char*	m_pszEffects[knNumEffects];

	//the number of effects provided
	uint32		m_nNumEffects;

	//the flags used for creating the new effect
	uint32		m_nFXFlags;

	//the number of effects to create
	uint32		m_nNumToCreate;
};

//-------------------------------------------
// CBaseCreateFX
//-------------------------------------------
class CBaseCreateFX : 
	public CBaseFX
{
public:

	CBaseCreateFX(CBaseFX::FXType eEffectType) :
		CBaseFX( eEffectType )
	{
	}

	void	Term()							{}
	bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps );

protected:

	//this function should be called from all derived classes to have this base class pick an
	//effect and create it, using the parent and flag information stored in the passed in creation
	//structure
	bool	CreateEffect(const CLIENTFX_CREATESTRUCT& CreateStruct);

private:

	const CBaseCreateProps*		GetProps()		{ return (const CBaseCreateProps*)m_pProps; }
};

//function to add the base create effect properties
void fxGetBaseCreateProps(CFastList<CEffectPropertyDesc> *pList);

#endif 