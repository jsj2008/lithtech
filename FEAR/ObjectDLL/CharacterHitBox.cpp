// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterHitBox.cpp
//
// PURPOSE : Character hit box object class implementation
//
// CREATED : 01/05/00
//
// (c) 2000-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "CharacterHitBox.h"
#include "Character.h"
#include "Projectile.h"
#include "ServerUtilities.h"
#include "VarTrack.h"
#include "MsgIDs.h"
#include "ObjectMsgs.h"
#include "Attachments.h"
#include "GameServerShell.h"
#include "resourceextensions.h"
#include "AIAssert.h"

LINKFROM_MODULE( CharacterHitBox );

static VarTrack g_vtShowNodeRadii;
static VarTrack g_vtShowPlayerNodeRadii;
static VarTrack g_vtNodeRadiusUseOverride;
static VarTrack g_vtHeadNodeRadius;
static VarTrack g_vtTorsoNodeRadius;
static VarTrack g_vtArmNodeRadius;
static VarTrack g_vtLegNodeRadius;
static VarTrack g_HitDebugTrack;

#define HB_DEFAULT_NODE_RADIUS	12.5f

#define	HB_DIMS_MIN_XZ	24.0f
#define HB_DIMS_MIN_Y	15.0f


BEGIN_CLASS(CCharacterHitBox)
END_CLASS_FLAGS(CCharacterHitBox, GameBase, CF_HIDDEN, "Defines the collision volume used for various collision tests")

CMDMGR_BEGIN_REGISTER_CLASS( CCharacterHitBox )
CMDMGR_END_REGISTER_CLASS( CCharacterHitBox, GameBase )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::CCharacterHitBox()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCharacterHitBox::CCharacterHitBox()
:	GameBase				( OT_NORMAL ),
	m_pHitBoxUser			( NULL ),
	m_hModel				( NULL ),
	m_vOffset				( 0.0f, 0.0f, 0.0f ),
	m_bCanActivate			( true ),
	m_bAnimControlsDims		( false ),
	m_bAnimControlsOffset	( false ),
	m_hControllingAnim		( INVALID_ANI ),
	m_bFollowVisNode		( false )
{
	// Setup the TransitionAggregate
	MakeTransitionable();
	
	m_NodeRadiusList.Init(true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::~CCharacterHitBox()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCharacterHitBox::~CCharacterHitBox()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Init()
//
//	PURPOSE:	Initialize object.  Takes as an argument the hObject of the 
//				object using the hitbox
//
// ----------------------------------------------------------------------- //

bool CCharacterHitBox::Init(HOBJECT hModel, IHitBoxUser* pUser)
{
	AIASSERT( pUser, m_hObject, "CharacterHitBox Inited with non existant IHitBoxUser" );
	AIASSERT( hModel, m_hObject, "CharacterHitBox Inited with non existant hObject" );
	AIASSERT( m_hObject, m_hObject, "CharacterHitBox does not exist" );
	AIASSERT( pUser, m_hObject, "" );
	AIASSERT( dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(hModel)) == pUser, hModel,  "Passed in hModel and pUser should reference the same class -- HandleToObject(hModel) should return what pUser points to" );

	m_hModel = hModel;
	m_pHitBoxUser = pUser;

	// Set my flags...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_RAYHIT | FLAG_TOUCHABLE, FLAGMASK_ALL);

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_HITBOX | USRFLG_CHARACTER, USRFLG_HITBOX | USRFLG_CHARACTER );

	// Set our user flags to USRFLG_CHARACTER, so the client will process
	// us like a character (for intersect segments)...
	
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CHARACTER, USRFLG_CHARACTER);

	if (!g_vtShowNodeRadii.IsInitted())
	{
        g_vtShowNodeRadii.Init(g_pLTServer, "HitBoxShowNodeRadii", NULL, 0.0f);
	}

	if (!g_vtNodeRadiusUseOverride.IsInitted())
	{
        g_vtNodeRadiusUseOverride.Init(g_pLTServer, "HitBoxNodeRadiusOverride", NULL, 0.0f);
	}

	if (!g_vtHeadNodeRadius.IsInitted())
	{
        g_vtHeadNodeRadius.Init(g_pLTServer, "HitBoxHeadNodeRadius", NULL, HB_DEFAULT_NODE_RADIUS);
	}

	if (!g_vtTorsoNodeRadius.IsInitted())
	{
        g_vtTorsoNodeRadius.Init(g_pLTServer, "HitBoxTorsoNodeRadius", NULL, HB_DEFAULT_NODE_RADIUS);
	}

	if (!g_vtArmNodeRadius.IsInitted())
	{
        g_vtArmNodeRadius.Init(g_pLTServer, "HitBoxArmNodeRadius", NULL, HB_DEFAULT_NODE_RADIUS);
	}

	if (!g_vtLegNodeRadius.IsInitted())
	{
        g_vtLegNodeRadius.Init(g_pLTServer, "HitBoxLegNodeRadius", NULL, HB_DEFAULT_NODE_RADIUS);
	}

	if (!g_HitDebugTrack.IsInitted())
	{
        g_HitDebugTrack.Init(g_pLTServer, "HitDebug", NULL, 0.0f);
	}

	if (!g_vtShowPlayerNodeRadii.IsInitted())
	{
		g_vtShowPlayerNodeRadii.Init(g_pLTServer, "HitBoxShowPlayerNodeRadii", NULL, 0.0f);
	}

	SetNextUpdate(UPDATE_NEVER);

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CCharacterHitBox::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pCS = ( ObjectCreateStruct* )pData;
			m_hModel = ( HOBJECT )pCS->m_UserData;
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

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CCharacterHitBox::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
    // Pass any messages to our model...

	if (m_hModel)
	{
		LPBASECLASS pBase = g_pLTServer->HandleToObject(m_hModel);
		if (pBase)
		{
			pBase->ObjectMessageFn(hSender, pMsg);
		}
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetNodeRadius()
//
//	PURPOSE:	Get the model node's radius, checking first to see if they
//				are being overridden.  If they are, then use the manually
//				set values.  If they are not, then use the ModelBute 
//				systems value for this node.
//
// ----------------------------------------------------------------------- //
float CCharacterHitBox::GetNodeRadius(ModelsDB::HNODE hModelNode) const
{
	// See if we're overriding the radius...

	if (g_vtNodeRadiusUseOverride.GetFloat())
	{
		switch ( g_pModelsDB->GetNodeLocation( hModelNode ))
		{
			case HL_HEAD :
				return ( g_vtHeadNodeRadius.GetFloat() );
			break;

			case HL_TORSO :
				return ( g_vtTorsoNodeRadius.GetFloat() );
			break;

			case HL_ARM_LEFT :
			case HL_ARM_RIGHT :
				return ( g_vtArmNodeRadius.GetFloat() );
			break;

			case HL_LEG_LEFT :
			case HL_LEG_RIGHT :
				return ( g_vtLegNodeRadius.GetFloat() );
			break;

			default :
				return ( HB_DEFAULT_NODE_RADIUS );
			break;
		}
	}
	else
	{
		return ( m_pHitBoxUser->GetNodeRadius( hModelNode ) );
	}
}

static LTVector GetNodeModelColor(ModelsDB::HNODE hModelNode)
{
	LTVector vColor(1, 1, 1);

	switch ( g_pModelsDB->GetNodeLocation( hModelNode ))
	{
		case HL_HEAD :
			vColor = LTVector(1, 0, 0);
		break;

		case HL_TORSO :
			vColor = LTVector(1, 1, 0);
		break;

		case HL_ARM_LEFT :
		case HL_ARM_RIGHT :
			vColor = LTVector(0, 1, 0);
		break;

		case HL_LEG_LEFT :
		case HL_LEG_RIGHT :
			vColor = LTVector(0, 0, 1);
		break;
	}

	return vColor;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::CreateNodeRadiusModels()
//
//	PURPOSE:	Create models representing the model node radii.  Iterates
//				over all of the nodes in a model, and if a node transform 
//				successfully can be retrieved, created the model and adds
//				it to the list.
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::CreateNodeRadiusModels()
{
	AIASSERT( m_pHitBoxUser, m_hObject, "Called with NULL HitBoxUser" );
	AIASSERT( m_hModel, m_hObject, "Called with NULL m_hModel" );
	AIASSERT( m_hObject, m_hObject, "Called with NULL m_hObject" );

	if (!m_hModel)
	{
		return;
	}

	ModelsDB::HSKELETON hModelSkeleton = m_pHitBoxUser->GetModelSkeleton();
	int cNodes = g_pModelsDB->GetSkeletonNumNodes(hModelSkeleton);

	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		ModelsDB::HNODE hCurNode = g_pModelsDB->GetSkeletonNode( hModelSkeleton, iNode );
		const char* szNodeName = g_pModelsDB->GetNodeName( hCurNode );

        float fNodeRadius = GetNodeRadius( hCurNode );
		if (fNodeRadius <= 0.0f)
		{
			continue;
		}

  		if (!szNodeName)
		{
			continue;
		}

        LTTransform transform;
		if ( GetNodeTransform(szNodeName, transform ) == false )
		{
			continue;
		}


		// Create the radius model...

		ObjectCreateStruct theStruct;

		theStruct.m_Pos = transform.m_vPos;
		theStruct.SetFileName("Models\\sphere." RESEXT_MODEL_PACKED);
		theStruct.SetMaterial( 0,"Materials\\Grid." RESEXT_MATERIAL );
		theStruct.m_Flags = FLAG_VISIBLE;
		theStruct.m_ObjectType = OT_MODEL;
		theStruct.m_eGroup = ePhysicsGroup_NonSolid;

        HCLASS hClass = g_pLTServer->GetClass("BaseClass");
        LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
		LTASSERT( pModel, "Failed to create BaseClass" );
		if (!pModel)
		{
			return;
		}

		// Don't eat ticks please...
		::SetNextUpdate(pModel->m_hObject, UPDATE_NEVER);

        g_pLTServer->SetObjectScale(pModel->m_hObject, fNodeRadius);

		NodeRadiusStruct* pNRS = debug_new(NodeRadiusStruct);

		pNRS->hNode	 = hCurNode;
		pNRS->hModel = pModel->m_hObject;

		// Add the model to our list...
		m_NodeRadiusList.AddTail(pNRS);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::RemoveNodeRadiusModels()
//
//	PURPOSE:	Remove models representing the model node radii
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::RemoveNodeRadiusModels()
{
	if (m_NodeRadiusList.GetLength())
	{
		m_NodeRadiusList.Clear();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::UpdateNodeRadiusModels()
//
//	PURPOSE:	Update models representing the model node radii
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::UpdateNodeRadiusModels()
{
	AIASSERT( m_pHitBoxUser, m_hObject, "Called with NULL HitBoxUser" );
	AIASSERT( m_hModel, m_hObject, "Called with NULL m_hModel" );
	AIASSERT( m_hObject, m_hObject, "Called with NULL m_hObject" );

	// Create the models if necessary...

	if (!m_NodeRadiusList.GetLength())
	{
		CreateNodeRadiusModels();
	}

	NodeRadiusStruct** pNRS = m_NodeRadiusList.GetItem(TLIT_FIRST);

	while (pNRS && *pNRS && (*pNRS)->hModel)
	{
		const char* szNodeName = g_pModelsDB->GetNodeName( (*pNRS)->hNode );

		LTTransform resultTransform;
		if ( GetNodeTransform( szNodeName, resultTransform ) != false )
		{
			g_pLTServer->SetObjectPos((*pNRS)->hModel, resultTransform.m_vPos);

			float fNodeRadius = GetNodeRadius((*pNRS)->hNode);

			g_pLTServer->SetObjectScale((*pNRS)->hModel, fNodeRadius);
		}
	
		pNRS = m_NodeRadiusList.GetItem(TLIT_NEXT);
	}
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacterHitBox::GetNodeTransform()
//              
//	PURPOSE:	Returns True if the function succeeded, false if it failed.
//
//				Returns by parameter the transform for the passed in node.  If
//				the owner is an attachment, then be sure to use the tranform 
//				information from the parent.
//              
//----------------------------------------------------------------------------
bool CCharacterHitBox::GetNodeTransform( const char* const pszNodeName, LTTransform& NodeTransform ) const
{
	LTASSERT( m_pHitBoxUser, "No registered Hitbox User" );
	LTASSERT( g_pCommonLT, "No g_pCommonLT" );
	LTASSERT( g_pModelLT, "No g_pModelLT" );

	LTRESULT ltResult;
	HMODELNODE hNode;
	ltResult = g_pModelLT->GetNode(m_hModel, const_cast<char*>(pszNodeName), hNode);
//		LTASSERT_PARAM1( ltResult == LT_OK, "Unable to find Socket %s", pszNodeName );
	if ( ltResult != LT_OK )
	{
		return false;
	}

    LTTransform transform;
    ltResult = g_pModelLT->GetNodeTransform(m_hModel, hNode, NodeTransform, true);
//		LTASSERT( ltResult == LT_OK, "" );
	if ( ltResult != LT_OK )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Update()
//
//	PURPOSE:	Update our position
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Update()
{
	AIASSERT( m_pHitBoxUser, m_hObject, "Called with NULL HitBoxUser" );
	AIASSERT( m_hModel, m_hObject, "Called with NULL m_hModel" );
	AIASSERT( m_hObject, m_hObject, "Called with NULL m_hObject" );
	if (!m_hModel || !m_hObject)
	{
		return;
	}

	// Position to move to...
	LTVector vPos;
	bool bUpdateClient = false;

	// KLS 1/23/04 - Follow the visibility node on our model if specified...

	if (m_bFollowVisNode)
	{
		// Get the model's vis node...
		
		HMODELNODE hNode;
		if ( LT_OK == g_pModelLT->GetPhysicsVisNode(m_hModel, hNode) )
		{
			LTTransform tf;

			if ( LT_OK == g_pModelLT->GetNodeTransform(m_hModel, hNode, tf, true) )
			{
				vPos = tf.m_vPos;
	
				// KLS 5/3/04 - If we're following the model's vis node, we want to move
				// the model to follow us...

				LTVector vModelPos;
				g_pLTServer->GetObjectPos(m_hModel, &vModelPos);

				if (vPos.DistSqr(vModelPos) > 0.1f)
				{
					g_pLTServer->SetObjectPos(m_hModel, vPos);
				}
			}
		}
	}
	else  // Use the model's position...
	{
		// Get current model position...
		g_pLTServer->GetObjectPos(m_hModel, &vPos);

		// Make sure the hit box offset is relative to the model...
		
		LTRotation rRot;
		g_pLTServer->GetObjectRotation( m_hModel, &rRot );
		vPos += (rRot * m_vOffset);
	}

	// Get the hitbox's position...
	LTVector vMyPos;
	g_pLTServer->GetObjectPos(m_hObject, &vMyPos);


	// Only move the hitbox if it isn't close to the target position...

	if (vPos.DistSqr(vMyPos) > 0.1)
	{
		bUpdateClient = true;
		g_pLTServer->SetObjectPos(m_hObject, vPos);
 	}

	if( (m_bAnimControlsDims || m_bAnimControlsOffset) && (m_hControllingAnim != INVALID_ANI) )
	{
		HMODELANIM hCurAnim = INVALID_ANI;
		if( LT_OK == g_pModelLT->GetCurAnim( m_hModel, MAIN_TRACKER, hCurAnim ))
		{
			if( hCurAnim != m_hControllingAnim )
			{
				// We changed animations from our controlling anim so default our dims and offset...

				// Set offset first since SetDimsToModel() will update the client
				SetOffset( LTVector(0,0,0) );
				SetDimsToModel();

				// The animation is no longer controlling us...

				m_bAnimControlsDims = m_bAnimControlsOffset = false;
				m_hControllingAnim = INVALID_ANI;
			}	
		}
	}
	
	// Make sure the hit box is at least at the minimum dims...

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );

	if( vDims.x < HB_DIMS_MIN_XZ ||
		vDims.z < HB_DIMS_MIN_XZ )
	{
		vDims.x = vDims.z = HB_DIMS_MIN_XZ;
		g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );
		bUpdateClient = true;

	}

	if( vDims.y < HB_DIMS_MIN_Y	)
	{
		vDims.y = HB_DIMS_MIN_Y;
		g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );
		bUpdateClient = true;

	}

	if (bUpdateClient)
	{
		m_pHitBoxUser->UpdateClientHitBox();
	}

	// See if we should show our model node radii...

#ifndef _FINAL
	float fShowNodeRadii = (g_vtShowNodeRadii.GetFloat() ? (IsPlayer(m_hModel) ? g_vtShowPlayerNodeRadii.GetFloat() : 1.0f) : 0.0f);
	if (fShowNodeRadii)
	{
		UpdateNodeRadiusModels();
	}
	else
	{
		RemoveNodeRadiusModels();
	}
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::SetDimsToModel
//
//	PURPOSE:	Set the dims of the hit box to be an enlarged percentage of the models dims.
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::SetDimsToModel()
{
	if( !m_hModel || !m_hObject )
		return;

	LTVector vModelDims;
	GetDefaultModelDims( vModelDims );

	EnlargeDims( vModelDims );

	SetDims( vModelDims );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetDefaultModelDims
//
//	PURPOSE:	Grab the dims of the model that owns us...
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::GetDefaultModelDims( LTVector &vDims )
{
	vDims.Init();

	if( !m_hModel || !m_hObject )
		return;

	HMODELANIM hAni = g_pModelLT->GetCurAnim( m_hModel, MAIN_TRACKER, hAni );
	if( hAni != INVALID_ANI )
	{
		g_pModelLT->GetModelAnimUserDims( m_hModel, hAni, &vDims );
	}
	else
	{
		g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::EnlargeDims
//
//	PURPOSE:	Enlarged percentage of the given dims.
//				Use this to be consistent when setting the hitbox dims.
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::EnlargeDims( LTVector &vDims )
{
	if( !m_hObject )
		return;

	// Enlarge the hit box dims by a percentage of the given dims...

	vDims *= HB_DIMS_ENLARGE_PERCENT;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::SetDims
//
//	PURPOSE:	Set the dims of the hit box.
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::SetDims( LTVector const& vDims )
{
	if( !m_hObject )
		return;

	g_pPhysicsLT->SetObjectDims( m_hObject, ( LTVector* )&vDims, 0 );

	// Let the HitBoxUser notify the clients of the change...
	if( m_pHitBoxUser )
	{
		m_pHitBoxUser->UpdateClientHitBox();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Save(ILTMessage_Write *pMsg)
{
	AIASSERT( pMsg, m_hObject,  "Save with no pMsg" );
	if (!pMsg)
	{
		return;
	}

	SAVE_HOBJECT(m_hModel);
	SAVE_VECTOR(m_vOffset);
	SAVE_BOOL(m_bCanActivate);
	SAVE_bool( m_bAnimControlsDims );
	SAVE_bool( m_bAnimControlsOffset );
	SAVE_bool( m_bFollowVisNode );
	SAVE_DWORD( m_hControllingAnim );
	AIASSERT( dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(m_hModel)) == m_pHitBoxUser, NULL, "" );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Load(ILTMessage_Read *pMsg)
{
	AIASSERT( pMsg, m_hObject, "Load with no pMsg" );
	if (!pMsg)
	{
		return;
	}

	LOAD_HOBJECT(m_hModel);
	LOAD_VECTOR(m_vOffset);
	LOAD_BOOL(m_bCanActivate);
	LOAD_bool( m_bAnimControlsDims );
	LOAD_bool( m_bAnimControlsOffset );
	LOAD_bool( m_bFollowVisNode );
	LOAD_DWORD_CAST( m_hControllingAnim, HMODELANIM );

	m_pHitBoxUser = dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(m_hModel));
	AIASSERT( m_pHitBoxUser, m_hModel, "Unable to get the HitBoxUser interface from m_hModel." );
}


