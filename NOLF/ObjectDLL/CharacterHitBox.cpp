// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterHitBox.cpp
//
// PURPOSE : Character hit box object class implementation
//
// CREATED : 01/05/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CharacterHitBox.h"
#include "Character.h"
#include "Body.h"
#include "Projectile.h"
#include "ServerUtilities.h"
#include "CVarTrack.h"
#include "MsgIds.h"
#include "ObjectMsgs.h"
#include "Attachments.h"

static CVarTrack g_vtShowNodeRadii;
static CVarTrack g_vtNodeRadiusUseOverride;
static CVarTrack g_vtHeadNodeRadius;
static CVarTrack g_vtTorsoNodeRadius;
static CVarTrack g_vtArmNodeRadius;
static CVarTrack g_vtLegNodeRadius;
static CVarTrack g_HitDebugTrack;

#define DFAULT_NODE_RADIUS	12.5f

BEGIN_CLASS(CCharacterHitBox)
END_CLASS_DEFAULT_FLAGS(CCharacterHitBox, GameBase, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::CCharacterHitBox()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCharacterHitBox::CCharacterHitBox() : GameBase(OT_NORMAL)
{
    m_hModel = LTNULL;
	m_vOffset.Init();
	m_bCanActivate = LTTRUE;

    m_NodeRadiusList.Init(LTTRUE);
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
//	ROUTINE:	CCharacterHitBox::Setup()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::Init(HOBJECT hModel)
{
	if (!m_hObject || !hModel ||
        (!IsCharacter(hModel) && !IsBody(hModel))) return LTFALSE;

	m_hModel = hModel;

	// Set my flags...

    g_pLTServer->SetObjectFlags(m_hObject, FLAG_RAYHIT | FLAG_TOUCH_NOTIFY);

	// Set our user flags to USRFLG_CHARACTER, so the client will process
	// us like a character (for intersect segments)...

    uint32 dwFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
    g_pLTServer->SetObjectUserFlags(m_hObject, dwFlags | USRFLG_CHARACTER);

	if (!g_vtShowNodeRadii.IsInitted())
	{
        g_vtShowNodeRadii.Init(g_pLTServer, "HitBoxShowNodeRadii", LTNULL, 0.0f);
	}

	if (!g_vtNodeRadiusUseOverride.IsInitted())
	{
        g_vtNodeRadiusUseOverride.Init(g_pLTServer, "HitBoxNodeRadiusOverride", LTNULL, 0.0f);
	}

	if (!g_vtHeadNodeRadius.IsInitted())
	{
        g_vtHeadNodeRadius.Init(g_pLTServer, "HitBoxHeadNodeRadius", LTNULL, DFAULT_NODE_RADIUS);
	}

	if (!g_vtTorsoNodeRadius.IsInitted())
	{
        g_vtTorsoNodeRadius.Init(g_pLTServer, "HitBoxTorsoNodeRadius", LTNULL, DFAULT_NODE_RADIUS);
	}

	if (!g_vtArmNodeRadius.IsInitted())
	{
        g_vtArmNodeRadius.Init(g_pLTServer, "HitBoxArmNodeRadius", LTNULL, DFAULT_NODE_RADIUS);
	}

	if (!g_vtLegNodeRadius.IsInitted())
	{
        g_vtLegNodeRadius.Init(g_pLTServer, "HitBoxLegNodeRadius", LTNULL, DFAULT_NODE_RADIUS);
	}

	if (!g_HitDebugTrack.IsInitted())
	{
        g_HitDebugTrack.Init(g_pLTServer, "HitDebug", LTNULL, 0.0f);
	}

	SetNextUpdate(0.0f);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CCharacterHitBox::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
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

uint32 CCharacterHitBox::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    // Pass any messages to our model...

	if (m_hModel)
	{
		BaseClass* pBase = g_pLTServer->HandleToObject(m_hModel);
		if (pBase)
		{
			pBase->ObjectMessageFn(hSender, messageID, hRead);
		}
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetBoundingBoxColor()
//
//	PURPOSE:	Get the color of the bounding box
//
// ----------------------------------------------------------------------- //

LTVector CCharacterHitBox::GetBoundingBoxColor()
{
    return LTVector(1.0, 1.0, 0.0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::HandleImpact()
//
//	PURPOSE:	Handle a vector impacting on us
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::HandleImpact(CProjectile* pProj, IntersectInfo & iInfo,
                                     LTVector vDir, LTVector & vFrom)
{
    if (!pProj || !m_hModel) return LTFALSE;

    LTBOOL bHitSomething = LTTRUE;
	ModelNode eModelNode = eModelNodeInvalid;

	if (UsingHitDetection())
	{
		StartTimingCounter();

		if ( g_pGameServerShell->GetGameType() == SINGLE && IsPlayer(m_hModel) )
		{
			bHitSomething = LTTRUE;
			iInfo.m_hObject = m_hModel;
			eModelNode = (ModelNode)0;
		}
		else
		{
			bHitSomething = HandleVectorImpact(pProj, iInfo, vDir, vFrom, eModelNode);
		}

		EndTimingCounter("CCharacterHitBox::HandleImpact()");

		// Did we hit something?

		if (bHitSomething)
		{
			// This is the object that *really* got hit...

			if (eModelNode != eModelNodeInvalid)
			{
				SetModelNodeLastHit(eModelNode);
				ModelSkeleton eModelSkeleton = GetModelSkeleton();

				// Don't adjust damage on players (unless it is
				// multiplayer?)...

				if ( IsCharacter(iInfo.m_hObject) )
				{
                    CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(iInfo.m_hObject);
					pProj->AdjustDamage(pCharacter->ComputeDamageModifier(eModelNode));
				}
			}
		}
	}

	return bHitSomething;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::HandleVectorImpact()
//
//	PURPOSE:	Handle being hit by a vector
//
// Peter Higley method...
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::HandleVectorImpact(CProjectile* pProj, IntersectInfo& iInfo, LTVector& vDir,
                                           LTVector& vFrom, ModelNode& eModelNode)
{
	ModelSkeleton eModelSkeleton = GetModelSkeleton();

	// This algorithm may need to change since our dims are probably much
	// bigger than the model's actual dims...

    LTVector vObjDims;
    g_pLTServer->GetObjectDims(m_hObject, &vObjDims);

	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
    const char* szNodeName = LTNULL;

	ModelNode aHitNodes[64];
	int cHitNodes = 0;

	{for (int iNode = 0; iNode < cNodes; iNode++)
	{
		ModelNode eCurrentNode = (ModelNode)iNode;
		szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);

		if (szNodeName)
		{
            ILTModel* pModelLT = g_pLTServer->GetModelLT();

			HMODELNODE hNode;
			pModelLT->GetNode(m_hModel, (char*)szNodeName, hNode);

			LTransform transform;
            pModelLT->GetNodeTransform(m_hModel, hNode, transform, LTTRUE);

            LTVector vPos;
            LTRotation rRot;
            ILTTransform* pTransLT = g_pLTServer->GetTransformLT();
			pTransLT->Get(transform, vPos, rRot);

            LTFLOAT fNodeRadius = GetNodeRadius(eModelSkeleton, eCurrentNode);

            const LTVector vRelativeNodePos = vPos - iInfo.m_Point;

			// Distance along ray to point of closest approach to node point

            const LTFLOAT fRayDist = vDir.Dot(vRelativeNodePos);
            const LTFLOAT fDistSqr = (vDir*fRayDist - vRelativeNodePos).MagSqr();

			if (fDistSqr < fNodeRadius*fNodeRadius)
			{
				aHitNodes[cHitNodes++] = (ModelNode)iNode;
			}
		}
	}}

	// Find highest priority node we hit

    LTFLOAT fMaxPriority = (LTFLOAT)(-INT_MAX);

	if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
        g_pLTServer->CPrint("Checking hit nodes..................");

	{for ( int iNode = 0 ; iNode < cHitNodes ; iNode++ )
	{
		if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
            g_pLTServer->CPrint("Hit ''%s'' node", g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, (ModelNode)aHitNodes[iNode]));

        LTFLOAT fPriority = g_pModelButeMgr->GetSkeletonNodeHitPriority(eModelSkeleton, (ModelNode)aHitNodes[iNode]);
		if ( fPriority > fMaxPriority )
		{
			eModelNode = (ModelNode)aHitNodes[iNode];
			fMaxPriority = fPriority;
		}
	}}

	// Did we hit something?

	if (eModelNode != eModelNodeInvalid)
	{
		if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
            g_pLTServer->CPrint("...........using ''%s''", g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eModelNode));

		// Set the hit object as us ... attachments may change this

		iInfo.m_hObject = m_hModel;

		// Let the character/body prop's attachments handle the impact...

		CAttachments* pAttachments = GetAttachments();;

		if (pAttachments)
		{
			pAttachments->HandleProjectileImpact(pProj, iInfo, vDir, vFrom, eModelSkeleton, eModelNode);
		}

        return LTTRUE;
	}

	if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
        g_pLTServer->CPrint("....................hit nothing");

	//
	// Shot didn't hit box, propagate through box and return that point in
	// vFrom
	//

	// This projects from intersected box face to the opposing face
	//  This assumes axis-aligned boxes.  To get a more general technique
	//	replace every z with a dot product with the forward vector, every
	//	x with a dot product with the right vector, and every y with a dot
	//	product with the up vector ("every x/y/z" doesn't include vObjDim)
/*
    LTVector vProjection(0.0f,0.0f,0.0f);
    LTVector vAbsDir((float)fabs(vDir.x),(float)fabs(vDir.y),(float)fabs(vDir.z));

	if (vAbsDir.x > vAbsDir.y && vAbsDir.x > vAbsDir.z)
	{
		VEC_MULSCALAR(vProjection, vDir, vObjDims.x*2.0f/vAbsDir.x);
	}
	else if (vAbsDir.y > vAbsDir.x  && vAbsDir.y > vAbsDir.z)
	{
		VEC_MULSCALAR(vProjection, vDir, vObjDims.y*2.0f/vAbsDir.y);
	}
	else if (vAbsDir.z > vAbsDir.x  && vAbsDir.z > vAbsDir.y)
	{
		VEC_MULSCALAR(vProjection, vDir, vObjDims.z*2.0f/vAbsDir.z);
	}
*/
	vFrom = iInfo.m_Point + vDir*5.0f;

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::DidProjectileImpact()
//
//	PURPOSE:	See if the projectile actually hit us
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::DidProjectileImpact(CProjectile* pProjectile)
{
    if (!m_hModel || !pProjectile) return LTFALSE;

	// Test for simple hit case...
	if (!pProjectile->CanTestImpact()) return LTTRUE;
	if (g_pGameServerShell->GetGameType() == SINGLE && IsPlayer(m_hModel)) return LTTRUE;

	ModelSkeleton eModelSkeleton = GetModelSkeleton();

	// This algorithm may need to change since our dims are probably much
	// bigger than the model's actual dims...

    LTVector vObjDims;
    g_pLTServer->GetObjectDims(m_hObject, &vObjDims);

	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
    const char* szNodeName = LTNULL;

	ModelNode aHitNodes[64];
	int cHitNodes = 0;

	const LTVector& vFirePos = pProjectile->GetFirePos();
	const LTVector& vFireDir = pProjectile->GetFireDir();

	{for (int iNode = 0; iNode < cNodes; iNode++)
	{
		ModelNode eCurrentNode = (ModelNode)iNode;
		szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);

		if (szNodeName)
		{
            ILTModel* pModelLT = g_pLTServer->GetModelLT();

			HMODELNODE hNode;
			pModelLT->GetNode(m_hModel, (char*)szNodeName, hNode);

			LTransform transform;
            pModelLT->GetNodeTransform(m_hModel, hNode, transform, LTTRUE);

            LTVector vPos;
            LTRotation rRot;
            ILTTransform* pTransLT = g_pLTServer->GetTransformLT();
			pTransLT->Get(transform, vPos, rRot);

            LTFLOAT fNodeRadius = GetNodeRadius(eModelSkeleton, eCurrentNode);

            const LTVector vRelativeNodePos = vPos - vFirePos;

			// Distance along ray to point of closest approach to node point

            const LTFLOAT fRayDist = vFireDir.Dot(vRelativeNodePos);
            const LTFLOAT fDistSqr = (vFireDir*fRayDist - vRelativeNodePos).MagSqr();

			if (fDistSqr < fNodeRadius*fNodeRadius)
			{
				aHitNodes[cHitNodes++] = (ModelNode)iNode;
			}
		}
	}}

	// Find highest priority node we hit

    LTFLOAT fMaxPriority = (LTFLOAT)(-INT_MAX);

	if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
        g_pLTServer->CPrint("Checking hit nodes..................");

	ModelNode eModelNode = eModelNodeInvalid;

	{for ( int iNode = 0 ; iNode < cHitNodes ; iNode++ )
	{
		if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
            g_pLTServer->CPrint("Hit ''%s'' node", g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, (ModelNode)aHitNodes[iNode]));

        LTFLOAT fPriority = g_pModelButeMgr->GetSkeletonNodeHitPriority(eModelSkeleton, (ModelNode)aHitNodes[iNode]);
		if ( fPriority > fMaxPriority )
		{
			eModelNode = (ModelNode)aHitNodes[iNode];
			fMaxPriority = fPriority;
		}
	}}

	// Did we hit something?

	if (eModelNode != eModelNodeInvalid)
	{
		if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
            g_pLTServer->CPrint("...........using ''%s''", g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eModelNode));

		// Set our last hit node

		SetModelNodeLastHit(eModelNode);

		// Adjust damage

		ModelSkeleton eModelSkeleton = GetModelSkeleton();

		if ( IsCharacter(m_hModel) )
		{
            CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(m_hModel);
			pProjectile->AdjustDamage(pCharacter->ComputeDamageModifier(eModelNode));
		}

        return LTTRUE;
	}

	if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
        g_pLTServer->CPrint("....................hit nothing");

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetNodeRadius()
//
//	PURPOSE:	Get the model node's radius
//
// ----------------------------------------------------------------------- //

LTFLOAT CCharacterHitBox::GetNodeRadius(ModelSkeleton eModelSkeleton,
									   ModelNode eModelNode)
{
    LTFLOAT fRadius = g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eModelNode);

	// See if we're overriding the radius...

	if (g_vtNodeRadiusUseOverride.GetFloat())
	{
		HitLocation eLocation =	g_pModelButeMgr->GetSkeletonNodeLocation(eModelSkeleton, eModelNode);
		switch (eLocation)
		{
			case HL_HEAD :
				fRadius = g_vtHeadNodeRadius.GetFloat();
			break;

			case HL_TORSO :
				fRadius = g_vtTorsoNodeRadius.GetFloat();
			break;

			case HL_ARM :
				fRadius = g_vtArmNodeRadius.GetFloat();
			break;

			case HL_LEG :
				fRadius = g_vtLegNodeRadius.GetFloat();
			break;

			default :
				fRadius = DFAULT_NODE_RADIUS;
			break;
		}
	}

	return fRadius;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetAttachments()
//
//	PURPOSE:	Get our model's attachments
//
// ----------------------------------------------------------------------- //

CAttachments* CCharacterHitBox::GetAttachments()
{
    if (!m_hModel) return LTNULL;

    CAttachments* pAttachments = LTNULL;

	if (IsCharacter(m_hModel))
	{
        CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(m_hModel);
		if (pChar)
		{
			pAttachments = pChar->GetAttachments();
		}
	}
	else if (IsBody(m_hModel))
	{
        Body* pProp = (Body*) g_pLTServer->HandleToObject(m_hModel);
		if (pProp)
		{
			pAttachments = pProp->GetAttachments();
		}
	}

	return pAttachments;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetModelSkeleton()
//
//	PURPOSE:	Get our model's skeleton
//
// ----------------------------------------------------------------------- //

ModelSkeleton CCharacterHitBox::GetModelSkeleton()
{
	if (!m_hModel) return eModelSkeletonInvalid;

	ModelSkeleton eModelSkeleton = eModelSkeletonInvalid;

	if (IsCharacter(m_hModel))
	{
        CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(m_hModel);
		if (pChar)
		{
			eModelSkeleton = pChar->GetModelSkeleton();
		}
	}
	else if (IsBody(m_hModel))
	{
        Body* pProp = (Body*) g_pLTServer->HandleToObject(m_hModel);
		if (pProp)
		{
			eModelSkeleton = pProp->GetModelSkeleton();
		}
	}

	return eModelSkeleton;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::SetModelNodeLastHit()
//
//	PURPOSE:	Set our model's last hit node
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::SetModelNodeLastHit(ModelNode eModelNode)
{
	if (!m_hModel) return;

	if (IsCharacter(m_hModel))
	{
        CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(m_hModel);
		if (pChar)
		{
			pChar->SetModelNodeLastHit(eModelNode);
		}
	}
	else if (IsBody(m_hModel))
	{
        Body* pBody = (Body*) g_pLTServer->HandleToObject(m_hModel);
		if (pBody)
		{
			pBody->SetModelNodeLastHit(eModelNode);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::UsingHitDetection()
//
//	PURPOSE:	Are we using hit detection?
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::UsingHitDetection()
{
    if (!m_hModel) return LTFALSE;

	if (IsCharacter(m_hModel))
	{
        CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(m_hModel);
		if (pChar)
		{
			return pChar->UsingHitDetection();
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::CreateNodeRadiusModels()
//
//	PURPOSE:	Create models representing the model node radii
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::CreateNodeRadiusModels()
{
	if (!m_hModel) return;

	ModelSkeleton eModelSkeleton = GetModelSkeleton();

	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
    const char* szNodeName = LTNULL;

	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		ModelNode eCurrentNode = (ModelNode)iNode;
		szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);

		if (szNodeName)
		{
            ILTModel* pModelLT = g_pLTServer->GetModelLT();

			HMODELNODE hNode;
			pModelLT->GetNode(m_hModel, (char*)szNodeName, hNode);

            LTransform transform;
            pModelLT->GetNodeTransform(m_hModel, hNode, transform, LTTRUE);

            LTVector vPos;
            LTRotation rRot;
            ILTTransform* pTransLT = g_pLTServer->GetTransformLT();
			pTransLT->Get(transform, vPos, rRot);

			// Create the radius model...

			ObjectCreateStruct theStruct;
			INIT_OBJECTCREATESTRUCT(theStruct);

			theStruct.m_Pos = vPos;
			SAFE_STRCPY(theStruct.m_Filename, "SFX\\Expl\\Models\\264.abc");
			theStruct.m_Flags = FLAG_VISIBLE;
			theStruct.m_ObjectType = OT_MODEL;

            HCLASS hClass = g_pLTServer->GetClass("BaseClass");
            LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
			if (!pModel) return;

            LTFLOAT fNodeRadius = GetNodeRadius(eModelSkeleton, eCurrentNode);

            LTVector vScale;
			vScale.Init(fNodeRadius, fNodeRadius, fNodeRadius);
            g_pLTServer->ScaleObject(pModel->m_hObject, &vScale);

			NodeRadiusStruct* pNRS = debug_new(NodeRadiusStruct);

			pNRS->eNode	 = eCurrentNode;
			pNRS->hModel = pModel->m_hObject;

			// Add the model to our list...

			m_NodeRadiusList.AddTail(pNRS);
		}
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
	if (!m_hModel) return;

	// Create the models if necessary...

	if (!m_NodeRadiusList.GetLength())
	{
		CreateNodeRadiusModels();
	}


	ModelSkeleton eModelSkeleton = GetModelSkeleton();
    const char* szNodeName = LTNULL;

	NodeRadiusStruct** pNRS = m_NodeRadiusList.GetItem(TLIT_FIRST);
	while (pNRS && *pNRS && (*pNRS)->hModel)
	{
        ILTModel* pModelLT = g_pLTServer->GetModelLT();

		szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, (*pNRS)->eNode);

		HMODELNODE hNode;
		pModelLT->GetNode(m_hModel, (char*)szNodeName, hNode);

		LTransform transform;
        pModelLT->GetNodeTransform(m_hModel, hNode, transform, LTTRUE);

        LTVector vPos;
        LTRotation rRot;
        ILTTransform* pTransLT = g_pLTServer->GetTransformLT();
		pTransLT->Get(transform, vPos, rRot);

        g_pLTServer->SetObjectPos((*pNRS)->hModel, &vPos);

        LTFLOAT fNodeRadius = GetNodeRadius(eModelSkeleton, (*pNRS)->eNode);

        LTVector vScale;
		vScale.Init(fNodeRadius, fNodeRadius, fNodeRadius);
        g_pLTServer->ScaleObject((*pNRS)->hModel, &vScale);

		pNRS = m_NodeRadiusList.GetItem(TLIT_NEXT);
	}
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
	if (!m_hModel || !m_hObject) return;

    LTVector vPos, vMyPos;
    g_pLTServer->GetObjectPos(m_hModel, &vPos);
	vPos += m_vOffset;

	g_pLTServer->GetObjectPos(m_hObject, &vMyPos);

	if ((vPos - vMyPos).MagSqr() > 0.1)
	{
		g_pLTServer->SetObjectPos(m_hObject, &vPos);
	}

	// See if we should show our model node radii...

	if (g_vtShowNodeRadii.GetFloat())
	{
		UpdateNodeRadiusModels();
	}
	else
	{
		RemoveNodeRadiusModels();
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hModel);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vOffset);
    g_pLTServer->WriteToMessageByte(hWrite, m_bCanActivate);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hModel);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vOffset);
    m_bCanActivate = g_pLTServer->ReadFromMessageByte(hRead);
}