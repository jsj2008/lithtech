//------------------------------------------------------------------
//
//	FILE	  : ParticleSystem.cpp
//
//	PURPOSE	  : Particle system instance header file.
//
//	CREATED	  : May 30 1997
//
//	COPYRIGHT : MONOLITH Inc 1997 All Rights Reserved
//
//------------------------------------------------------------------

#include "bdefs.h"

#include "clientmgr.h"
#include "particlesystem.h"
#include "setupobject.h"
#include "sprite.h"
#include "dutil.h"
#include "iltclient.h"

//---------------------
// Console variables

extern LTBOOL g_CV_CollideParticles;


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);





#define VINTERP(dest, v1, v2, t1, t2, t3)  (dest).Init((v1).x+((v2).x-(v1).x)*(t1), (v1).y+((v2).y-(v1).y)*(t2), (v1).z+((v2).z-(v1).z)*t3);


LTRESULT ps_SetTexture(LTParticleSystem *pSystem, const char *pName)
{
	FileRef ref;
	int len;
	char endChars[4];
	LTRESULT dResult;

	pSystem->m_pCurTexture = LTNULL;
	pSystem->m_pSprite = LTNULL;

	ref.m_FileType = FILE_CLIENTFILE;
	ref.m_pFilename = pName;

	len = strlen(pName);
	if(len > 3)
	{
		endChars[0] = pName[len-3];
		endChars[1] = pName[len-2];
		endChars[2] = pName[len-1];
		endChars[3] = 0;

		if(du_UpperStrcmp(endChars, "DTX"))
		{
			pSystem->m_pCurTexture = g_pClientMgr->AddSharedTexture(&ref);
		}
		else if(du_UpperStrcmp(endChars, "SPR"))
		{
			dResult = LoadSprite(&ref, &pSystem->m_pSprite);
			if(dResult != LT_OK)
				return dResult;

			spr_InitTracker(&pSystem->m_SpriteTracker, pSystem->m_pSprite);
			if(pSystem->m_SpriteTracker.m_pCurFrame)
				pSystem->m_pCurTexture = pSystem->m_SpriteTracker.m_pCurFrame->m_pTex;
		}
	}

	return LT_OK;
}


void ps_AddParticles(LTParticleSystem *pSystem, uint32 nParticles,
					const LTVector *minOffset, const LTVector *maxOffset,
					const LTVector *minVel, const LTVector *maxVel,
					const LTVector *minColor, const LTVector *maxColor,
					LTFLOAT minLifetime, LTFLOAT maxLifetime )
{
	uint32		i;
	LTFLOAT		t[6];
	LTVector	offset, vel, color;
	LTFLOAT		lifeTime;

	for(i=0; i < nParticles; i++)
	{
		t[0] = (LTFLOAT)rand() / RAND_MAX;
		t[1] = (LTFLOAT)rand() / RAND_MAX;
		t[2] = (LTFLOAT)rand() / RAND_MAX;
		t[3] = (LTFLOAT)rand() / RAND_MAX;
		t[4] = (LTFLOAT)rand() / RAND_MAX;
		t[5] = (LTFLOAT)rand() / RAND_MAX;

		VINTERP(offset, (*minOffset), (*maxOffset), t[0], t[1], t[2]);
		VINTERP(vel, (*minVel), (*maxVel), t[3], t[4], t[5]);
		VINTERP(color, (*minColor), (*maxColor), t[0], t[5], t[3]);
		lifeTime = minLifetime + (maxLifetime-minLifetime) * t[3];

		//offset the position if it is in worldspace
		if(pSystem->m_psFlags & PS_WORLDSPACE)
		{
			offset += pSystem->GetPos();
		}

		ps_AddParticle(pSystem, &offset, &color, &vel, lifeTime);
	}
}

//handles objects colliding with a particle system so that they can flow around them
static void ps_HandleParticleCollisions(LTParticleSystem *pSystem, LTFLOAT t)
{
	//the list of objects that can block particles:
	static const uint32 ObjectTypes[]		= { OT_MODEL };
	static const uint32 nNumObjectTypes		= sizeof(ObjectTypes) / sizeof(ObjectTypes[0]);

	//the radius of the particle system
	float		fRad	= pSystem->m_SystemRadius;

	//radius of the particle system squared
	float		fRadSqr = fRad * fRad;

	//the radius of the object being tested
	float		fObjRad;

	//the radius of the object being tested squared
	float		fObjRadSqr;

	float		fObjDist;
	LTObject	*pObject;

	//for particle iteration
	PSParticle	*pParticle, *pEnd;

	//values for updating intersecting particles
	LTVector vToParticle;
	float	 fDistToPartSqr;
	LTVector vIncomingNormal;

	//the transformation matrix for the world to particle system
	bool	 bMatValid = false;
	LTMatrix mWorldToPart;

	LTVector vObjPos;
	LTVector vPartVel;

	//search through all objects that could possibly block them...
	for(uint32 nCurrType = 0; nCurrType < nNumObjectTypes; nCurrType++)
	{
		LTLink *pListHead = &g_pClientMgr->m_ObjectMgr.m_ObjectLists[ ObjectTypes[nCurrType] ].m_Head;

		for(LTLink *pCurr = pListHead->m_pNext; pCurr != pListHead; pCurr = pCurr->m_pNext)
		{
			pObject = (LTObject*)pCurr->m_pData;

			//get bounding sphere info about the object
			fObjDist	= (pObject->GetPos() - pSystem->m_SystemCenter).MagSqr();
			fObjRadSqr	= pObject->GetDims().MagSqr();
			fObjRad		= (float)sqrt(fObjRadSqr);

			//see if it is out of range
			if(fObjDist > (fRad + fObjRad) * (fRad + fObjRad))
				continue;

			//now we need to transform the object into the particle space if applicable
			vObjPos = pObject->GetPos();	
			if(!(pSystem->m_psFlags & PS_WORLDSPACE))
			{
				if(!bMatValid)
				{
					pSystem->SetupTransform(mWorldToPart);
					mWorldToPart.Inverse();
					bMatValid = true;
				}
				vObjPos = mWorldToPart * vObjPos;				
			}


			//run through the particles
			pParticle	= pSystem->m_ParticleHead.m_pNext;
			pEnd		= &pSystem->m_ParticleHead;

			while(pParticle != pEnd)
			{
				//see if it intersects with the blocker
				vToParticle = pParticle->m_Pos - vObjPos;
				fDistToPartSqr = vToParticle.MagSqr();

				if(fDistToPartSqr < fObjRadSqr)
				{
					//we need to update the velocity so that it moves perpindicular
					//to the sphere
					
					if(vToParticle.Dot(pObject->m_Velocity) > 0.1f)
					{
						vPartVel = vToParticle * (fObjRad / (float)sqrt(fDistToPartSqr) - 1.0f); 
						pParticle->m_Velocity += vPartVel;
					}
					if(vToParticle.Dot(pParticle->m_Velocity) < 0.0f)
					{
						//now find the normal of the incoming and outgoing vector
						vIncomingNormal = pParticle->m_Velocity.Cross(vToParticle);

						//and get the final velocity
						pParticle->m_Velocity = vToParticle.Cross(vIncomingNormal);

						//now here is a neat little trick. Since A cross B has mag |A||B|
						//incoming normal has mag |V||T|, then the new velocity has mag
						//|V||T||T|, so we can just divide by the distance squared (|T|), saving
						//us a sqrt
						pParticle->m_Velocity /= fDistToPartSqr;
					}
				}

				pParticle = pParticle->m_pNext;
			}
		}
	}
}


void ps_UpdateParticles(LTParticleSystem *pSystem, LTFLOAT t)
{
	LTVector basePos = pSystem->GetPos();
	uint32 flags = pSystem->m_psFlags;

	//see if we need to handle intersections with other objects
	if(g_CV_CollideParticles && (flags & PS_COLLIDE))
	{
		ps_HandleParticleCollisions(pSystem, t);
	}

	//see if this is a dumb particle system. If it is, bail
	if(flags & PS_DUMB)
		return;

	float gravityAccel = pSystem->m_GravityAccel * t;

	// Take one of 2 loops.
	PSParticle *pParticle = pSystem->m_ParticleHead.m_pNext;
	PSParticle *pEnd = &pSystem->m_ParticleHead;
	PSParticle *pNext;

	if(flags & PS_NEVERDIE)
	{
		while(pParticle != pEnd)
		{
			pParticle->m_Pos.x += pParticle->m_Velocity.x * t;
			pParticle->m_Pos.y += pParticle->m_Velocity.y * t;
			pParticle->m_Pos.z += pParticle->m_Velocity.z * t;
			pParticle->m_fAngle += pParticle->m_fAngularVelocity * t;
			ps_UpdateBox(pSystem, pParticle->m_Pos, pParticle->m_Size);
			pParticle->m_Velocity.y += gravityAccel;

			pParticle = pParticle->m_pNext;
		}
	}
	else
	{
		while(pParticle != pEnd)
		{
			pParticle->m_Lifetime -= t;
			if(pParticle->m_Lifetime < 0.0f)
			{
				pNext = pParticle->m_pNext;
				ps_RemoveParticle(pSystem, pParticle);
				pParticle = pNext;
				continue;
			}

			pParticle->m_Pos.x += pParticle->m_Velocity.x * t;
			pParticle->m_Pos.y += pParticle->m_Velocity.y * t;
			pParticle->m_Pos.z += pParticle->m_Velocity.z * t;
			pParticle->m_fAngle += pParticle->m_fAngularVelocity * t;
			ps_UpdateBox(pSystem, pParticle->m_Pos, pParticle->m_Size);
			pParticle->m_Velocity.y += gravityAccel;

			pParticle = pParticle->m_pNext;
		}
	}


	// Bounce the particles.
	if(flags & PS_BOUNCE)
	{
		ClientIntersectQuery iQuery;
		ClientIntersectInfo iInfo;

		VEC_COPY(iQuery.m_From, basePos);
		VEC_COPY(iQuery.m_To, iQuery.m_From);
		iQuery.m_To.y -= 400.0f;

		if (world_bsp_client->IntersectSegment(&iQuery, &iInfo))
		{
			//The original equation used to update the particle position
			//was yCoord = iInfo.m_Plane.m_Normal.y * iInfo.m_Plane.m_Dist;
			//but this was causing issues on sloped surfaces. So this
			//has been replaced with simply the point of collision -JohnO
			float yCoord = iInfo.m_Point.y;
			
			if(!(pSystem->m_psFlags & PS_WORLDSPACE))
				yCoord -= basePos.y;

			pParticle = pSystem->m_ParticleHead.m_pNext;
			pEnd = &pSystem->m_ParticleHead;
			while(pParticle != pEnd)
			{
				if(pParticle->m_Velocity.y < 0.0f)
				{
					if(pParticle->m_Pos.y < yCoord)
					{
						pParticle->m_Pos.y = yCoord;
						pParticle->m_Velocity.y = -pParticle->m_Velocity.y * 0.5f;
					}
				}

				pParticle = pParticle->m_pNext;
			}
		}
	}
}


void ps_UpdateParticleBoundingBox(LTParticleSystem *pSystem)
{
	LTVector halfBox = (pSystem->m_MaxPos - pSystem->m_MinPos) * 0.5f;

	if(!(pSystem->m_psFlags & PS_WORLDSPACE))
	{
		LTMatrix mTransform;
		pSystem->SetupTransform(mTransform);
		mTransform.Apply(pSystem->m_MinPos + halfBox, pSystem->m_SystemCenter);
	}
	else
	{
		pSystem->m_SystemCenter = pSystem->m_MinPos + halfBox;
	}

	pSystem->m_SystemRadius = halfBox.Mag() * LTMAX(pSystem->m_Scale.x, LTMAX(pSystem->m_Scale.y, pSystem->m_Scale.z));
	pSystem->m_Radius = pSystem->m_SystemRadius;
}


void ps_StartUpdatingPositions(LTParticleSystem *pSystem)
{
	pSystem->m_OldCenter = pSystem->m_SystemCenter;
	pSystem->m_OldRadius = pSystem->m_SystemRadius;
}


LTBOOL ps_EndUpdatingPositions(LTParticleSystem *pSystem)
{
	ps_UpdateParticleBoundingBox(pSystem);

	if(pSystem->m_OldCenter.x != pSystem->m_SystemCenter.x ||
		pSystem->m_OldCenter.y != pSystem->m_SystemCenter.y ||
		pSystem->m_OldCenter.z != pSystem->m_SystemCenter.z ||
		pSystem->m_SystemRadius > pSystem->m_OldRadius)
	{
		return LTTRUE;
	}

	return LTFALSE;
}

//sorts the particles in a system based upon the direction specified
void ps_SortParticles(LTParticleSystem *pSystem, const LTVector& vDir, uint32 nNumIters)
{
	assert(pSystem);

	//bail if we don't have any particles
	if(pSystem->m_nParticles < 2)
		return;

	//we need to make sure that the direction vector is in the same space as the particle
	//system to prevent us from having to multiply all the particles by the object
	//transform
	LTVector vActualDir = vDir;
	if(!(pSystem->m_psFlags & PS_WORLDSPACE))
	{
		LTMatrix mInverse;
		pSystem->SetupTransform(mInverse);
		mInverse.Inverse();

		//now transform the direction into object space
		mInverse.Apply3x3(vActualDir);
	}

	//the dot product of the current particles
	float fCurrDot;
	float fNextDot;

	//run through the specified number of times
	for(uint32 nCurrIter = 0; nCurrIter < nNumIters; nCurrIter++)
	{
		//now sort the list
		PSParticle* pCurr = pSystem->m_ParticleHead.m_pNext;
		PSParticle* pNext;

		//get the current distance
		fCurrDot = pCurr->m_Pos.Dot(vActualDir);

		uint32 nParticlesLeft = pSystem->m_nParticles - 1;

		while(nParticlesLeft)
		{
			//get the next particle
			pNext = pCurr->m_pNext;

			//find the distance along the vector
			fNextDot = pNext->m_Pos.Dot(vActualDir);

			//now we need to see if they must be swapped or not
			if(fNextDot < fCurrDot)
			{
				//next needs to come before the current, so swap them around

				//patch up the outside links
				pNext->m_pPrev = pCurr->m_pPrev;
				pCurr->m_pNext = pNext->m_pNext;

				//now the inside links
				pCurr->m_pPrev = pNext;
				pNext->m_pNext = pCurr;

				//we can keep current as it is since it is now in the appropriate place
			}
			else
			{
				//don't need to swap them around, so just move on through the list
				pCurr		= pNext;
				fCurrDot	= fNextDot;
			}

			//one less particle to worry about...
			nParticlesLeft--;
		}
	}
}

void ps_OptimizeParticles(LTParticleSystem *pSystem)
{
	PSParticle *pCur;

	if(pSystem->m_ParticleHead.m_pNext == &pSystem->m_ParticleHead)
	{
		pSystem->m_MinPos.Init();
		pSystem->m_MaxPos.Init();
		pSystem->m_SystemCenter.Init();
		pSystem->m_SystemRadius = 1.0f;
	}
	else
	{
		pSystem->m_MinPos.Init(100000.0f, 100000.0f, 100000.0f);
		pSystem->m_MaxPos = -pSystem->m_MinPos;

		pCur = pSystem->m_ParticleHead.m_pNext;
		while(pCur != &pSystem->m_ParticleHead)
		{
			ps_UpdateBox(pSystem, pCur->m_Pos, pCur->m_Size);
			pCur = pCur->m_pNext;
		}

		ps_UpdateParticleBoundingBox(pSystem);
	}
}





