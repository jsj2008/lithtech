// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialMoveFX.h
//
// PURPOSE : 
//
// CREATED : 02/07/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SPECIALMOVEFX_H__
#define __SPECIALMOVEFX_H__

#include "SpecialFX.h"
#include "ActivateObjectFX.h"


class CSpecialMoveFX : public CSpecialFX
{
	public:
		CSpecialMoveFX();
		virtual ~CSpecialMoveFX();

		virtual bool Update();
		virtual bool Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
		virtual bool OnServerMessage( ILTMessage_Read *pMsg );
		virtual uint32 GetSFXID() { return SFX_SPECIALMOVE_ID; }
		virtual bool IsEnabled() const { return m_bOn; }
		virtual bool CanLookAt() const;
		virtual bool CanReach() const;
		virtual bool ShouldDisableWeapons() const { return true; }
		virtual bool ShouldPositionPlayer() const { return true; }

		virtual void OnReleased(){}
		virtual void OnLookedAt();
		virtual void OnUnLookedAt(){}

	protected:

		virtual bool HandleServerMsg(uint8 nMsgId, ILTMessage_Read *pMsg);

	public:
		LTVector		m_vPos;
		LTVector		m_vDim;
		LTRotation		m_rRot;
		EnumAnimProp	m_eAnimation;
		std::string		m_sStimulus;	// not exposed in level object since there's no easy way to populate it, and we don't want to be sending strings across the network.
		float			m_fActivateDist;
		bool			m_bOn;
		bool			m_bRadial;

	private:
		CActivateObjectHandler	m_ActivateObjectHandler;	//!!ARL: Inherit from CActivateObjectFX instead?
};

#endif // __SPECIALMOVEFX_H__
