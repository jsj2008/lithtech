#include "precompile.h"

#include "d3d_device.h"
#include "d3d_utils.h"
#include "texturescriptinstance.h"
#include "texturescriptevaluator.h"
#include "texturescriptvarmgr.h"
#include "common_draw.h"
#include "renderstruct.h"

#define MAX_CHANNELS	4

CTextureScriptInstance::CTextureScriptInstance() :
	m_bFirstUpdate(true),
	m_fOldTime(0.0f),
	m_nOldFrameCode(0),
	m_nRefCount(0)
{
}

CTextureScriptInstance::~CTextureScriptInstance()
{
}

//sets up a texture transform as a unique evaluator
bool CTextureScriptInstance::SetupStage(uint32 nStage, uint32 nVarID, ETextureScriptChannel eChannel, ITextureScriptEvaluator* pEvaluator)
{
	//check range
	if(nStage >= NUM_STAGES)
		return false;

	//don't let them set up a null channel
	if(eChannel == TSChannel_Null)
		return true;

	//setup the stage, but make sure it isn't currently setup
	ASSERT(m_Stages[nStage].m_bValid == false);
	ASSERT(pEvaluator);

	//set up the stage
	m_Stages[nStage].m_nID			= nVarID;
	m_Stages[nStage].m_eChannel		= eChannel;
	m_Stages[nStage].m_pEvaluator	= pEvaluator;
	m_Stages[nStage].m_bValid		= true;
	m_Stages[nStage].m_pOverride	= NULL;

	return true;
}

//sets up a texture transform as a reference to another transform (it will piggy back the
//transform from there)
bool CTextureScriptInstance::SetupStageAsReference(uint32 nStage, ETextureScriptChannel eChannel, uint32 nReferTo)
{
	//check range
	if((nStage >= NUM_STAGES) || (nReferTo >= NUM_STAGES))
		return false;

	//don't let them set up a null channel
	if(eChannel == TSChannel_Null)
		return true;

	//setup the stage, but make sure it isn't currently setup
	ASSERT(m_Stages[nStage].m_bValid == false);

	//set up the stage
	m_Stages[nStage].m_eChannel		= eChannel;
	m_Stages[nStage].m_bValid		= true;
	m_Stages[nStage].m_pOverride	= &m_Stages[nReferTo];

	return true;
}

//called to update the matrix transform. If force is not set, it will
//attempt to determine if the matrix is dirty, and not set it if it isn't
bool CTextureScriptInstance::Evaluate(bool bForce)
{
	//determine the current time
	float fTime = m_fOldTime + g_pSceneDesc->m_FrameTime;

	//determine what items are dirty
	bool bFrameDirty = (g_CurFrameCode != m_nOldFrameCode);

	for(uint32 nCurrStage = 0; nCurrStage < NUM_STAGES; nCurrStage++)
	{
		//get the stage
		CTextureScriptInstanceStage* pStage = &m_Stages[nCurrStage];

		//ignore if invalid or is overridden
		if(!pStage->m_bValid || pStage->m_pOverride)
			continue;

		//we need to see if we need to evaluate the matrix
		uint32 nFlags = pStage->m_pEvaluator->GetFlags();

		//evaluate if the appropriate data is dirty, or if we are forcing it
		bool bEvaluate = bForce || m_bFirstUpdate;

		//see if the framecode is dirty
		if(	((nFlags & ITextureScriptEvaluator::FLAG_DIRTYONFRAME) ||
			 (nFlags & ITextureScriptEvaluator::FLAG_WORLDSPACE)) && bFrameDirty)
		{
			bEvaluate = true;
		}

		//get the variables
		float* pVars = CTextureScriptVarMgr::GetSingleton().GetVars(pStage->m_nID);

		if(pVars == NULL)
			continue;

		//see if the variables are dirty
		if( (nFlags & ITextureScriptEvaluator::FLAG_DIRTYONVAR) &&
			(memcmp(pVars, pStage->m_fOldVars, sizeof(float) * CTextureScriptVarMgr::NUM_VARS) != 0))
		{
			bEvaluate = true;
		}

		//bail if we don't need to evaluate it
		if(!bEvaluate)
			continue;

		//ok, we need to actually evaluate it, so fill out the parameters for the evaluation
		CTextureScriptEvaluateVars Vars;
		Vars.m_fTime		= fTime;
		Vars.m_fElapsed		= fTime - m_fOldTime;
		Vars.m_fUserVars	= pVars;

		//now let the evaluator evaluate
		pStage->m_pEvaluator->Evaluate(Vars, pStage->m_mTransform);

		//transform the matrix to be in the appropriate space
		if(nFlags & ITextureScriptEvaluator::FLAG_WORLDSPACE)
		{
			if(pStage->m_pEvaluator->GetInputType() == ITextureScriptEvaluator::INPUT_POS)
			{
				//do a full matrix multiply
				pStage->m_mTransform = pStage->m_mTransform * g_ViewParams.m_mInvView;
			}
			else
			{
				//since we are not doing the position we only want to do an orientation
				//transform
				LTMatrix mTrans;
				mTrans.Init(	g_ViewParams.m_Right.x, g_ViewParams.m_Up.x, g_ViewParams.m_Forward.x, 0.0f,
								g_ViewParams.m_Right.y, g_ViewParams.m_Up.y, g_ViewParams.m_Forward.y, 0.0f,
								g_ViewParams.m_Right.z, g_ViewParams.m_Up.z, g_ViewParams.m_Forward.z, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f);
				pStage->m_mTransform = pStage->m_mTransform * mTrans;
			}
		}

		//update our old stuff for proper dirty detection
		memcpy(pStage->m_fOldVars, pVars, sizeof(float) * CTextureScriptVarMgr::NUM_VARS);
	}

	//update our other info for dirty detection
	if( m_bFirstUpdate || (m_nOldFrameCode != g_CurFrameCode) )
	{
	m_fOldTime			= fTime;
	m_nOldFrameCode		= g_CurFrameCode;
	}
	m_bFirstUpdate		= false;

	//success
	return true;
}

//Installs the matrix into the specified channel with the appropriate flags.
//If evaluate is true it will (unforcefully) evaluate the matrix
bool CTextureScriptInstance::Install(uint32 nNumChannels, ...)
{
	//check the parameters
	ASSERT(nNumChannels > 0);

	//build up our channel list
	nNumChannels = LTMIN(MAX_CHANNELS, nNumChannels);

	ETextureScriptChannel ChannelList[MAX_CHANNELS];
	va_list vl;
	va_start(vl, nNumChannels);

	for(uint32 nCurrChannel = 0; nCurrChannel < nNumChannels; nCurrChannel++)
	{
		ChannelList[nCurrChannel] = (ETextureScriptChannel)va_arg(vl, ETextureScriptChannel);
	}
	va_end(vl);


	//evaluate our matrices
	if(!Evaluate())
		return false;

	//now setup for all stages
	for(uint32 nCurrStage = 0; nCurrStage < NUM_STAGES; nCurrStage++)
	{
		//get the stage
		CTextureScriptInstanceStage* pStage = &m_Stages[nCurrStage];

		//just keep on moving if it isn't valid
		if(!pStage->m_bValid)
			continue;

		//determine the override
		CTextureScriptInstanceStage* pOverride = (pStage->m_pOverride) ? pStage->m_pOverride : pStage;

		//make sure that the override stage is valid
		ASSERT(pOverride->m_bValid);
		ASSERT(pOverride->m_pOverride == NULL);

		//determine the input parameter type
		uint32 nInput;
		switch(pOverride->m_pEvaluator->GetInputType())
		{
		case ITextureScriptEvaluator::INPUT_REFLECTION: 
			nInput = D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR;
			break;
		case ITextureScriptEvaluator::INPUT_NORMAL: 
			nInput = D3DTSS_TCI_CAMERASPACENORMAL;
			break;
		case ITextureScriptEvaluator::INPUT_UV:
			nInput = D3DTSS_TCI_PASSTHRU;
			break;
		case ITextureScriptEvaluator::INPUT_POS: 
		default:
			nInput = D3DTSS_TCI_CAMERASPACEPOSITION;
			break;
		}

		//determine the texture transform flags now
		uint32 nEvalFlags = pOverride->m_pEvaluator->GetFlags();
		uint32 nTTF = 0;

		if(nEvalFlags & ITextureScriptEvaluator::FLAG_COORD1)
			nTTF = D3DTTFF_COUNT1;
		else if(nEvalFlags & ITextureScriptEvaluator::FLAG_COORD2)
			nTTF = D3DTTFF_COUNT2;
		else if(nEvalFlags & ITextureScriptEvaluator::FLAG_COORD3)
			nTTF = D3DTTFF_COUNT3;
		else
			nTTF = D3DTTFF_COUNT4;

		//check for projection
		if(nEvalFlags & ITextureScriptEvaluator::FLAG_PROJECTED)
			nTTF |= D3DTTFF_PROJECTED;

		LTMatrix& mSrcMat = pStage->m_mTransform;

		//convert our matrix to a D3D matrix (our source transposed)
		D3DXMATRIX mMat(	mSrcMat.m[0][0], mSrcMat.m[1][0], mSrcMat.m[2][0], mSrcMat.m[3][0],
							mSrcMat.m[0][1], mSrcMat.m[1][1], mSrcMat.m[2][1], mSrcMat.m[3][1],
							mSrcMat.m[0][2], mSrcMat.m[1][2], mSrcMat.m[2][2], mSrcMat.m[3][2],
							mSrcMat.m[0][3], mSrcMat.m[1][3], mSrcMat.m[2][3], mSrcMat.m[3][3] );

		//see if the channel this maps to is valid
		for(uint32 nChannel = 0; nChannel < nNumChannels; nChannel++)
		{
			//see if this channel matches
			if(ChannelList[nChannel] == pStage->m_eChannel)
			{
				//found a match, so lets go ahead and update it
				D3D_CALL(PD3DDEVICE->SetTextureStageState(nChannel, D3DTSS_TEXTURETRANSFORMFLAGS, nTTF));
				D3D_CALL(PD3DDEVICE->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + nChannel), &mMat));
				if( (nInput == D3DTSS_TCI_PASSTHRU) && (pStage->m_eChannel == TSChannel_DualTexture || pStage->m_eChannel == TSChannel_Detail) )
				{
					D3D_CALL(PD3DDEVICE->SetTextureStageState(nChannel, D3DTSS_TEXCOORDINDEX, nInput|1));
				}
				else
				{
					D3D_CALL(PD3DDEVICE->SetTextureStageState(nChannel, D3DTSS_TEXCOORDINDEX, nInput));
				}
			}
		}
	}

	return true;
}

//disables all transforms that were set
bool CTextureScriptInstance::Uninstall(uint32 nNumChannels, ...)
{
	//check the parameters
	ASSERT(nNumChannels > 0);

	//build up our channel list
	nNumChannels = LTMIN(MAX_CHANNELS, nNumChannels);

	ETextureScriptChannel ChannelList[MAX_CHANNELS];
	va_list vl;
	va_start(vl, nNumChannels);

	for(uint32 nCurrChannel = 0; nCurrChannel < nNumChannels; nCurrChannel++)
	{
		ChannelList[nCurrChannel] = (ETextureScriptChannel)va_arg(vl, ETextureScriptChannel);
	}
	va_end(vl);

	//now setup for all stages
	for(uint32 nCurrStage = 0; nCurrStage < NUM_STAGES; nCurrStage++)
	{
		//get the stage
		CTextureScriptInstanceStage* pStage = &m_Stages[nCurrStage];

		//just keep on moving if it isn't valid
		if(!pStage->m_bValid)
			continue;

		//see if the channel this maps to is valid
		for(uint32 nChannel = 0; nChannel < nNumChannels; nChannel++)
		{
			//see if this channel matches
			if(ChannelList[nChannel] == pStage->m_eChannel)
			{
				//found a match
				D3D_CALL(PD3DDEVICE->SetTextureStageState(nChannel, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
				D3D_CALL(PD3DDEVICE->SetTextureStageState(nChannel, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | nChannel));
			}
		}
	}
	return true;
}

