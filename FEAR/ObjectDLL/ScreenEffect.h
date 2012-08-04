//------------------------------------------------------------------
//
//   MODULE    : SCREENEFFECT.H
//
//   PURPOSE   : Defines the ScreenEffect class
//
//   CREATED   : 11/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
//------------------------------------------------------------------

#ifndef _SCREENEFFECT_H_
#define _SCREENEFFECT_H_

LINKTO_MODULE( ScreenEffect );

#include "GameBase.h"

class ScreenEffect : public GameBase
{
public:
	ScreenEffect();
	~ScreenEffect();

private:
	// Engine event handlers
	
	virtual uint32	OnObjectCreated(const GenericPropList* pProps, float createType);

private:
	// Message handlers

	DECLARE_MSG_HANDLER( ScreenEffect, HandleOnMsg );
	DECLARE_MSG_HANDLER( ScreenEffect, HandleOffMsg );
	DECLARE_MSG_HANDLER( ScreenEffect, HandleToggleMsg );
	DECLARE_MSG_HANDLER( ScreenEffect, HandleParamMsg );
	
public:
	static bool ValidateMsgParam(ILTPreInterface *pInterface, ConParse &cParams);

private:
	// Internal functions
	
	const char* GetFXName();
	
	bool	IsOn();
	void	SetOnOff(bool bOn);

	void	CreateSFX();
	enum	{ k_nInvalidParamIndex = (uint32)-1 };
	static uint32	GetParamIndex(HATTRIBUTE hAttribute, const char *pParam);
	uint32	GetParamIndex(const char* pParam) { return GetParamIndex(m_hParameters, pParam); }
	
private:
	HRECORD		m_hRecord;
	HATTRIBUTE	m_hParameters;
	
};


#endif // _SCREENEFFECT_H_
