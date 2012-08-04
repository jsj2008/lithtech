//------------------------------------------------------------------
//
//   MODULE  : FXMGR.CPP
//
//   PURPOSE : Implements class CClientFXMgr
//
//   CREATED : On 10/2/98 At 5:33:14 PM
//
//------------------------------------------------------------------

//
// Includes...
//
	
#include "stdafx.h"
#include "FxFlags.h"
#include "FxDefs.h"
#include "iltmessage.h"
#include "iltdrawprim.h"
#include "ClientFXMgr.h"
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "WinUtil.h"
#include "ClientFXDB.h"

// Defines....

#define FXLOD_DIST_LOW					6000.0f
#define FXLOD_DIST_MED					6000.0f
#define FXLOD_DIST_HIGH					6000.0f

// Globals....

uint32			g_dwID = 0;
ILTClient		*CClientFXMgr::s_pClientDE = LTNULL;
CClientFXMgr	*g_pClientFXMgr = LTNULL;		

typedef CBankedList<CLIENTFX_INSTANCE> ClientFXBank;
ClientFXBank* g_pCLIENTFX_INSTANCE_Bank = NULL;


float GetSystemFrameTime(uint32 nTimeBase, uint32& nPrevTime)
{
	uint32 nCurrTime = GetTickCount() - nTimeBase;
	float fElapsed = (nCurrTime - nPrevTime) / 1000.0f;
	nPrevTime = nCurrTime;

	return fElapsed;
}

void CLIENTFX_LINK::ClearLink()
{
	//the instance should always point to us if it points to anything
	if(m_pInstance)
	{
		if(m_pInstance->m_pLink == this)
			m_pInstance->m_pLink = LTNULL;
		else
			assert(!"Corrupted ClientFXLink found");
	}
	
	m_pInstance = LTNULL;
};

//-------------------------------------------------------------------------------------------
// CLIENTFX_INSTANCE
//-------------------------------------------------------------------------------------------

CLIENTFX_INSTANCE::CLIENTFX_INSTANCE()
:	
	m_tmElapsed( 0.0f ),
	m_tmSuspended( 0.0f ),
	m_fDuration( 0.0f ),
	m_vPos( 0.0f, 0.0f, 0.0f ),
	m_rRot( 0.0f, 0.0f, 0.0f, 1.0f ),
	m_dwID( -1 ),
	m_hParent( LTNULL ),
	m_dwObjectFlags( 0 ),
	m_dwObjectFlags2( 0 ),
	m_hTarget( LTNULL ),
	m_bLoop( LTFALSE ),
	m_bSmoothShutdown( true ),
	m_bShutdown( false ),
	m_bUseTargetData( LTFALSE ),
	m_vTargetPos( 0.0f, 0.0f, 0.0f ),
	m_vTargetNorm( 0.0f, 0.0f, 0.0f ),
	m_hAlternateParent( NULL ),
	m_bSuspended( false ),
	m_bShow( LTFALSE ),
	m_bPlayerView( LTFALSE ),
	m_pLink( LTNULL)
{
}

CLIENTFX_INSTANCE::~CLIENTFX_INSTANCE()
{
	ClearLink();

	//make sure to free all of our effects
	RemoveAllEffects();

	if (m_hAlternateParent)
	{
		CClientFXMgr::GetClientDE()->RemoveObject(m_hAlternateParent);
	}
	
}

bool CLIENTFX_INSTANCE::ExistFX(CBaseFX *pFX)
{
	CLinkListNode<FX_LINK> *pNode = m_collActiveFX.GetHead();

	while (pNode)
	{
		if (pNode->m_Data.m_pFX == pFX) 
			return true;
		
		pNode = pNode->m_pNext;
	}

	return false;
}

//are all FX inactive?
bool CLIENTFX_INSTANCE::IsDone()
{
	return(!m_collActiveFX.GetHead());
}

//is this suspended
bool CLIENTFX_INSTANCE::IsSuspended() const
{
	return m_bSuspended;
}

void CLIENTFX_INSTANCE::Hide()
{
	// Force us to be hidden
	m_bShow = LTFALSE;
}

void CLIENTFX_INSTANCE::Show()
{
	// Force us to be shown
	m_bShow = LTTRUE;
}

void CLIENTFX_INSTANCE::SetPos( const LTVector &vWorldPos, const LTVector &vCamRelPos )
{
	// Loop through all of the Muzzle flash's active FX and set the positions...

	if( m_collActiveFX.GetSize() )
	{
		uint32		dwFlags;
		CBaseFX		*pFX;
		LTVector	vPos;
		CLinkListNode<FX_LINK> *pActiveFX = m_collActiveFX.GetHead();
		
		while( pActiveFX )
		{
			pFX = pActiveFX->m_Data.m_pFX;
			if( m_bPlayerView )
			{
				g_pCommonLT->GetObjectFlags( pFX->GetFXObject(), OFT_Flags, dwFlags );
				dwFlags & FLAG_REALLYCLOSE ? vPos = vCamRelPos : vPos = vWorldPos;
			}
			else
			{
				vPos = vWorldPos;	
			}
			
			pFX->SetPos( vPos );
									
			pActiveFX = pActiveFX->m_pNext;
		}
	}
}

void CLIENTFX_INSTANCE::ClearLink()
{
	//delegate this out to our link if we have one and tell it to break the connection
	if (m_pLink)
	{
		//if we have a link, the link had better point to us, otherwise something
		//has gotten horribly out of sync
		assert((m_pLink->GetInstance() == this) && "Corrupted ClientFXLink found");

		//clear the connection. This will clear out our side as well
		m_pLink->ClearLink();

		//this disconnection should clear out our link
		assert((m_pLink == NULL) && "Corrupted ClientFXLink found");
	}
}

bool CLIENTFX_INSTANCE::IsFinished()
{
	if (IsDone()) 
		return true;

	CLinkListNode<FX_LINK>	*pActiveNode = m_collActiveFX.GetHead();
	CBaseFX	*pFX = LTNULL;
	while (pActiveNode)
	{
		pFX = pActiveNode->m_Data.m_pFX;

		// Check for expiration
		if( pFX ) 
		{				
			//determine if this effect has expired
			bool bExpired = ((pFX->GetElapsed() >= pFX->GetEndTime()) || pFX->IsShuttingDown()) && 
							 (pFX->IsFinishedShuttingDown() || !pActiveNode->m_Data.m_pRef->m_bSmoothShutdown);
			
			if (!bExpired) 
				return false;
		}

		pActiveNode = pActiveNode->m_pNext;
	}

	return true;
}

void CLIENTFX_INSTANCE::DeleteFX(CLinkListNode<FX_LINK> *pDelNode)
{
	if( !pDelNode ) 
		return;

	CBaseFX* pDelFX = pDelNode->m_Data.m_pFX;

	if(pDelFX)
	{
		// Make sure no other active FX in this instance have this pDelFX has their parent...
		CLinkListNode<FX_LINK>	*pActiveNode = m_collActiveFX.GetHead();
		while( pActiveNode )
		{
			CBaseFX *pPossibleChildFX = pActiveNode->m_Data.m_pFX;

			if( pPossibleChildFX && (pPossibleChildFX->GetParent() == pDelFX->GetFXObject()))
			{
				// NULL out the parent otherwise it will reference an object that is no longer there
				pPossibleChildFX->SetParent( LTNULL );
			}
			
			pActiveNode = pActiveNode->m_pNext;
		}

		// Give the FX a chance to clean itself up
		pDelFX->Term();
		CClientFXDB::GetSingleton().DeleteEffect(pDelFX);
	}

	//now remove this node from our list
	m_collActiveFX.Remove(pDelNode);
}
	


void CLIENTFX_INSTANCE::RemoveAllEffects()
{
	//we need to run through all effects and delete them
	CLinkListNode<FX_LINK> *pActiveFxNode = m_collActiveFX.GetHead();
	CLinkListNode<FX_LINK> *pNextActiveFxNode = NULL;

	while( pActiveFxNode )
	{
		//cache the next in case this node is deleted
		pNextActiveFxNode = pActiveFxNode->m_pNext;
		DeleteFX(pActiveFxNode);
		pActiveFxNode = pNextActiveFxNode;
	}
}


//------------------------------------------------------------------
//
//   FUNCTION : CClientFXMgr()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CClientFXMgr::CClientFXMgr()
{
	m_eDetailLevel		= FXLOD_HIGH;
	m_dDetailDistSqr[FXLOD_LOW]	= FXLOD_DIST_LOW * FXLOD_DIST_LOW;
	m_dDetailDistSqr[FXLOD_MED]	= FXLOD_DIST_MED * FXLOD_DIST_MED;
	m_dDetailDistSqr[FXLOD_HIGH]	= FXLOD_DIST_HIGH * FXLOD_DIST_HIGH;
	m_nPrevSystemTime	= 0;
	m_nSystemTimeBase	= GetTickCount();
	m_bPaused			= false;
	m_bGoreEnabled		= true;
	m_hCamera			= NULL;

	if( !g_pCLIENTFX_INSTANCE_Bank )
	{
		g_pCLIENTFX_INSTANCE_Bank = debug_new( ClientFXBank );
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CClientFXMgr
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CClientFXMgr::~CClientFXMgr()
{
	// Call Term()

	Term();

	// Check if we have the bank initialized, which it must be
	// since it was created in the constructor.
	if( g_pCLIENTFX_INSTANCE_Bank )
	{
		// Check if it's now empty, which means we can delete it.
		if( !g_pCLIENTFX_INSTANCE_Bank->GetSize( ))
		{
			debug_delete( g_pCLIENTFX_INSTANCE_Bank );
			g_pCLIENTFX_INSTANCE_Bank = NULL;
		}
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CClientFXMgr and Load all .fxd's
//
//------------------------------------------------------------------

bool CClientFXMgr::Init(ILTClient *pClientDE, LTBOOL bGlobal)
{
	m_pClientDE = pClientDE;
	CClientFXMgr::s_pClientDE = pClientDE;
	if (bGlobal)
	{
		ASSERT(!g_pClientFXMgr);
		g_pClientFXMgr = this;
	}

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CClientFXMgr
//
//------------------------------------------------------------------

void CClientFXMgr::Term()
{
	// Delete all the FX group instances

	CLinkListNode<CLIENTFX_INSTANCE *> *pInstNode = m_collActiveGroupFX.GetHead();

	while (pInstNode)
	{
		pInstNode->m_Data->RemoveAllEffects();
				
		g_pCLIENTFX_INSTANCE_Bank->Delete( pInstNode->m_Data );
				
		pInstNode = pInstNode->m_pNext;
	}

	m_collActiveGroupFX.RemoveAll();

}

//------------------------------------------------------------------
//
//   FUNCTION : GetClientDE()
//
//   PURPOSE  : sets the camera that will be used for this effect manager
//
//------------------------------------------------------------------
void CClientFXMgr::SetCamera(HOBJECT hCamera)
{
	m_hCamera = hCamera;
}


//------------------------------------------------------------------
//
//   FUNCTION : GetClientDE()
//
//   PURPOSE  : Returns static pointer to ILTClient interface
//
//------------------------------------------------------------------

ILTClient* CClientFXMgr::GetClientDE()
{
	return CClientFXMgr::s_pClientDE;
}

//------------------------------------------------------------------
//
//   FUNCTION : CreateClientFXCallback
//
//   PURPOSE  : callback for ClientFX to call to make new effects
//
//------------------------------------------------------------------
static bool CreateClientFXCallback(const CLIENTFX_CREATESTRUCT& CreateInfo, bool bStartInst, void* pUser)
{
	//make sure we have the client effect manager
	if(!pUser)
		return false;

	//convert types
	CClientFXMgr* pMgr = (CClientFXMgr*)pUser;

	CLIENTFX_CREATESTRUCT Test = CreateInfo;

	//now create that effect
	return pMgr->CreateClientFX(NULL, Test, bStartInst, true);
}

//------------------------------------------------------------------
//
//   FUNCTION : CreateFX()
//
//   PURPOSE  : Creates a named FX
//
//------------------------------------------------------------------

CBaseFX* CClientFXMgr::CreateFX(const char *sName, FX_BASEDATA *pBaseData, CBaseFXProps* pProps, HOBJECT hInstParent)
{
	CBaseFX *pNewFX = NULL;

	// Locate the named FX

	FX_REF *pFxRef = CClientFXDB::GetSingleton().FindFX(sName);

	if( pFxRef ) 
	{
		pNewFX = pFxRef->m_pfnCreate();
	}

	// If we have a new fx, go ahead and add it onto the active list

	if( pNewFX ) 
	{
		// Assign a unique ID for this FX

		pBaseData->m_dwID = GetUniqueID();
		
		if( !pNewFX->Init(m_pClientDE, pBaseData, pProps) )
		{
			// See if the FX->Init() filled out data to create a new instance...
			if( pBaseData->m_sNode[0] )
			{
				CLIENTFX_CREATESTRUCT	fxCS( pBaseData->m_sNode, pBaseData->m_dwFlags, pBaseData->m_vPos, pBaseData->m_rRot );
				fxCS.m_vTargetNorm		= pBaseData->m_vTargetNorm;
				fxCS.m_hParent			= hInstParent;
				
				CreateClientFX( fxCS, LTTRUE );
			}

			pNewFX->Term();
			CClientFXDB::GetSingleton().DeleteEffect(pNewFX);

			pNewFX = NULL;
		}
	}

	// All done....
	
	return pNewFX;
}

//specifies whether or not gore is enabled
void CClientFXMgr::SetGoreEnabled(bool bEnabled)
{
	m_bGoreEnabled = bEnabled;
}


//------------------------------------------------------------------
//
//   FUNCTION : UpdateInstanceSuspended()
//
//   PURPOSE  : Updates the suspended status of the instance and returns the status
//
//------------------------------------------------------------------

bool CClientFXMgr::UpdateInstanceSuspended(const LTVector& vCameraPos, CLIENTFX_INSTANCE* pInst)
{
	bool bSuspendInstance = false;
		
	// Check our parent to see if we are turned off or on
	bool bIsParentOn = false;
	if(pInst->m_hParent)
	{
		uint32 dwUsrFlags = 0;
		g_pCommonLT->GetObjectFlags( pInst->m_hParent, OFT_User, dwUsrFlags );
		bIsParentOn = (dwUsrFlags & USRFLG_SFX_ON) != 0;
	}

	//first off see if we should be suspended based upon our flags and our parents flags
	if( !bIsParentOn && !pInst->m_bShow )
	{
		//neither us or our parent is turned on, so suspend us
		bSuspendInstance = true;
	}
	else
	{
		//either us or our parent is on, so assume we are unsuspended.
		bSuspendInstance = false;

		//but we need to run through and make sure that none of our effects want to disable at
		//a distance
		if( pInst->m_collActiveFX.GetSize() && !pInst->m_bPlayerView )
		{
			float fDistToEffectSqr = pInst->m_vPos.DistSqr( vCameraPos );

			//see if they are outside or inside the range.
			// Really close FX should always be in range...
			if( fDistToEffectSqr > m_dDetailDistSqr[m_eDetailLevel] )
			{
				bSuspendInstance	= true;

				//we need to run through all of our active effects and determine if they can be
				//suspended at a distance (thus saving performance)
				CLinkListNode<FX_LINK> *pActiveNode = pInst->m_collActiveFX.GetHead();

				while( pActiveNode )
				{
					if(!pActiveNode->m_Data.m_pRef->m_bDisableAtDistance)
					{
						//can't freeze based upon distance
						bSuspendInstance = false;
						break;
					}
					
					pActiveNode = pActiveNode->m_pNext;
				}
			}
		}
	}

	//now set the appropriate suspension on the effect
	if(bSuspendInstance)
		SuspendInstance(pInst);
	else
		UnsuspendInstance(pInst);

	//return our result
	return pInst->IsSuspended();
}

//------------------------------------------------------------------
//
//   FUNCTION : ApplyEffectStartingOffset()
//
//   PURPOSE  : given an effect, it will look at the properties for the random offset, and apply
//				a series of fake updates in order to simulate offsetting the effect. This is good
//				for effects that need to start in the middle somewhere or be randomized to prevent
//				all effects looking the same
//
//------------------------------------------------------------------
void CClientFXMgr::ApplyEffectStartingOffset(CBaseFX* pFX, const FX_KEY* pKey)
{
	assert(pFX);

	//figure out the amoun of time to offset
	float fOffsetAmount = (pKey->m_bRandomStartOffset) ? GetRandom(0.0f, pKey->m_fMaxStartOffset) : pKey->m_fMaxStartOffset;

	//determine if this effect needs a random offset
	if(fOffsetAmount < 0.01f)
		return;

	//alright, we now need to fake several updates
	float fTimeInc = pKey->m_fStartOffsetInterval;

	//if they specify something small, just use a single update
	if(fTimeInc < 0.001f)
		fTimeInc = fOffsetAmount;

	//now actually update
	
	float fCurrTime;
	for(fCurrTime = 0.0f; fCurrTime + fTimeInc <= fOffsetAmount; fCurrTime += fTimeInc)
	{
		pFX->Update(fTimeInc);

		//make sure the initial frame is cleared so effects receive only one initial frame
		pFX->ClearState(FS_INITIALFRAME);
	}

	//and of course update with the remainder of the time
	if(fOffsetAmount - fCurrTime > 0.01f)
	{
		pFX->Update(fOffsetAmount - fCurrTime);
		pFX->ClearState(FS_INITIALFRAME);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : HandleShutdownEffect()
//
//   PURPOSE  : Given an instance and an effect that has just finished shutting down,
//				it will take the appropriate course of action. Note that this will
//				invalidate the node that is passed into it
//
//------------------------------------------------------------------
void CClientFXMgr::HandleShutdownEffect(CLIENTFX_INSTANCE* pInst, CLinkListNode<FX_LINK>* pKeyNode)
{
	//sanity check
	assert(pInst && pKeyNode);

	CBaseFX	*pFX = pKeyNode->m_Data.m_pFX;

	//we are finished shutting down, if we aren't looping, we need to destroy
	//the effect, otherwise just disable it
	if(pInst->m_bLoop && !pInst->m_bShutdown)
	{
		pFX->SetVisible(false);
		pFX->ClearState(FS_ACTIVE | FS_SHUTTINGDOWN | FS_INITIALFRAME);
	}
	else
	{
		pInst->DeleteFX(pKeyNode);
	}
}


//------------------------------------------------------------------
//
//   FUNCTION : UpdateInstanceInterval()
//
//   PURPOSE  : Given an instance and a time interval, this will appropriately update
//				all effects contained within that interval
//
//------------------------------------------------------------------
void CClientFXMgr::UpdateInstanceInterval(CLIENTFX_INSTANCE* pInst, float fStartInterval, float fEndInterval)
{
	//here are the possible scenarios:
	// Inactive
	// Inactive -> Active -> Shutting down
	// Inactive -> Active -> Shutting down -> Inactive
	// Inactive -> Active
	// Shutting Down
	// Shutting Down -> Inactive
	// Shutting Down -> Active -> Shutting Down
	// Shutting Down -> Active -> Shutting Down -> Inactive
	// Shutting Down -> Active
	// Active

	//A note about intervals: The interval used is inclusive of both start and ends. This does
	//mean that a boundary such as a key beginning or end can be hit twice, but because of the
	//way the state flow works, this will not cause any issues, since when it is inactive
	//it will look for active and vice a versa. Therefore since the beginning and end cannot
	//lie on the same position this should not introduce any issues, but ensures that the
	//full track length is handled when updating

	//alright, run through all active effects and determine what to do
	CLinkListNode<FX_LINK>				*pActiveNode;
	CLinkListNode<FX_LINK>				*pNextActiveNode;

	//determine if this is a looping instance
	bool bLoopInstance = pInst->m_bLoop;

	pActiveNode = pInst->m_collActiveFX.GetHead();
	while( pActiveNode )
	{
		//cache the next node in case we delete this key
		pNextActiveNode = pActiveNode->m_pNext;

		//alright, first off look at the state of the key and see what to do
		CBaseFX	*pFX		= pActiveNode->m_Data.m_pFX;
		const FX_KEY* pKey	= pActiveNode->m_Data.m_pRef;

		//this is our time slice beginning
		float fCurrStart = fStartInterval;

		//skip dead space on inactive effects
		if(!pFX->IsActive())
		{
			//sanity check, the shutting down flag should never be set without active
			assert(!pFX->IsShuttingDown());

			//This check is to verify that we aren't shutting down the instance and have inactive effects.
			//If we are shutting down, inactive effects are immediately pruned, and then active effects
			//should be removed as they are completed
			assert(!pInst->m_bShutdown);

			//figure out the start time of this effect
			float fKeyStart = pKey->m_tmStart;

			//this effect is not active. See if it should be (loop keys always should be)
			if(pKey->m_bContinualLoop || ((fKeyStart >= fCurrStart) && (fKeyStart <= fEndInterval)))
			{
				//our effect has just become active, we need to reset its elapsed time to 0,
				//and switch it over to an initial frame
				pFX->SetElapsed(0.0f);
				pFX->SetState(FS_ACTIVE | FS_INITIALFRAME);
				pFX->ClearState(FS_SHUTTINGDOWN);
				pFX->SetVisible(true);

				//handle applying a fake effect offset
				ApplyEffectStartingOffset(pFX, pKey);

				//move our timeslice forward to the beginning of the key
				fCurrStart = fKeyStart;
			}
		}

		if(pFX->IsActive())
		{
			//we are an active effect, which means we are either in the time block or are
			//shutting down. 

			//see if we are currently shutting down, and if we are going to transition into
			//becoming active
			if(pFX->IsShuttingDown())
			{
				float fKeyStart = pKey->m_tmStart;

				//however, we can only bring it into the active state from here if the instance
				//will allow us
				bool bCanActivate = pInst->m_bLoop && !pInst->m_bShutdown;

				//this effect is not active. See if it should be
				if(bCanActivate && (fKeyStart >= fCurrStart) && (fKeyStart <= fEndInterval))
				{
					//it will become active in this range, so move up to there and change state
					pFX->Update(fKeyStart - fCurrStart);

					pFX->SetElapsed(0.0f);
					pFX->SetState(FS_INITIALFRAME | FS_ACTIVE);
					pFX->ClearState(FS_SHUTTINGDOWN);

					//handle applying a fake effect offset
					ApplyEffectStartingOffset(pFX, pKey);

					//move our time position up to the key start
					fCurrStart = fKeyStart;
				}
			}

			//alright, we are now in a chunk of time where either the block is active
			//or shutting down. If it is active, we need to update until the end of the slice
			//or the end of the effect
			if(!pFX->IsShuttingDown())
			{
				//we aren't shutting down, so we can update like normal
				float fTimeBlockEnd = fEndInterval;
				bool bCompleteKey	= ((pKey->m_tmEnd <= fTimeBlockEnd) && (pKey->m_tmEnd >= fCurrStart));

				//see if we hit the end of the key
				if(bCompleteKey)
				{
					fTimeBlockEnd = pKey->m_tmEnd;
				}

				//update based upon the interval length
				pFX->Update(fTimeBlockEnd - fCurrStart);

				//the initial update state should be cleared now
				pFX->ClearState(FS_INITIALFRAME);

				//see if we completed the key
				if(bCompleteKey)
				{
					//we did, so now switch to a shutting down state if it isn't continually looping,
					//otherwise we need to reset the elapsed time to 0
					if(pKey->m_bContinualLoop && bLoopInstance)
					{
						//we have looped our key, so reset the elapsed amount. However, if we are a
						//continually looping effect that started with an offset, we don't want
						//to lose that offset and should threfore just wrap based upon the lifespan
						if(pKey->m_fMaxStartOffset >= 0.001f)
						{
							pFX->SetElapsed((float)fmod(pFX->GetElapsed(), pFX->GetLifespan()));
						}
						else
						{
							//no starting offset, so just to make sure that everything syncs
							//up correctly and no error gets introduced, reset it to 0
							pFX->SetElapsed(0.0f);
						}
					}
					else
					{
						//we're past our key, so start shutting down
						pFX->SetState(FS_SHUTTINGDOWN);
					}
				}

				//update our time start
				fCurrStart = fTimeBlockEnd;
			}

			//alright, now handle shutting down, in which case we just want to update however
			//much time we have left in this interval, and see if the effect is completed
			if(pFX->IsShuttingDown())
			{
				//allow it to update
				pFX->Update(fEndInterval - fCurrStart);

				//can this effect do a smooth shutdown?
				bool bSmoothShutdown = pKey->m_bSmoothShutdown && pInst->m_bSmoothShutdown;

				//see if this effect is done shutting down
				if(pFX->IsFinishedShuttingDown() || !bSmoothShutdown)
				{
					//notify of an effect that has finished shutting down
					HandleShutdownEffect(pInst, pActiveNode);

					//move onto the next node and keep processing
					pActiveNode = pNextActiveNode;
					continue;
				}
			}
		}
		
		//and onto the next node
		pActiveNode = pNextActiveNode;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : UpdateAllActiveFX()
//
//   PURPOSE  : Updates all the active FX in the world
//
//------------------------------------------------------------------

bool CClientFXMgr::UpdateAllActiveFX(LTBOOL bAppHasFocus)
{
	if(!m_hCamera)
	{
		assert(!"No camera specified for this effect manager");
		return false;
	}

	//Update our frame time, before any early outs so there aren't giant pops when the early
	//out fails
	float fFrameTime = GetFrameTime();

	//add in all the effects from our next update list and clear that out
	m_collActiveGroupFX.AppendList(m_collNextUpdateGroupFX);

	HCONSOLEVAR hVar = m_pClientDE->GetConsoleVar("UpdateClientFX");
	if (hVar)
	{
		float fVal = m_pClientDE->GetVarValueFloat(hVar);

		if (!fVal) 
			return true;
	}

	//see if we should even update
	if(IsPaused())
	{
		//no time has elapsed, don't bother updating
		return true;
	}

	//setup the callback
	SetupCreateEffectCallback();

	//get the position of the camera
	LTVector vCameraPos;
	g_pLTClient->GetObjectPos(m_hCamera, &vCameraPos);

	// Set params....
	CClientFXDB::GetSingleton().SetAppFocus( bAppHasFocus ? true : false );
	CClientFXDB::GetSingleton().SetPlayer( g_pPlayerMgr->GetMoveMgr()->GetObject() );
	
	//
	// Update the group Instances
	//
	
	CLinkListNode<CLIENTFX_INSTANCE *>	*pInstNode = m_collActiveGroupFX.GetHead();
	CLinkListNode<CLIENTFX_INSTANCE *>	*pNextNode;

	while( pInstNode )
	{
		CLIENTFX_INSTANCE *pInst		= pInstNode->m_Data;

		//cache the next pointer in case the instance gets removed
		pNextNode	= pInstNode->m_pNext;

		//see if this instance is suspended, if so, just call the suspended update
		if(UpdateInstanceSuspended(vCameraPos, pInst))
		{
			//just run through all effects and give them a suspended updata
			CLinkListNode<FX_LINK> *pActiveNode = pInst->m_collActiveFX.GetHead();
			while( pActiveNode )
			{
				pActiveNode->m_Data.m_pFX->SuspendedUpdate(fFrameTime);
				pActiveNode = pActiveNode->m_pNext;
			}

			//don't bother with any interval updating
			pInstNode = pNextNode;
			continue;
		}


		//determine the start and end of our update interval, relative to the instance
		//time frame
		float fStartInterval	= pInst->m_tmElapsed;
		float fEndInterval		= fStartInterval + fFrameTime;

		//we now need to iteratively break this interval down into a series of intervals that
		//do not extend past the end of the effect
		bool bLastSegment = false;
		while(!bLastSegment)
		{
			//pick whichever is closest, the end of the interval, or the duration of the
			//effect
			float fEndSegment = pInst->m_fDuration;

			if(fEndInterval < pInst->m_fDuration)
			{
				bLastSegment = true;
				fEndSegment = fEndInterval;
			}
			
			//alright, we now have an interval, update all the effects that lie within it
			UpdateInstanceInterval(pInst, fStartInterval, fEndSegment);

			//now move on to the next interval if necessary
			if(!bLastSegment)
			{
				fStartInterval	= 0.0f;
				fEndInterval	-= pInst->m_fDuration;
			}
		}

		//all done, save our time
		pInst->m_tmElapsed = fEndInterval;

		//see if we are done with this effect
		if( pInst->m_collActiveFX.GetSize() == 0 )
		{				
			// Destroy the instance
			g_pCLIENTFX_INSTANCE_Bank->Delete( pInst );
			m_collActiveGroupFX.Remove(pInstNode);
		}

		//and move onto the next node
		pInstNode = pNextNode;
	}

	// Success !!
	return true;
}	

//------------------------------------------------------------------
//
//   FUNCTION : RenderAllActiveFX()
//
//   PURPOSE  : Goes through the list of active effects and gives them a chance to render
//
//------------------------------------------------------------------

bool CClientFXMgr::RenderAllActiveFX(LTBOOL bAppHasFocus)
{
	if(!m_hCamera)
	{
		assert(!"No camera specified for this effect manager");
		return false;
	}

	//setup the callback
	SetupCreateEffectCallback();

	// Set params....
	CClientFXDB::GetSingleton().SetAppFocus( bAppHasFocus ? true : false );
	CClientFXDB::GetSingleton().SetPlayer( g_pPlayerMgr->GetMoveMgr()->GetObject() );
	
	CLIENTFX_INSTANCE					*pInst;
	CLinkListNode<FX_LINK>				*pActiveNode;
	CLinkListNode<FX_LINK>				*pNextActiveNode;

	//
	// Update the group Instances
	//
	
	CLinkListNode<CLIENTFX_INSTANCE *>	*pInstNode = m_collActiveGroupFX.GetHead();
	CLinkListNode<CLIENTFX_INSTANCE *>	*pNextNode;

	while( pInstNode )
	{
		pInst		= pInstNode->m_Data;

		//cache the next pointer in case the instance gets removed
		pNextNode	= pInstNode->m_pNext;

		pActiveNode = pInst->m_collActiveFX.GetHead();

		while( pActiveNode )
		{
			pNextActiveNode = pActiveNode->m_pNext;

			//don't worry about suspended effects
			if(pInst->IsSuspended())
			{
				pActiveNode = pNextActiveNode;
				continue;
			}

			pActiveNode->m_Data.m_pFX->SetCamera(m_hCamera);
			pActiveNode->m_Data.m_pFX->Render();

			pActiveNode = pNextActiveNode;
		} 

		pInstNode = pNextNode;
	}

	// Success !!
	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : ShutdownAllFX()
//
//   PURPOSE  : Shuts down all active FX
//
//------------------------------------------------------------------

void CClientFXMgr::ShutdownAllFX()
{
	// Delete all the FX group instances

	CLinkListNode<CLIENTFX_INSTANCE *> *pInstNode = m_collActiveGroupFX.GetHead();

	while (pInstNode)
	{
		pInstNode->m_Data->RemoveAllEffects();		
		g_pCLIENTFX_INSTANCE_Bank->Delete( pInstNode->m_Data );
			
		pInstNode = pInstNode->m_pNext;
	}

	m_collActiveGroupFX.RemoveAll();
}

//------------------------------------------------------------------
//
//   FUNCTION : ShutdownGroupByRef()
//
//   PURPOSE  : Shuts down a group FX given a reference
//
//------------------------------------------------------------------
void CClientFXMgr::ShutdownClientFX(CLIENTFX_LINK *pLink)
{
	if (pLink && pLink->m_pInstance)
	{
		ShutdownClientFX(pLink->m_pInstance);
		pLink->ClearLink();
	}
}

void CClientFXMgr::ShutdownClientFX(CLIENTFX_INSTANCE *pFxGroup)
{	
	CLinkListNode<CLIENTFX_INSTANCE *> *pActiveNode = m_collActiveGroupFX.GetHead();

	//setup the callback in case any create effects as they are destroyed
	SetupCreateEffectCallback();

	while (pActiveNode)
	{	
		CLIENTFX_INSTANCE* pInst = pActiveNode->m_Data;

		if (pInst == pFxGroup)
		{		
			// Shut it down !!
			pInst->m_bShutdown = true;

			HOBJECT hReplaceParent = NULL;
			
			//see if this effect performs a smooth shutdown, if so, we need to create a dummy
			//object to place everything under a dummy object
			if (pInst->m_bSmoothShutdown)
			{
				LTVector	vPos;
				LTRotation	rRot;

				if (pFxGroup->m_hParent)
				{
					m_pClientDE->GetObjectPos(pFxGroup->m_hParent, &vPos);
					m_pClientDE->GetObjectRotation(pFxGroup->m_hParent, &rRot);
				}
				else
				{
					vPos = pFxGroup->m_vPos;
					rRot = pFxGroup->m_rRot;
				}

				// Create a temporary parent object....

				ObjectCreateStruct ocs;
				INIT_OBJECTCREATESTRUCT(ocs);

				ocs.m_ObjectType	= OT_NORMAL;
				ocs.m_Pos			= vPos;
				ocs.m_Rotation		= rRot;		 
				ocs.m_Flags			= 0;

				hReplaceParent = m_pClientDE->CreateObject(&ocs);
				pFxGroup->m_hAlternateParent = hReplaceParent;
				pFxGroup->m_hParent = hReplaceParent;				
			}


			if (!hReplaceParent)
			{
				//we couldn't create or didn't need a replacement object, so just remove all fx
				pInst->m_hAlternateParent	= NULL;
				pInst->m_hParent			= NULL;

				pInst->RemoveAllEffects();
			}
			else
			{
				// We have a parent to set, but in addition we need to notify all effects that
				//we are shutting down, and remove any effects that don't need a smooth shutdown (this
				//way it can be polled instantly after to determine if it is done)

				CLinkListNode<FX_LINK> *pActiveFxNode = pInst->m_collActiveFX.GetHead();
				CLinkListNode<FX_LINK> *pNextActiveFxNode = NULL;

				while( pActiveFxNode )
				{
					//cache the next in case this node is deleted
					pNextActiveFxNode = pActiveFxNode->m_pNext;

					CBaseFX *pFX = pActiveFxNode->m_Data.m_pFX;

					//we want to remove any inactive nodes
					if(!pFX->IsActive())
					{
						pInst->DeleteFX(pActiveFxNode);
					}
					else
					{
						const FX_KEY* pKey = pActiveFxNode->m_Data.m_pRef;

						//reassign the parent of this object
						pFX->SetParent(hReplaceParent);

						//we have an active effect, set it to shutting down
						pFX->SetState(FS_SHUTTINGDOWN);

						//now that it is shutting down, see if it is complete
						if(pFX->IsFinishedShuttingDown() || !pInst->m_bSmoothShutdown || !pKey->m_bSmoothShutdown)
						{
							//we can just remove the effect
							pInst->DeleteFX(pActiveFxNode);
						}
					}
					
					pActiveFxNode = pNextActiveFxNode;
				}
			}

			break;
		}

		pActiveNode = pActiveNode->m_pNext;
	}
}


//Called to setup the create effect callback so that any effects created during the updating of an
//object will be associated with this client effect manager
void CClientFXMgr::SetupCreateEffectCallback()
{
	CClientFXDB::GetSingleton().SetCreateCallback(CreateClientFXCallback, this);
}


//------------------------------------------------------------------
//
//   FUNCTION : CreateClientFX()
//
//   PURPOSE  : Creates a group FX at a given position
//
//------------------------------------------------------------------
bool CClientFXMgr::CreateClientFX(CLIENTFX_LINK *pLink, const CLIENTFX_CREATESTRUCT &fxInit, LTBOOL bStartInst, bool bAddNextUpdate)
{
	//make sure to remove any old link
	if(pLink)
	{
		pLink->ClearLink();
	}

	//try and actually create the effect
	CLIENTFX_INSTANCE* pNewInst = CreateClientFX(fxInit,bStartInst, bAddNextUpdate);
	if (!pNewInst) 
		return false;
	
	//setup a new link if necessary
	if (pLink)
	{
		//if this is not null, we will be breaking a connection and invalidating the referencing
		//system
		assert(pNewInst->m_pLink == NULL);

		pLink->m_pInstance = pNewInst;
		pNewInst->m_pLink = pLink;
	}

	return true;
}

CLIENTFX_INSTANCE* CClientFXMgr::CreateClientFX(const CLIENTFX_CREATESTRUCT &fxInit, LTBOOL bStartInst, bool bAddNextUpdate)
{
	//setup the callback in case it creates any effects when it is created
	SetupCreateEffectCallback();


	FX_GROUP *pRef = CClientFXDB::GetSingleton().FindGroupFX(fxInit.m_sName);
	if (!pRef) 
		return NULL;

	CLIENTFX_INSTANCE *pNewInst = g_pCLIENTFX_INSTANCE_Bank->New();  //debug_new( CLIENTFX_INSTANCE );
	if( !pNewInst ) 
		return NULL;

	pNewInst->m_pData			= pRef;
	pNewInst->m_dwID			= GetUniqueID();

	pNewInst->m_hParent			= fxInit.m_hParent;	
	pNewInst->m_bLoop			= (fxInit.m_dwFlags & FXFLAG_LOOP) ? true : false;
	pNewInst->m_bSmoothShutdown	= (fxInit.m_dwFlags & FXFLAG_NOSMOOTHSHUTDOWN) ? false : true;
	pNewInst->m_dwObjectFlags	|= (fxInit.m_dwFlags & FXFLAG_REALLYCLOSE) ? FLAG_REALLYCLOSE : 0;
	pNewInst->m_bPlayerView		= !!(fxInit.m_dwFlags & FXFLAG_REALLYCLOSE);
	pNewInst->m_bShutdown		= false;
	pNewInst->m_hTarget			= fxInit.m_hTarget;
	
	pNewInst->m_bUseTargetData  = fxInit.m_bUseTargetData;
	pNewInst->m_vTargetPos		= fxInit.m_vTargetPos;
	pNewInst->m_vTargetNorm		= fxInit.m_vTargetNorm;

	pNewInst->m_pLink			= LTNULL;

	// Load the instance with all the inactive FX
	pNewInst->m_fDuration		= pRef->m_tmTotalTime;

	// Add it to the appropriate list
	if(bAddNextUpdate)
	{
		m_collNextUpdateGroupFX.AddTail(pNewInst);
	}
	else
	{
		m_collActiveGroupFX.AddTail(pNewInst);
	}

	// Set the position
	if( pNewInst->m_bPlayerView )
	{
		pNewInst->m_vPos.Init();
		pNewInst->m_rRot.Init();
	}
	else if (fxInit.m_hParent)
	{
		g_pLTClient->GetObjectPos(fxInit.m_hParent,&pNewInst->m_vPos);
		g_pLTClient->GetObjectRotation( fxInit.m_hParent, &pNewInst->m_rRot );
	}
	else
	{
		pNewInst->m_vPos = fxInit.m_vPos;
		pNewInst->m_rRot = fxInit.m_rRot;
	}
	pNewInst->m_tmElapsed = 0.0f;

	//now go through and actually create all of our keys
	for(uint32 nCurrKey = 0; nCurrKey < pRef->m_nNumKeys; nCurrKey++)
	{
		//make sure that the detail level is enabled
		if(!IsDetailLevelEnabled(pRef->m_pKeys[nCurrKey].m_nDetailLevel))
		{
			continue;
		}

		//make sure that if it is gore, we can show gore
		if(!m_bGoreEnabled && pRef->m_pKeys[nCurrKey].m_bGore)
		{
			continue;
		}

		CreateFXKey(pNewInst, &pRef->m_pKeys[nCurrKey]);
	}

	// Immediatly start the instance if told to. 
	if( bStartInst )
	{
		//make sure it is visible
		pNewInst->Show();

		//and create any effects that start out instantly
		UpdateInstanceInterval(pNewInst, 0.0f, 0.0f);
	}

	// Success !!
	return pNewInst;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientFXMgr::StartInstance
//
//  PURPOSE:	Actually create all the FX in the Instance
//
// ----------------------------------------------------------------------- //

bool CClientFXMgr::CreateFXKey( CLIENTFX_INSTANCE* pInst, FX_KEY* pKey )
{
	if( !pInst || !pKey ) 
		return false;
	//
	// We need to go ahead and create this effect
	//

	assert(m_hCamera);

	FX_BASEDATA	fxData;
	fxData.m_vPos				= pInst->m_vPos;
	fxData.m_rRot				= pInst->m_rRot;
	fxData.m_dwID				= pInst->m_dwID;
	fxData.m_hTarget			= pInst->m_hTarget;
	fxData.m_dwObjectFlags		= pInst->m_dwObjectFlags;
	fxData.m_dwObjectFlags2		= pInst->m_dwObjectFlags2;
	fxData.m_bUseTargetData		= pInst->m_bUseTargetData;
	fxData.m_vTargetPos			= pInst->m_vTargetPos;
	fxData.m_vTargetNorm		= pInst->m_vTargetNorm;
	fxData.m_hCamera			= m_hCamera;

	// Save the parent
	fxData.m_hParent = pInst->m_hParent;

	// Is this FX supposed to be motion linked to another FX?
	if (pKey->m_bLinked)
	{
		CLinkListNode<FX_LINK>	*pNode = pInst->m_collActiveFX.GetHead();

		while (pNode)
		{
			if (pNode->m_Data.m_dwID == pKey->m_dwLinkedID)
			{
				// This is the one !!!
				if (pInst->ExistFX(pNode->m_Data.m_pFX))
				{
					CBaseFX	 *pMotionLinkFX	= pNode->m_Data.m_pFX;
					fxData.m_hParent = pMotionLinkFX->GetFXObject();
				}

				// done
				break;
			}
			
			pNode = pNode->m_pNext;
		}
	}

	// Create the FX
	CBaseFX *pNewFX = CreateFX(pKey->m_pFxRef->m_sName, &fxData, pKey->m_pProps, pInst->m_hParent);

	if( pNewFX )
	{
		pNewFX->SetVisible(false);
		pNewFX->ClearState(FS_INITIALFRAME | FS_ACTIVE | FS_SHUTTINGDOWN | FS_SUSPENDED);

		// Add it onto the list for link referencing

		FX_LINK	fxLink;
		fxLink.m_dwID = pKey->m_dwID;
		fxLink.m_pFX  = pNewFX;
		fxLink.m_pRef = pKey;

		pInst->m_collActiveFX.AddHead(fxLink);
	}
	else
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : GetUniqueID()
//
//   PURPOSE  : Returns a unique ID
//
//------------------------------------------------------------------

uint32 CClientFXMgr::GetUniqueID()
{
	return g_dwID ++;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSpecialEffectNotify()
//
//   PURPOSE  : OnSpecialEffectNotify
//
//------------------------------------------------------------------

void CClientFXMgr::OnSpecialEffectNotify(HOBJECT hObject, ILTMessage_Read *pMsg)
{
	uint32		dwServerID = 0;
	char		sName[256];
	uint32		dwFxFlags;
	LTVector	vPos;
	LTRotation	rRot;
	uint8		nId;
	bool		bUseTargetData;
	HOBJECT		hTargetObj = LTNULL;
	LTVector	vTargetPos(0.0f, 0.0f, 0.0f);
	bool		bStartInst = true;

	// Read in the type of client fx
	nId = pMsg->Readuint8();

	switch( nId )
	{
		case SFX_CLIENTFXGROUP :
		{
			if( !hObject )
				return;

			// Retrieve the ID of the object
			
			pMsg->ReadString(sName, sizeof(sName));
			dwFxFlags = pMsg->Readuint32();

			bUseTargetData = !!(pMsg->Readuint8());
			if( bUseTargetData )
			{
				hTargetObj	= pMsg->ReadObject();
				vTargetPos	= pMsg->ReadCompPos();
			}

			m_pClientDE->GetObjectPos(hObject, &vPos);
			m_pClientDE->GetObjectRotation(hObject, &rRot);

			uint32 nUserFlags;
			m_pClientDE->Common()->GetObjectFlags(hObject, OFT_User, nUserFlags);
			if(hObject && !(nUserFlags & USRFLG_SFX_ON))
			{
				bStartInst = false;
			}

		}
		break;

		case SFX_CLIENTFXGROUPINSTANT :
		{
			// Retrieve the ID of the object

			pMsg->ReadString(sName, sizeof(sName));
			dwFxFlags = pMsg->Readuint32();
			vPos = pMsg->ReadCompPos();
			rRot = pMsg->ReadCompLTRotation();
			hObject = pMsg->ReadObject();

			bUseTargetData = !!(pMsg->Readuint8());
			if( bUseTargetData )
			{
				hTargetObj = pMsg->ReadObject();
				vTargetPos = pMsg->ReadCompPos();
			}
		}
		break;

		default:
			return;
	}

	// If we got here we don't yet have this special FX so we have to start it running

	CLIENTFX_CREATESTRUCT fxcs(sName, dwFxFlags, vPos, rRot);
	fxcs.m_hParent			= hObject;
	fxcs.m_bUseTargetData	= bUseTargetData;
	fxcs.m_hTarget			= hTargetObj;
	fxcs.m_vTargetPos		= vTargetPos;

	CLIENTFX_INSTANCE *pNewInst = CreateClientFX(fxcs, bStartInst);
	if (!pNewInst) 
		return;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnObjectRemove()
//
//   PURPOSE  : Removes an object
//
//------------------------------------------------------------------

bool CClientFXMgr::OnObjectRemove(HOBJECT hObject)
{
	CLinkListNode<CLIENTFX_INSTANCE *> *pNode = m_collActiveGroupFX.GetHead();

	while (pNode)
	{
		CLIENTFX_INSTANCE *pInst = pNode->m_Data;

		if (pInst->m_hParent == hObject)
		{		
			ShutdownClientFX(pInst);
		}
		
		pNode = pNode->m_pNext;
	}

	return false;
}

//------------------------------------------------------------------
//
//   FUNCTION : SuspendInstance()
//
//   PURPOSE  : Freezes all the FX associated with this instance
//
//------------------------------------------------------------------

void CClientFXMgr::SuspendInstance(CLIENTFX_INSTANCE *pInst)
{
	assert( pInst );

	//see if we are already suspended
	if( pInst->IsSuspended() ) 
		return;

	//freeze our instance
	pInst->m_bSuspended = true;

	// supsend all the FX in this instance
	CLinkListNode<FX_LINK> *pNode = pInst->m_collActiveFX.GetHead();

	while (pNode)
	{
		pNode->m_Data.m_pFX->SetState(FS_SUSPENDED);		

		//handle the visible flag
		if(!pNode->m_Data.m_pFX->IsVisibleWhileSuspended())
			pNode->m_Data.m_pFX->SetVisible(false);

		pNode = pNode->m_pNext;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : UnfreezeInstance()
//
//   PURPOSE  : Unfreezes all the FX associated with this instance
//
//------------------------------------------------------------------

void CClientFXMgr::UnsuspendInstance(CLIENTFX_INSTANCE *pInst)
{
	assert( pInst );

	//only bother if we were actually suspended
	if(!pInst->IsSuspended())
		return;

	//clear out the flag
	pInst->m_bSuspended = false;

	//now unsuspend each of our effects, and let them compensate for lost time
	CLinkListNode<FX_LINK> *pNode = pInst->m_collActiveFX.GetHead();

	while (pNode)
	{
		pNode->m_Data.m_pFX->ClearState(FS_SUSPENDED);

		if(pNode->m_Data.m_pFX->IsActive())
			pNode->m_Data.m_pFX->SetVisible(true);

		pNode = pNode->m_pNext;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : SetGroupParent()
//
//   PURPOSE  : Sets a parent for all FX in a group
//
//------------------------------------------------------------------

void CClientFXMgr::SetGroupParent(CLIENTFX_INSTANCE *pInstance, HOBJECT hParent)
{
	CLinkListNode<FX_LINK> *pLinkNode = pInstance->m_collActiveFX.GetHead();

	while (pLinkNode)
	{
		pLinkNode->m_Data.m_pFX->SetParent(hParent);

		pLinkNode = pLinkNode->m_pNext;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRendererShutdown()
//
//   PURPOSE  : Informs each FX that the renderer just shut down
//
//------------------------------------------------------------------

void CClientFXMgr::OnRendererShutdown()
{
	CLinkListNode<CLIENTFX_INSTANCE *> *pNode = m_collActiveGroupFX.GetHead();

	while (pNode)
	{
		CLinkListNode<FX_LINK> *pLinkNode = pNode->m_Data->m_collActiveFX.GetHead();

		while (pLinkNode)
		{
			pLinkNode->m_Data.m_pFX->OnRendererShutdown();

			pLinkNode = pLinkNode->m_pNext;
		}

		pNode = pNode->m_pNext;
	}
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientFXMgr::SetDetailLevel
//
//  PURPOSE:	Sets the level of detail for all ClientFX
//
// ----------------------------------------------------------------------- //

void CClientFXMgr::SetDetailLevel( int nLOD )
{
	nLOD = LTCLAMP(nLOD, FXLOD_LOW, FXLOD_HIGH);

	// Set our detail vars
	switch( nLOD )
	{
		case 0 : 
			{
				m_eDetailLevel		= FXLOD_LOW;
			}
			break;

		case 1 :
			{
				m_eDetailLevel		= FXLOD_MED;
			}
			break;

		default :
			{
				m_eDetailLevel		= FXLOD_HIGH;
			}
			break;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientFXMgr::SetDisableDistances
//
//  PURPOSE:	Sets the distances that effect are disabled at if the 
//					disable at dist flag is set
//
// ----------------------------------------------------------------------- //

void CClientFXMgr::SetDisableDistances( float fLow, float fMed, float fHigh )
{
	SetDisableDistance(FXLOD_LOW, fLow);
	SetDisableDistance(FXLOD_MED, fMed);
	SetDisableDistance(FXLOD_HIGH, fHigh);
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientFXMgr::SetDisableDistance
//
//  PURPOSE:	Sets the distances that effect are disabled at if the 
//					disable at dist flag is set
//
// ----------------------------------------------------------------------- //

void CClientFXMgr::SetDisableDistance( int nLOD, float fDistance )
{
	nLOD = LTCLAMP(nLOD, FXLOD_LOW, FXLOD_HIGH);

	m_dDetailDistSqr[nLOD] = fDistance * fDistance;

}


float CClientFXMgr::GetFrameTime()
{
	if (m_bUseSystemTime)
		return GetSystemFrameTime(m_nSystemTimeBase, m_nPrevSystemTime);
	else
		return g_pLTClient->GetFrameTime();
}

void CClientFXMgr::Pause(bool bPause)
{
	if(bPause == IsPaused())
		return;

	//we need to run through all active instances and let all of their effects pause
	CLinkListNode<CLIENTFX_INSTANCE *> *pNode = m_collActiveGroupFX.GetHead();
	while( pNode )
	{
		CLinkListNode<FX_LINK> *pLinkNode = pNode->m_Data->m_collActiveFX.GetHead();
		while( pLinkNode )
		{
			pLinkNode->m_Data.m_pFX->Pause(bPause);

			pLinkNode = pLinkNode->m_pNext;
		}

		pNode = pNode->m_pNext;
	}

	m_bPaused = bPause;
}


//given a detail level of an effect, this will determine if the effect key should
//be played based upon the current LOD settings on the object
bool CClientFXMgr::IsDetailLevelEnabled(uint32 nDetailLevel)
{
	//helper table for the different detail settings on an effect. Each has 3 bools, one for
	//if it is enabled in low detail level, medium, and high. Note that this table must match
	//the prop settings as listed in clientfx.cpp
	static bool	bDetailTable[FX_NUM_DETAIL_SETTINGS][3]	=
				{
					{ true, true, true },		//All
					{ false, false, true },		//High
					{ false, true, false },		//Medium
					{ true, false, false },		//Low
					{ false, true, true },		//Medium+High
					{ true, true, false },		//Low+Medium
					{ true, false, true }		//Low+High
				};

	//check for out of bounds detail levels
	if(nDetailLevel >= FX_NUM_DETAIL_SETTINGS)
		return true;

	//use the table as a guide
	return bDetailTable[nDetailLevel][m_eDetailLevel];
}
