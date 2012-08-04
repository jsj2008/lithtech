// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerVehicle.h
//
// PURPOSE : An PlayerVehicle object
//
// CREATED : 8/31/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_VEHICLE_H__
#define __PLAYER_VEHICLE_H__

#include "ltengineobjects.h"
#include "Prop.h"
#include "Timer.h"
#include "SharedMovement.h"

class PlayerVehicle : public Prop
{
	public :

		PlayerVehicle();
		~PlayerVehicle();

		void	Respawn();

		PlayerPhysicsModel GetPhysicsModel() const { return m_ePPhysicsModel; }

	protected :

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		void	ReadProp(ObjectCreateStruct *pData);
		void	PostPropRead(ObjectCreateStruct* pData);

	private :

		void	InitialUpdate();
		void	Update();
		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
		void	DoActivate(HOBJECT hSender);
		void	CreateSFXMsg();

	private :

		CTimer		m_RespawnTimer;
        LTFLOAT     m_fRespawnTime;

		LTVector    m_vOriginalDims;
        LTVector    m_vOriginalPos;
        LTRotation  m_rOriginalRot;

		PlayerPhysicsModel m_ePPhysicsModel;  // Corresponds to vehicle type

		PVCREATESTRUCT	m_PlayerVehicleStruct;
};

class CPlayerVehiclePlugin : public IObjectPlugin
{
  public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);
};

#endif // __PLAYER_VEHICLE_H__