// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrush.h
//
// PURPOSE : VolumeBrush definition
//
// CREATED : 1/29/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VOLUME_BRUSH_H__
#define __VOLUME_BRUSH_H__

LINKTO_MODULE( VolumeBrush );

#include "GameBase.h"
#include "ContainerCodes.h"
#include "DamageTypes.h"
#include "..\shared\SoundFilterMgr.h"
#include "SharedMovement.h"


class CSoundFilterMgrPlugin;
class CSurfaceMgrPlugin;
enum SurfaceType;

class VolumeBrush : public GameBase
{
	public :

		VolumeBrush();
		~VolumeBrush();

		LTFLOAT				GetGravity()		const { return m_fGravity; }
		LTFLOAT				GetFriction()		const { return m_fFriction; }
		LTFLOAT				GetViscosity()		const { return m_fViscosity; }
		ContainerCode		GetCode()			const { return m_eContainerCode; }
		LTVector			GetCurrent()		const { return m_vCurrent; }
		bool				GetHidden()			const { return m_bHidden; }
		PlayerPhysicsModel	GetPhysicsModel()	const { return m_ePPhysicsModel; }

	protected :

		uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);

		virtual void ReadProp(ObjectCreateStruct *pStruct);
		virtual void UpdatePhysics(ContainerPhysics* pCPStruct);
		virtual void UpdateLiquidPhysics(ContainerPhysics* pCPStruct);

		virtual void WriteSFXMsg(ILTMessage_Write *pMsg);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		void Show();
		void Hide();

		// These variables don't need to be saved since they will be saved in the
		// special fx message...

		LTVector		m_vTintColor;
		LTVector		m_vLightAdd;
		LTFLOAT			m_fFogFarZ;
		LTFLOAT			m_fFogNearZ;
		uint8			m_nSfxMsgId;
		uint8			m_nSoundFilterId;
		bool			m_bFogEnable;
		bool			m_bCanPlayMoveSnds;
		SurfaceType		m_eSurfaceOverrideType; // This associates an override surface, from surfaces.txt, with the volume.


		// The following need to be saved...

		LTVector			m_vCurrent;
		LTVector			m_vFogColor;
		LTFLOAT				m_fViscosity;
		LTFLOAT				m_fFriction;
		LTFLOAT				m_fDamage;
		LTFLOAT				m_fGravity;
		uint32				m_dwSaveFlags;
		DamageType			m_eDamageType;
		ContainerCode		m_eContainerCode;
		bool				m_bHidden;
		PlayerPhysicsModel	m_ePPhysicsModel;


		// The following doesn't need to be saved at all...

		uint32			m_dwFlags;

	private :

		void CreateSpecialFXMsg();

		void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void PostPropRead(ObjectCreateStruct *pStruct);
		void InitialUpdate();
};

class CVolumePlugin : public IObjectPlugin
{
	public:
		CVolumePlugin();
		virtual ~CVolumePlugin();

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

	protected :

		CSoundFilterMgrPlugin* m_pSoundFilterMgrPlugin;
		CSurfaceMgrPlugin*	m_pSurfaceMgrPlugin;
};

#endif // __VOLUME_BRUSH_H__