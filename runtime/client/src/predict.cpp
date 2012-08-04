#include "bdefs.h"

#include "predict.h"
#include "clientmgr.h"
#include "systimer.h"
#include "sysdebugging.h"
#include "clientshell.h"
#ifndef _FINAL
#include "linesystem.h"
#include "ltobjectcreate.h"
#endif _FINAL

extern int32 g_bPrediction;

extern int32 g_nPredictionLines;

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//ILTClientPhysics holder
#include "iltphysics.h"
static ILTPhysics *i_client_physics;
define_holder_to_instance(ILTPhysics, i_client_physics, Client);

#ifndef _FINAL
//the ILTClient game interface
#include "iltclient.h"
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);
#endif //_FINAL


/*

OnObjectMove for each object using ServerPeriod
Set ServerPeriod
pd_Update

OnObjectMove for each object using ServerPeriod
Set ServerPeriod
pd_Update


When the client hears about an object moving, it says:
Move the object to where the server wants it in X seconds.
X = the time between this update and the previous one.



Does 20 updates with 0.05 as the frame time
Gets new position for player2.  It should interpolate him there in 1 second.


Move player1 dist  5 0.03
Move player1 dist  5 0.03
Move player1 dist  5 0.03

Move player2 dist 15 0.03

Move player1 dist  5 0.03
Move player1 dist  5 0.03
Move player1 dist  5 0.03

Move player2 dist 15 0.03


*/


// Scale the server periods by this amount.. 1.1 looks good.
static float g_fServerPeriodMultiplier = 1.1f;
extern float g_CV_MaxExtrapolateTime;


void pd_InitialServerUpdate(CClientShell *pShell, float gameTime)
{
	pShell->m_ClientGameTime = gameTime;
	pShell->m_ClientGameTimerSync = g_pClientMgr->m_CurTime;


	dl_TieOff( &pShell->m_MovingObjects );
	dl_TieOff( &pShell->m_RotatingObjects );
}


void pd_OnObjectMove(CClientShell *pShell, LTObject *pObject, LTVector *pNewPos, LTVector *pNewVel, bool bNew, bool bTeleport)
{
	ClientData *pData;

	pData = &pObject->cd;

	// Teleport the object if the new update is the same as the previous update
	// and it's not moving
	if (pNewPos->NearlyEquals(pData->m_LastUpdatePosServer, 0.1f) && (pNewVel->Mag() < 0.01f))
		bTeleport = true;


	// Remember the last update we got from the server
	LTFLOAT fUpdateDelta = pShell->m_ClientGameTime - pData->m_fLastUpdatePosTime;

	pData->m_fLastUpdatePosTime = pShell->m_ClientGameTime;
	pData->m_LastUpdatePosServer = *pNewPos;
	pData->m_LastUpdateVelServer = *pNewVel;

	if(g_bPrediction && ((pObject->m_Flags2 & FLAG2_DISABLEPREDICTION) == 0))
	{
		if(bNew || bTeleport)
		{
			// It's a new object.. just initialize it as nonmoving.
			dl_Remove(&pData->m_MovingLink);
			dl_TieOff(&pData->m_MovingLink);

			// Just move it since we won't be interpolating its position.
			i_client_shell->OnObjectMove((HOBJECT)pObject, bNew||bTeleport, pNewPos);
			g_pClientMgr->MoveObject(pObject, pNewPos, LTTRUE);
		}
		else
		{
#ifndef _FINAL
			// Create the line system if it's turned on
			if (!pData->m_hLineSystem && g_nPredictionLines)
			{
				ObjectCreateStruct createStruct;
				createStruct.Clear();
				createStruct.m_ObjectType = OT_LINESYSTEM;
				createStruct.m_Flags = FLAG_VISIBLE;
				pData->m_hLineSystem = ilt_client->CreateObject(&createStruct);
			}
			// Delete the line system if it gets turned off
			else if (pData->m_hLineSystem && !g_nPredictionLines)
			{
				ilt_client->RemoveObject(pData->m_hLineSystem);
				pData->m_hLineSystem = LTNULL;
			}
#endif //_FINAL

			dl_Remove(&pData->m_MovingLink);
			dl_Insert(&pShell->m_MovingObjects, &pData->m_MovingLink);
		}
	}
	else
	{
		// Just move the object like normal.
		i_client_shell->OnObjectMove((HOBJECT)pObject, bNew||bTeleport, pNewPos);
		g_pClientMgr->MoveObject(pObject, pNewPos, LTTRUE);
	}
}

void pd_OnObjectRotate(CClientShell *pShell, LTObject *pObject, LTRotation *pNewRot, bool bNew, bool bSnap)
{
	ClientData *pClientData;

	pClientData = &pObject->cd;
	if(g_bPrediction && ((pObject->m_Flags2 & FLAG2_DISABLEPREDICTION) == 0))
	{
		if(bNew || bSnap || pShell->m_ServerPeriod == 0.0f)
		{
			// It's a new object.. just initialize it as nonmoving.
			dl_Remove(&pClientData->m_RotatingLink);
			dl_TieOff(&pClientData->m_RotatingLink);

			pClientData->m_fRotAccumulatedTime = 0.0f;

			pClientData->m_rLastUpdateRotServer = *pNewRot;

			// Just snap it since we won't be interpolating its rotation.
			i_client_shell->OnObjectRotate((HOBJECT)pObject, bNew||bSnap, pNewRot);
			g_pClientMgr->RotateObject(pObject, pNewRot );
		}
		else
		{
			// Put it in the rotating object list.
			dl_Remove(&pClientData->m_RotatingLink);
			dl_Insert(&pShell->m_RotatingObjects, &pClientData->m_RotatingLink);

			pClientData->m_rLastUpdateRotServer = *pNewRot;

			// Add in the time...
			pClientData->m_fRotAccumulatedTime = pShell->m_ServerPeriod * g_fServerPeriodMultiplier;
		}
	}
	else
	{
		// Just move the object like normal.
		i_client_shell->OnObjectRotate((HOBJECT)pObject, bNew||bSnap, pNewRot);
		g_pClientMgr->RotateObject(pObject, pNewRot );
	}
}


static LTVector predict_EvaluateCurve(
	LTObject *pObj,
	const ClientData *pClientData,
	const LTVector &vGravity,
	float fInterpolant,
	float fTotalTime,
	LTBOOL bFullCalc
)
{
	if (bFullCalc)
	{
		// Save the object's state
		LTVector vOldPos = pObj->GetPos();
		LTVector vOldVel = pObj->m_Velocity;
		LTVector vOldAccel = pObj->m_Acceleration;
		uint32 nOldFlags = pObj->m_InternalFlags;
		float fOldFriction = pObj->m_FrictionCoefficient;

		// Put it where the server last said it was
		pObj->SetPos(pClientData->m_LastUpdatePosServer);
		pObj->m_Velocity = pClientData->m_LastUpdateVelServer;
		pObj->m_Acceleration.Init();
		pObj->m_FrictionCoefficient = 0.0f;

		// Calculate the movement info
		MotionInfo motionInfo;
		motionInfo.m_SlideRatio = 1.0;
		motionInfo.SetForce(&vGravity);

		LTVector vDelta;

		CalcMotion(&motionInfo,
			pObj,
			vDelta,
			pObj->m_Velocity,
			pObj->m_Acceleration,
			(pObj->m_Flags & FLAG_GRAVITY) != 0,
			fInterpolant * fTotalTime);

		// Put the object back to its old state
		pObj->SetPos(vOldPos);
		pObj->m_Velocity = vOldVel;
		pObj->m_Acceleration = vOldAccel;
		pObj->m_InternalFlags = nOldFlags;
		pObj->m_FrictionCoefficient = fOldFriction;

		// Return the new position
		return vDelta + pClientData->m_LastUpdatePosServer;
	}
	else
	{
		float fInterpTime = fTotalTime * fInterpolant;
		LTVector vServerPos = pClientData->m_LastUpdatePosServer + (pClientData->m_LastUpdateVelServer * fInterpTime) + vGravity * (fInterpTime * fInterpTime * 0.5f);
		return vServerPos;
	}
}


void pd_Update(CClientShell *pShell)
{
	float timeDelta, curTime;
	LTLink *pCur;
	LTObject *pObj;
	LTVector newPos;
	float fTimeLeft, fParam;
	LTRotation rRot;


	if(g_bPrediction)
	{
		// Figure out the delta..
		curTime = g_pClientMgr->m_CurTime;
		timeDelta = curTime - pShell->m_ClientGameTimerSync;
		if(timeDelta < 0.0f)
			timeDelta = 0.0f;

		pShell->m_ClientGameTime += timeDelta;
		pShell->m_ClientGameTimerSync = curTime;

		// Get global gravity
		LTVector vGravity, vGlobalGravity;
		i_client_physics->GetGlobalForce(vGlobalGravity);

		// Interpolate movement of each object.
		pCur = pShell->m_MovingObjects.m_pNext;
		while(pCur != &pShell->m_MovingObjects)
		{
			pObj = (LTObject*)pCur->m_pData;
			pCur = pCur->m_pNext;

			ClientData *pClientData = &(pObj->cd);

			// [KLS 3/12/02 - Updated to support per-object gravity override...

			if ((pObj->m_Flags & FLAG_GRAVITY) != 0)
			{
				vGravity = pObj->GetGlobalForceOverride();

				// If the global force override is zero, use the global
				// gravity...

				if (LTVector(0.0, 0.0, 0.0) == vGravity)
				{
					vGravity = vGlobalGravity;
				}
			}
			else
			{
				vGravity.Init();
			}

			float fTimeOffset = pShell->m_ClientGameTime - pClientData->m_fLastUpdatePosTime;

			fTimeOffset = LTCLAMP(fTimeOffset, 0.0f, g_CV_MaxExtrapolateTime);

			LTBOOL bTeleport = LTFALSE;

			if ((pClientData->m_LastUpdateVelServer.Mag() <= 0.0f) ||
				(g_CV_MaxExtrapolateTime <= 0.0f) ||
				(fTimeOffset == g_CV_MaxExtrapolateTime))
			{
				newPos = predict_EvaluateCurve(pObj, pClientData, LTVector(0.0f, 0.0f, 0.0f), 0.0f, g_CV_MaxExtrapolateTime, LTTRUE);
				bTeleport = LTTRUE;
				if (fTimeOffset < g_CV_MaxExtrapolateTime)
				{
					newPos = (newPos + pObj->GetPos()) * 0.5f;
				}
				else
				{
					dl_Remove(&pObj->cd.m_MovingLink);
					dl_TieOff(&pObj->cd.m_MovingLink);
				}
			}
			else
			{
				newPos = predict_EvaluateCurve(pObj, pClientData, vGravity, fTimeOffset / g_CV_MaxExtrapolateTime, g_CV_MaxExtrapolateTime, LTTRUE);
				newPos = (newPos + pObj->GetPos()) * 0.5f;
			}

			LTVector vOldPos = pObj->GetPos();
			// Move it..
			i_client_shell->OnObjectMove((HOBJECT)pObj, LTFALSE, &newPos);
			// Use the physics if it's solid, unless it needs to teleport
			if (((pObj->m_Flags & FLAG_SOLID) != 0) && (!bTeleport))
				i_client_physics->MoveObject((HOBJECT)pObj, &newPos, 0);
			else
				g_pClientMgr->MoveObject(pObj, &newPos, LTTRUE);

#ifndef _FINAL
			// Update the line system
			if (pClientData->m_hLineSystem && (g_nPredictionLines == 1))
			{
				LTLine line;

				// Red line - this is where the object actually moved
				line.m_Points[0].m_Pos = vOldPos;
				line.m_Points[0].r = 1.0f;
				line.m_Points[0].g = 0.0f;
				line.m_Points[0].b = 0.0f;
				line.m_Points[0].a = 1.0f;
				line.m_Points[1].m_Pos = pObj->GetPos();
				line.m_Points[1].r = 1.0f;
				line.m_Points[1].g = 0.0f;
				line.m_Points[1].b = 0.0f;
				line.m_Points[1].a = 1.0f;
				linesystem_AddLine(pClientData->m_hLineSystem, &line);
				// Green lines - this is where the prediction tried to move it
				line.m_Points[0].m_Pos = vOldPos;
				line.m_Points[0].r = 0.0f;
				line.m_Points[0].g = 1.0f;
				line.m_Points[0].b = 0.0f;
				line.m_Points[0].a = 1.0f;
				line.m_Points[1].m_Pos = newPos;
				line.m_Points[1].r = 0.0f;
				line.m_Points[1].g = 1.0f;
				line.m_Points[1].b = 0.0f;
				line.m_Points[1].a = 1.0f;
				linesystem_AddLine(pClientData->m_hLineSystem, &line);
				// Blue lines - this represents the server updates (initial location + velocity * max extrapolation)
				float fPrevTime = fTimeOffset - timeDelta;
				fPrevTime = LTMAX(fPrevTime, 0.0f);
				line.m_Points[0].m_Pos = pClientData->m_LastUpdatePosServer;
				line.m_Points[0].r = 0.0f;
				line.m_Points[0].g = 0.0f;
				line.m_Points[0].b = 1.0f;
				line.m_Points[0].a = 1.0f;
				line.m_Points[1].m_Pos = pClientData->m_LastUpdatePosServer + pClientData->m_LastUpdateVelServer * g_CV_MaxExtrapolateTime;
				line.m_Points[1].r = 0.0f;
				line.m_Points[1].g = 0.0f;
				line.m_Points[1].b = 1.0f;
				line.m_Points[1].a = 1.0f;
				linesystem_AddLine(pClientData->m_hLineSystem, &line);
			}
#endif //_FINAL
		}


		// Interpolate rotation of each object.
		pCur = pShell->m_RotatingObjects.m_pNext;
		while(pCur != &pShell->m_RotatingObjects)
		{
			pObj = (LTObject*)pCur->m_pData;
			pCur = pCur->m_pNext;

			fTimeLeft = MIN(pObj->cd.m_fRotAccumulatedTime, timeDelta);
			fParam = fTimeLeft / pObj->cd.m_fRotAccumulatedTime;

			LTRotation rTemp;
			quat_Slerp((float*)&rTemp, (float*)&pObj->m_Rotation, (float*)&pObj->cd.m_rLastUpdateRotServer, fParam);
			quat_Slerp((float*)&rRot, (float*)&pObj->m_Rotation, (float*)&rTemp, 0.5f);

			pObj->cd.m_fRotAccumulatedTime -= fTimeLeft;

			if( pObj->cd.m_fRotAccumulatedTime <= 0.0f )
			{
				rRot = pObj->cd.m_rLastUpdateRotServer;
				pObj->cd.m_fRotAccumulatedTime = 0.0f;
				dl_Remove(&pObj->cd.m_RotatingLink);
				dl_TieOff(&pObj->cd.m_RotatingLink);
			}

			i_client_shell->OnObjectRotate((HOBJECT)pObj, LTFALSE, &rRot);
			g_pClientMgr->RotateObject(pObj, &rRot);

		}
	}
}
