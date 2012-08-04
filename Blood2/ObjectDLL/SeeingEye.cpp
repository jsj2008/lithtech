// ----------------------------------------------------------------------- //
//
// MODULE  : SeeingEye.cpp
//
// PURPOSE : DefenseGlobe AI
//
// CREATED : 12/01/97   
//
// ----------------------------------------------------------------------- //


// 9.4.5 The all-seeing Eye!

// The all-seeing eye looks like a normal human eye (dislodged from someone's head, of course).  
// The player can drop it anywhere, or throw it by holding down on it's hot key.  Once the eye is 
// in place the player can look through it and see what it sees by pressing it's hot key a second 
// time.  The eye is easily destroyed, and if they player is looking through it when this happens 
// they are blinded for several seconds.  Once the eye is destroyed it will automatically return 
// to the player's inventory.

// Need a Eye model
// Reposition the Camera to eye models location
// Mlook only
// Change Screen border?
// 

#include "SeeingEye.h"
#include "cpp_server_de.h"
#include "SharedDefs.h"
#include "PlayerObj.h"
#include "ClientServerShared.h"

//BEGIN_CLASS(CPlayerObj)
//END_CLASS_DEFAULT(CPlayerObj, CBaseCharacter, NULL, NULL)

BEGIN_CLASS(SeeingEye)
	ADD_DESTRUCTABLE_AGGREGATE()
END_CLASS_DEFAULT_FLAGS(SeeingEye, BaseClass, NULL, NULL, CF_HIDDEN)

#define PINGPERIOD 1.0f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
SeeingEye::SeeingEye() : BaseClass(OT_MODEL)
{
	AddAggregate(&m_damage);
	m_hOwner = DNULL;
	m_hClient = DNULL;
    m_bDropped = DFALSE;
    m_hStuckObject = DNULL;
	m_fNextPingTime = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //
SeeingEye::~SeeingEye()
{
	Remove( DTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SeeingEye::Init()
//
//	PURPOSE:	Initializes the SeeingEye
//
// ----------------------------------------------------------------------- //
void SeeingEye::Init(HOBJECT hOwner, HCLIENT hClient)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	m_hOwner = hOwner;
	if( m_hOwner )
	{
		g_pServerDE->CreateInterObjectLink( m_hObject, m_hOwner );
	}
	m_hClient = hClient;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD SeeingEye::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			if( fData != INITIALUPDATE_SAVEGAME )
				PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}


		case MID_INITIALUPDATE:
		{
			if( fData != INITIALUPDATE_SAVEGAME )
				InitialUpdate((DVector *)pData);
			break;
		}

		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			ObjectTouch((HOBJECT)pData);
		}
		break;

		case MID_LINKBROKEN:
		{
			if( m_hStuckObject == (HOBJECT)pData )
			{
				Pickup( );
			}
			else if( m_hOwner == (HOBJECT)pData )
			{
				Remove( DTRUE );
			}

		}
		break;

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		default : break;
	}


	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	SeeingEye::ObjectMessageFn()
//
//	PURPOSE:	Processes a message from a server object.
//
// --------------------------------------------------------------------------- //
DDWORD SeeingEye::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	DDWORD dwRet;

	switch(messageID)
	{
		case MID_UNATTACH:
		{
			if( hSender == m_hStuckObject )
			{
				Pickup( );
			}
		}
		break;

		case MID_DAMAGE:
		{
			dwRet = BaseClass::ObjectMessageFn(hSender, messageID, hRead);

			if(m_damage.IsDead() )
			{
				Remove( DTRUE );
			}

			return dwRet;
		}
		break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void SeeingEye::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;
	char* pFilename = "Models\\Powerups\\theeye_pu.abc";
	char* pSkin = "Skins\\Powerups\\theeye_pu.dtx";

	_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
	_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);

	pStruct->m_Flags = FLAG_VISIBLE | FLAG_GRAVITY | FLAG_SOLID | FLAG_TOUCH_NOTIFY | FLAG_NOSLIDING | FLAG_REMOVEIFOUTSIDE;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectTouch
//
//	PURPOSE:	Handle object touch
//
// ----------------------------------------------------------------------- //

void SeeingEye::ObjectTouch (HOBJECT hObj)
{
	DDWORD dwFlags;
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// return if it hit a non solid object, but not if it hit a world object
	HOBJECT hWorld = pServerDE->GetWorldObject();

	if (hObj != hWorld && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
		return;

	if (!m_bDropped)
	{
		if (hObj == m_hOwner)
			return; // Let it get out of our bounding box...

		m_bDropped = DTRUE;
		DVector vVel;
		VEC_SET(vVel, 0, 0, 0)
		// Stop moving
		pServerDE->SetVelocity(m_hObject, &vVel);
		pServerDE->SetAcceleration(m_hObject, &vVel);

		dwFlags = pServerDE->GetObjectFlags( m_hObject );

		if (hObj != hWorld)
		{
			m_hStuckObject = hObj;

			Attach( );

			// Not solid anymore, and no gravity.
			dwFlags &= ~( FLAG_SOLID | FLAG_GRAVITY );
		}
		
		pServerDE->SetObjectFlags(m_hObject, dwFlags );
	}
	else
	{
		// Pickup the eye
		if (hObj == m_hOwner)
		{
			Pickup( );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Attach()
//
//	PURPOSE:	Attaches eye to object
//
// ----------------------------------------------------------------------- //

void SeeingEye::Attach( )
{
	HMESSAGEWRITE hMsg;

	g_pServerDE->CreateInterObjectLink( m_hObject, m_hStuckObject );
	hMsg = g_pServerDE->StartMessageToObject( this, m_hStuckObject, MID_ATTACH );
	g_pServerDE->WriteToMessageObject( hMsg, m_hObject );
	g_pServerDE->EndMessage( hMsg );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Pickup()
//
//	PURPOSE:	Pickup the eye
//
// ----------------------------------------------------------------------- //

void SeeingEye::Pickup( )
{
	DVector vPos;

	// Make sure it's a player
	if( IsPlayer( m_hOwner ))
	{
		CPlayerObj *pOwner = (CPlayerObj*)g_pServerDE->HandleToObject(m_hOwner);

		if( pOwner )
		{
			// Deactivate the Eye (in InventoryMgr)
			CInvItem *pInvItem = pOwner->m_InventoryMgr.GetItem(INV_THEEYE);
			if( pInvItem )
			{
				pInvItem->PickItUp();

				g_pServerDE->GetObjectPos( m_hObject, &vPos );
				PlaySoundFromPos( &vPos, "sounds\\powerups\\inventory1.wav", 300.0f, SOUNDPRIORITY_MISC_HIGH);
			}
		}
	}

	Remove( DFALSE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL SeeingEye::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	m_damage.Init(m_hObject);
    
	m_damage.SetHitPoints(10.0f);
	m_damage.SetMaxHitPoints(10.0f);
	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);
	m_damage.SetMass(30.0f);
    
	// Set the dims...
	DVector vDims;
	VEC_SET(vDims, 7, 7, 7);
	pServerDE->SetObjectDims(m_hObject, &vDims);

    // Toss the Eye up
	DRotation rRot;
	DVector vU, vR, vF;
	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
	DVector vVel;
    
    DFLOAT fForward = pServerDE->Random(150.0f, 250.0f);
	VEC_MULSCALAR(vF, vF, fForward);

	DFLOAT fUpVel = pServerDE->Random(100.0f, 150.0f);
	VEC_MULSCALAR(vU, vU, fUpVel);
    
	VEC_ADD(vVel, vU, vF);
	pServerDE->SetVelocity(m_hObject, &vVel);

	pServerDE->SetForceIgnoreLimit(m_hObject, 0.0f);
	pServerDE->SetFrictionCoefficient(m_hObject, 18.0);

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	pServerDE->SetObjectUserFlags( m_hObject, USRFLG_SAVEABLE );

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SeeingEye::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void SeeingEye::Update()
{
	CServerDE* pServerDE = GetServerDE();

	pServerDE->SetNextUpdate(m_hObject, 0.001f);

	if( pServerDE->GetTime( ) > m_fNextPingTime )
	{
		m_fNextPingTime = pServerDE->GetTime( ) + PINGPERIOD;
		pServerDE->PingObjects(m_hObject);
	}

    if (m_bDropped)
	{
		DVector vVel;
		VEC_SET(vVel, 0, 0, 0);
		pServerDE->SetVelocity(m_hObject, &vVel);
	}

	if (m_hStuckObject)
	{
		// Move to keep up with what we hit
	    DVector vPos, vDims;
        pServerDE->GetObjectPos(m_hStuckObject, &vPos);
    	pServerDE->GetObjectDims(m_hStuckObject, &vDims);
        
        // Position the Eye at Eye Level...
    	vPos.y += vDims.y / 2.0f;
	    vPos.y -= 10;       // on Players Back
	    vPos.z -= 10;       // on Players Back
	
		pServerDE->TeleportObject(m_hObject, &vPos);
	}

	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SeeingEye::Remove
//
//	PURPOSE:	Removes the object
//
// ----------------------------------------------------------------------- //

void SeeingEye::Remove( DBOOL bClearInv )
{
	// Make the Inventory Item NULL
	if (m_hOwner)
	{
		if( IsPlayer( m_hOwner ))
    	{
			CPlayerObj *pOwner = (CPlayerObj*)g_pServerDE->HandleToObject(m_hOwner);

			if( pOwner )
			{
				if ( pOwner->m_InventoryMgr.IsItemActive(INV_THEEYE))
				{
					int nOff = 0;
					if (m_hClient)
					{
						HMESSAGEWRITE hMsg;
						// Turn off the eye
						hMsg = g_pServerDE->StartMessage(m_hClient, SMSG_ALL_SEEING_EYE);
						g_pServerDE->WriteToMessageByte(hMsg, (DBYTE)nOff);
						g_pServerDE->WriteToMessageObject(hMsg, m_hObject);
    					g_pServerDE->EndMessage(hMsg);

						// Blind the Player 
						hMsg = g_pServerDE->StartMessage(m_hClient, SMSG_BLIND);
						g_pServerDE->WriteToMessageFloat(hMsg, (DFLOAT)10.0f);
						g_pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED);
					}
				}
				
				if( bClearInv )
					pOwner->m_InventoryMgr.RemoveItem(INV_THEEYE);
			}
		}

		m_hOwner = DNULL;
		m_hClient = DNULL;
	}

  	g_pServerDE->RemoveObject( m_hObject );
}
    
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SeeingEye::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SeeingEye::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveType)
{
	if (!hWrite) return;

	g_pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hOwner);
	g_pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hStuckObject);
	g_pServerDE->WriteToMessageByte(hWrite, m_bDropped);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SeeingEye::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SeeingEye::Load(HMESSAGEREAD hRead, DDWORD dwLoadType)
{
	if( !hRead ) return;

	g_pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hOwner);
	g_pServerDE->CreateInterObjectLink( m_hObject, m_hOwner );
	g_pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hStuckObject);
	if( m_hStuckObject )
		Attach( );
	m_bDropped = g_pServerDE->ReadFromMessageByte(hRead);
}
