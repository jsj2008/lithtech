// ----------------------------------------------------------------------- //
//
// MODULE  : ForensicObjectFX.h
//
// PURPOSE : ForensicObjectFX - Definition
//
// CREATED : 11/22/04
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FORENSICOBJECTFX_H__
#define __FORENSICOBJECTFX_H__

// ----------------------------------------------------------------------- //

#include "SpecialMoveFX.h"
#include "idatabasemgr.h"

// ----------------------------------------------------------------------- //

struct FORENSICOBJECTCREATESTRUCT : public SFXCREATESTRUCT
{
	FORENSICOBJECTCREATESTRUCT()
	{
		m_bPrimary = false;
		m_vPos.Init( 0.0f, 0.0f, 0.0f );
		m_vDir.Init( 0.0f, 0.0f, 0.0f );
		m_fCoreRadius = 0.0f;
		m_fObjectFOV = 0.0f;
		m_fCameraFOV = 0.0f;
		m_dwForensicTypeMask = 0;
		m_rDetectionTool = NULL;
		m_rCollectionTool = NULL;
		m_rSecondaryInfo = NULL;
	}

	virtual void Read( ILTMessage_Read *pMsg );

	bool m_bPrimary;
	LTVector m_vPos;
	LTVector m_vDir;
	float m_fMaxDistance;
	float m_fCoreRadius;
	float m_fObjectFOV;
	float m_fCameraFOV;
	uint32 m_dwForensicTypeMask;
	HRECORD m_rDetectionTool;
	HRECORD m_rCollectionTool;
	HRECORD m_rSecondaryInfo;
};

// ----------------------------------------------------------------------- //

class CForensicObjectFX : public CSpecialMoveFX
{
	public:

		CForensicObjectFX();
		virtual ~CForensicObjectFX();

		virtual bool Init( HLOCALOBJ hServObj, ILTMessage_Read* pMsg );

		virtual uint32 GetSFXID() { return SFX_FORENSICOBJECT_ID; }

		virtual bool IsEnabled() { return CSpecialMoveFX::IsEnabled(); }
		virtual bool CanReach() const;
		virtual bool ShouldDisableWeapons() const { return false; }
		virtual bool ShouldPositionPlayer() const { return false; }

		virtual float GetDistance(HOBJECT hFrom, float fUpdateRate=0.5f, float fMaxLatency=1.0f);

		virtual void OnToolSelect();
		virtual void OnReleased();
		virtual void OnLookedAt();
		virtual void OnUnLookedAt();

	protected:

		virtual bool HandleServerMsg(uint8 nMsgId, ILTMessage_Read *pMsg);

		void RequestUpdate(HOBJECT hObj);

		bool CanShowInfo();

		float		m_fLastDistance;
		HOBJECT		m_hLastFrom;
		double		m_tLastUpdate;

	public:

		FORENSICOBJECTCREATESTRUCT m_cs;


	private:

		// Registered with CPlayerMgr::m_ForensicObjectDetector;
		ObjectDetectorLink	m_iObjectDetectorLink;
};

// ----------------------------------------------------------------------- //

#endif//__FORENSICOBJECTFX_H__
