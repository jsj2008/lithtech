
//----------------------------------------------------------------------------
//              
//	MODULE:		PowerArmor.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	13.03.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#include "PowerArmor.h"		
#include "ltserverobj.h"
#include "ObjectMsgs.h"
#include "Attachments.h"
#include "ModelButeMgr.h"
#include "CharacterHitBox.h"

// Forward declarations

// Globals

// Statics

LINKFROM_MODULE( PowerArmor );

BEGIN_CLASS(PowerArmor)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP(1), PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS(PowerArmor, Prop, NULL, NULL, CF_HIDDEN)



//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::PowerArmor()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
PowerArmor::PowerArmor()
{
	m_eState = kPA_TurningOff;
	m_bDisabled = LTFALSE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::~PowerArmor()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
PowerArmor::~PowerArmor()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::EngineMessageFn()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ uint32 PowerArmor::EngineMessageFn(uint32 messageID,void *pData,LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
            uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);
			
			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData);
		}
		break;

		case MID_INITIALUPDATE:
		{
			m_damage.SetCanDamage(LTFALSE);
			m_damage.m_bRemoveOnDeath = LTFALSE;
		}
		break;
	}
	uint32 iResult = Prop::EngineMessageFn(messageID, pData, fData);

	// UGLY!  There are 2 reasons for this code here:
	// 
	// 1:  The Destructable aggregate sets the touchnotify flag in the 
	// InitialUpdate.  As this is called in the aggregates INITIAL_UPDATE,
	// we need to kill this flag or the armor and wearer will collide
	// constantly and performance will die.
	//
	// 2. Props flag setting only ADDS flags, it doesn't remove them.  This
	// will insure that the flags are truely and totally gone.

	if ( messageID == MID_INITIALUPDATE )
	{
		if (fData != INITIALUPDATE_SAVEGAME)
		{
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_TOUCH_NOTIFY );
		}
	}

	return iResult;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::ObjectMessageFn()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ uint32 PowerArmor::ObjectMessageFn(HOBJECT hSender,ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			uint32 flags;
			g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, flags );
			UBER_ASSERT( !(flags & FLAG_TOUCH_NOTIFY), "Touch Notify Enabled!" );
			UBER_ASSERT( !(flags & FLAG_RAYHIT), "Ray Hit Enabled!" );

			const char* szMsg = (const char*)pMsg->Readuint32();

			if ( !strcmp(szMsg, KEY_TURNON) )
			{
				m_eState = kPA_TurningOn;
				SetNextUpdate(0.1f);
			}
			if ( !strcmp(szMsg, KEY_TURNOFF) )
			{
				m_eState = kPA_TurningOff;
				SetNextUpdate(0.1f);
			}
		}
		break;

		case MID_DAMAGE:
		{
			if ( m_bDisabled != LTTRUE )
			{
				m_bDisabled = LTTRUE;
				HandleTurningOff();
			}
		}
		break;

		default:
			break;
	}

	return Prop::ObjectMessageFn(hSender, pMsg);

}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::Update()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void PowerArmor::Update(void)
{
	SetNextUpdate(UPDATE_NEVER);

	uint32 flags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, flags );
	UBER_ASSERT( !(flags & FLAG_TOUCH_NOTIFY), "Touch Notify Enabled!" );
	UBER_ASSERT( !(flags & FLAG_RAYHIT), "Ray Hit Enabled!" );

	switch (m_eState)
	{
	case kPA_On:
		HandleOn();
		break;
	
	case kPA_Off:
		HandleOff();
		break;
	
	case kPA_TurningOn:
		HandleTurningOn();
		break;

	case kPA_TurningOff:
		HandleTurningOff();
		break;
	
	default:
		UBER_ASSERT( 0, "PowerArmor::Update: Unexpected State" );
	};
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::HandleOff()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void PowerArmor::HandleOff()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::HandleOn()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void PowerArmor::HandleOn()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::HandleTurningOn()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void PowerArmor::HandleTurningOn()
{
	if ( m_bDisabled == LTTRUE )
	{
		m_eState = kPA_Off;
		return;
	}

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
	m_eState = kPA_On;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::HandleTurningOff()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void PowerArmor::HandleTurningOff()
{
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_VISIBLE );
	m_eState = kPA_Off;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::Save()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void PowerArmor::Save(ILTMessage_Write *pMsg)
{
	UBER_ASSERT( pMsg, "PowerArmor::Save: NULL pMsg" );
	SAVE_DWORD(m_eState);
	SAVE_BOOL(m_bDisabled);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::Load()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void PowerArmor::Load(ILTMessage_Read *pMsg)
{
	UBER_ASSERT( pMsg, "PowerArmor::Load: NULL pMsg" );
	LOAD_DWORD_CAST(m_eState, ePA_State);
	LOAD_BOOL(m_bDisabled);
}


BEGIN_CLASS(ShellArmor)
END_CLASS_DEFAULT_FLAGS(ShellArmor, Prop, NULL, NULL, CF_HIDDEN)



//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::ShellArmor()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
ShellArmor::ShellArmor()
{
	// Construct
	m_hHitBox = NULL;
	m_hParentObject = NULL;
	m_hCurrentAnimation = INVALID_MODEL_ANIM;
	m_eModelId = eModelIdInvalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::~ShellArmor()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ ShellArmor::~ShellArmor()
{
	// Destruct

	if (m_hHitBox)
	{
        g_pLTServer->RemoveObject(m_hHitBox);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::EngineMessageFn()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ uint32 ShellArmor::EngineMessageFn(uint32 messageID,void *pData, LTFLOAT lData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (lData == PRECREATE_WORLDFILE || lData == PRECREATE_STRINGPROP)
			{
				ReadShieldProps(static_cast<ObjectCreateStruct*>(pData));
			}
		}
		break;

		case MID_OBJECTCREATED:
		{
			if (lData != OBJECTCREATED_SAVEGAME  )
			{
				CreateHitBox();
			}

			// The Armor is NOT deleted when is life is expired.  We want to
			// handle that manually on our level.  Attachments which delete
			// themselves don't clean up references properly.
			m_damage.m_bRemoveOnDeath = LTFALSE;
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save(static_cast<ILTMessage_Write*>(pData));
		}
		break;

		case MID_LOADOBJECT:
		{
            Load(static_cast<ILTMessage_Read*>(pData));
		}
		break;

		case MID_MODELSTRINGKEY:
		{
			HandleModelString( static_cast<ArgList*>(pData) );
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;
	}

	return ( Prop::EngineMessageFn(messageID, pData, lData) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::ObjectMessageFn()
//              
//	PURPOSE:	handles the damage itself -- does 
//              
//----------------------------------------------------------------------------
/*virtual*/ uint32 ShellArmor::ObjectMessageFn(HOBJECT hSender,ILTMessage_Read *pMsg)
{
	static const char cKeyDerezz[] = "REMOVE";

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch ( messageID )
	{
		case MID_TRIGGER:
		{
			// If this is an attach message, then the sender is the owner.
			const char* szMsg = reinterpret_cast<const char*>(pMsg->Readuint32());
			if ( 0 == strcmp( KEY_ATTACH, szMsg ) )
			{
				m_hParentObject = hSender;
			}

			if ( 0 == strcmp( cKeyDerezz, szMsg ) )
			{
				HandleDeath();
			}
		}
		break;

		case MID_DAMAGE:
		{
			// Intentionally NEVER pass MID_Damage to the base classes.  Prop
			// currently automaticly deletes the instance if passed down 
			// through the HandleDestroy() function.
			if ( CanDamage() )
			{
				GameBase::ObjectMessageFn(hSender, pMsg);

				if ( m_damage.IsDead() )
				{
					HandleDeath();
				}
			}
			return ( 0 );
		}
		break;

		// If we are told to play an animation, then just start playing it.  
		// This object is going to function pretty much entirely on KeyStrings
		case MID_ATTACHMENTANIM:
		{
			char szAnimationName[128];
			pMsg->ReadHStringAsString( szAnimationName, sizeof(szAnimationName));	

			SetAnimation(szAnimationName);
		}
		break;
	}

	return ( Prop::ObjectMessageFn(hSender, pMsg) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::Save()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void ShellArmor::Save(ILTMessage_Write *pMsg)
{
	// 
	SAVE_HOBJECT(m_hHitBox);
	SAVE_HOBJECT(m_hParentObject);

	SAVE_DWORD(m_hCurrentAnimation);
	SAVE_BYTE(m_eShieldState);
	SAVE_BYTE(m_eModelId);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::Load()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void ShellArmor::Load(ILTMessage_Read *pMsg)
{
	// 
	LOAD_HOBJECT(m_hHitBox);
	LOAD_HOBJECT(m_hParentObject);

	LOAD_DWORD(m_hCurrentAnimation);
	LOAD_BYTE_CAST(m_eShieldState, eShieldState);
	LOAD_BYTE_CAST(m_eModelId, ModelId);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::ReadShieldProps()
//              
//	PURPOSE:	Read the additional fields the ShellArmor cares about.
//              
//----------------------------------------------------------------------------
void ShellArmor::ReadShieldProps(ObjectCreateStruct* pData)
{
	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("ModelId", &genProp) == LT_OK)
	{
		AIASSERT( genProp.m_String[0], m_hObject, "ShellArmor requires a ModelId to set a skeleton"  );
		if(genProp.m_String[0])
		{
			m_eModelId = g_pModelButeMgr->GetModelId(genProp.m_String);
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::GetAttachments()
//              
//	PURPOSE:	ShellArmor does not have attachments, so return NULL when
//				queried for them.
//              
//----------------------------------------------------------------------------
/*virtual*/ CAttachments* ShellArmor::GetAttachments()
{
	return ( NULL );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::GetModelSkeleton()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ ModelSkeleton ShellArmor::GetModelSkeleton() const
{
	return ( g_pModelButeMgr->GetModelSkeleton(m_eModelId) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::SetModelNodeLastHit()
//              
//	PURPOSE:	Ignore the the LastNodeHit currently(?)
//              
//----------------------------------------------------------------------------
/*virtual*/ void ShellArmor::SetModelNodeLastHit(ModelNode eModelNode)
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::UsingHitDetection()
//              
//	PURPOSE:	This object ALWAYS uses HitDetection.	
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL ShellArmor::UsingHitDetection() const
{
	return ( LTTRUE );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::GetParentTransform()
//              
//	PURPOSE:	If now parent was specified, return a cleared LTransform.  If 
//				a parent was specified, then return the 'correct' transform
//              
//----------------------------------------------------------------------------
/*virtual*/ HATTACHMENT ShellArmor::GetAttachment( ) const
{
	HATTACHMENT hAttachment;
	LTRESULT  ltResult = g_pLTServer->FindAttachment( m_hParentObject, m_hObject, &hAttachment );
	UBER_ASSERT( ltResult==LT_OK, "Failed to find attachment");
	return ( hAttachment );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::ComputeDamageModifier()
//              
//	PURPOSE:	Returns the modifier for the node hit.
//              
//----------------------------------------------------------------------------
/*virtual*/ LTFLOAT ShellArmor::ComputeDamageModifier(ModelNode eModelNode)
{
	// 
	return ( 1.0f ); 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::GetNodeRadius()
//              
//	PURPOSE:	Hack the node radii depending on the state of the AI.  When
//				the armor is up, the root node needs to have a huge radius, 
//				and the other nodes need to have none(?)
//              
//----------------------------------------------------------------------------
/*virtual*/ float ShellArmor::GetNodeRadius(ModelSkeleton eModelSkeleton,ModelNode eModelNode)
{
	return ( g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eModelNode) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::Update()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void ShellArmor::Update()
{
	CCharacterHitBox* pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(m_hHitBox);
	UBER_ASSERT( pHitBox, "No m_hHitBox found" );

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->SetObjectPos(m_hHitBox, &vPos);

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);
	g_pLTServer->SetObjectRotation(m_hHitBox, &rRot);

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);
	g_pPhysicsLT->SetObjectDims(m_hHitBox, &vDims, 0);

	pHitBox->Update();

	uint32 dwFlags;
	LTRESULT ltResult = g_pModelLT->GetPlaybackState(m_hObject, MAIN_TRACKER, dwFlags );
	AIASSERT( ltResult == LT_OK, m_hObject, "Failed GetPlaybackState");

	SetNextUpdate(0.1f);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::CreateHitBox()
//              
//	PURPOSE:	Created a new hitbox for this object
//              
//----------------------------------------------------------------------------
void ShellArmor::CreateHitBox()
{
	UBER_ASSERT(m_hHitBox==NULL, "Expected HitBox to be NULL" );

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	HCLASS hClass = g_pLTServer->GetClass("CCharacterHitBox");
	UBER_ASSERT(hClass!=NULL, "GetClass CCharacterHitBox failed" );
	if (!hClass)
	{
		return;
	}

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
	theStruct.m_Pos = vPos;
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	// Allocate an object...

	CCharacterHitBox* pHitBox = (CCharacterHitBox *)g_pLTServer->CreateObject(hClass, &theStruct);
	UBER_ASSERT(pHitBox!=NULL, "CCharacterHitBox allocation failed" );
	if (!pHitBox)
	{
		return;
	}

	m_hHitBox = pHitBox->m_hObject;
	pHitBox->Init(m_hObject, this);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::HandleDeath()
//              
//	PURPOSE:	Handles all of the trigger message dispatching which goes off
//				when the ShellArmor dies.
//              
//----------------------------------------------------------------------------
void ShellArmor::HandleDeath()
{
	// Do a pretty effect, and become invisible and unshootable
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_TOUCH_NOTIFY | FLAG_SOLID | FLAG_RAYHIT | FLAG_VISIBLE);
	m_damage.SetCanDamage(LTFALSE);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::HandleModelString()
//              
//	PURPOSE:	Handles strings passed in through the animation.
//              
//----------------------------------------------------------------------------
void ShellArmor::HandleModelString(ArgList* pArgList)
{
	AIASSERT( pArgList, m_hObject, ""  );
	AIASSERT( pArgList->argc > 0, m_hObject, ""  );
	AIASSERT( pArgList->argv[0], m_hObject, ""  );

	const char* const szKey = pArgList->argv[0];

	if ( 0 == strcmp( szKey, "STATE" ) )
	{
		AIASSERT( pArgList->argv[1], m_hObject, ""  );
		const char* const szState = pArgList->argv[1];
		
		if ( 0 == strcmp( szState, "ATTACK" ) )
		{
			SetState( eShieldAttack );
		}
		else if ( 0 == strcmp( szState, "BLOCK" ) )
		{
			SetState( eShieldBlock );
		}
		else if ( 0 == strcmp( szState, "NORMAL" ) )
		{
			SetState( eShieldNormal );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::SetState()
//              
//	PURPOSE:	Unified location where the armors state can be changed for 
//				making debugging a bit easier.
//              
//----------------------------------------------------------------------------
void ShellArmor::SetState( eShieldState eNewState )
{
	m_eShieldState = eNewState;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::SetAnimation()
//              
//	PURPOSE:	Sets an animation (or resets the animation) based on the name 
//				of an animation string passed in.  Keeps a reference to the 
//				selected to play and clears the state.  If the passed in 
//				animation is also the current animation, then reset it.
//
//----------------------------------------------------------------------------
void ShellArmor::SetAnimation(const char* const szAnimationName)
{
	LTRESULT ltResult;

	HMODELANIM hAnim;
	ltResult = g_pModelLT->GetAnimIndex( m_hObject, const_cast<char*>(szAnimationName), hAnim );
	AIASSERT1( ltResult==LT_OK, m_hObject, "Failed to find animation %s", szAnimationName );

	if ( m_hCurrentAnimation == hAnim )
	{
		g_pModelLT->ResetAnim( m_hObject, MAIN_TRACKER );
		AIASSERT( ltResult==LT_OK, m_hObject, "Failed to Reset the animation" );
	}
	else
	{
		g_pModelLT->SetCurAnim( m_hObject, MAIN_TRACKER, hAnim );
		AIASSERT( ltResult==LT_OK, m_hObject, "Failed to set the animation" );
	}

	g_pModelLT->SetLooping( m_hObject, MAIN_TRACKER, false );

	// Remember the animation currently playing.
	m_hCurrentAnimation = hAnim;

	// Clear the state, just in case we missed any clean up keys.
	SetState( eShieldNormal );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::CanDamage()
//              
//	PURPOSE:	Returns true if the armor can currently be damaged, false if 
//				it cannot.
//              
//----------------------------------------------------------------------------
LTBOOL ShellArmor::CanDamage() const
{
	if ( m_eShieldState != eShieldAttack )
	{
		// If we are not in the attack state, then we are undamageable.
		return ( LTFALSE );
	}

	if ( m_damage.IsDead() )
	{
		// If we are dead, we can't be more damaged
		return ( LTFALSE );
	}

	return ( LTTRUE );
}