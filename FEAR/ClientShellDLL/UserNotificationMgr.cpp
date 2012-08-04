#include "stdafx.h"
#include "UserNotificationMgr.h"
#include "iltrenderer.h"

//----------------------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------------------

//determines the alpha for an icon given the display time left and the active time of the
//icon
static float GetIconAlpha(float fDisplayTime, float fActiveTime)
{
	//just fade out over the last tenth of the lifetime
	float fFadeStart = fActiveTime / 10.0f;

	//if we aren't in the fade range just return full alpha
	if(fDisplayTime >= fFadeStart)
		return 1.0f;

	//otherwise fade out as it approaches 0
    return fDisplayTime / fFadeStart;
}

//determines the color to use given an icon severity
static uint32 GetSeverityColor(ENotificationSeverity eSeverity, float fAlpha)
{
	uint32 nAlpha = (uint32)(fAlpha * 255.0f + 0.5f);

	switch(eSeverity)
	{
	case eNotificationSeverity_High:	
		return SETRGBA(255, 0, 0, nAlpha);
		break;
	case eNotificationSeverity_Medium:	
		return SETRGBA(255, 236, 0, nAlpha);
		break;
	case eNotificationSeverity_Low:	
		return SETRGBA(0, 255, 0, nAlpha);
		break;
	default:	
		return SETRGBA(0, 0, 0, nAlpha);
		break;
	}
}

//renders the border around the icon with the specified width
static void RenderIconBorder(uint32 nX, uint32 nY, uint32 nWidth, uint32 nHeight, uint32 nBorderSize, uint32 nColor)
{
	//setup four rectangles that form the border

	LT_POLYG4 vVerts[4];

	//setup the color of the rectangles
	for(uint32 nCurrQuad = 0; nCurrQuad < LTARRAYSIZE(vVerts); nCurrQuad++)
	{
		DrawPrimSetRGBA(vVerts[nCurrQuad], nColor);
	}

	//convert to floating point types
	float fX			= (float)nX;
	float fY			= (float)nY;
	float fWidth		= (float)nWidth;
	float fHeight		= (float)nHeight;
	float fBorderSize	= (float)nBorderSize;

	//top
	DrawPrimSetXYWH(vVerts[0], fX, fY, fWidth, fBorderSize);

	//bottom
	DrawPrimSetXYWH(vVerts[1], fX, fY + fHeight - fBorderSize, fWidth, fBorderSize);

	//left
	DrawPrimSetXYWH(vVerts[2], fX, fY + fBorderSize, fBorderSize, fHeight - fBorderSize * 2);

	//right
	DrawPrimSetXYWH(vVerts[3], fX + fWidth - fBorderSize, fY + fBorderSize, fBorderSize, fHeight - fBorderSize * 2);

	//and render away
	g_pLTClient->GetDrawPrim()->DrawPrim(vVerts, LTARRAYSIZE(vVerts));
}

//renders the actual icon given the dimensions and the texture
static void RenderIcon(uint32 nX, uint32 nY, uint32 nWidth, uint32 nHeight, HTEXTURE hIcon, float fAlpha)
{
	LT_POLYGT4 Quad;

	//setup the quad to be in the correct position and render the icon once
	DrawPrimSetXYWH(Quad, (float)nX, (float)nY, (float)nWidth, (float)nHeight);
	DrawPrimSetRGBA(Quad, 0xFF, 0xFF, 0xFF, (uint32)(fAlpha * 255.0f + 0.5f));
	SetupQuadUVs(Quad, hIcon, 0.0f, 0.0f, 1.0f, 1.0f);

	//setup the draw prim for rendering
	g_pLTClient->GetDrawPrim()->SetTexture(hIcon);
	
	//and render away
	g_pLTClient->GetDrawPrim()->DrawPrim(&Quad, 1);
}

//----------------------------------------------------------------------------------------
// CUserNotificationMgr
//----------------------------------------------------------------------------------------

// lifetime operations
CUserNotificationMgr::CUserNotificationMgr() :
	m_fTotalTimeActive(0.0f)
{
}

CUserNotificationMgr::~CUserNotificationMgr()
{
	Term();
}

//called to shut down the system and free any associated resources
void CUserNotificationMgr::Term()
{
	//just clear out the list of notifications
	m_Notifications.clear();

	//and reset our active time
	m_fTotalTimeActive = 0.0f;
}

//operations to render a notification handler. This handler will be called once each update
//and each time it can return true to indicate that the notification should be provided
bool CUserNotificationMgr::RegisterNotification(TNotificationCallbackFn pfnCallback, void* pUser, const char* pszIcon, float fActivationTime)
{
	//don't allow NULL callbacks
	if(!pfnCallback)
		return false;

	//attempt to load the referenced icon
	TextureReference hIcon(pszIcon);
	if(!(HTEXTURE)hIcon)
		return false;

	//everything is valid, so create a notification and add it onto our list
	CNotification NewNot;
	NewNot.m_pfnCallback	= pfnCallback;
	NewNot.m_pUserData		= pUser;
	NewNot.m_hIcon			= hIcon;
	NewNot.m_fActiveTime	= fActivationTime;

	m_Notifications.push_back(NewNot);
	return true;
}

//called to update. Existing triggered notifications will be aged by the specified time frame, and
//then each notification checked to see if they should be considered active
void CUserNotificationMgr::UpdateNotifications(float fFrameTime)
{
	//just run through all of our notifications and for each oen see if they are active or not. If
	//they are, reset the display time and add this frame time to their total time
	for(TNotificationList::iterator it = m_Notifications.begin(); it != m_Notifications.end(); it++)
	{
		//determine the severity of this callback
		ENotificationSeverity eSeverity = it->m_pfnCallback(it->m_pUserData);

		//see if it is enabled or not
		if(eSeverity != eNotificationSeverity_Disabled)
		{
			//it is active, so reset the time to display
			it->m_fDisplayTimeLeft	= it->m_fActiveTime;

			//and update the time that this has been active
			it->m_fTotalTimeActive += fFrameTime;

			//and update our severity
			it->m_eSeverity = eSeverity;
		}
		else
		{
			//it is not currently active, so reduce the time to display
			it->m_fDisplayTimeLeft -= fFrameTime;
			
			//see if the display time has elapsed
			if(it->m_fDisplayTimeLeft <= 0.0f)
			{
				it->m_fDisplayTimeLeft = 0.0f;
				it->m_eSeverity = eNotificationSeverity_Disabled;
			}			
		}
	}

	//and update the total time that this user notifier has been active
	m_fTotalTimeActive += fFrameTime;
}

//called to render the active notification icons to the screen. This must be called within a begin/end3d
//block.
void CUserNotificationMgr::RenderActiveNotifications()
{
	//determine the screen dimensions
	uint32 nScreenWidth = 640, nScreenHeight = 480;
	if(g_pLTClient->GetRenderer()->GetCurrentRenderTargetDims(nScreenWidth, nScreenHeight) != LT_OK)
		return;

	//and now determine how large the icons should be (based upon the Y size since they stack on the
	//Y and using the X would cause less to be visible in wide screen formats)
	static const float kfIconScreenSize = 0.08f;
	const uint32 nIconSize = (uint32)(kfIconScreenSize * nScreenHeight + 0.5f);

	//determine the border size of the icons
	const uint32 nBorderSize = 4;

	//and the spacing between them in the Y direction
	const uint32 nIconSpacing = 4;

	//the total size of an icon including the border
	const uint32 nTotalIconSize = nIconSize + nBorderSize * 2;

	//determine the starting height
	const uint32 nStartingY = nIconSpacing;

	//and now determine our starting positions on the screen
	static const float kfScreenPadding = 0.05f;
	uint32 nX = (uint32)(nScreenWidth - nTotalIconSize - nIconSpacing);
	uint32 nY = nStartingY;

	//optimize our draw primitive calls by batching state changes
	g_pLTClient->GetDrawPrim()->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
	g_pLTClient->GetDrawPrim()->BeginDrawPrimBlock();

	//just run through and render each icon to the screen
	for(TNotificationList::iterator it = m_Notifications.begin(); it != m_Notifications.end(); it++)
	{
		//skip over any notifications that are currently disabled
		if(it->m_eSeverity == eNotificationSeverity_Disabled)
			continue;

		//determine the alpha of this icon 
		float fAlpha = GetIconAlpha(it->m_fDisplayTimeLeft, it->m_fActiveTime);

		//determine the color for the border
		uint32 nBorderColor = GetSeverityColor(it->m_eSeverity, fAlpha);

		//render the border for the icon
		RenderIconBorder(nX, nY, nTotalIconSize, nTotalIconSize, nBorderSize, nBorderColor);

		//render the actual icon
		RenderIcon(nX + nBorderSize, nY + nBorderSize, nIconSize, nIconSize, it->m_hIcon, fAlpha);

		//move the position down
		nY += nTotalIconSize + nIconSpacing;

		//if there isn't any more room for the icon though, move back to the top and start a new column
		//to the left
		if(nY + nTotalIconSize >= nScreenHeight - nIconSpacing)
		{
			nX -= nTotalIconSize + nIconSpacing;
			nY = nStartingY;
		}
	}

	//we are finished with our draw primitive
	g_pLTClient->GetDrawPrim()->EndDrawPrimBlock();
}

CUserNotificationMgr::CNotification::CNotification() :
	m_pfnCallback(NULL),
	m_pUserData(NULL),
	m_eSeverity(eNotificationSeverity_Disabled),
	m_fDisplayTimeLeft(0.0f),
	m_fActiveTime(0.0f),
	m_fTotalTimeActive(0.0f)
{
}

CUserNotificationMgr::CNotification::~CNotification()
{
	//we can't free up our icons here since we are copied around a lot, so it is just much
	//easier to release the icons prior to flushing the list
}

