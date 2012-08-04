#include "stdafx.h"
#include "ClientFXInstance.h"
#include "ClientFXDB.h"

//-------------------------------------------------------------------------------------------
// CClientFXLink
//-------------------------------------------------------------------------------------------

//called to set the effect that this link points to
void CClientFXLink::SetLink(CClientFXInstance* pInstance)
{
	ClearLink();

	if(pInstance)
	{
		if(pInstance->m_pLink)
		{
			LTERROR( "Warning: Tried to bind a new link to an effect with an existing link");
			m_pInstance->m_pLink->ClearLink();
		}

		m_pInstance = pInstance;
		pInstance->m_pLink = this;
	}
}

void CClientFXLink::ClearLink()
{
	//the instance should always point to us if it points to anything
	if(m_pInstance)
	{
		if(m_pInstance->m_pLink == this)
		{
            m_pInstance->m_pLink = NULL;
		}
		else
		{
			LTERROR( "Warning: Corrupted ClientFXLink found");
		}
	}
	
	m_pInstance = NULL;
};

//-------------------------------------------------------------------------------------------
// CClientFXInstance
//-------------------------------------------------------------------------------------------

CClientFXInstance::CClientFXInstance()
:	
	m_tmElapsed( 0.0f ),
	m_tmSuspended( 0.0f ),
	m_fDuration( 0.0f ),
	m_hParent( NULL ),
	m_hTarget( NULL ),
	m_bLoop( false ),
	m_bSmoothShutdown( true ),
	m_bShutdown( false ),
	m_bSuspended( false ),
	m_bShow( false ),
	m_pLink( NULL)
{
	m_hParent.SetReceiver(*this);
	m_hTarget.SetReceiver(*this);
	m_FXListLink.SetData(this);
}

CClientFXInstance::~CClientFXInstance()
{
	ClearLink();

	//make sure to free all of our effects
	RemoveAllEffects();

	//make sure to release our reference to the rigid body
#ifndef _FINAL
	g_pLTClient->PhysicsSim()->ReleaseRigidBody(m_CreateStruct.m_hParentRigidBody);
#endif

	//remove our link
	m_FXListLink.Remove();
}

// Implementing classes will have this function called
// when HOBJECT ref points to gets deleted.
void CClientFXInstance::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if(pRef == &m_hParent)
	{
		//our parent is going away, we need to shutdown our effect, and update the effects to no
		//longer refere to that parent
		Shutdown();

		//run through our remaining effects and clear out any references to this parent object
		LTListIter<CBaseFX*> itActiveFX = m_ActiveFXList.Begin();
		while(itActiveFX != m_ActiveFXList.End())
		{
			CBaseFX *pFX = *itActiveFX;
			itActiveFX++;

			//set it to no longer track an object, but preserve the current position in space if
			//that is our parent (but only for those that have a matching parent, which isn't always
			//the case such as when a key is motion linked to another key)
			if(pFX->GetParentObject() == hObj)
			{
				pFX->SetParent(NULL, INVALID_MODEL_NODE, INVALID_MODEL_SOCKET, pFX->GetParentTransform());
			}
		}
	}
	else if(pRef == &m_hTarget)
	{
		//run through our remaining effects and clear out any references to this target object
		LTListIter<CBaseFX*> itActiveFX = m_ActiveFXList.Begin();
		while(itActiveFX != m_ActiveFXList.End())
		{
			CBaseFX *pFX = *itActiveFX;
			itActiveFX++;

			//let the effect handle the target going away
			pFX->ReleaseTargetObject(hObj);
		}
	}
}

//called to shutdown the effect, removing finished effects, and placing appropriate
//effects into a shutdown state so that they can properly finish.
void CClientFXInstance::Shutdown()
{
	// Shut it down !!
	m_bShutdown = true;
    
	//if we aren't performing a smooth shutdown, just destroy our object
	if (!m_bSmoothShutdown)
	{
		RemoveAllEffects();
	}
	else
	{
		//we are performing a smooth shutdown, so run through and shutdown the effects that we
		//can, and set the others into a smooth shutdown state
		LTListIter<CBaseFX*> itActiveFX = m_ActiveFXList.Begin();
		while(itActiveFX != m_ActiveFXList.End())
		{
			//cache the next in case this node is deleted
			CBaseFX* pFX = *itActiveFX;
			itActiveFX++;

			//we want to remove any inactive nodes
			if(!pFX->IsActive())
			{
				DeleteFX(pFX);
			}
			else
			{
				//we have an active effect, set it to shutting down
				pFX->SetState(FS_SHUTTINGDOWN);

				//now that it is shutting down, see if it is complete
				if(pFX->IsFinishedShuttingDown() || !pFX->GetSmoothShutdown())
				{
					//we can just remove the effect
					DeleteFX(pFX);
				}
			}
		}
	}
}

//freezes all the FX associated with this instance
void CClientFXInstance::Suspend()
{
	//see if we are already suspended
	if( IsSuspended() ) 
		return;

	//freeze our instance
	m_bSuspended = true;

	// supsend all the FX in this instance
	LTListIter<CBaseFX*> itFX = m_ActiveFXList.Begin();
	while (itFX != m_ActiveFXList.End())
	{
		CBaseFX* pFX = *itFX;
		itFX++;

		pFX->SetState(FS_SUSPENDED);		

		//handle the visible flag
		if(!pFX->IsVisibleWhileSuspended())
			pFX->SetVisible(false);
	}
}

//unfreezes all the FX associated with this instance
void CClientFXInstance::Unsuspend()
{
	//only bother if we were actually suspended
	if(!IsSuspended())
		return;

	//clear out the flag
	m_bSuspended = false;

	//now unsuspend each of our effects, and let them compensate for lost time
	LTListIter<CBaseFX*> itActiveFX = m_ActiveFXList.Begin();
	while(itActiveFX != m_ActiveFXList.End())
	{
		CBaseFX* pFX = *itActiveFX;
		itActiveFX++;

		pFX->ClearState(FS_SUSPENDED);
		if(pFX->IsActive())
			pFX->SetVisible(true);
	}
}

//called to update the parent information for this effect, and all the effect keys contained.
//This can be in node/socket space, object space, or world space depending upon which parameters
//are provided. The transform is in the corresponding space. This will update even fixed
//effect positions.
void CClientFXInstance::SetParent(HOBJECT hObject, HMODELNODE hNode, HMODELSOCKET hSocket, const LTRigidTransform& tTransform)
{
	//update our own parent handle
	m_hParent = hObject;

	//now run through and update the parent information on each of the children
	LTListIter<CBaseFX*> itActiveFX = m_ActiveFXList.Begin();
	while(itActiveFX != m_ActiveFXList.End())
	{
		CBaseFX* pFX = *itActiveFX;
		itActiveFX++;

		pFX->SetParent(hObject, hNode, hSocket, tTransform);
	}
}

//same as above, but will set the parent of the effects to the provided physics rigid body. This will
//hold onto those rigid bodies until the effects are destroyed or have a new parent set
void CClientFXInstance::SetParent(HPHYSICSRIGIDBODY hParent, const LTRigidTransform& tTransform)
{
	//update our own parent handle
	m_hParent = NULL;

	//now run through and update the parent information on each of the children
	LTListIter<CBaseFX*> itActiveFX = m_ActiveFXList.Begin();
	while(itActiveFX != m_ActiveFXList.End())
	{
		CBaseFX* pFX = *itActiveFX;
		itActiveFX++;

		pFX->SetParent(hParent, tTransform);
	}
}

bool CClientFXInstance::ExistFX(CBaseFX *pExistFX)
{
	LTListIter<CBaseFX*> itActiveFX = m_ActiveFXList.Begin();
	while(itActiveFX != m_ActiveFXList.End())
	{
		CBaseFX* pFX = *itActiveFX;
		itActiveFX++;

		if (pFX == pExistFX) 
			return true;
	}

	return false;
}

//are all FX inactive?
bool CClientFXInstance::IsDone()
{
	return (m_ActiveFXList.IsEmpty());
}

//is this suspended
bool CClientFXInstance::IsSuspended() const
{
	return m_bSuspended;
}

void CClientFXInstance::Hide()
{
	// Force us to be hidden
	m_bShow = false;
}

void CClientFXInstance::Show()
{
	// Force us to be shown
	m_bShow = true;
}

void CClientFXInstance::ClearLink()
{
	//delegate this out to our link if we have one and tell it to break the connection
	if (m_pLink)
	{
		//if we have a link, the link had better point to us, otherwise something
		//has gotten horribly out of sync
		LTASSERT((m_pLink->GetInstance() == this), "Corrupted ClientFXLink found");

		//clear the connection. This will clear out our side as well
		m_pLink->ClearLink();

		//this disconnection should clear out our link
		LTASSERT((m_pLink == NULL), "Corrupted ClientFXLink found");
	}
}

void CClientFXInstance::DeleteFX(CBaseFX* pDelFX)
{
	//we don't need to worry about this FX being the parent of another FX since when that object
	//is removed, the LTObjRef link will be broken and the parent link will be cleaned up
	//so we can save time by not bothering to check

	// Give the FX a chance to clean itself up
	pDelFX->Term();
	pDelFX->m_FXListLink.Remove();
	CClientFXDB::GetSingleton().DeleteEffect(pDelFX);
}
	


void CClientFXInstance::RemoveAllEffects()
{
	//we need to run through all effects and delete them
	LTListIter<CBaseFX*> itActiveFX = m_ActiveFXList.Begin();
	while(itActiveFX != m_ActiveFXList.End())
	{
		CBaseFX *pFX = *itActiveFX;
		itActiveFX++;

		DeleteFX(pFX);
	}
}


//------------------------------------------------------------------
//
//   FUNCTION : UpdateSuspended()
//
//   PURPOSE  : Updates the suspended status of the instance and returns the status
//
//------------------------------------------------------------------

bool CClientFXInstance::UpdateSuspended()
{
	// Check our parent to see if we are turned off or on
	bool bIsParentOn = false;
	if(m_hParent)
	{
		uint32 dwUsrFlags = 0;
		g_pCommonLT->GetObjectFlags( m_hParent, OFT_User, dwUsrFlags );
		bIsParentOn = (dwUsrFlags & USRFLG_SFX_ON) != 0;
	}

	//first off see if we should be suspended based upon our flags and our parents flags
	if( !bIsParentOn && !m_bShow )
	{
		//neither us or our parent is turned on, so suspend us
		Suspend();
	}
	else
	{
		//either us or our parent is on, so assume we are unsuspended.
		Unsuspend();
	}

	//return our result
	return IsSuspended();
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
void CClientFXInstance::HandleShutdownEffect(CBaseFX* pFX)
{
	//we are finished shutting down, if we aren't looping, we need to destroy
	//the effect, otherwise just disable it (however, if a continuous effect was shut down,
	//we never want to restart it, so just destroy it)
	if(m_bLoop && !m_bShutdown && !pFX->IsContinuous())
	{
		pFX->SetVisible(false);
		pFX->ClearState(FS_ACTIVE | FS_SHUTTINGDOWN | FS_INITIALFRAME);
	}
	else
	{
		DeleteFX(pFX);
	}
}


//------------------------------------------------------------------
//
//   FUNCTION : UpdateInterval()
//
//   PURPOSE  : Given an instance and a time interval, this will appropriately update
//				all effects contained within that interval
//
//------------------------------------------------------------------
void CClientFXInstance::UpdateInterval(float fStartInterval, float fEndInterval)
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

	//determine if this is a looping instance
	bool bLoopInstance = m_bLoop;

	LTListIter<CBaseFX*> itActiveFX = m_ActiveFXList.Begin();
	while(itActiveFX != m_ActiveFXList.End())
	{
		//alright, first off look at the state of the key and see what to do
		CBaseFX *pFX = *itActiveFX;
		itActiveFX++;

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
			assert(!m_bShutdown);

			//figure out the start time of this effect
			float fKeyStart = pFX->GetStartTime();

			//this effect is not active. See if it should be 
			if((fStartInterval <= fKeyStart) && (fEndInterval > fKeyStart))
			{
				//our effect has just become active, we need to reset its elapsed time to 0,
				//and switch it over to an initial frame
				pFX->SetElapsed(0.0f);
				pFX->SetState(FS_ACTIVE | FS_INITIALFRAME);
				pFX->ClearState(FS_SHUTTINGDOWN);
				pFX->SetVisible(true);

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
				float fKeyStart = pFX->GetStartTime();

				//however, we can only bring it into the active state from here if the instance
				//will allow us
				bool bCanActivate = m_bLoop && !m_bShutdown;

				//this effect is not active. See if it should be
				if(bCanActivate && (fKeyStart >= fCurrStart) && (fKeyStart <= fEndInterval))
				{
					//it will become active in this range, so move up to there and change state
					pFX->Update(fKeyStart - fCurrStart);

					pFX->SetElapsed(0.0f);
					pFX->SetState(FS_INITIALFRAME | FS_ACTIVE);
					pFX->ClearState(FS_SHUTTINGDOWN);

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
				bool bCompleteKey	= ((pFX->GetEndTime() <= fTimeBlockEnd) && (pFX->GetEndTime() >= fCurrStart));

				//see if we hit the end of the key
				if(bCompleteKey)
				{
					fTimeBlockEnd = pFX->GetEndTime();
				}

				//update based upon the interval length
				if(!pFX->Update(fTimeBlockEnd - fCurrStart))
				{
					//they returned false, so they want to enter the shutting down state
					pFX->SetState(FS_SHUTTINGDOWN);
				}
				else if(bCompleteKey)
				{
					//we completed the key, so now switch to a shutting down state if it isn't continually looping,
					//otherwise we need to reset the elapsed time to 0
					if(pFX->IsContinuous() && bLoopInstance)
					{
						//no starting offset, so just to make sure that everything syncs
						//up correctly and no error gets introduced, reset it to 0
						pFX->SetElapsed(0.0f);
					}
					else
					{
						//we're past our key, so start shutting down
						pFX->SetState(FS_SHUTTINGDOWN);
					}
				}

				//the initial update state should be cleared now
				pFX->ClearState(FS_INITIALFRAME);

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
				bool bSmoothShutdown = pFX->GetSmoothShutdown() && m_bSmoothShutdown;

				//see if this effect is done shutting down
				if(pFX->IsFinishedShuttingDown() || !bSmoothShutdown)
				{
					//notify of an effect that has finished shutting down
					HandleShutdownEffect(pFX);
					continue;
				}
			}
		}
	}
}

#ifndef _FINAL

void CClientFXInstance::SetCreateStruct(const CLIENTFX_CREATESTRUCT& cs)
{
	// Release the old parent reference AFTER setting up the new create 
	// struct.  If the current m_hParentRigidBody is the same as the 
	// m_hParentRigidBody in the passed in cs, we could release the last 
	// reference.

	HPHYSICSRIGIDBODY hOldParent = m_CreateStruct.m_hParentRigidBody;

	m_CreateStruct = cs;

	//make sure to add a reference onto our rigid body so it doesn't go away
	g_pLTClient->PhysicsSim()->AddRigidBodyRef(m_CreateStruct.m_hParentRigidBody);

	// free any existing rigid body reference we might have
	g_pLTClient->PhysicsSim()->ReleaseRigidBody(hOldParent);
}

#endif
