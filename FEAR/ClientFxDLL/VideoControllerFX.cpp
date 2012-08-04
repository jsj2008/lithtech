//------------------------------------------------------------------------
//
// MODULE  : VideoControllerFX.cpp
//
// PURPOSE : Provides a ClientFX key that can control video textures
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
//------------------------------------------------------------------------
#include "stdafx.h"
#include "VideoControllerFX.h"
#include "resourceextensions.h"

//------------------------------------------------------------------------
// CVideoControllerProps
//------------------------------------------------------------------------
CVideoControllerProps::CVideoControllerProps() :
	m_pszVideo(NULL),
	m_eOperation(eVideoControllerOp_SinglePlay)
{
}

//handle loading in the properties from the file
bool CVideoControllerProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	if(LTStrIEquals(pszName, "VideoName"))
	{
		m_pszVideo = CFxProp_String::Load(pStream, pszStringTable);
	}
	else if(LTStrIEquals(pszName, "VideoOperation"))
	{
		m_eOperation = (EVideoControllerOp)CFxProp_Enum::Load(pStream);
	}
	else
	{
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void CVideoControllerProps::CollectResources(IFXResourceCollector& Collector)
{
	Collector.CollectResource(m_pszVideo);
}

void fxGetVideoControllerProps(CFastList<CEffectPropertyDesc> *pList)
{
	// Add the generic "every effect has these" props
	AddBaseProps( pList );

	CEffectPropertyDesc fxProp;

	//the video file that will be manipulated
	fxProp.SetupPath( "VideoName", "", "Video Files (*." RESEXT_VIDEO_BINK ")|*." RESEXT_VIDEO_BINK "|All Files (*.*)|*.*||", eCurve_None, "Specifies the name of the video file that will have the operation performed on it" );
	pList->AddTail( fxProp );

	fxProp.SetupEnum( "VideoOperation", "SinglePlay", "Pause,Unpause,Restart,SinglePlay", eCurve_None, "Indicates the operation to perform on a video, it can either pause the video, unpause the video, restart the video at the beginning, or perform a single play, which unpauses and restarts the video when it starts, and then pauses again when it ends");
	pList->AddTail( fxProp );
}

//------------------------------------------------------------------------
// CVideoControllerFX
//------------------------------------------------------------------------

CVideoControllerFX::CVideoControllerFX() :
	CBaseFX(CBaseFX::eVideoControllerFX),
	m_hVideo(NULL)
{
}

//Initialises class CVideoControllerFX
bool CVideoControllerFX::Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps )
{
	if( !CBaseFX::Init(pData, pProps ) )
		return false;

	//we need to find our video handle
	m_hVideo = g_pLTClient->GetVideoTexture()->FindVideoTexture(GetProps()->m_pszVideo);
	if(!m_hVideo)
		return false;

	//success
	return true;
}

void CVideoControllerFX::Term()
{
	g_pLTClient->GetVideoTexture()->ReleaseVideoTexture(m_hVideo);
	m_hVideo = NULL;
}

bool CVideoControllerFX::Update( float tmFrameTime )
{
	//update our base object
	BaseUpdate(tmFrameTime);

	//our return value (false indicates that we want to enter a shutting down state)
	bool bRV = true;

	//we want to perform the starting operations as we become active
	if(IsInitialFrame())
	{
		//we can shut down after performing this operation for any but the single play, 
		//which needs to update to the end and then shut down
		bRV = false;

		//we need to perform starting up actions
		switch(GetProps()->m_eOperation)
		{
		case eVideoControllerOp_Unpause:
			g_pLTClient->GetVideoTexture()->PauseVideoTexture(m_hVideo, false);
			break;

		case eVideoControllerOp_Pause:
			g_pLTClient->GetVideoTexture()->PauseVideoTexture(m_hVideo, true);
			break;

		case eVideoControllerOp_Restart:
			g_pLTClient->GetVideoTexture()->RestartVideoTexture(m_hVideo);
			break;

		case eVideoControllerOp_SinglePlay:
			g_pLTClient->GetVideoTexture()->PauseVideoTexture(m_hVideo, false);
			g_pLTClient->GetVideoTexture()->RestartVideoTexture(m_hVideo);
			bRV = true;
			break;
		}
	}
	
	//note that we can potentially have initial frame and shutting down set on same frame in rare
	//cases
	if(IsShuttingDown())
	{
		//we need to pause the video again after we are done with a single play
		if(GetProps()->m_eOperation == eVideoControllerOp_SinglePlay)
		{
			g_pLTClient->GetVideoTexture()->PauseVideoTexture(m_hVideo, true);
		}
	}

	//success
	return true;
}

