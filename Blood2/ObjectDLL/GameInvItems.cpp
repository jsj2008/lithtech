//----------------------------------------------------------
//
// MODULE  : GameInvItems.cpp
//
// PURPOSE : Inventory items
//
// CREATED : 2/25/98
//
//----------------------------------------------------------

// Includes....
#include <stdlib.h>
#include "GameInvItems.h"
#include "PlayerObj.h"
#include "SeeingEye.h"
#include "LightFX.h"
#include "Gib.h"
#include "SfxMsgIDs.h"
#include "ClientServerShared.h"
#include "ClientRes.h"


// *********************************************************************** //
//
//	CLASS:		CInvFlashlight
//
//	PURPOSE:	Flashlight inventory item
//
// *********************************************************************** //


CInvFlashlight::CInvFlashlight() : CInvItem(INV_FLASHLIGHT)
{
	m_hLight = DNULL;
	m_fDischargeTime = FLASHLIGHTCHARGETIME;
	m_nCount = FLASHLIGHTCHARGE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvFlashlight::Term()
//
//	PURPOSE:	Terminates the flashlight
//
// ----------------------------------------------------------------------- //

void CInvFlashlight::Init(HOBJECT hOwner)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	CInvItem::Init(hOwner);

	m_hstrItemName = pServerDE->FormatString(IDS_ITEM_FLASHLIGHT);
	m_hstrDisplayName = pServerDE->CopyString( m_hstrItemName );

	m_hstrPic = pServerDE->CreateString("flashlight.pcx");
	m_hstrPicH = pServerDE->CreateString("flashlight_h.pcx");

	m_nCount = FLASHLIGHTCHARGE; 
	m_fDischargeTime = FLASHLIGHTCHARGETIME;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvFlashlight::CreateLight()
//
//	PURPOSE:	Creates the light object for the special effect
//
// ----------------------------------------------------------------------- //

DBOOL CInvFlashlight::CreateLight()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	if( m_hLight )
	{
		pServerDE->RemoveObject( m_hLight );
		m_hLight = DNULL;
	}

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ocStruct.m_Flags = FLAG_FORCECLIENTUPDATE;
	ocStruct.m_ObjectType = OT_NORMAL; 

	HCLASS hClass = pServerDE->GetClass("BaseClass");
	LPBASECLASS	pLight = pServerDE->CreateObject(hClass, &ocStruct);

	if (pLight)
	{
		m_hLight = pLight->m_hObject;

		// Send special effect message
		HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(pLight);
		pServerDE->WriteToMessageByte(hMessage, SFX_FLASHLIGHT_ID);
		pServerDE->EndMessage(hMessage);
		return DTRUE;
	}
	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvFlashlight::AddItem()
//
//	PURPOSE:	Try to recharge the flashlight
//
// ----------------------------------------------------------------------- //

DBOOL CInvFlashlight::AddItem(DBYTE nCount)
{
	DBOOL bRet = DFALSE;
	if (m_nCount < FLASHLIGHTCHARGE)
	{
		m_nCount = FLASHLIGHTCHARGE;
		m_fDischargeTime = FLASHLIGHTCHARGETIME;
		bRet = DTRUE;
	}
	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvFlashlight::Activate()
//
//	PURPOSE:	Activates the item
//
// ----------------------------------------------------------------------- //

int CInvFlashlight::Activate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	if (!m_hLight)
	{
		if (!CreateLight())
			return DFALSE;
	}

	if( m_fDischargeTime <= 0.0f )
		return DFALSE;
	
	DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hLight);
	dwUsrFlags |= USRFLG_VISIBLE;
	pServerDE->SetObjectUserFlags(m_hLight, dwUsrFlags);

	m_bIsActive = DTRUE;
	SendActionMessage( );		
	return m_bIsActive;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvFlashlight::Deactivate()
//
//	PURPOSE:	Deactivates the item
//
// ----------------------------------------------------------------------- //

int CInvFlashlight::Deactivate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hLight) return DFALSE;
	
	DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hLight);
	dwUsrFlags &= ~USRFLG_VISIBLE;
	pServerDE->SetObjectUserFlags(m_hLight, dwUsrFlags);

	m_bIsActive = DFALSE;
	SendActionMessage( );		
	return m_bIsActive;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvFlashlight::Update()
//
//	PURPOSE:	Updates this item's position
//
// ----------------------------------------------------------------------- //

void CInvFlashlight::Update()
{
	DFLOAT fFrameTime;
	CPlayerObj* pPlayer;
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	CInvItem::Update();

	if( IsPlayer( m_hOwner ))
		pPlayer = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
	else
		pPlayer = NULL;

	if (m_hLight)
	{
		// Move the item's position to the same place as it's owner
		DVector vPos;
		pServerDE->GetObjectPos(m_hOwner, &vPos);
		pServerDE->TeleportObject(m_hLight, &vPos);
		DRotation rRot;
		if( pPlayer )
		{
			// Set the rotation to the player's internal rotation
			pPlayer->GetPlayerRotation(&rRot);
		}
		else
		{
			pServerDE->GetObjectRotation(m_hOwner, &rRot);
		}
		pServerDE->SetObjectRotation(m_hLight, &rRot);

	}

	if( m_bIsActive )
	{
		fFrameTime = pServerDE->GetFrameTime( );
		m_fDischargeTime -= fFrameTime;
		if( m_fDischargeTime <= 0.0f )
		{
			m_nCount = 0;
			if( pPlayer )
				pPlayer->m_InventoryMgr.SetActiveItem(INV_FLASHLIGHT);
			else
				Deactivate( );
		}
		else
		{
			m_nCount = ( DBYTE )(( FLASHLIGHTCHARGE * m_fDischargeTime / FLASHLIGHTCHARGETIME ) + 0.5f );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvFlashlight::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CInvFlashlight::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	CInvItem::Save(hWrite, dwSaveFlags);

	pServerDE->WriteToMessageFloat(hWrite, m_fDischargeTime);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvFlashlight::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CInvFlashlight::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	CInvItem::Load(hRead, dwLoadFlags);

	m_fDischargeTime = pServerDE->ReadFromMessageFloat(hRead);

	CreateLight();

	// deactivate now if keepalive restore
	DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hLight);
	dwUsrFlags &= ~USRFLG_VISIBLE;
}


// *********************************************************************** //
//
//	CLASS:		CInvMedkit
//
//	PURPOSE:	Medkit inventory item
//
// *********************************************************************** //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvMedkit::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

void CInvMedkit::Init(HOBJECT hOwner)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	CInvItem::Init(hOwner);

	m_nCount = 100; 
	m_hstrItemName = pServerDE->FormatString(IDS_ITEM_MEDKIT);
	m_hstrDisplayName = pServerDE->CopyString( m_hstrItemName );
	m_hstrPic = pServerDE->CreateString("medkit.pcx");
	m_hstrPicH = pServerDE->CreateString("medkit_h.pcx");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvMedkit::AddItem()
//
//	PURPOSE:	Try to recharge the medkit
//
// ----------------------------------------------------------------------- //

DBOOL CInvMedkit::AddItem(DBYTE nCount)
{
	DBOOL bRet = DFALSE;
	if (m_nCount < 100)
	{
		m_nCount = 100;
		bRet = DTRUE;
	}
	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvMedkit::Activate()
//
//	PURPOSE:	Activates the item
//
// ----------------------------------------------------------------------- //

int CInvMedkit::Activate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return 0;

	// Make sure it's a character
	HCLASS hOwnerClass = pServerDE->GetObjectClass(m_hOwner);
	if(pServerDE->IsKindOf(hOwnerClass, pServerDE->GetClass("CBaseCharacter"))) 
	{
		CBaseCharacter *pObj = (CBaseCharacter*)pServerDE->HandleToObject(m_hOwner);
		CDestructable *pDest = pObj->GetDestructable();

		DFLOAT fHitPoints = pDest->GetHitPoints();
		DFLOAT fHealthGain = __min( 100.0f - fHitPoints, (DFLOAT)m_nCount);

		if( fHealthGain > 0.0f )
		{
			pDest->Heal(fHealthGain);

			// If this is a player, then play the use sound with CLIENTLOCAL flag...
			if( IsPlayer( m_hOwner ))
			{
				SendActionMessage( );		
			}
			
			m_nCount -= (int)fHealthGain;
			
		}
		return (int)fHealthGain;
	}
	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvMedkit::ActivateItem()
//
//	PURPOSE:	default activate item behavior
//
// ----------------------------------------------------------------------- //

DBOOL CInvMedkit::ActivateItem(char *msgbuf)
{
	if ( !Activate( ))
		return DFALSE;

	_mbscpy((unsigned char*)msgbuf, (const unsigned char*)GetDisplayName());
	return DTRUE;
}



// *********************************************************************** //
//
//	CLASS:		CInvNightGoggles
//
//	PURPOSE:	Night Goggles inventory item
//
// *********************************************************************** //


CInvNightGoggles::CInvNightGoggles() : CInvItem(INV_NIGHTGOGGLES)
{
	m_fDischargeTime = GOGGLESCHARGETIME;
	m_nCount = GOGGLESCHARGE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvNightGoggles::Activate()
//
//	PURPOSE:	Activates the item
//
// ----------------------------------------------------------------------- //

void CInvNightGoggles::Init(HOBJECT hOwner)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	CInvItem::Init(hOwner);

	m_hstrItemName = pServerDE->FormatString(IDS_ITEM_GOGGLES);
	m_hstrDisplayName = pServerDE->CopyString( m_hstrItemName );

	m_hstrPic = pServerDE->CreateString("night_vision.pcx");
	m_hstrPicH = pServerDE->CreateString("night_vision_h.pcx");

	m_nCount = GOGGLESCHARGE; 
	m_fDischargeTime = GOGGLESCHARGETIME;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvFlashlight::AddItem()
//
//	PURPOSE:	Try to recharge the flashlight
//
// ----------------------------------------------------------------------- //

DBOOL CInvNightGoggles::AddItem(DBYTE nCount)
{
	DBOOL bRet = DFALSE;
	if (m_nCount < GOGGLESCHARGE)
	{
		m_nCount = GOGGLESCHARGE;
		m_fDischargeTime = GOGGLESCHARGETIME;
		bRet = DTRUE;
	}
	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvNightGoggles::Activate()
//
//	PURPOSE:	Activates the item
//
// ----------------------------------------------------------------------- //

int CInvNightGoggles::Activate()
{
	if( m_fDischargeTime <= 0.0f )
		return DFALSE;
	
	m_bIsActive = DTRUE;
	SendActionMessage( );		
	return m_bIsActive;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvNightGoggles::Deactivate()
//
//	PURPOSE:	Deactivates the item
//
// ----------------------------------------------------------------------- //

int CInvNightGoggles::Deactivate()
{
	m_bIsActive = DFALSE;
	SendActionMessage( );		
	return m_bIsActive;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvNightGoggles::Update()
//
//	PURPOSE:	Updates this item's position
//
// ----------------------------------------------------------------------- //

void CInvNightGoggles::Update()
{
	DFLOAT fFrameTime;
	CPlayerObj* pPlayer;
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	CInvItem::Update();

	if( IsPlayer( m_hOwner ))
		pPlayer = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
	else
		pPlayer = NULL;

	if( m_bIsActive )
	{
		fFrameTime = pServerDE->GetFrameTime( );
		m_fDischargeTime -= fFrameTime;
		if( m_fDischargeTime <= 0.0f )
		{
			m_nCount = 0;
			if( pPlayer )
				pPlayer->m_InventoryMgr.SetActiveItem(INV_NIGHTGOGGLES);
			else
				Deactivate( );
		}
		else
		{
			m_nCount = ( DBYTE )(( GOGGLESCHARGE * m_fDischargeTime / GOGGLESCHARGETIME ) + 0.5f );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvNightGoggles::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CInvNightGoggles::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	CInvItem::Save(hWrite, dwSaveFlags);

	pServerDE->WriteToMessageFloat(hWrite, m_fDischargeTime);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvNightGoggles::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CInvNightGoggles::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	CInvItem::Load(hRead, dwLoadFlags);

	m_fDischargeTime = pServerDE->ReadFromMessageFloat(hRead);
}


// *********************************************************************** //
//
//	CLASS:		CInvBinoculars
//
//	PURPOSE:	Binoculars inventory item
//
// *********************************************************************** //


void CInvBinoculars::Init(HOBJECT hOwner)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	CInvItem::Init(hOwner);

	m_hstrItemName = pServerDE->FormatString(IDS_ITEM_BINOCULARS);
	m_hstrDisplayName = pServerDE->CopyString( m_hstrItemName );

	m_hstrPic = pServerDE->CreateString("binoculars.pcx");
	m_hstrPicH = pServerDE->CreateString("binoculars_h.pcx");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvBinoculars::Activate()
//
//	PURPOSE:	Activates the item
//
// ----------------------------------------------------------------------- //

int CInvBinoculars::Activate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	// Make sure it's a player
	if( IsPlayer( m_hOwner ))
	{
		CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
		
		// If already in zoom mode because of something else (sniper), then ignore this...
		if( pObj->GetZoomMode( ))
			return DFALSE;

		m_bIsActive = DTRUE;

		SendActionMessage( );		

		pObj->SetZoomMode(DTRUE, 98); // Number is the cursor type (AM)
	}
	else
	{
		m_bIsActive = DTRUE;
	}

	return m_bIsActive;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvBinoculars::Deactivate()
//
//	PURPOSE:	Deactivates the item
//
// ----------------------------------------------------------------------- //

int CInvBinoculars::Deactivate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	// Make sure it's a player
	if( IsPlayer( m_hOwner )) 
	{
		CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
		m_bIsActive = DFALSE;
		SendActionMessage( );		
		pObj->SetZoomMode(DFALSE);
	}
	else
		m_bIsActive = DFALSE;

	return m_bIsActive;
}


// *********************************************************************** //
//
//	CLASS:		CInvTheEye
//
//	PURPOSE:	TheEye inventory item
//
// *********************************************************************** //


CInvTheEye::CInvTheEye() : CInvItem(INV_THEEYE)
{
    m_hSeeingObject = DNULL;
}


void CInvTheEye::Init(HOBJECT hOwner)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	CInvItem::Init(hOwner);

	m_hstrItemName = pServerDE->FormatString(IDS_ITEM_EYE);
	m_hstrDisplayName = pServerDE->CopyString( m_hstrItemName );
    m_hSeeingObject = NULL;
    m_bDropped = DFALSE;

	m_hstrPic = pServerDE->CreateString("the_eye.pcx");
	m_hstrPicH = pServerDE->CreateString("the_eye_h.pcx");
}


void CInvTheEye::Term()
{
	CInvItem::Term();

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (m_hSeeingObject)
	{
		pServerDE->RemoveObject(m_hSeeingObject);
		m_hSeeingObject = DNULL;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvTheEye::SetRotation()
//
//	PURPOSE:	Sets the rotation for the eye item
//
// ----------------------------------------------------------------------- //

void CInvTheEye::SetRotation(DRotation *pRot)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hSeeingObject) return;

	pServerDE->SetObjectRotation(m_hSeeingObject, pRot);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvTheEye::Dropit()
//
//	PURPOSE:	Drops the eye
//
// ----------------------------------------------------------------------- //

int CInvTheEye::Dropit()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

    if (!m_hSeeingObject)
    {
        ObjectCreateStruct theStruct;
	    INIT_OBJECTCREATESTRUCT(theStruct);

	    DRotation rRot;
        ROT_INIT(rRot);
        
    	// Make sure it's a player
	    if(IsPlayer(m_hOwner)) 
    	{
    		CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
            pObj->GetPlayerRotation(&rRot);

	        DVector vPos, vDims, vF, vU, vR;
            pServerDE->GetObjectPos(m_hOwner, &vPos);
        	pServerDE->GetObjectDims(m_hOwner, &vDims);
			pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

			VEC_MULSCALAR(vF, vF, 300.0f);

			// Position the Eye at Eye Level...
        	vPos.y += vDims.y / 2.0f;
	        vPos.y -= 1;
         	
            ROT_COPY(theStruct.m_Rotation, rRot);
	        VEC_COPY(theStruct.m_Pos, vPos);

	        theStruct.m_NextUpdate = 0.001f;
    
    	    HCLASS pClass = pServerDE->GetClass("SeeingEye");
    
            // Check to make sure this is a valid class.
            if (pClass)
            {    
            	LPBASECLASS pObject = pServerDE->CreateObject(pClass, &theStruct);
      
                if (pObject)
                {
                    m_hSeeingObject = pServerDE->ObjectToHandle(pObject);
                    
					pServerDE->SetVelocity(m_hSeeingObject, &vF);

                    SeeingEye *pEye = (SeeingEye*)pObject;
                    
                    // Initialize the eye with the Owner and Client
        		    pEye->Init(m_hOwner, pObj->GetClient() );
                
    				m_bDropped = DTRUE;
                    return DTRUE;
                }
            }     
    	}
        
    }
    
	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvTheEye::Activate()
//
//	PURPOSE:	Activates the item
//
// ----------------------------------------------------------------------- //

int CInvTheEye::Activate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

    int nOn = 1;
  
	if(m_hSeeingObject && pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hOwner), pServerDE->GetClass("CPlayerObj"))) 
	{
		CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
        
		HMESSAGEWRITE hMsg = pServerDE->StartMessage(pObj->GetClient(), SMSG_ALL_SEEING_EYE);
		pServerDE->WriteToMessageByte(hMsg, (DBYTE)nOn);
		pServerDE->WriteToMessageObject(hMsg, m_hSeeingObject);
		pServerDE->EndMessage(hMsg);
    }            

    m_bIsActive = DTRUE;
	return m_bIsActive;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvTheEye::Deactivate()
//
//	PURPOSE:	Deactivates the item
//
// ----------------------------------------------------------------------- //

int CInvTheEye::Deactivate()
{
   	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

    m_bIsActive = DFALSE;
    
    int nOff = 0;
  
	if(m_hSeeingObject && pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hOwner), pServerDE->GetClass("CPlayerObj"))) 
    {
		CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
        
        HMESSAGEWRITE hMsg = pServerDE->StartMessage(pObj->GetClient(), SMSG_ALL_SEEING_EYE);
        pServerDE->WriteToMessageByte(hMsg, (DBYTE)nOff);
        pServerDE->WriteToMessageObject(hMsg, m_hSeeingObject);
        pServerDE->EndMessage(hMsg);
    }        
    
	return m_bIsActive;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvTheEye::ActivateItem()
//
//	PURPOSE:	activates or deactivates the eye
//
// ----------------------------------------------------------------------- //
DBOOL CInvTheEye::ActivateItem(char *msgbuf)
{
    if ( IsDropped() )
    {
		if (!IsActive())
		{
			if( !Activate())
				return DFALSE;

    		_mbscat((unsigned char*)msgbuf, (const unsigned char*)" on");
	    }
		else
		{
			Deactivate();
			_mbscat((unsigned char*)msgbuf, (const unsigned char*)" off");
		}
    }
    else
    {
		Dropit();
        
    	if ( IsDropped() )
	    	_mbscpy((unsigned char*)msgbuf, (const unsigned char*)GetDisplayName());
    }                        

	_mbscpy((unsigned char*)msgbuf, (const unsigned char*)GetDisplayName());
	return DTRUE;   
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvTheEye::PickItUp()
//
//	PURPOSE:	Pick up the eye
//
// ----------------------------------------------------------------------- //

void CInvTheEye::PickItUp()
{
   	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_bDropped = DFALSE;
    m_bIsActive = DFALSE;
    
    m_hSeeingObject = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvTheEye::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CInvTheEye::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	CInvItem::Save(hWrite, dwSaveFlags);

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hSeeingObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvTheEye::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CInvTheEye::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	CInvItem::Load(hRead, dwLoadFlags);

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hSeeingObject);

	if( m_hSeeingObject )
	{
        SeeingEye *pEye = ( SeeingEye * )pServerDE->HandleToObject( m_hSeeingObject );
    	CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
        
        // Initialize the eye with the Owner and Client
        pEye->Init(m_hOwner, pObj->GetClient() );
	}
	else
	{
		m_bDropped = DFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvProxBomb::Init()
//
//	PURPOSE:	Initializes the bomb.
//
// ----------------------------------------------------------------------- //

void CInvProxBomb::Init(HOBJECT hOwner)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	CInvItem::Init(hOwner);

	m_hstrItemName = pServerDE->FormatString(IDS_ITEM_PROX);
	m_hstrDisplayName = pServerDE->CopyString( m_hstrItemName );

	m_hstrPic = pServerDE->CreateString("p_bombs.pcx");
	m_hstrPicH = pServerDE->CreateString("p_bombs_h.pcx");
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvProxBomb::ActivateItem()
//
//	PURPOSE:	default activate item behavior
//
// ----------------------------------------------------------------------- //

DBOOL CInvProxBomb::ActivateItem(char *msgbuf)
{
	if ( !Activate( ))
		return DFALSE;

	_mbscpy((unsigned char*)msgbuf, (const unsigned char*)GetDisplayName());
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvProxBomb::Activate()
//
//	PURPOSE:	Activates the item
//
// ----------------------------------------------------------------------- //

int CInvProxBomb::Activate()
{
	CPlayerObj *pPlayer;

	if( IsPlayer( m_hOwner ))
	{
		pPlayer = (CPlayerObj*)g_pServerDE->HandleToObject(m_hOwner);
		pPlayer->m_InventoryMgr.SelectInventoryWeapon( WEAP_PROXIMITYBOMB );
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvRemoteBomb::Init()
//
//	PURPOSE:	Initializes the bomb.
//
// ----------------------------------------------------------------------- //

void CInvRemoteBomb::Init(HOBJECT hOwner)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	CInvItem::Init(hOwner);

	m_hstrItemName = pServerDE->FormatString(IDS_ITEM_REMOTE);
	m_hstrDisplayName = pServerDE->CopyString( m_hstrItemName );

	m_hstrPic = pServerDE->CreateString("r_bombs.pcx");
	m_hstrPicH = pServerDE->CreateString("r_bombs_h.pcx");
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvRemoteBomb::ActivateItem()
//
//	PURPOSE:	default activate item behavior
//
// ----------------------------------------------------------------------- //

DBOOL CInvRemoteBomb::ActivateItem(char *msgbuf)
{
	if ( !Activate( ))
		return DFALSE;

	_mbscpy((unsigned char*)msgbuf, (const unsigned char*)GetDisplayName());
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvRemoteBomb::Activate()
//
//	PURPOSE:	Activates the item
//
// ----------------------------------------------------------------------- //

int CInvRemoteBomb::Activate()
{
	CPlayerObj *pPlayer;

	if( IsPlayer( m_hOwner ))
	{
		pPlayer = (CPlayerObj*)g_pServerDE->HandleToObject(m_hOwner);
		pPlayer->m_InventoryMgr.SelectInventoryWeapon( WEAP_REMOTEBOMB );
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvTimeBomb::Init()
//
//	PURPOSE:	Initializes the bomb.
//
// ----------------------------------------------------------------------- //

void CInvTimeBomb::Init(HOBJECT hOwner)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	CInvItem::Init(hOwner);

	m_hstrItemName = pServerDE->FormatString(IDS_ITEM_TIME);
	m_hstrDisplayName = pServerDE->CopyString( m_hstrItemName );

	m_hstrPic = pServerDE->CreateString("t_bombs.pcx");
	m_hstrPicH = pServerDE->CreateString("t_bombs_h.pcx");
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvTimeBomb::ActivateItem()
//
//	PURPOSE:	default activate item behavior
//
// ----------------------------------------------------------------------- //

DBOOL CInvTimeBomb::ActivateItem(char *msgbuf)
{
	if ( !Activate( ))
		return DFALSE;

	_mbscpy((unsigned char*)msgbuf, (const unsigned char*)GetDisplayName());
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvTimeBomb::Activate()
//
//	PURPOSE:	Activates the item
//
// ----------------------------------------------------------------------- //

int CInvTimeBomb::Activate()
{
	CPlayerObj *pPlayer;

	if( IsPlayer( m_hOwner ))
	{
		pPlayer = (CPlayerObj*)g_pServerDE->HandleToObject(m_hOwner);
		pPlayer->m_InventoryMgr.SelectInventoryWeapon( WEAP_TIMEBOMB );
	}

	return DTRUE;
}


