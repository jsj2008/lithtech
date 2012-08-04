
#include "bdefs.h"
#include "animtracker.h"
#include "model_ops.h"
#include "ltsysoptim.h"


extern int32 g_CV_ModelTransitionMS;

void trk_ScanToKeyFrame(LTAnimTracker *pTracker, uint32 msDelta, bool bProcessKeys);


//  ----------------------------------------------------------------
//  Initialize the tracker with animation from model
//  ----------------------------------------------------------------
void trk_Init(LTAnimTracker *pTracker, Model *pModel, uint32 iAnim)
{
	pTracker->m_StringKeyCallback = NULL;
	pTracker->SetModel(pModel);
	pTracker->m_CurKey = 0;
	pTracker->m_Flags = 0;
	pTracker->m_InterpolationMS = 0;
	pTracker->m_TimeRef.m_Prev.Clear();
	pTracker->m_TimeRef.m_Cur.Clear();
	pTracker->m_TimeRef.m_Percent = 0.0f;
	pTracker->m_RateModifier = 1.0f ;
	pTracker->m_hHintNode = INVALID_MODEL_NODE;
	pTracker->m_hLastHintAnim = INVALID_MODEL_ANIM;
	pTracker->m_dwLastHintTime = 0;
    
	if(pModel)
	{
		//see if this animation index is out of our range. If so, wrap it back to 0 unless
		//we allow invalid animations
		if(!pTracker->AllowInvalid() && (iAnim >= pModel->NumAnims()))
			iAnim = 0;

		pTracker->m_TimeRef.m_Prev.m_iAnim = pTracker->m_TimeRef.m_Cur.m_iAnim = (uint16)iAnim;
		pTracker->m_TimeRef.m_Prev.m_iFrame = pTracker->m_TimeRef.m_Cur.m_iFrame = 0;

		ASSERT(pTracker->IsValid());
		
		pTracker->m_Flags |= AT_PLAYING;
		pTracker->m_InterpolationMS = NOT_INTERPOLATING;
	}

	pTracker->m_Flags |= AT_LOOPING;
}


//  ----------------------------------------------------------------
//  Increment the tracker by dt
//  ----------------------------------------------------------------
void trk_Update(LTAnimTracker *pTracker, uint32 msDelta)
{
	if(pTracker->m_Flags & AT_PLAYING)
	{
        trk_ScanToKeyFrame(pTracker, (uint32)(msDelta * pTracker->m_RateModifier), TRUE);
	}
}


bool trk_IsStopped(LTAnimTracker *pTracker)
{
	//if we are invalid we are stopped
	if(!pTracker->IsValid())
	{
		return true;
	}

	//if we are looping we are never stopped
	if(pTracker->m_Flags & AT_LOOPING)
	{
		return false;
	}

	//figure out the animation that we are playing
	ModelAnim *pAnim = pTracker->GetCurAnim();

	//if we have no actual length to our animation we are stopped
	if(pAnim->m_KeyFrames.GetSize() <= 1)
	{
		return true;
	}

	//now determine if our current time has gone beyond the end of the animation
	
	//figure out the final time of our animation
	uint32 iEndKey = pAnim->m_KeyFrames.GetSize() - 1;
	uint32 endTime = pAnim->m_KeyFrames[iEndKey].m_Time;

	//see if we are beyond that time
	if(pTracker->m_TimeRef.m_Cur.m_Time > endTime)
		return true;

	//we are a single play animation but are currently still playing
	return false;
}

void trk_SetCurTime(LTAnimTracker *pTracker, uint32 msTime, bool bTransition)
{
	// If we're not transitioning, then prev should move to the same keyframe as cur.
	if(!bTransition || !pTracker->IsValid())
	{
		pTracker->m_TimeRef.m_Prev.m_Time = 0;
		pTracker->m_TimeRef.m_Prev.m_iFrame = 0;
	}

	pTracker->m_TimeRef.m_Cur.m_Time = 0;
	pTracker->m_TimeRef.m_Cur.m_iFrame = 0;
	pTracker->m_CurKey = 0;

	trk_ScanToKeyFrame(pTracker, msTime, FALSE);
}


bool trk_SetCurAnim(LTAnimTracker *pTracker, uint32 iAnim, bool bTransition)
{
	if(!pTracker->AllowInvalid())
	{
		if(iAnim >= pTracker->GetModel()->NumAnims())
			return FALSE;
	}

	pTracker->m_TimeRef.m_Prev = pTracker->m_TimeRef.m_Cur;

	pTracker->m_TimeRef.m_Percent = 0.0f;
	pTracker->m_TimeRef.m_Cur.m_iAnim = (uint16)iAnim;
	pTracker->m_CurKey = 0;
	pTracker->m_TimeRef.m_Cur.m_iFrame = 0;
	pTracker->m_TimeRef.m_Cur.m_Time = 0;

	if(bTransition && pTracker->IsValid())
	{
		// Start the interpolation.
		pTracker->m_InterpolationMS = 0;
	}
	else
	{
		pTracker->m_InterpolationMS = NOT_INTERPOLATING;
		pTracker->m_TimeRef.m_Prev = pTracker->m_TimeRef.m_Cur;
	}
	
	return TRUE;
}


void trk_Reset(LTAnimTracker *pTracker)
{
	pTracker->m_TimeRef.m_Prev.m_iAnim	= pTracker->m_TimeRef.m_Cur.m_iAnim;
	pTracker->m_TimeRef.m_Prev.m_iFrame = 0;
	pTracker->m_TimeRef.m_Prev.m_Time	= 0;
	pTracker->m_TimeRef.m_Cur.m_iFrame	= 0;
	pTracker->m_TimeRef.m_Cur.m_Time	= 0;

	pTracker->m_TimeRef.m_Percent = 0.0f;
	pTracker->m_CurKey = 0;
}


static AnimKeyFrame* trk_NextPositionFrame(LTAnimTracker *pTracker, uint32 iStart, uint32 &iNextFrame)
{
	uint32 i;

	ASSERT(pTracker->IsValid());

	ModelAnim *pAnim = pTracker->GetCurAnim();
	if(pAnim)
	{
		for(i=iStart; i < pAnim->m_KeyFrames; i++)
		{
			iNextFrame = i;
			return &pAnim->m_KeyFrames[i];
		}
	}

	return NULL;
}

// ----------------------------------------------------------------
// ProcessKey
// Process keyframes that have callbacks associated with them.
// ----------------------------------------------------------------
static void trk_ProcessKey(LTAnimTracker *pTracker, ModelAnim *pAnim, uint32 iFrame)
{
	AnimKeyFrame *pNextPosition;
	uint32 iNextPosition;
	AnimKeyFrame *pFrame;


	pFrame = &pAnim->m_KeyFrames[iFrame];	

	// Is there a string for this key?
	if(pTracker->m_StringKeyCallback != NULL && pFrame->m_pString[0] != 0)
	{
		// Save the current keyframe
		AnimKeyFrame *pOldKeyFrame = pTracker->GetCurFrame();
		pTracker->m_StringKeyCallback(pTracker, pFrame);
		// Jump out if they changed it on us...
		if (pTracker->GetCurFrame() != pOldKeyFrame)
			return;
	}

	pNextPosition = trk_NextPositionFrame(pTracker, iFrame+1, iNextPosition);
	if(pNextPosition)
	{
		if(pTracker->m_TimeRef.m_Cur.m_Time <= (uint32)pNextPosition->m_Time)
		{
			pTracker->m_TimeRef.m_Prev.m_iFrame = (uint16)iFrame;
			pTracker->m_TimeRef.m_Cur.m_iFrame = pNextPosition - pAnim->m_KeyFrames.GetArray();
		}
		else
		{
			pTracker->m_TimeRef.m_Prev.m_iFrame = pTracker->m_TimeRef.m_Cur.m_iFrame = 
				pNextPosition - pAnim->m_KeyFrames.GetArray();
		}
	}
	else
	{
		pTracker->m_TimeRef.m_Prev.m_iFrame = pTracker->m_TimeRef.m_Cur.m_iFrame = (uint16)iFrame;
	}

	ASSERT(pTracker->IsValid());
}

// ----------------------------------------------------------------
// UpdatePositionInterpolant( tracker )
// Finds the position between key frames. For doing linear/spherical
// interpolation between keyframes.
// ----------------------------------------------------------------

float* g_fDebug = NULL;

inline void trk_UpdatePositionInterpolant(LTAnimTracker *pTracker)
{
	AnimKeyFrame *pPrevFrame, *pCurFrame;

	// We're not good, just go to the beginning.
	if( !pTracker->IsValid( ))
	{
		pTracker->m_TimeRef.m_Percent = 0.0f;
		return;
	}

	// We reached the end of the keyframe.
	if( pTracker->m_TimeRef.m_Prev.m_iFrame == pTracker->m_TimeRef.m_Cur.m_iFrame )
	{
		// If we're looping, then wrap to the first keyframe.
		if( pTracker->m_Flags & AT_LOOPING )
		{
			// Get the current keyframe.
			pCurFrame = pTracker->GetCurFrame();

			// The percentage is just from time 0 of the first keyframe.
			if( pCurFrame->m_Time != 0 )
			{
				pTracker->m_TimeRef.m_Percent = ( float )pTracker->m_TimeRef.m_Cur.m_Time / pCurFrame->m_Time;
				pTracker->m_TimeRef.m_Percent = Min( pTracker->m_TimeRef.m_Percent, 1.0f );
			}
			else
				pTracker->m_TimeRef.m_Percent = 0.0f;
		}
		// Not looping, so just consider us at 100%.
		else
		{
			pTracker->m_TimeRef.m_Percent = 1.0f;
		}
	}
	// We're in between two keyframes.
	else
	{
		pPrevFrame = pTracker->GetPrevFrame();
		pCurFrame = pTracker->GetCurFrame();

		// Calculate the percentage into the current keyframe.
		pTracker->m_TimeRef.m_Percent = 
			(float)(pTracker->m_TimeRef.m_Cur.m_Time - pPrevFrame->m_Time) / 
			(pCurFrame->m_Time - pPrevFrame->m_Time);

	}
}

// ----------------------------------------------------------------
// SetAtKeyFrame( tracker, time );
// sets the animation to the keyframe closest to msTime
// ----------------------------------------------------------------
void trk_SetAtKeyFrame(LTAnimTracker *pTracker, uint32 msTime)
{
	//clear out anything old
	trk_Reset(pTracker);

	//get the current animation that is being played
	ModelAnim *pCurAnim = pTracker->GetCurAnim();

	//see if we even have any keys to set to
	if(pCurAnim->m_KeyFrames.GetSize() <= 1)
		return;
		
	uint32 iEndKey = pCurAnim->m_KeyFrames.GetSize() - 1;

	// Clamp the time that we are setting to the maximum time of the animation
	uint32 endTime = pCurAnim->m_KeyFrames[iEndKey].m_Time; 
	if ( msTime > endTime ) 
	{
		msTime = endTime;
	}
	
	if(!(pTracker->m_Flags & AT_LOOPING))
	{
		pTracker->m_TimeRef.m_Cur.m_Time = MIN(pTracker->m_TimeRef.m_Cur.m_Time, (endTime+1));
	}

	// Now find the keyframes..
	uint32 lastkeyTime = 0;
	uint32 keyTime;
	while(pTracker->m_CurKey <= iEndKey)
	{
		keyTime = pCurAnim->m_KeyFrames[pTracker->m_CurKey].m_Time;
		if(msTime <= keyTime)
		{
			pTracker->m_TimeRef.m_Prev.m_iFrame = (uint16)pTracker->m_CurKey > 0 ? (uint16)pTracker->m_CurKey-1 : 0;
			pTracker->m_TimeRef.m_Cur.m_iFrame = (uint16)pTracker->m_CurKey;
			// Compute the percent into the next keyframe (if there is one).  Avoid any divide by zero's.
			if ( (keyTime - lastkeyTime) == 0 ) 
			{
				pTracker->m_TimeRef.m_Percent = 0.0f;
			}
			else
			{
				pTracker->m_TimeRef.m_Prev.m_Time = lastkeyTime;	
				pTracker->m_TimeRef.m_Cur.m_Time = msTime;
				pTracker->m_TimeRef.m_Percent = (LTFLOAT)(msTime - lastkeyTime)/(LTFLOAT)(keyTime - lastkeyTime);
				ASSERT((pTracker->m_TimeRef.m_Percent >= 0.0f) && (pTracker->m_TimeRef.m_Percent <= 1.0f));
			}

			break;
		}

		lastkeyTime = keyTime;
		++pTracker->m_CurKey;
	}

	ASSERT(pTracker->IsValid());
}

//  ----------------------------------------------------------------
//  ScanToKeyFrame
// Private function to update the anims in the trackers.
//  ----------------------------------------------------------------
void trk_ScanToKeyFrame(LTAnimTracker *pTracker, uint32 msDelta, bool bProcessKeys)
{
	// Handle if it's invalid.
	if(!pTracker->IsValid())
	{   // ok no anims, but let's up the time anyway.
        pTracker->m_TimeRef.m_Cur.m_Time += msDelta;
		return;
	}

	ModelAnim *pCurAnim = pTracker->GetCurAnim();

	// Are we interpolating between two animations?
	if(pTracker->m_InterpolationMS != NOT_INTERPOLATING)
	{
		//see if we really shouldn't be interpolating though
		if(pCurAnim->m_InterpolationMS == 0)
		{
			//we need to stop the interpolation of this
			pTracker->m_InterpolationMS = NOT_INTERPOLATING;
			pTracker->m_TimeRef.m_Prev = pTracker->m_TimeRef.m_Cur;
		}
		else
		{
			pTracker->m_InterpolationMS += msDelta;
			// if we are done with interp, switch to next anim, move on.
			if(pTracker->m_InterpolationMS > pCurAnim->m_InterpolationMS)
			{
				trk_Reset(pTracker);
				msDelta = pTracker->m_InterpolationMS - pCurAnim->m_InterpolationMS;
				pTracker->m_InterpolationMS = NOT_INTERPOLATING; // Stop interpolating.
				// Fall thru to update the anim with the extra time.
			}
			else // interpolate between last frame of prev and first frame of cur
			{
				if (pCurAnim->m_InterpolationMS)
					pTracker->m_TimeRef.m_Percent = (float)pTracker->m_InterpolationMS / pCurAnim->m_InterpolationMS;
				else
					pTracker->m_TimeRef.m_Percent = 0.0f;

				ASSERT((pTracker->m_TimeRef.m_Percent >= 0.0f) && (pTracker->m_TimeRef.m_Percent <= 1.0f));
				return;
			}
		}
	}
	
	// Determine whether or not we are really animated
	if(pCurAnim->m_KeyFrames.GetSize() <= 1)
	{
		//we aren't, so just reset it and bail
		pTracker->m_CurKey = 0;
		pTracker->m_TimeRef.m_Percent = 0.0f;
		pTracker->m_TimeRef.m_Cur.m_Time = 0;
		return;
	}

	// Update anim time.
	uint32 startTime	= pTracker->m_TimeRef.m_Cur.m_Time;
	uint32 iEndKey		= pCurAnim->m_KeyFrames.GetSize() - 1;

	// Find the final time where we end up.
	uint32 endTime = pCurAnim->m_KeyFrames[iEndKey].m_Time;

	// Update time
	pTracker->m_TimeRef.m_Prev.m_Time = pTracker->m_TimeRef.m_Cur.m_Time;
    pTracker->m_TimeRef.m_Cur.m_Time += msDelta;
	
	if(!(pTracker->m_Flags & AT_LOOPING))
	{
		pTracker->m_TimeRef.m_Cur.m_Time = MIN(pTracker->m_TimeRef.m_Cur.m_Time, (endTime+1));
	}
	
	// Now scan thru the keyframes..
	uint32 keyTime;
	while(pTracker->m_CurKey <= iEndKey)
	{
		keyTime = pCurAnim->m_KeyFrames[pTracker->m_CurKey].m_Time;
		if(pTracker->m_TimeRef.m_Cur.m_Time <= keyTime)
		{
			break;
		}
		else
		{
			if(bProcessKeys)
			{
				trk_ProcessKey(pTracker, pCurAnim, pTracker->m_CurKey);
			}
		}

		++pTracker->m_CurKey;
	}

	if(pTracker->m_CurKey >= (iEndKey+1) && (pTracker->m_Flags & AT_LOOPING))
	{
		pTracker->m_TimeRef.m_Cur.m_Time %= endTime;
		pTracker->m_CurKey = 0;
	}

	
	// Make sure their m_pFrames are updated if we didn't process keys.
	if(!bProcessKeys)
	{
		// Get our current frame.
		pTracker->m_TimeRef.m_Cur.m_iFrame = (uint16)pTracker->m_CurKey;
 
		// If we haven't passed the last frame, then set the previous frame to the keyframe before the current.
		if(pTracker->m_TimeRef.m_Cur.m_iFrame > 0)
		{
			if(pTracker->m_TimeRef.m_Cur.m_iFrame <= iEndKey)
			{
				pTracker->m_TimeRef.m_Prev.m_iFrame = pTracker->m_TimeRef.m_Cur.m_iFrame - 1;
			}
			//prevent overflow
			else
			{
				pTracker->m_TimeRef.m_Prev.m_iFrame = iEndKey;
				pTracker->m_TimeRef.m_Cur.m_iFrame  = iEndKey;
			}
		}
		// Otherwise, set the previous frame to our current keyframe.
		else
		{
			pTracker->m_TimeRef.m_Prev.m_iFrame = 0;
			pTracker->m_TimeRef.m_Cur.m_iFrame  = 0;
		}

		ASSERT(pTracker->IsValid());
	}

	// Update the position interpolant if necessary.
	trk_UpdatePositionInterpolant(pTracker);
}




///////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------ //
// AnimTimeRef.
// ------------------------------------------------------------------------ //
bool AnimTimeRef::IsValid()
{
	return m_pModel && 
		m_Prev.m_iAnim < m_pModel->NumAnims() && 
		m_Cur.m_iAnim < m_pModel->NumAnims() &&
		m_Prev.m_iFrame < m_pModel->GetAnim(m_Prev.m_iAnim)->m_KeyFrames &&
		m_Cur.m_iFrame < m_pModel->GetAnim(m_Cur.m_iAnim)->m_KeyFrames &&
		m_Percent >= 0.0f && m_Percent <= 1.0f;
}

ModelAnim* AnimTimeRef::GetAnim1()
{
	assert(m_Prev.m_iAnim < m_pModel->NumAnims());
	return m_pModel->GetAnim(m_Prev.m_iAnim);
}

ModelAnim* AnimTimeRef::GetAnim2()
{
	assert(m_Cur.m_iAnim < m_pModel->NumAnims());
	return m_pModel->GetAnim(m_Cur.m_iAnim);
}


bool AnimTimeRef::SetKeyframePosition(uint32 iAnim,
                                        uint32 iFrame1, 
                                        uint32 iFrame2, 
                                        float fPercent)
{
	ModelAnim *pAnim;
	uint32 dwTime, dwTime1, dwTime2;


	if(!m_pModel)
		return false;

	if(iAnim >= m_pModel->NumAnims())
		return false;

	pAnim = m_pModel->GetAnim(iAnim);
	if(iFrame1 >= pAnim->NumKeyFrames() || iFrame2 >= pAnim->NumKeyFrames())
		return false;

	dwTime1 = pAnim->m_KeyFrames[iFrame1].m_Time;
	dwTime2 = pAnim->m_KeyFrames[iFrame2].m_Time;
	dwTime = ltfptoui(dwTime1 + (float)(dwTime2 - dwTime1) * fPercent);

	m_Prev.m_iAnim = (uint16)iAnim;
	m_Prev.m_iFrame = (uint16)iFrame1;
	m_Prev.m_Time = dwTime;

	m_Cur.m_iAnim = (uint16)iAnim;
	m_Cur.m_iFrame = (uint16)iFrame2;
	m_Cur.m_Time = dwTime;

	m_Percent = fPercent;
	ASSERT(IsValid());
	return true;
}







