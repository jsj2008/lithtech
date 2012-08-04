// ----------------------------------------------------------------------- //
//
// MODULE  : VideoControllerFX.h
//
// PURPOSE : Provides a ClientFX key that can control video textures
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef	__VIDEOCONTROLLERFX_H__
#define __VIDEOCONTROLLERFX_H__

#include "basefx.h"
#include "ClientFX.h"

//the different operations that the video controller can perform
enum EVideoControllerOp
{
	eVideoControllerOp_Pause,
	eVideoControllerOp_Unpause,
	eVideoControllerOp_Restart,
	eVideoControllerOp_SinglePlay,
};

//-------------------------------------------
// CVideoControllerProps
//-------------------------------------------
class CVideoControllerProps : 
	public CBaseFXProps
{
public:

	CVideoControllerProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	//the name of the video file to update
	const char*				m_pszVideo;

	//the operations we want to perform with this effect
	EVideoControllerOp		m_eOperation;
};

//-------------------------------------------
// CVideoControllerFX
//-------------------------------------------
class CVideoControllerFX : 
	public CBaseFX
{
public:

	CVideoControllerFX();

	virtual bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps );
	virtual void	Term();

	virtual bool	Update(float tmElapsed);

private:

	//we cannot copy these objects
	PREVENT_OBJECT_COPYING(CVideoControllerFX);

	//the video file that we are manipulating
	HVIDEOTEXTURE	m_hVideo;

	const CVideoControllerProps*	GetProps()		{ return (const CVideoControllerProps*)m_pProps; }
};

//function to add the base create effect properties
void fxGetVideoControllerProps(CFastList<CEffectPropertyDesc> *pList);

#endif 