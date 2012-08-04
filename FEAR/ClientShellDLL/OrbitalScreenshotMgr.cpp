// ----------------------------------------------------------------------- //
//
// MODULE  : OrbitalScreenshotMgr.h
//
// PURPOSE : Provides the implementation of a manager that hooks into the console
//			 system and upon activation takes a series of screenshots on an orbit
//			 which can then be pieced together into movies.
//
// CREATED : 10/28/04
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "OrbitalScreenshotMgr.h"
#include "VarTrack.h"
#include "PlayerCamera.h"
#include "iltfilemgr.h"
#include "ltfileoperations.h"
#include "PlayerBodyMgr.h"

// -----------------------------------------------------------------------
// Utility functions
// ----------------------------------------------------------------------- //

//called to obtain the current position of the camera. This will return the origin upon
//failure
static LTVector GetCurrentCameraPosition()
{
	return g_pGameClientShell->GetPlayerMgr()->GetPlayerCamera()->GetCameraPos();
}

//given a series and image number, this will determine the relative user file name and return it
//in the provided buffer
static void GetRelativeImageFile(uint32 nSeriesNumber, uint32 nImageNumber, char* pszFileBuffer, uint32 nBufferLen)
{
	LTSNPrintF(pszFileBuffer, nBufferLen, "Orbital_%d_%04d.bmp", nSeriesNumber, nImageNumber);
}

//given a series number and an image number, this will determine the full user file name and 
//return this in the provided buffer
static void GetAbsoluteImageFile(uint32 nSeriesNumber, uint32 nImageNumber, char* pszFileBuffer, uint32 nBufferLen)
{
	char pszRelFilename[MAX_PATH];
	GetRelativeImageFile(nSeriesNumber, nImageNumber, pszRelFilename, LTARRAYSIZE(pszRelFilename));
	g_pLTClient->FileMgr()->GetAbsoluteUserFileName(pszRelFilename, pszFileBuffer, nBufferLen);
}

//called to determine the series number that should be used for a collection of images. This
//can fail if too many series exist (which would most likely be a sign of a failure in the file
//system)
static bool DetermineSeriesNumber(uint32& nSeriesNumber)
{
	//we need to determine the file name that we need to use. Files are named in the manner of:
	//Orbital_<n>_<p> where n is the series number and p is the image number
	nSeriesNumber = 0;

	while(1)
	{
		//see if this series already exists
		char pszAbsoluteName[MAX_PATH];
		GetAbsoluteImageFile(nSeriesNumber, 0, pszAbsoluteName, LTARRAYSIZE(pszAbsoluteName));

		//see if this file exists
		if(!LTFileOperations::FileExists(pszAbsoluteName))
			return true;

		//this exists, move onto the next one
		nSeriesNumber++;

		//handle a safety limit
		static const uint32 knSafetyLimit = 1000;
		if(nSeriesNumber > knSafetyLimit)
			return false;
	}
}

// -----------------------------------------------------------------------
// Console variables
// ----------------------------------------------------------------------- //

static VarTrack	g_varOrbitalSSRadius;
static VarTrack g_varOrbitalSSNumImages;
static VarTrack g_varOrbitalSSVerticalOffset;
static VarTrack g_varOrbitalSSHidePlayer;

//track whether or not we are offsetting by the world offset
extern VarTrack g_vtApplyWorldOffset;

// -----------------------------------------------------------------------
// Console programs
// ----------------------------------------------------------------------- //

//this console program handles capturing the current camera position as the marker position
static void OrbitalSSPlaceCameraMarkerHandler(int argc, char **argv)
{
	COrbitalScreenshotMgr::Singleton().SetMarker(GetCurrentCameraPosition());
}

//this console program handles using the provided coordinates as the marker position
static void OrbitalSSPlaceMarkerAtHandler(int argc, char **argv)
{
	//verify the number of parameters
	if(argc < 3)
	{
		g_pLTClient->CPrint("Invalid number of parameters to OrbitalSSPlaceMarkerAtHandler: Format is Msg x y z");
		return;
	}

	LTVector vMarker((float)atof(argv[0]), (float)atof(argv[1]), (float)atof(argv[2]));
	if((uint32)g_vtApplyWorldOffset.GetFloat(1.0f))
	{
		LTVector vOffset;
		g_pLTClient->GetSourceWorldOffset(vOffset);
		vMarker -= vOffset;
	}

	COrbitalScreenshotMgr::Singleton().SetMarker(vMarker);
}

//this console program clears out any existing marker from the orbital screenshot so it will
//just use the camera's position
static void OrbitalSSClearMarkerHandler(int argc, char **argv)
{
	COrbitalScreenshotMgr::Singleton().ClearMarker();
}

//this console program actually triggers the capture of the orbital screenshots
static void OrbitalSSCaptureHandler(int argc, char **argv)
{
	//extract our parameters from the console variables
	float fRadius			= g_varOrbitalSSRadius.GetFloat();
	float fVerticalOffset	= g_varOrbitalSSVerticalOffset.GetFloat();
	uint32 nNumImages		= LTMAX(0, (int32)(g_varOrbitalSSNumImages.GetFloat() + 0.5f));
	bool bHidePlayer		= g_varOrbitalSSHidePlayer.GetFloat() != 0.0f;

	//don't hide the player though if it is already hidden as this can mess up the other status
	bHidePlayer = bHidePlayer && !CPlayerBodyMgr::Instance().IsPlayerBodyHidden();

	if(bHidePlayer)
		CPlayerBodyMgr::Instance().HidePlayerBody(true, true);

	//and now trigger the capture
	bool bResult = COrbitalScreenshotMgr::Singleton().Capture(fRadius, nNumImages, fVerticalOffset);

	if(bHidePlayer)
		CPlayerBodyMgr::Instance().HidePlayerBody(false, true);

	//print out the status
	g_pLTClient->CPrint("Orbital screenshot capture %s", (bResult) ? "Succeeded" : "Failed");
}

// -----------------------------------------------------------------------
// COrbitalScreenshotMgr
// ----------------------------------------------------------------------- //


COrbitalScreenshotMgr::COrbitalScreenshotMgr() :
	m_bMarkerSet(false)
{
}

COrbitalScreenshotMgr::~COrbitalScreenshotMgr()
{
}

//singleton access
COrbitalScreenshotMgr& COrbitalScreenshotMgr::Singleton()
{
	static COrbitalScreenshotMgr s_Singleton;
	return s_Singleton;
}

//this must be called on this manager at some point in order to register the
//console variables and programs with the engine so that this manager can be
//used. This assumes that the global client interface is valid when called.
void COrbitalScreenshotMgr::InitConsolePrograms()
{
	//register our marker console programs
	g_pLTClient->RegisterConsoleProgram("OrbitalSSPlaceCameraMarker", OrbitalSSPlaceCameraMarkerHandler);
	g_pLTClient->RegisterConsoleProgram("OrbitalSSPlaceMarkerAt", OrbitalSSPlaceMarkerAtHandler);
	g_pLTClient->RegisterConsoleProgram("OrbitalSSClearMarker", OrbitalSSClearMarkerHandler);

	//register the program that takes the actual screenshot
	g_pLTClient->RegisterConsoleProgram("OrbitalSSCapture", OrbitalSSCaptureHandler);

	//and register our console variables
	g_varOrbitalSSRadius.Init(g_pLTClient, "OrbitalSSRadius", NULL, 100.0f);
	g_varOrbitalSSNumImages.Init(g_pLTClient, "OrbitalSSNumImages", NULL, 36.0f);
	g_varOrbitalSSVerticalOffset.Init(g_pLTClient, "OrbitalSSVerticalOffset", NULL, 0.0f);
	g_varOrbitalSSHidePlayer.Init(g_pLTClient, "OrbitalSSHidePlayer", NULL, 0.0f);
}

//called to capture a screenshot using the current marker, provided radius, number
//of images, and vertical offset
bool COrbitalScreenshotMgr::Capture(float fOrbitalRadius, uint32 nNumImages, float fVerticalOffset)
{
	//fail if there are no images!
	if(nNumImages == 0)
		return false;

	//determine the series number to use for these images
	uint32 nSeriesNumber = 0;
	if(!DetermineSeriesNumber(nSeriesNumber))
		return false;

	//determine our marker position
	const LTVector vMarkerPos = IsMarkerSet() ? GetMarker() : GetCurrentCameraPosition();

	//extract camera information
	HOBJECT hCameraObject = g_pGameClientShell->GetPlayerMgr()->GetPlayerCamera()->GetCamera();

	//extract the FOV
	LTVector2 vCamFOV;
	g_pLTClient->GetCameraFOV(hCameraObject, &vCamFOV.x, &vCamFOV.y);

	//and also the viewport
	LTRect2f rViewport;
	g_pLTClient->GetCameraRect(hCameraObject, rViewport);

	//now we need to handle each and every image
	for(uint32 nCurrImage = 0; nCurrImage < nNumImages; nCurrImage++)
	{
		//determine the angle for this image
		float fImageAngle = nCurrImage * MATH_TWOPI / nNumImages;

		//and determine the position of this capture point on the orbit
		LTVector vCapturePos = vMarkerPos;
		vCapturePos.x += LTSin(fImageAngle) * fOrbitalRadius;
		vCapturePos.y += fVerticalOffset;
		vCapturePos.z += LTCos(fImageAngle) * fOrbitalRadius;

		//and now we need to determine the orientation of this capture camera
		LTRotation rCaptureRot((vMarkerPos - vCapturePos).GetUnit(), LTVector(0.0f, 1.0f, 0.0f));

		//now we have all the information we need, so setup the device for rendering and render away!
		if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
		{

			//handle updating of the render targets
			g_pGameClientShell->GetSFXMgr()->UpdateRenderTargets(LTRigidTransform(vCapturePos, rCaptureRot), vCamFOV);

			//now render the main scene
			g_pLTClient->GetRenderer()->ClearRenderTarget( CLEARRTARGET_ALL, 0 );
			g_pLTClient->GetRenderer()->RenderCamera(LTRigidTransform(vCapturePos, rCaptureRot), vCapturePos, vCamFOV, rViewport, NULL);

			//and dirty the render targets accordingly
			g_pGameClientShell->GetSFXMgr()->DirtyVisibleRenderTargets();

			//end the rendering
			g_pLTClient->GetRenderer()->End3D();
			g_pLTClient->GetRenderer()->FlipScreen();
		}

		//and now capture the screenshot
		char pszRelFilename[MAX_PATH];
		GetRelativeImageFile(nSeriesNumber, nCurrImage, pszRelFilename, LTARRAYSIZE(pszRelFilename));

		// get the user path to the screenshot file, as we need this to check if the file exists.
		char pszUserFilename[MAX_PATH];
		g_pLTClient->FileMgr()->GetAbsoluteUserFileName( pszRelFilename, pszUserFilename, LTARRAYSIZE( pszUserFilename ) );

		g_pLTClient->GetRenderer()->MakeScreenShot(pszUserFilename);        
	}

	//success
	return true;
}





