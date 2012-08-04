// ----------------------------------------------------------------------- //
//
// MODULE  : CClientWeaponSFX.cpp
//
// PURPOSE : CClientWeaponSFX - Implementation
//
// CREATED : 1/17/98
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponMgr.h"
#include "ClientWeaponSFX.h"
#include "SFXMsgIds.h"
#include "SurfaceFunctions.h"
#include "WeaponFXTypes.h"
#include "ServerMark.h"
#include "CommonUtilities.h"
#include "ServerUtilities.h"
#include "GameBase.h"

static void CreateServerMark(CLIENTWEAPONFX & theStruct);
static void CreateServerExitMark(const CLIENTWEAPONFX & theStruct);

#define REGION_DIAMETER			100.0f  // Squared distance actually
#define MAX_MARKS_IN_REGION		10
#define MAX_MARKS_PER_OBJECT	40

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateClientWeaponFX
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CreateClientWeaponFX(CLIENTWEAPONFX & theStruct)
{
    if (!g_pLTServer) return;

	// If this is a moveable object, set the flags of fx to ignore
	// marks and smoke...

	if (IsMoveable(theStruct.hObj))
	{
		theStruct.wIgnoreFX |= WFX_MARK;

		// Create a server-side mark if applicable...

		if (CanMarkObject(theStruct.hObj))
		{
			AMMO* pAmmo = g_pWeaponMgr->GetAmmo(theStruct.nAmmoId);
			if (pAmmo)
			{
				if (pAmmo->pImpactFX)
				{
					if (WFX_MARK & pAmmo->pImpactFX->nFlags)
					{
                        CreateServerMark((CLIENTWEAPONFX)theStruct);
					}
				}

				// Create an exit mark if applicable...

				if (pAmmo->pFireFX)
				{
					if (WFX_EXITMARK & pAmmo->pFireFX->nFlags)
					{
						CreateServerExitMark((const CLIENTWEAPONFX)theStruct);
					}
				}
			}
		}
	}

	// Do impact dings if applicable...

	if (!(IsMultiplayerGame() && IsCharacter(theStruct.hObj)))
	{
		theStruct.wIgnoreFX |= WFX_IMPACTDING;
	}

	// Tell all the clients who can see this fx about the fx...

    HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&(theStruct.vPos));
    g_pLTServer->WriteToMessageByte(hMessage, SFX_WEAPON_ID);
    g_pLTServer->WriteToMessageObject(hMessage, theStruct.hFiredFrom);
    g_pLTServer->WriteToMessageByte(hMessage, theStruct.nWeaponId);
    g_pLTServer->WriteToMessageByte(hMessage, theStruct.nAmmoId);
    g_pLTServer->WriteToMessageByte(hMessage, theStruct.nSurfaceType);
    g_pLTServer->WriteToMessageWord(hMessage, theStruct.wIgnoreFX);
    g_pLTServer->WriteToMessageByte(hMessage, theStruct.nShooterId);
    g_pLTServer->WriteToMessageVector(hMessage, &(theStruct.vFirePos));
    g_pLTServer->WriteToMessageVector(hMessage, &(theStruct.vPos));
    g_pLTServer->WriteToMessageVector(hMessage, &(theStruct.vSurfaceNormal));
	// This doesn't always work correctly...
    //g_pLTServer->WriteToMessageCompPosition(hMessage, &(theStruct.vFirePos));
    //g_pLTServer->WriteToMessageCompPosition(hMessage, &(theStruct.vPos));
    //g_pLTServer->WriteToMessageCompPosition(hMessage, &(theStruct.vSurfaceNormal));
    g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateServerMark
//
//	PURPOSE:	Create a server side mark
//
// ----------------------------------------------------------------------- //

static void CreateServerMark(CLIENTWEAPONFX & theStruct)
{
	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(theStruct.nAmmoId);
	if (!pAmmo) return;

	// If this isn't a GameBase object, return...

	if (!IsGameBase(theStruct.hObj)) return;

	// See if we should create a mark, or simply move one of the GameBase's
	// marks.

	// If the GameBase has the max number of marks or this mark is very close
	// to a pre-existing mark, just move that mark to the new position.

    GameBase* pObj = (GameBase*) g_pLTServer->HandleToObject(theStruct.hObj);
	if (!pObj) return;

    HOBJECT hMoveObj = LTNULL;
    HOBJECT hFarObj  = LTNULL;

	ObjectList* pMarkList = pObj->GetMarkList();
	if (pMarkList)
	{
        uint8 nNumMarks = pMarkList->m_nInList;
		ObjectLink* pLink = pMarkList->m_pFirstLink;

        LTFLOAT  fClosestMarkDist  = REGION_DIAMETER;
        LTFLOAT  fFarthestMarkDist = 0.0f;
        uint8   nNumInRegion      = 0;
        LTVector vPos;

		for (int i=0; i < nNumMarks && pLink; i++)
		{
			if (pLink->m_hObject)
			{
				HATTACHMENT hAttachment;
                if (LT_OK == g_pLTServer->FindAttachment(theStruct.hObj, pLink->m_hObject, &hAttachment))
				{
					LTransform transform;
                    g_pLTServer->Common()->GetAttachmentTransform(hAttachment, transform, LTTRUE);

					vPos = transform.m_Pos;
				}

                LTFLOAT fDist = VEC_DISTSQR(vPos, theStruct.vPos);
				if (fDist < REGION_DIAMETER)
				{
					if (fDist < fClosestMarkDist)
					{
						fClosestMarkDist = fDist;
						hMoveObj = pLink->m_hObject;
					}

					if (++nNumInRegion > MAX_MARKS_IN_REGION)
					{
						// Just move this mark to the correct pos...
						hMoveObj = hMoveObj ? hMoveObj : pLink->m_hObject;
						break;
					}
				}

				if (fDist > fFarthestMarkDist)
				{
					fFarthestMarkDist = fDist;
					hFarObj = pLink->m_hObject;
				}
			}

			pLink = pLink->m_pNext;
		}

		// If we've got the max number of marks on this object, just move
		// the closest one to the new position...

		if (nNumMarks >= MAX_MARKS_PER_OBJECT)
		{
			hMoveObj = hMoveObj ? hMoveObj : (hFarObj ? hFarObj : pMarkList->m_pFirstLink->m_hObject);
		}
		else
		{
            hMoveObj = LTNULL; // Need to create one...
		}
	}

	// Re-setup the object to move it...

	if (hMoveObj && IsKindOf(hMoveObj, "CServerMark"))
	{
        CServerMark* pMoveMark = (CServerMark*) g_pLTServer->HandleToObject(hMoveObj);
		if (!pMoveMark) return;

		// Since this mark is already attached to pObj, remove the attachment
		// (since CServerMark::Setup() will re-attach it)...

		HATTACHMENT hAttachment;
        if (LT_OK == g_pLTServer->FindAttachment(theStruct.hObj, hMoveObj, &hAttachment))
		{
            g_pLTServer->RemoveAttachment(hAttachment);
		}

        pMoveMark->Setup((CLIENTWEAPONFX)theStruct);
		return;
	}


	// Okay, no luck, need to create a new mark...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

    LTFLOAT fScaleAdjust = 1.0f;
	if (!GetImpactSprite((SurfaceType)theStruct.nSurfaceType, fScaleAdjust,
		theStruct.nAmmoId, createStruct.m_Filename, ARRAY_LEN(createStruct.m_Filename)))
	{
		return;
	}

	createStruct.m_ObjectType = OT_SPRITE;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT | FLAG_ROTATEABLESPRITE;
	createStruct.m_Pos = theStruct.vPos;

    g_pLTServer->AlignRotation(&(createStruct.m_Rotation), &((LTVector)theStruct.vSurfaceNormal), LTNULL);


    HCLASS hClass = g_pLTServer->GetClass("CServerMark");
    CServerMark* pMark = (CServerMark*) g_pLTServer->CreateObject(hClass, &createStruct);
	if (!pMark) return;

	// Add the mark to the object...

	pObj->AddMark(pMark->m_hObject);


	// Randomly adjust the mark's scale to add a bit o spice...

	if (pAmmo->pImpactFX)
	{
        LTFLOAT fScale = fScaleAdjust * pAmmo->pImpactFX->fMarkScale;

        LTVector vScale;
		VEC_SET(vScale, fScale, fScale, fScale);
        g_pLTServer->ScaleObject(pMark->m_hObject, &vScale);
	}

	pMark->Setup((CLIENTWEAPONFX)theStruct);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateExitMark
//
//	PURPOSE:	Create a server side exit mark
//
// ----------------------------------------------------------------------- //

static void CreateServerExitMark(const CLIENTWEAPONFX & theStruct)
{
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface((SurfaceType)theStruct.nSurfaceType);
	if (!pSurf || !pSurf->bCanShootThrough) return;

	int nMaxThickness = pSurf->nMaxShootThroughThickness;
	if (nMaxThickness < 1) return;

	// Determine if there is an "exit" surface...

	IntersectQuery qInfo;
	IntersectInfo iInfo;

    LTVector vDir = theStruct.vPos - theStruct.vFirePos;
	vDir.Norm();

    qInfo.m_From = theStruct.vPos + (vDir * (LTFLOAT)(nMaxThickness + 1));
	qInfo.m_To   = theStruct.vPos;

	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	SurfaceType eType = ST_UNKNOWN;

    if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
	{
		eType = GetSurfaceType(iInfo);
		if (ShowsMark(eType))
		{
            LTRotation rNormRot;
            g_pLTServer->AlignRotation(&rNormRot, &(iInfo.m_Plane.m_Normal), LTNULL);

			CLIENTWEAPONFX exitStruct = theStruct;
			exitStruct.vPos = iInfo.m_Point + vDir;
			exitStruct.vSurfaceNormal = iInfo.m_Plane.m_Normal;

            CreateServerMark(exitStruct);
		}
	}
}