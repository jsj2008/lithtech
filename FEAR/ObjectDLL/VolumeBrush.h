// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrush.h
//
// PURPOSE : VolumeBrush definition
//
// CREATED : 1/29/98
//
// (c) 1998-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VOLUME_BRUSH_H__
#define __VOLUME_BRUSH_H__

LINKTO_MODULE( VolumeBrush );

#include "GameBase.h"
#include "ContainerCodes.h"
#include "DamageTypes.h"
#include "SharedMovement.h"
#include "ForceVolume.h"
#include "SurfaceDefs.h"

class SoundFilterDBPlugin;

class VolumeBrush : public GameBase
{
	public :

		VolumeBrush();
		~VolumeBrush();

		float				GetGravity()		const { return m_fGravity; }
		float				GetFriction()		const { return m_fFriction; }
		float				GetViscosity()		const { return m_fViscosity; }
		ContainerCode		GetCode()			const { return m_eContainerCode; }
		LTVector			GetCurrent()		const { return m_vCurrent; }
		bool				GetHidden()			const { return m_bHidden; }
		PlayerPhysicsModel	GetPhysicsModel()	const { return m_ePPhysicsModel; }

	protected :

		uint32 EngineMessageFn(uint32 messageID, void *pData, float lData);

		virtual void ReadProp(const GenericPropList* pProps);
		virtual void UpdatePhysics(ContainerPhysics* pCPStruct);
		virtual void UpdateLiquidPhysics(ContainerPhysics* pCPStruct);

		virtual void WriteSFXMsg(ILTMessage_Write *pMsg);

		void Show();
		void Hide();
		void UpdatePlayerContainerInfo( );

		void CreateSpecialFXMsg();

		// These variables don't need to be saved since they will be saved in the
		// special fx message...

		LTVector		m_vTintColor;
		LTVector		m_vLightAdd;
		float			m_fFogFarZ;
		float			m_fFogNearZ;
		uint8			m_nSfxMsgId;
		HRECORD			m_hSoundFilterRecord;
		bool			m_bFogEnable;
		bool			m_bCanPlayMoveSnds;
		SurfaceType		m_eSurfaceOverrideType; // This associates an override surface, from surfaces.txt, with the volume.


		// The following need to be saved...

		LTVector			m_vCurrent;
		LTVector			m_vFogColor;
		float				m_fViscosity;
		float				m_fFriction;
		float				m_fDamage;
		float				m_fGravity;
		uint32				m_dwSaveFlags;
		DamageType			m_eDamageType;
		ContainerCode		m_eContainerCode;
		bool				m_bHidden;
		PlayerPhysicsModel	m_ePPhysicsModel;
		bool				m_bAllowSwimming;

		//properties for the physics force volume, and the volume itself
		//For more information on the purpose of each, see ForceVolume.h
		bool				m_bForceVolume;
		LTVector			m_vForceDir;	
		float				m_fForceMag;
		float				m_fWaveAmplitude;
		float				m_fWaveFrequency;
		float				m_fWaveBaseOffset;
		float				m_fDensity;
		float				m_fLinearDrag;
		float				m_fAngularDrag;

		CForceVolume		m_ForceVolume;

		// The following doesn't need to be saved at all...

		uint32				m_dwFlags;

		// Flag to determine if the volume should be active when the game starts...
		bool				m_bStartOn;

	private :

		void Update();

		void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void PostPropRead(ObjectCreateStruct *pStruct);
		void InitialUpdate();

		//called to handle initialization of the force volume
		void InitForceVolume();


		// Message Handlers...

		DECLARE_MSG_HANDLER( VolumeBrush, HandleOnMsg );
		DECLARE_MSG_HANDLER( VolumeBrush, HandleOffMsg );
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

		SoundFilterDBPlugin* m_pSoundFilterDBPlugin;
};

#endif // __VOLUME_BRUSH_H__
