
#include "bdefs.h"
#include "animtracker.h"
#include "model_ops.h"


extern int32 g_CV_ModelTransitionMS;


bool AnimTimeRef::SetKeyframePosition(
	uint32 iAnim,
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
	if(iFrame1 >= pAnim->NumKeyframes() || iFrame2 >= pAnim->NumKeyframes())
		return false;

	dwTime1 = pAnim->m_KeyFrames[iFrame1].m_Time;
	dwTime2 = pAnim->m_KeyFrames[iFrame1].m_Time;
	dwTime = (uint32)(dwTime1 + (float)(dwTime2 - dwTime1) * fPercent);

	m_Prev.m_iAnim = (uint16)iAnim;
	m_Prev.m_iFrame = (uint16)iFrame1;
	m_Prev.m_Time = dwTime;

	m_Cur.m_iAnim = (uint16)iAnim;
	m_Cur.m_iFrame = (uint16)iFrame2;
	m_Cur.m_Time = dwTime;

	m_Percent = fPercent;
	ASSERT((m_Percent >= 0.0f) && (m_Percent <= 1.0f));
	return true;
}


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
	
	if(pModel)
	{
		if(pTracker->AllowInvalid())
		{
			pTracker->m_TimeRef.m_Prev.m_iAnim = pTracker->m_TimeRef.m_Cur.m_iAnim = (uint16)iAnim;
			pTracker->m_TimeRef.m_Prev.m_iFrame = pTracker->m_TimeRef.m_Cur.m_iFrame = 0;
		}
		else
		{
			if(iAnim >= pModel->m_Anims)
				iAnim = 0;

			pTracker->m_TimeRef.m_Prev.m_iAnim = pTracker->m_TimeRef.m_Cur.m_iAnim = (uint16)iAnim;
			pTracker->m_TimeRef.m_Prev.m_iFrame = pTracker->m_TimeRef.m_Cur.m_iFrame = 0;
		}

		pTracker->m_Flags |= AT_PLAYING;
		pTracker->m_InterpolationMS = NOT_INTERPOLATING;
	}

	pTracker->m_Flags |= AT_LOOPING;
}


void trk_Update(LTAnimTracker *pTracker, uint32 msDelta)
{
	if(pTracker->m_Flags & AT_PLAYING)
	{
		trk_ScanToKeyFrame(pTracker, msDelta, TRUE);
	}
}


bool trk_IsStopped(LTAnimTracker *pTracker)
{
	uint32 iEndKey, endTime;
	ModelAnim *pAnim;


	if(pTracker->IsValid())
	{
		pAnim = pTracker->GetCurAnim();

		if(!(pTracker->m_Flags & AT_LOOPING))
		{
			iEndKey = pAnim->m_KeyFrames.GetSize() - 1;
			endTime = pAnim->m_KeyFrames[iEndKey].m_Time;
			if(pTracker->m_TimeRef.m_Cur.m_Time == (endTime+1))
				return TRUE;
		}
	}

	return FALSE;
}


void trk_SetCurTime(LTAnimTracker *pTracker, uint32 msTime, bool bTransition)
{
	// If we're not transitioning, then prev should move to the same keyframe as cur.
	if(!pTracker->IsValid() || !bTransition)
	{
		pTracker->m_TimeRef.m_Prev.m_Time = pTracker->m_TimeRef.m_Cur.m_Time = 0;
		pTracker->m_TimeRef.m_Prev.m_iFrame = pTracker->m_TimeRef.m_Cur.m_iFrame = 0;
		pTracker->m_CurKey = 0;
	}

	pTracker->m_TimeRef.m_Cur.m_Time = 0;
	pTracker->m_TimeRef.m_Cur.m_iFrame = 0;
	pTracker->m_TimeRef.m_Cur.m_iFrame = 0;
	pTracker->m_CurKey = 0;

	trk_ScanToKeyFrame(pTracker, msTime, FALSE);
}


bool trk_SetCurAnim(LTAnimTracker *pTracker, char *sName, bool bTransition)
{
	ModelAnim *pAnim;
	uint32 iAnim;
	
	pAnim = pTracker->GetModel()->FindAnim(sName, &iAnim);
	if(!pAnim)
		return FALSE;

	return trk_SetCurAnim(pTracker, iAnim, bTransition);
}


bool trk_SetCurAnim(LTAnimTracker *pTracker, uint32 iAnim, bool bTransition)
{
	if(!pTracker->AllowInvalid())
	{
		if(iAnim >= pTracker->GetModel()->m_Anims)
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
	pTracker->m_TimeRef.m_Prev.m_iAnim = pTracker->m_TimeRef.m_Cur.m_iAnim;
	pTracker->m_CurKey = 0;
	pTracker->m_TimeRef.m_Prev.m_iFrame = pTracker->m_TimeRef.m_Cur.m_iFrame = 0;
	pTracker->m_TimeRef.m_Prev.m_Time = pTracker->m_TimeRef.m_Cur.m_Time = 0;
	pTracker->m_TimeRef.m_Percent = 0.0f;
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
			if(pAnim->m_KeyFrames[i].m_KeyType == KEYTYPE_POSITION)
			{
				iNextFrame = i;
				return &pAnim->m_KeyFrames[i];
			}
		}
	}

	return NULL;
}


static void trk_ProcessKey(LTAnimTracker *pTracker, ModelAnim *pAnim, uint32 iFrame)
{
	AnimKeyFrame *pNextPosition;
	uint32 iNextPosition;
	AnimKeyFrame *pFrame;


	pFrame = &pAnim->m_KeyFrames[iFrame];	
	if(pFrame->m_KeyType == KEYTYPE_CALLBACK)
	{
		if(pTracker->m_Flags & AT_DOCALLBACKS)
		{
			if(pFrame->m_Callback)
			{
				pFrame->m_Callback(pTracker, pFrame);
			}
		}
	}
	else if(pFrame->m_KeyType == KEYTYPE_POSITION)
	{
		// Is there a string for this key?
		if(pFrame->m_pString[0] != 0 && pTracker->m_StringKeyCallback != NULL)
		{
			pTracker->m_StringKeyCallback(pTracker, pFrame);
		}
		
		pNextPosition = trk_NextPositionFrame(pTracker, iFrame+1, iNextPosition);
		if(pNextPosition)
		{
			if(pTracker->m_TimeRef.m_Cur.m_Time <= pNextPosition->m_Time)
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
	}
}


static inline void trk_UpdatePositionInterpolant(LTAnimTracker *pTracker)
{
	AnimKeyFrame *pPrevFrame, *pCurFrame;

	if(pTracker->m_TimeRef.m_Prev.m_iFrame == pTracker->m_TimeRef.m_Cur.m_iFrame || !pTracker->IsValid())
	{
		pTracker->m_TimeRef.m_Percent = 0.0f;
	}
	else
	{
		pPrevFrame = pTracker->GetPrevFrame();
		pCurFrame = pTracker->GetCurFrame();
		
		pTracker->m_TimeRef.m_Percent = 
			(CReal)(pTracker->m_TimeRef.m_Cur.m_Time - pPrevFrame->m_Time) / 
			(pCurFrame->m_Time - pPrevFrame->m_Time);
	}
}

void trk_SetAtKeyFrame(LTAnimTracker *pTracker, uint32 msDelta)
{
	uint32 iEndKey;
	uint32 endTime, lastkeyTime, keyTime;
	ModelAnim *pCurAnim;

	trk_Reset(pTracker);

	pCurAnim = pTracker->GetCurAnim();
	if(pCurAnim->m_KeyFrames.GetSize() <= 1)
	{
		return;
	}

	iEndKey = pCurAnim->m_KeyFrames.GetSize() - 1;

	// Find the final time where we end up.
	endTime = pCurAnim->m_KeyFrames[iEndKey].m_Time;
	if ( msDelta > endTime ) 
	{
		msDelta = endTime;
	}
	
	if(!(pTracker->m_Flags & AT_LOOPING))
	{
		pTracker->m_TimeRef.m_Cur.m_Time = MIN(pTracker->m_TimeRef.m_Cur.m_Time, (endTime+1));
	}

	// Now find the keyframes..
	lastkeyTime = 0;
	while(pTracker->m_CurKey <= iEndKey)
	{
		keyTime = pCurAnim->m_KeyFrames[pTracker->m_CurKey].m_Time;
		if(msDelta <= keyTime)
		{
			pTracker->m_TimeRef.m_Prev.m_iFrame = (uint16)pTracker->m_CurKey > 0 ? (uint16)pTracker->m_CurKey-1 : 0;
			pTracker->m_TimeRef.m_Cur.m_iFrame = (uint16)pTracker->m_CurKey;
			// Compute the percent into the next keyframe (if there is one)
			if ( (keyTime - lastkeyTime) == 0 ) 
			{
				pTracker->m_TimeRef.m_Percent = 0.0f;
			}
			else
			{
				pTracker->m_TimeRef.m_Prev.m_Time = lastkeyTime;	
				pTracker->m_TimeRef.m_Cur.m_Time = msDelta;
				pTracker->m_TimeRef.m_Percent = (LTFLOAT)(msDelta - lastkeyTime)/(LTFLOAT)(keyTime - lastkeyTime);
				ASSERT((pTracker->m_TimeRef.m_Percent >= 0.0f) && (pTracker->m_TimeRef.m_Percent <= 1.0f));
			}

			break;
		}

		lastkeyTime = keyTime;
		++pTracker->m_CurKey;
	}
}

void trk_ScanToKeyFrame(LTAnimTracker *pTracker, uint32 msDelta, bool bProcessKeys)
{
	uint32 startTime;
	uint32 iEndKey;
	uint32 endTime, keyTime;
	ModelAnim *pCurAnim;


	// Handle if it's invalid.
	if(!pTracker->IsValid())
	{
		pTracker->m_TimeRef.m_Cur.m_Time += msDelta;
		return;
	}

	pCurAnim = pTracker->GetCurAnim();


	// Are we interpolating between two animations?
	if(pTracker->m_InterpolationMS != NOT_INTERPOLATING)
	{
		pTracker->m_InterpolationMS += (unsigned short)msDelta;
		if(pTracker->m_InterpolationMS > (uint32)pCurAnim->m_InterpolationMS)
		{
			trk_Reset(pTracker);
			msDelta = pTracker->m_InterpolationMS - pCurAnim->m_InterpolationMS;
			pTracker->m_InterpolationMS = NOT_INTERPOLATING; // Stop interpolating.
			// Fall thru to update the anim with the extra time.
		}
		else
		{
			if (pCurAnim->m_InterpolationMS)
				pTracker->m_TimeRef.m_Percent = (float)pTracker->m_InterpolationMS / pCurAnim->m_InterpolationMS;
			else
				pTracker->m_TimeRef.m_Percent = 0.0f;
			ASSERT((pTracker->m_TimeRef.m_Percent >= 0.0f) && (pTracker->m_TimeRef.m_Percent <= 1.0f));
			return;
		}
	}


	startTime = pTracker->m_TimeRef.m_Cur.m_Time;
	
	// Not really animated...
	if(pCurAnim->m_KeyFrames.GetSize() <= 1)
	{
		pTracker->m_CurKey = 0;
		pTracker->m_TimeRef.m_Percent = 0.0f;
		pTracker->m_TimeRef.m_Cur.m_Time = 0;
		return;
	}

	iEndKey = pCurAnim->m_KeyFrames.GetSize() - 1;


	// Find the final time where we end up.
	endTime = pCurAnim->m_KeyFrames[iEndKey].m_Time;

	pTracker->m_TimeRef.m_Prev.m_Time = pTracker->m_TimeRef.m_Cur.m_Time;	
	pTracker->m_TimeRef.m_Cur.m_Time += msDelta;
	
	if(!(pTracker->m_Flags & AT_LOOPING))
	{
		pTracker->m_TimeRef.m_Cur.m_Time = MIN(pTracker->m_TimeRef.m_Cur.m_Time, (endTime+1));
	}

   

	// Now scan thru the keyframes..
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

	if(pTracker->m_CurKey == (iEndKey+1) && (pTracker->m_Flags & AT_LOOPING))
	{
		pTracker->m_TimeRef.m_Cur.m_Time %= endTime;
		pTracker->m_CurKey = 0;
	}

	// Make sure their m_pFrames are updated if we didn't process keys.
	if(!bProcessKeys)
	{
		pTracker->m_TimeRef.m_Prev.m_iFrame = pTracker->m_TimeRef.m_Cur.m_iFrame = (uint16)pTracker->m_CurKey;
	}

	// Update the position interpolant if necessary.
	trk_UpdatePositionInterpolant(pTracker);
}


