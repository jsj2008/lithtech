 // ----------------------------------------------------------------------- //
//
// MODULE  : ScreenEffectFX.h
//
// PURPOSE : Polygrid special fx class - Definition
//
// CREATED : 10/13/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENEFFECTFX_H__
#define __SCREENEFFECTFX_H__

#include "SpecialFX.h"
#include "SFXMsgIds.h"

class CScreenEffectFX : public CSpecialFX
{
public:
	CScreenEffectFX();
	~CScreenEffectFX();

	virtual bool	Init(HLOCALOBJ hServObj, ILTMessage_Read* pMsg);
    virtual bool	Update();
	virtual bool	OnServerMessage(ILTMessage_Read* pMsg);
	virtual uint32	GetSFXID() { return SFX_SCREENEFFECT_ID; }

private:
	const char* GetOverlayFXName();
	const char* GetOverlayMaterialName();
	
	const char* GetParameterName(uint32 nParameter);
	float		GetParameterValue(uint32 nParameter);
	uint32		GetNumParameters();

private:
	struct SRampInfo
	{
		uint32 m_nParameter;
		LTVector2 m_vValues;
		LTVector2 m_vTimes;
		
		float GetCurValue(float fTime);
		bool IsFinished(float fTime);
	};
	typedef std::vector<SRampInfo> TRampInfoList;
	
private:
	HRECORD		m_hRecord;
	HATTRIBUTE	m_hParameters;

	CClientFXLink m_linkClientFX;
	HMATERIAL m_hMaterial;
	
	TfloatList m_aParameterValues;
	TRampInfoList m_aRampInfo;
};

#endif // __SCREENEFFECTFX_H__
