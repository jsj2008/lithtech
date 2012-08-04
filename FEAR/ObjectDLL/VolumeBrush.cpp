// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrush.cpp
//
// PURPOSE : VolumeBrush implementation
//
// CREATED : 1/29/98
//
// (c) 1998-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "VolumeBrush.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "PolyGrid.h"
#include "Character.h"
#include "SFXMsgIds.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "MsgIDs.h"
#include "SurfaceFunctions.h"
#include "PlayerObj.h"
#include "SoundFilterDB.h"
#include "SurfaceFlagsOverrideHelpers.h"

LINKFROM_MODULE( VolumeBrush );

#define UPDATE_DELTA					0.01f
#define LIQUID_GRAVITY					-200.0f
#define TRIGGER_MSG_ON					"ON"
#define TRIGGER_MSG_OFF					"OFF"

#include "VarTrack.h"
static VarTrack vtRemoveFilters;

BEGIN_CLASS(VolumeBrush)
	ADD_VISIBLE_FLAG(0, 0)
    ADD_BOOLPROP(Hidden, false, "This flag allows you to make a volume begin hidden in the level. Later you can send a (hidden 0) message to the VolumeBrush object to unhide it.")
	ADD_REALPROP(Resistance, 0.0f, "This value affects the resistance of the players movement through the volume.")
	ADD_REALPROP(Friction, 1.0f, "This property allows you to alter the friction coefficient of surfaces to create slippery surfaces.")
	ADD_VECTORPROP_VAL(Current, 0.0f, 0.0f, 0.0f, "This is a force that pushes against the player. Use positive or negative values along each axis (x, y, and z) to indicate the direction and strength of the current. In general, you'll need values in the thousands in order to lift a player off the ground. This works well to create moving streams or areas of rushing wind.")
	ADD_REALPROP(Damage, 0.0f, "Sets amount of damage dealt by this area for every second the player is in it.")
 	ADD_STRINGPROP_FLAG(DamageType, "CHOKE", PF_STATICLIST, "This is a dropdown menu that allows you to choose the damage type that will be inflicted by the Volume.")
	ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 255.0f, 0, "You can define a tint for the player's view if you want the area to look as if it's filled with light or liquid.")
    ADD_COLORPROP_FLAG(LightAdd, 0.0f, 0.0f, 0.0f, 0, "This allows you to further alter the view from inside a Volume.")
 	ADD_STRINGPROP_FLAG(SoundFilter, "Dynamic", PF_STATICLIST, "You could have sound behave differently inside the volume. In the case of liquid, you could mute out high frequencies. In the case of a dimensional gate, you might add a strange echo or other filter.")
    ADD_BOOLPROP(CanPlayMovementSounds, true, "This flag disables the footstep sounds and head-bob motion in a volume.")
	ADD_BOOLPROP_FLAG( AllowSwimming, true, PF_HIDDEN, "If true, players will be allowed to swim in the volume.  If false, players will not play their swimming animations." )

	PROP_DEFINEGROUP(FogStuff, PF_GROUP(2), "This is a subset of properties that define the fog effects that will be seen when the player is within the Volume Brush.")
        ADD_BOOLPROP_FLAG(FogEnable, false, PF_GROUP(2), "This flag toggles the fog effects on and off.")
		ADD_REALPROP_FLAG(FogFarZ, 300.0f, PF_GROUP(2), "This is a value that determines at what distance the fog reaches full saturation.")
		ADD_REALPROP_FLAG(FogNearZ, -100.0f, PF_GROUP(2), "This value determines at what distance the fog is at zero saturation.")
		ADD_COLORPROP_FLAG(FogColor, 0.0f, 0.0f, 0.0f, PF_GROUP(2), "This color picker determines the color of the fog effect.")

	PROP_DEFINEGROUP(ForceVolumeProps, PF_GROUP(3), "This is a subset of properties that define the physics force volume that can be associated with this volume to control how rigid bodies are affected in this volume")
		ADD_BOOLPROP_FLAG(ForceVolume, false, PF_GROUP(3), "Determines if this volume brush should have a force volume associated with it or not")
		ADD_ROTATIONPROP_VAL_FLAG(ForceDir, 270.0f, 0.0f, 0.0f, PF_GROUP(3) | PF_RELATIVEORIENTATION, "This specifies the direction of the constant force in the volume, relative to the volume")
		ADD_REALPROP_FLAG(ForceMag, 100.0f, PF_GROUP(3), "This specifies the strength of the constant force, and can range from 0 to +inf")
		ADD_REALPROP_FLAG(ForceWaveFreq, 0.5f, PF_GROUP(3), "This is used to control the fluctuation of the constant force, and describes how many times per second the force will cycle between +1 and -1. The full formula is sin(Time / Frequency) * Amplitude + Base")
		ADD_REALPROP_FLAG(ForceWaveAmp, 1.0f, PF_GROUP(3), "This is used to control the how much the fluctuation of the wave influences the scale of the force. The full formula is sin(Time / Frequency) * Amplitude + Base")
		ADD_REALPROP_FLAG(ForceWaveBaseOffset, 1.0f, PF_GROUP(3), "The is used to control the base offset for the wave fluctuation of the scale. The full formula is sin(Time / Frequency) * Amplitude + Base")
		ADD_REALPROP_FLAG(Density, 1.0f, PF_GROUP(3), "This is used to specify the density of the volume. Setting this to zero will disable buoyancy, anything else will cause the volume to apply a force based upon how dense this liquid is and how large the objects inside of it are. This is in g/cm^3. The density of water is 1 g/cm^3")
		ADD_REALPROP_FLAG(LinearDrag, 0.002f, PF_GROUP(3), "This is used to specify how much drag is introduced on an object as it moves through the water. The higher this value, the more drag incurred. This is scaled based upon the speed of the object and how much surface area the object has in the water.")
		ADD_REALPROP_FLAG(AngularDrag, 0.3f, PF_GROUP(3), "This is used to specify how much drag is introduced on an object as it rotates in the water. The higher this value, the more drag incurred. This is scaled based upon the speed of the object and how much surface area the object has in the water.")

	ADD_STRINGPROP_FLAG(SURFACE_FLAGS_OVERRIDE, SURFACE_FLAGS_UNKNOWN_STR, PF_STATICLIST, "This dropdown menu allows you to choose a specific surface flag for the VolumeBrush.  NOTE:  If the Unknown surface flag is used, the surface flag of the material applied to the brush will be used.")
	ADD_STRINGPROP_FLAG(PhysicsModel, "", PF_STATICLIST | PF_HIDDEN, "This dropdown menu allows you to choose what physics model is used with this volume.")
    ADD_BOOLPROP_FLAG(RayHit, false, 0, "This flag toggles whether or not the object will be rayhit.")

	ADD_BOOLPROP_FLAG(StartOn, true, 0, "This flag toggles whether or not the volume will be considered ON when the level first loads." )

END_CLASS_FLAGS_PLUGIN(VolumeBrush, GameBase, CF_WORLDMODEL, CVolumePlugin, "VolumeBrushes allow spaces to be defined that have different movement and physics properties than the default physics and movement.")


//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( VolumeBrush )

	ADD_MESSAGE( ON, 1, NULL, MSG_HANDLER( VolumeBrush, HandleOnMsg ), "ON", "Enables the effects associated with the volume", "msg VolumeBrush ON" )
	ADD_MESSAGE( OFF, 1, NULL, MSG_HANDLER( VolumeBrush, HandleOffMsg ), "OFF", "Disables the effects associated with the volume", "msg VolumeBrush OFF" )

CMDMGR_END_REGISTER_CLASS( VolumeBrush, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::VolumeBrush()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

VolumeBrush::VolumeBrush() : GameBase(OT_CONTAINER)
{
	m_nSfxMsgId			= SFX_VOLUMEBRUSH_ID;
	m_dwSaveFlags		= 0;
	m_eContainerCode	= CC_VOLUME;
	m_fDamage			= 0.0f;
	m_eDamageType		= DT_UNSPECIFIED;
	m_fViscosity		= 0.0f;
	m_fFriction			= 1.0f;
    m_bHidden           = false;
	m_fGravity			= LIQUID_GRAVITY;
	m_hSoundFilterRecord= NULL;
	m_bCanPlayMoveSnds	= true;
	m_bAllowSwimming	= true;

	m_vForceDir.Init(0.0f, 1.0f, 0.0f);	
	m_bForceVolume		= false;
	m_fForceMag			= 0.0f;
	m_fWaveAmplitude	= 0.0f;
	m_fWaveFrequency	= 1.0f;
	m_fWaveBaseOffset	= 1.0f;
	m_fDensity			= 0.0f;
	m_fLinearDrag		= 0.01f;
	m_fAngularDrag		= 1.0f;

	m_vCurrent.Init();
	m_vTintColor.Init(255.0f, 255.0f, 255.0f);
	m_vLightAdd.Init();

    m_bFogEnable    = false;
	m_fFogFarZ		= 300.0f;
	m_fFogNearZ		= -100.0f;
	m_vFogColor.Init();

	m_dwFlags = FLAG_CONTAINER | FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;

	m_eSurfaceOverrideType  = ST_UNKNOWN;
	m_ePPhysicsModel		= PPM_NORMAL;

	m_bStartOn	= true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::~VolumeBrush
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

VolumeBrush::~VolumeBrush()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 VolumeBrush::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
    uint32 dwRet;

	switch(messageID)
	{
		case MID_AFFECTPHYSICS:
		{
			UpdatePhysics((ContainerPhysics*)pData);
		}
		break;

		case MID_PRECREATE:
		{
			dwRet = GameBase::EngineMessageFn(messageID, pData, fData);
			if (fData == PRECREATE_WORLDFILE)
			{
				ObjectCreateStruct	*pOCS = (ObjectCreateStruct*)pData;
				ReadProp(&pOCS->m_cProperties);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::HandleOnMsg
//
//	PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void VolumeBrush::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	Show();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::HandleOffMsg
//
//	PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void VolumeBrush::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	Hide();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void VolumeBrush::ReadProp(const GenericPropList* pProps)
{
	if( !pProps )
		return;

	m_bHidden		= pProps->GetBool( "Hidden", m_bHidden );
	m_vTintColor	= pProps->GetColor( "TintColor", m_vTintColor );
	m_vLightAdd		= pProps->GetColor( "LightAdd", m_vLightAdd );
	m_bFogEnable	= pProps->GetBool( "FogEnable", m_bFogEnable );
	m_fFogFarZ		= pProps->GetReal( "FogFarZ", m_fFogFarZ );
	m_fFogNearZ		= pProps->GetReal( "FogNearZ", m_fFogNearZ );
	m_vFogColor		= pProps->GetColor( "FogColor", m_vFogColor );

	m_bForceVolume		= pProps->GetBool("ForceVolume", m_bForceVolume);
	m_fForceMag			= pProps->GetReal("ForceMag", m_fForceMag);
	m_fWaveAmplitude	= pProps->GetReal("ForceWaveAmp", m_fWaveAmplitude);
	m_fWaveFrequency	= pProps->GetReal("ForceWaveFreq", m_fWaveFrequency);
	m_fWaveBaseOffset	= pProps->GetReal("ForceWaveBaseOffset", m_fWaveBaseOffset);
	m_fDensity			= pProps->GetReal("Density", m_fDensity) / 1000.0f;
	m_fLinearDrag		= pProps->GetReal("LinearDrag", m_fLinearDrag);
	m_fAngularDrag		= pProps->GetReal("AngularDrag", m_fAngularDrag);

	const char *pszDamageType = pProps->GetString( "DamageType", "" );
	if( pszDamageType && pszDamageType[0] )
	{
		m_eDamageType = StringToDamageType( pszDamageType );
	}

	m_fViscosity		= pProps->GetReal( "Resistance", m_fViscosity );
	m_fFriction			= pProps->GetReal( "Friction", m_fFriction );
	m_vCurrent			= pProps->GetVector( "Current", m_vCurrent );
	m_fDamage			= pProps->GetReal( "Damage", m_fDamage );
	m_bCanPlayMoveSnds	= pProps->GetBool( "CanPlayMovementSounds", m_bCanPlayMoveSnds );
	m_bAllowSwimming	= pProps->GetBool( "AllowSwimming", m_bAllowSwimming );

	if( pProps->GetBool( "RayHit", false ))
	{
		m_dwFlags |= FLAG_RAYHIT;
	}

	m_bStartOn			= pProps->GetBool( "StartOn", m_bStartOn );

	const char *pszSoundFilter = pProps->GetString( "SoundFilter", "" );
	if( pszSoundFilter && pszSoundFilter[0] )
	{
		m_hSoundFilterRecord = SoundFilterDB::Instance().GetFilterRecord(pszSoundFilter);
	}

	// Check for a specified surface to override the default.  Ignore overrides of "Unknown"...
	const char *pszSurfaceOverride = pProps->GetString( SURFACE_FLAGS_OVERRIDE_STR, "" );
	if( !LTStrEmpty( pszSurfaceOverride ) && !LTStrIEquals( pszSurfaceOverride, SURFACE_FLAGS_UNKNOWN_STR ))
	{
		HSURFACE hSurface = g_pSurfaceDB->GetSurface( pszSurfaceOverride );
		if( hSurface )
		{
			m_eSurfaceOverrideType = g_pSurfaceDB->GetSurfaceType(hSurface);
		}
	}

	const char *pszPhysicsModel = pProps->GetString( "PhysicsModel", "" );
	if( pszPhysicsModel && pszPhysicsModel[0] )
		{
		m_ePPhysicsModel = GetPlayerPhysicsModelFromPropertyName( pszPhysicsModel );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::PostPropRead
//
//	PURPOSE:	Set some final values.
//
// ----------------------------------------------------------------------- //

void VolumeBrush::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	pStruct->m_Flags |= m_dwFlags;
	pStruct->m_ObjectType = OT_CONTAINER;
	pStruct->SetFileName(pStruct->m_Name);
    pStruct->m_ContainerCode = (uint16)m_eContainerCode;
    
	m_vForceDir = (pStruct->m_Rotation * pStruct->m_cProperties.GetRotation("ForceDir", LTRotation::GetIdentity())).Forward();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void VolumeBrush::InitialUpdate()
{
	// Tell the client about any special fx (fog)...

	CreateSpecialFXMsg();

	// Save volume brush's initial flags...

	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, m_dwSaveFlags);


	uint32 dwUsrFlags = (((m_bHidden) ? 0 : USRFLG_VISIBLE) | USRFLG_IGNORE_PROJECTILES | SurfaceToUserFlag(m_eSurfaceOverrideType));
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, dwUsrFlags, 0xFF000000 | USRFLG_VISIBLE | USRFLG_IGNORE_PROJECTILES);

	// Normalize friction (1 = normal, 0 = no friction, 2 = double)...

	if (m_fFriction < 0.0) m_fFriction = 0.0f;
	else if (m_fFriction > 1.0) m_fFriction = 1.0f;


	// Normalize viscosity (1 = no movement, 0 = full movement)...

	if (m_fViscosity < 0.0) m_fViscosity = 0.0f;
	else if (m_fViscosity > 1.0) m_fViscosity = 1.0f;

	if( !m_bStartOn )
		Hide( );

	//setup our force volume
	InitForceVolume();

}

//called to handle initialization of the force volume
void VolumeBrush::InitForceVolume()
{
	if(m_ForceVolume.IsInitted())
		return;

	//if we have a force volume, set it up
	if(m_bForceVolume)
	{
		m_ForceVolume.Init(	m_hObject, m_fDensity, m_vForceDir, m_fForceMag, m_fWaveFrequency, 
			m_fWaveAmplitude, m_fWaveBaseOffset, m_fLinearDrag, m_fAngularDrag);
		m_ForceVolume.SetActive(!m_bHidden);

		//and we need to now update to affect objects
		SetNextUpdate(UPDATE_NEXT_FRAME);
	}
	else
	{
		//we have no force volume, so no need to update
		SetNextUpdate(UPDATE_NEVER);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Update()
//
//	PURPOSE:	Handle updating the force volume if one is associated with this object
//
// ----------------------------------------------------------------------- //
void VolumeBrush::Update()
{
	m_ForceVolume.Update(g_pLTServer->GetFrameTime());

	//and update next frame
	SetNextUpdate(UPDATE_NEXT_FRAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::CreateSpecialFXMsg()
//
//	PURPOSE:	Create the special fx message
//
// ----------------------------------------------------------------------- //

void VolumeBrush::CreateSpecialFXMsg()
{
	CAutoMessage cMsg;
	WriteSFXMsg(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::WriteSFXMsg()
//
//	PURPOSE:	Write the volume brush specific sfx
//
// ----------------------------------------------------------------------- //

void VolumeBrush::WriteSFXMsg(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    pMsg->Writeuint8(m_nSfxMsgId);

    pMsg->Writebool(m_bFogEnable);
    pMsg->Writefloat(m_fFogFarZ);
    pMsg->Writefloat(m_fFogNearZ);
    pMsg->WriteLTVector(m_vFogColor);
    pMsg->WriteLTVector(m_vTintColor);
    pMsg->WriteLTVector(m_vLightAdd);
    pMsg->WriteDatabaseRecord(g_pLTDatabase, m_hSoundFilterRecord);
    pMsg->Writebool(m_bCanPlayMoveSnds);
	pMsg->Writebool( m_bAllowSwimming );
	pMsg->Writeuint8(m_eSurfaceOverrideType);

	pMsg->WriteLTVector(GetCurrent());
	pMsg->Writefloat(GetGravity());
	pMsg->Writefloat(GetViscosity());
	pMsg->Writefloat(GetFriction());
	pMsg->Writeuint8(GetPhysicsModel());

	pMsg->Writebool(m_bForceVolume);

	if(m_bForceVolume)
	{
		pMsg->WriteLTVector(m_vForceDir);
		pMsg->Writefloat(m_fForceMag);
		pMsg->Writefloat(m_fWaveAmplitude);
		pMsg->Writefloat(m_fWaveFrequency);
		pMsg->Writefloat(m_fWaveBaseOffset);
		pMsg->Writefloat(m_fDensity);
		pMsg->Writefloat(m_fLinearDrag);
		pMsg->Writefloat(m_fAngularDrag);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::UpdatePhysics()
//
//	PURPOSE:	Update the physics of the passed in object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::UpdatePhysics(ContainerPhysics* pCPStruct)
{
	if (m_bHidden || !pCPStruct || !pCPStruct->m_hObject) return;

    float fUpdateDelta = g_pLTServer->GetFrameTime();


	// Let the character know if they are in liquid...

	if (IsLiquid(m_eContainerCode) && IsCharacter(pCPStruct->m_hObject))
	{
        CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(pCPStruct->m_hObject);
		if (pCharacter)
		{
			pCharacter->UpdateInLiquid(this, pCPStruct);
		}
	}


	// Player container physics is done on the client...

	if (!IsPlayer(pCPStruct->m_hObject))
	{
		// Dampen velocity based on the viscosity of the container...

        LTVector vVel, vCurVel;
		vVel = vCurVel = pCPStruct->m_Velocity;

		if (m_fViscosity > 0.0f && vCurVel.Mag() > 1.0f)
		{
            LTVector vDir = vCurVel.GetUnit();

            float fAdjust = MAX_CONTAINER_VISCOSITY * m_fViscosity * fUpdateDelta;

			vVel = (vDir * fAdjust);

			if (vVel.MagSqr() < vCurVel.MagSqr())
			{
				vVel = vCurVel - vVel;
			}
			else
			{
				vVel.Init();
			}

			vVel += (m_vCurrent * fUpdateDelta);

			pCPStruct->m_Velocity = vVel;
		}


		// Do special liquid handling...

		if (IsLiquid(m_eContainerCode))
		{
			UpdateLiquidPhysics(pCPStruct);
		}
	}


	// Update damage...

	// [RP] 8/13/03 - NOTE: We do not need to factor in the update delta when applying the 
	//		progressive damage since that factors it in every update anyways.  Just apply
	//		the full excpected damage amount here otherwise we get very miniscule amounts
	//		of damage per update.

	// Damage using progressive damage.  This insures that the correct
	// damage effect is shown on the client...

	if( m_fDamage > 0.0f )
	{
		DamageStruct damage;

		damage.eType	  = m_eDamageType;
		damage.fDamage	  = m_fDamage;
		damage.hDamager   = m_hObject;

		// Use progressive damage...
		damage.fDuration  = 0.25f;
		damage.hContainer = m_hObject;

		damage.DoDamage(m_hObject, pCPStruct->m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::UpdateLiquidPhysics()
//
//	PURPOSE:	Update liquid specific physics of the passed in object
//				(really, under liquid physics)
//
// ----------------------------------------------------------------------- //

void VolumeBrush::UpdateLiquidPhysics(ContainerPhysics* pCPStruct)
{
	if (!pCPStruct || !pCPStruct->m_hObject) return;

	// Apply liquid gravity to object...

	if (pCPStruct->m_Flags & FLAG_GRAVITY)
	{
		pCPStruct->m_Flags &= ~FLAG_GRAVITY;
		pCPStruct->m_Acceleration.y += m_fGravity;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_VECTOR(m_vCurrent);
    SAVE_VECTOR(m_vFogColor);
    SAVE_FLOAT(m_fViscosity);
    SAVE_FLOAT(m_fFriction);
    SAVE_FLOAT(m_fDamage);
    SAVE_FLOAT(m_fGravity);
    SAVE_DWORD(m_dwSaveFlags);
    SAVE_BYTE(m_eDamageType);
    SAVE_BYTE(m_eContainerCode);
    SAVE_BOOL(m_bHidden);
	SAVE_BYTE(m_ePPhysicsModel);
	SAVE_BOOL(m_bForceVolume);
	SAVE_VECTOR(m_vForceDir);	
	SAVE_FLOAT(m_fForceMag);
	SAVE_FLOAT(m_fWaveAmplitude);
	SAVE_FLOAT(m_fWaveFrequency);
	SAVE_FLOAT(m_fWaveBaseOffset);
	SAVE_FLOAT(m_fDensity);
	SAVE_FLOAT(m_fLinearDrag);
	SAVE_FLOAT(m_fAngularDrag);
	SAVE_bool( m_bAllowSwimming );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

    LOAD_VECTOR(m_vCurrent);
    LOAD_VECTOR(m_vFogColor);
    LOAD_FLOAT(m_fViscosity);
    LOAD_FLOAT(m_fFriction);
    LOAD_FLOAT(m_fDamage);
    LOAD_FLOAT(m_fGravity);
    LOAD_DWORD(m_dwSaveFlags);
    LOAD_BYTE_CAST(m_eDamageType, DamageType);
    LOAD_BYTE_CAST(m_eContainerCode, ContainerCode);
    LOAD_BOOL(m_bHidden);
	LOAD_BYTE_CAST(m_ePPhysicsModel, PlayerPhysicsModel);
	LOAD_BOOL(m_bForceVolume);
	LOAD_VECTOR(m_vForceDir);	
	LOAD_FLOAT(m_fForceMag);
	LOAD_FLOAT(m_fWaveAmplitude);
	LOAD_FLOAT(m_fWaveFrequency);
	LOAD_FLOAT(m_fWaveBaseOffset);
	LOAD_FLOAT(m_fDensity);
	LOAD_FLOAT(m_fLinearDrag);
	LOAD_FLOAT(m_fAngularDrag);
	LOAD_bool( m_bAllowSwimming );

	//setup our force volume
	InitForceVolume();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Show()
//
//	PURPOSE:	Show the volume brush
//
// --------------------------------------------------------------------------- //

void VolumeBrush::Show()
{
	if (!m_bHidden)
	{
		return;
	}

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_VISIBLE, USRFLG_VISIBLE);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwSaveFlags, FLAGMASK_ALL);

	m_ForceVolume.SetActive(true);
    m_bHidden = false;

	UpdatePlayerContainerInfo( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Hide()
//
//	PURPOSE:	Hide the volume brush
//
// --------------------------------------------------------------------------- //

void VolumeBrush::Hide()
{
	if (m_bHidden)
	{
		return;
	}

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_VISIBLE);
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, m_dwSaveFlags);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAGMASK_ALL);

	m_ForceVolume.SetActive(false);
    m_bHidden = true;

	UpdatePlayerContainerInfo( );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	VolumeBrush::UpdatePlayerContainerInfo
//
//  PURPOSE:	The players were given the clientsave data in HandleSaveData.
//				When we're done with the data, call this to clear them out.
//
// ----------------------------------------------------------------------- //

void VolumeBrush::UpdatePlayerContainerInfo( )
{
	// Tell all the players to resend volume information.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	for( ; iter != CPlayerObj::GetPlayerObjList( ).end( ); iter++ )
	{
		CPlayerObj* pPlayerObj = *iter;
		if( pPlayerObj )
		{
			pPlayerObj->OnVolumeBrushChanged( );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVolumePlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

CVolumePlugin::CVolumePlugin()
{
	m_pSoundFilterDBPlugin = debug_new( SoundFilterDBPlugin );
}

CVolumePlugin::~CVolumePlugin()
{
	debug_delete( m_pSoundFilterDBPlugin );
}


LTRESULT CVolumePlugin::PreHook_EditStringList(const char* szRezPath,
											   const char* szPropName,
											   char** aszStrings,
                                               uint32* pcStrings,
                                               const uint32 cMaxStrings,
                                               const uint32 cMaxStringLength)
{
	ASSERT( m_pSoundFilterDBPlugin != NULL );
	
	// See if we can handle the property...

	if( LTStrIEquals( "DamageType", szPropName ))
	{
		LTASSERT( aszStrings && pcStrings, "Invalid paramaters" );
		
		if (!aszStrings || !pcStrings)
			return LT_UNSUPPORTED;
		
		// Add an entry for each supported damage type

		for (int i=0; i < kNumDamageTypes; i++)
		{
			LTASSERT( cMaxStrings > (*pcStrings) + 1, "Max limit of strings reached!" );

			DamageType eDT = static_cast<DamageType>(i);
			uint32 dwNameLen = LTStrLen( DamageTypeToString(eDT) );

			if (dwNameLen < cMaxStringLength &&
				((*pcStrings) + 1) < cMaxStrings)
			{
				LTStrCpy( aszStrings[(*pcStrings)++], DamageTypeToString(eDT), cMaxStringLength );
			}
		}

		return LT_OK;
	}
	else if( LTStrIEquals( "SoundFilter", szPropName ))
	{
		if (!m_pSoundFilterDBPlugin->PopulateStringList(aszStrings, pcStrings,
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}
	else if( LTStrIEquals( SURFACE_FLAGS_OVERRIDE_STR, szPropName ))
	{

		if (CSurfaceDBPlugin::Instance().PopulateStringList(aszStrings, pcStrings,
			 cMaxStrings, cMaxStringLength))
		{
			return LT_OK;
		}
	}
	else if( LTStrIEquals( "PhysicsModel", szPropName ))
	{
		for( int i = PPM_FIRST; i < PPM_NUM_MODELS; ++i )
		{
			LTASSERT( cMaxStrings > (*pcStrings) + 1, "Max limit of strings reached!" );
			if( cMaxStrings > (*pcStrings) + 1 )
			{
				LTStrCpy( aszStrings[(*pcStrings)++], GetPropertyNameFromPlayerPhysicsModel( (PlayerPhysicsModel)i ), cMaxStringLength );
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
