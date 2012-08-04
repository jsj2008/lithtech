#include "stdafx.h"
#include "GameStreamingMgr.h"
#include "iltresourcelistener.h"
#include "iltrefcount.h"
#include "soundmgr.h"
#include "MissionMgr.h"


//-----------------------------------------------------------------------------------------------------
// Console Variables

//control whether or not the HUD should be displayed
VarTrack	g_vtResourceStreamingHUD;

//anchor position for the streaming hud
VarTrack	g_vtResourceStreamingHUDX;
VarTrack	g_vtResourceStreamingHUDY;

//control the source path to the icons
VarTrack	g_vtResourceStreamingIconPath;

//should we display the hud to the console? Useful for consoles while font rendering is not available
//or is illegible
VarTrack	g_vtResourceStreamingConsoleHUD;

//should we prevent flusching of auto loads when crossing streaming regions?
VarTrack	g_vtResourceStreamingAutoFlush;

//should we disable visibility in sectors that we aren't in?
VarTrack	g_vtResourceStreamingUpdateVis;

//-----------------------------------------------------------------------------------------------------
// Layout constants and HUD rendering utilities
//
// Constants that are used to layout the HUD to allow for easier tweaking and adjusting
//

//the height of a line of text
static const int32 knFontHeight = 8;

//the dimensions of the progress bars and memory
static const int32 knProgressBarWidth = 100;

//the width to allow for the labels
static const int32 knLabelColumnWidth = 40;

//the padding between the columns
static const int32 knColumnPadding = 5;

//the padding between the rows
static const int32 knRowPadding = 3;
static const int32 knRowHeight = knFontHeight + 4;

//the dimensions of the icons
static const int32 knIconSize = 20;
static const int32 knIconOffset = 50;

//the amount of time an icon should be displayed in response to an event
static const float kfIconDisplayTime = 1.0f;

//the amount of time that an icon will fade out during (must be less than icon display time)
static const float kfIconFadeOutTime = 0.1f;


//called to determine what the alpha should be for the provided icon with the specified lifetime
static float GetIconAlpha(float fIconTime)
{
	//see if we are fading out or disabled
	if(fIconTime <= 0.0f)
		return 0.0f;

	if(fIconTime >= kfIconFadeOutTime)
		return 1.0f;

	//we are fading
	return fIconTime / kfIconFadeOutTime;
}

//called to render a progress bar within the specified rectangle with the specified percentage
static void RenderProgressBar(int32 nLeft, int32 nTop, int32 nWidth, int32 nHeight, uint32 nColor, uint32 nFill, float fPercentage)
{
	LT_POLYG4 Quads[2];
	float fMidPoint = (float)nWidth * LTCLAMP(fPercentage, 0.0f, 1.0f);

	DrawPrimSetXYWH(Quads[0], (float)nLeft, (float)nTop, fMidPoint, (float)nHeight);
	DrawPrimSetRGBA(Quads[0], nColor);

	DrawPrimSetXYWH(Quads[1], (float)nLeft + fMidPoint, (float)nTop, (float)nWidth - fMidPoint, (float)nHeight);
	DrawPrimSetRGBA(Quads[1], nFill);

	g_pDrawPrim->DrawPrim(Quads, LTARRAYSIZE(Quads));
}

//called to render a progress bar filled from both ends
static void RenderDualProgressBar(int32 nLeft, int32 nTop, int32 nWidth, int32 nHeight, 
								  uint32 nColor1, uint32 nColor2, uint32 nFill, float fProgress1,
								  float fProgress2)
{
	LT_POLYG4 Quads[3];

	//determine the range of the bars
	float fFillLeft  = (float)nWidth * LTCLAMP(fProgress1, 0.0f, 1.0f);
	float fFillRight = (float)nWidth * (1.0f - LTCLAMP(fProgress2, 0.0f, 1.0f));

	//make sure the bars don't overlap
	if(fFillRight < fFillLeft)
		fFillRight = fFillLeft;

	//render progress one
	DrawPrimSetXYWH(Quads[0], (float)nLeft, (float)nTop, fFillLeft, (float)nHeight);
	DrawPrimSetRGBA(Quads[0], nColor1);

	//render progress two
	DrawPrimSetXYWH(Quads[1], (float)nLeft + fFillRight, (float)nTop, (float)nWidth - fFillRight, (float)nHeight);
	DrawPrimSetRGBA(Quads[1], nColor2);

	//render the fill inbetween
	DrawPrimSetXYWH(Quads[2], (float)nLeft + fFillLeft, (float)nTop, fFillRight - fFillLeft, (float)nHeight);
	DrawPrimSetRGBA(Quads[2], nFill);

	g_pDrawPrim->DrawPrim(Quads, LTARRAYSIZE(Quads));
}

//called to render an icon for the HUD with the specified alpha
static void RenderIcon(int32 nLeft, int32 nTop, int32 nWidth, int32 nHeight, HTEXTURE hTexture, float fAlpha)
{
	//bail if there is no opacity
	if(fAlpha <= 0.0f)
		return;

	//setup the color
	uint32 nColor = SETRGBA(0xFF, 0xFF, 0xFF, (uint8)(0xFF * fAlpha));

	LT_POLYGT4 Quad;
	DrawPrimSetXYWH(Quad, (float)nLeft, (float)nTop, (float)nWidth, (float)nHeight);
	DrawPrimSetUVWH(Quad, 0.0f, 0.0f, 1.0f, 1.0f);
	DrawPrimSetRGBA(Quad, nColor);

	g_pDrawPrim->SetTexture(hTexture);
	g_pDrawPrim->DrawPrim(&Quad);
}

//-----------------------------------------------------------------------------------------------------
// CGameStreamingListener
//
// The game implementation of the resource listener that will handle notification of events to the
// game streaming system

class CGameStreamingListener :
	public ILTResourceListener
{
public:

	virtual ~CGameStreamingListener()	{}

	//singleton access
	static CGameStreamingListener&	GetSingleton()
	{
		static CGameStreamingListener s_Singleton;
		return s_Singleton;
	}

	//reference counting support
	virtual void	AddRef()		{ m_RefCount.AddRef(); }
	virtual void	Release()		{ m_RefCount.Release(); }

	//called whenever a resource handle is requested for a resource that is not prefetched. This
	//is commonly used to perform prefetching of the asset if all the prefetching is meant to be
	//done ahead of time
	virtual void	OnObtainUnprefetchedResource(const char* pszResource)
	{
	}

	//called whenever an unprefetched resource is locked. It is very important to note that this will NOT
	//be called during final builds for performance. Typically this is used for updating of
	//asset lists dynamically.
	virtual void	OnUnprefetchedResourceLocked(const char* pszResource)
	{
		CGameStreamingMgr::Singleton().OnAccessUnprefetchedResource(pszResource);
	}

	//called whenever a resource is locked that is prefetched, but has not yet been loaded. This is
	//typically used to indicate that the prefetching needs to be setup in a more optimal manner. This
	//will NOT be called in final builds for performance reasons
	virtual void	OnUnloadedResourceLocked(const char* pszResource)
	{
		CGameStreamingMgr::Singleton().OnAccessUnloadedResource(pszResource);
	}

private:

	//singleton only
	CGameStreamingListener() :
		m_RefCount(0)
	{
	}
	PREVENT_OBJECT_COPYING(CGameStreamingListener);

	//the reference count for this object
	CRefCount32		m_RefCount;
};


//-----------------------------------------------------------------------------------------------------
// CGameStreamingMgr

CGameStreamingMgr::CGameStreamingMgr()
{
	//initialize the streaming data
	m_hRegion = NULL;
	m_hLinked = NULL;

	//initialize the HUD data
	m_TextFont = CFontInfo("Terminal", knFontHeight);

	m_bInitializedHUDAssets = false;

	m_fNotPrefetchedTime	= 0.0f;
	m_fNotLoadedTime		= 0.0f;
	m_fOutOfMemoryTime		= 0.0f;

	m_hCurrentRegionName	= NULL;
	m_hLinkedRegionName		= NULL;
	m_hValueSource			= NULL;
	m_hRegionLabel			= NULL;
	m_hLinkedLabel			= NULL;
	m_hGlobalLabel			= NULL;
	m_hMemoryLabel			= NULL;
	m_hNotPrefetched		= NULL;
	m_hNotLoaded			= NULL;
	m_hOutOfMemory			= NULL;
}

CGameStreamingMgr::~CGameStreamingMgr()
{
	ClearCurrentRegions();
}

//singleton access
CGameStreamingMgr& CGameStreamingMgr::Singleton()
{
	static CGameStreamingMgr s_Singleton;
	return s_Singleton;
}

//called to initialize the streaming system at an application wide level
void CGameStreamingMgr::Init()
{
	//install our listener into the resource manager
	g_pLTClient->ResourceMgr()->RegisterResourceListener(&CGameStreamingListener::GetSingleton());

	//setup our console variables
	g_vtResourceStreamingHUD.Init(g_pLTClient, "ResourceStreamingHUD", NULL, 1.0f);
	g_vtResourceStreamingHUDX.Init(g_pLTClient, "ResourceStreamingHUDX", NULL, 0.0f);
	g_vtResourceStreamingHUDY.Init(g_pLTClient, "ResourceStreamingHUDY", NULL, 30.0f);
	g_vtResourceStreamingIconPath.Init(g_pLTClient, "ResourceStreamingIconPath", "Internal\\ResourceStreaming\\", 0.0f);
	g_vtResourceStreamingConsoleHUD.Init(g_pLTClient, "ResourceStreamingConsoleHUD", NULL, 0.0f);	
	g_vtResourceStreamingAutoFlush.Init(g_pLTClient, "ResourceStreamingAutoFlush", NULL, 1.0f);
	g_vtResourceStreamingUpdateVis.Init(g_pLTClient, "ResourceStreamingUpdateVis", NULL, 1.0f);
}

//called to clean up the streaming system at an application wide level
void CGameStreamingMgr::Term()
{
	//remove our resource listener from the resource manager
	g_pLTClient->ResourceMgr()->UnregisterResourceListener(&CGameStreamingListener::GetSingleton());
	TermHUDAssets();
}

//called after a level has been loaded to allow the streaming to perform any necessary operations
void CGameStreamingMgr::OnLevelLoaded()
{
	if(g_vtResourceStreamingUpdateVis.GetFloat() != 0.0f)
	{
		//run through each streaming region and turn off the sectors in the region
		uint32 nNumRegions = 0;
		g_pLTClient->ResourceMgr()->GetNumStreamingRegions(nNumRegions);
		for(uint32 nCurrRegion = 0; nCurrRegion < nNumRegions; nCurrRegion++)
		{
			//get the region handle
			HSTREAMINGREGION hRegion = g_pLTClient->ResourceMgr()->GetStreamingRegion(nCurrRegion);

			//and disable the visibility of the region
			g_pLTClient->EnableRegionSectors(hRegion, false);
		}
	}
}

//called to update the current streaming data based upon the point provided to the callback
void CGameStreamingMgr::UpdateRegions(const LTVector& vPos)
{
	//determine what streaming regions this point is in
	HSTREAMINGREGION	hNewRegion = NULL;
	HSTREAMINGREGION	hNewLinked = NULL;
	if(g_pLTClient->ResourceMgr()->GetPointStreamingRegions(vPos, hNewRegion, hNewLinked) != LT_OK)
		return;

	//safety check that we should NEVER have the linked region be the same as the region we are in
	if(hNewRegion == hNewLinked)
		hNewLinked = NULL;

	//see if we are already using these regions, meaning we don't need to do anything
	if((m_hRegion == hNewRegion) && (m_hLinked == hNewLinked))
		return;

	//see if we have just swapped our regions (if we don't handle this explicitly, the current
	//region will be overridden which will make this less efficient and involve a deactivate
	//and activate)
	if((m_hRegion == hNewLinked) &&	(m_hLinked == hNewRegion))
	{
		//both are already activated, so just assign them directly
		m_hRegion = hNewRegion;
		m_hLinked = hNewLinked;

		//update our active streaming region so assets can be tracked correctly
		g_pLTClient->ResourceMgr()->SetActiveRegion(m_hRegion);

		//and swap our label names
		std::swap(m_hCurrentRegionName, m_hLinkedRegionName);

		//flush the auto loaded resources if appropriate
		if(g_vtResourceStreamingAutoFlush.GetFloat() != 0.0f)
			g_pLTClient->ResourceMgr()->FlushAutoLoadedResources();

		//and we can stop processing at this time
		return;
	}

	//see if we are now in a different streaming region
	if(hNewRegion && (hNewRegion != m_hRegion))
	{
		//see if this is a new region, or if this was our linked region (a very common case)
		if(hNewRegion == m_hLinked)
		{
			//this is a region we have already loaded, so switch it over 
			m_hLinked = m_hRegion;
			m_hRegion = hNewRegion;

			//update our active streaming region so assets can be tracked correctly
			g_pLTClient->ResourceMgr()->SetActiveRegion(m_hRegion);

			//and transfer our label names
			std::swap(m_hCurrentRegionName, m_hLinkedRegionName);
		}
		else
		{
			//this is a new region of which we didn't anticipate, we need to activate this region.
			//This should rarely if ever be the case.
			DeactivateRegion(m_hRegion);
			ActivateRegion(hNewRegion);
			m_hRegion = hNewRegion;

			//update our active streaming region so assets can be tracked correctly
			g_pLTClient->ResourceMgr()->SetActiveRegion(m_hRegion);

			//and clear out our label
			g_pLTClient->GetTextureString()->ReleaseTextureString(m_hCurrentRegionName);
			m_hCurrentRegionName = NULL;
		}

		//flush the auto loaded resources if appropriate
		if(g_vtResourceStreamingAutoFlush.GetFloat() != 0.0f)
			g_pLTClient->ResourceMgr()->FlushAutoLoadedResources();
	}

	//now handle updating our linked region
	if(hNewLinked && (hNewLinked != m_hLinked))
	{
		//this is a new region of which we didn't anticipate, we need to activate this region
		DeactivateRegion(m_hLinked);
		ActivateRegion(hNewLinked);
		m_hLinked = hNewLinked;

		//and clear out our label
		g_pLTClient->GetTextureString()->ReleaseTextureString(m_hLinkedRegionName);
		m_hLinkedRegionName = NULL;
	}
}

//called to clear out any currently associated regions
void CGameStreamingMgr::ClearCurrentRegions()
{
	DeactivateRegion(m_hRegion);
	m_hRegion = NULL;

	DeactivateRegion(m_hLinked);
	m_hLinked = NULL;
}

//called to display the HUD to the screen
void CGameStreamingMgr::DisplayStreamingHUD(float fElapsedTime)
{
	//decrement our timing counts
	m_fNotPrefetchedTime	= LTMAX(m_fNotPrefetchedTime - fElapsedTime, 0.0f);
	m_fNotLoadedTime		= LTMAX(m_fNotLoadedTime - fElapsedTime, 0.0f);
	m_fOutOfMemoryTime		= LTMAX(m_fOutOfMemoryTime - fElapsedTime, 0.0f);

	//bail if the HUD is deactivated
	if((g_vtResourceStreamingConsoleHUD.GetFloat() == 0.0f) && (g_vtResourceStreamingHUD.GetFloat() == 0.0f))
		return;

	//collect the memory information, and progress
	const uint32 knMegabyte = 1024 * 1024;

	//determine the global limit
	uint32 nGlobalLimit = 0;
	g_pLTClient->ResourceMgr()->GetGlobalPoolSize(nGlobalLimit);
	nGlobalLimit /= knMegabyte;

	//determine the total pool size
	uint32 nTotalLimit = 0;
	g_pLTClient->ResourceMgr()->GetStreamingRegionPoolSize(nTotalLimit);
	nTotalLimit /= knMegabyte;

	//determine each regions limits
	uint32 nRegionLimit  = 0;
	g_pLTClient->ResourceMgr()->GetStreamingRegionResourceLimit(m_hRegion, nRegionLimit);
	nRegionLimit /= knMegabyte;

	uint32 nLinkedLimit	 = 0;
	g_pLTClient->ResourceMgr()->GetStreamingRegionResourceLimit(m_hLinked, nLinkedLimit);
	nLinkedLimit /= knMegabyte;

	//and now get the stats for the global region
	uint32 nGlobalLoadedAssets = 0, nGlobalAssets = 0, nGlobalAssetMem = 0;
	HREGIONASSETLIST hGlobalAsset = g_pLTClient->ResourceMgr()->GetGlobalAssetList();
	g_pLTClient->ResourceMgr()->GetRegionAssetListStats(hGlobalAsset, nGlobalLoadedAssets, nGlobalAssets, nGlobalAssetMem);
	nGlobalAssetMem /= knMegabyte;

	//and now get the stats for the current region
	uint32 nRegionLoadedAssets = 0, nRegionAssets = 0, nRegionAssetMem = 0;
	HREGIONASSETLIST hRegionAsset = g_pLTClient->ResourceMgr()->GetStreamingRegionAssetList(m_hRegion);
	g_pLTClient->ResourceMgr()->GetRegionAssetListStats(hRegionAsset, nRegionLoadedAssets, nRegionAssets, nRegionAssetMem);
	nRegionAssetMem /= knMegabyte;
	float fRegionProgress = (nRegionAssets) ? (float)nRegionLoadedAssets / (float)nRegionAssets : 0.0f;
	
	//and now get the stats for the linked region
	uint32 nLinkedLoadedAssets = 0, nLinkedAssets = 0, nLinkedAssetMem = 0;
	HREGIONASSETLIST hLinkedAsset = g_pLTClient->ResourceMgr()->GetStreamingRegionAssetList(m_hLinked);
	g_pLTClient->ResourceMgr()->GetRegionAssetListStats(hLinkedAsset, nLinkedLoadedAssets, nLinkedAssets, nLinkedAssetMem);
	nLinkedAssetMem /= knMegabyte;
	float fLinkedProgress = (nLinkedAssets) ? (float)nLinkedLoadedAssets / (float)nLinkedAssets : 0.0f;

	if(g_vtResourceStreamingConsoleHUD.GetFloat() != 0.0f)
	{
		char pszRegionName[128];

		//print out our current region information
		g_pLTClient->ResourceMgr()->GetStreamingRegionName(m_hRegion, pszRegionName, LTARRAYSIZE(pszRegionName));
		g_pLTClient->CPrint("Current: %s (%02d%%) %d/%d", pszRegionName, (int32)(fRegionProgress * 100.0f), nRegionAssetMem, nRegionLimit);

		//print out our linked region information
		g_pLTClient->ResourceMgr()->GetStreamingRegionName(m_hLinked, pszRegionName, LTARRAYSIZE(pszRegionName));
		g_pLTClient->CPrint("Linked: %s (%02d%%) %d/%d", pszRegionName, (int32)(fLinkedProgress * 100.0f), nLinkedAssetMem, nLinkedLimit);

		//and print out our global memory statistics and any events
		g_pLTClient->CPrint("Global: %d/%d", nGlobalAssetMem, nGlobalLimit);
		g_pLTClient->CPrint("Events: %c %c %c", ((m_fNotPrefetchedTime > 0.0f) ? 'P' : ' '),
												((m_fNotLoadedTime > 0.0f) ? 'L' : ' '),
												((m_fOutOfMemoryTime > 0.0f) ? 'M' : ' '));
	}

	if(g_vtResourceStreamingHUD.GetFloat() != 0.0f)
	{
		if(!m_bInitializedHUDAssets)
			InitHUDAssets();

		//make sure that our current regions have their labels generated for them
		UpdateRegionLabels();

		//the upper left anchor for this HUD
		const LTVector2n vAnchor(	(int32)(g_vtResourceStreamingHUDX.GetFloat() + 0.5f), 
									(int32)(g_vtResourceStreamingHUDY.GetFloat() + 0.5f));

		//cache our texture string interface, and setup our rendering
		ILTTextureString* pILTTexString = g_pLTClient->GetTextureString();
		pILTTexString->SetupTextRendering(g_pDrawPrim);
		g_pDrawPrim->BeginDrawPrimBlock();

		//calculate the y offset for each row
		const int32 nCurrentRegionY = vAnchor.y;
		const int32 nLinkedRegionY  = nCurrentRegionY + knRowHeight + knRowPadding;
		const int32 nRegionMemoryY  = nLinkedRegionY + knRowHeight + knRowPadding;
		const int32 nGlobalMemoryY  = nRegionMemoryY + knRowHeight + knRowPadding;

		//render the labels for each column
		const float fLabelRight = (float)vAnchor.x + knLabelColumnWidth;
		const int32 nCenterFontY = (knRowHeight - knFontHeight) / 2;

		pILTTexString->RenderString(m_hRegionLabel, g_pDrawPrim, LTVector2(fLabelRight, (float)(nCurrentRegionY + nCenterFontY)), 0xFFFFFFFF, LTVector2(1.0f, 0.0f)); 
		pILTTexString->RenderString(m_hLinkedLabel, g_pDrawPrim, LTVector2(fLabelRight, (float)(nLinkedRegionY + nCenterFontY)), 0xFFFFFFFF, LTVector2(1.0f, 0.0f)); 
		pILTTexString->RenderString(m_hMemoryLabel, g_pDrawPrim, LTVector2(fLabelRight, (float)(nRegionMemoryY + nCenterFontY)), 0xFFFFFFFF, LTVector2(1.0f, 0.0f)); 
		pILTTexString->RenderString(m_hGlobalLabel, g_pDrawPrim, LTVector2(fLabelRight, (float)(nGlobalMemoryY + nCenterFontY)), 0xFFFFFFFF, LTVector2(1.0f, 0.0f)); 

		//now render the graphs
		const uint8 nBarAlpha	 = 0x8F;
		const uint32 knCurrentRegionColor = SETRGBA(0xCF, 0x00, 0x00, nBarAlpha);
		const uint32 knLinkedRegionColor = SETRGBA(0x00, 0x00, 0xCF, nBarAlpha);
		const uint32 knBackgroundColor = SETRGBA(0x00, 0x00, 0x00, nBarAlpha);
		const int32 nGraphLeft   = vAnchor.x + knLabelColumnWidth + knColumnPadding;

		//render the current region progress
		RenderProgressBar(	nGraphLeft, nCurrentRegionY, knProgressBarWidth, knRowHeight, 
							knCurrentRegionColor, knBackgroundColor,
							fRegionProgress);
		pILTTexString->RenderString(m_hCurrentRegionName, g_pDrawPrim, LTVector2((float)nGraphLeft, (float)(nCurrentRegionY + nCenterFontY))); 

		//render the linked region progress
		RenderProgressBar(	nGraphLeft, nLinkedRegionY, knProgressBarWidth, knRowHeight, 
							knLinkedRegionColor, knBackgroundColor,
							fLinkedProgress);
		pILTTexString->RenderString(m_hLinkedRegionName, g_pDrawPrim, LTVector2((float)nGraphLeft, (float)(nLinkedRegionY + nCenterFontY))); 

		//render the memory graph
		float fRegionMemPercent = (float)nRegionAssetMem / (float)nTotalLimit;
		float fLinkedMemPercent = (float)nLinkedAssetMem / (float)nTotalLimit;
		RenderDualProgressBar(	nGraphLeft, nRegionMemoryY, knProgressBarWidth, knRowHeight,
								knCurrentRegionColor, knLinkedRegionColor, knBackgroundColor,
								fRegionMemPercent, fLinkedMemPercent);

		//now handle displaying the memory statistics
		const float fMemoryLeft = (float)(vAnchor.x + knLabelColumnWidth + knColumnPadding + knProgressBarWidth + knColumnPadding);
		wchar_t szFormatBuffer[256];

		//region memory limits
		LTSNPrintF(szFormatBuffer, LTARRAYSIZE(szFormatBuffer), L"%d/%dMB", nRegionAssetMem, nRegionLimit);
		pILTTexString->RenderSubString(m_hValueSource, szFormatBuffer, g_pDrawPrim, LTVector2(fMemoryLeft, (float)(nCurrentRegionY + nCenterFontY))); 

		//linked memory limits
		LTSNPrintF(szFormatBuffer, LTARRAYSIZE(szFormatBuffer), L"%d/%dMB", nLinkedAssetMem, nLinkedLimit);
		pILTTexString->RenderSubString(m_hValueSource, szFormatBuffer, g_pDrawPrim, LTVector2(fMemoryLeft, (float)(nLinkedRegionY + nCenterFontY))); 

		//total memory limit
		LTSNPrintF(szFormatBuffer, LTARRAYSIZE(szFormatBuffer), L"%dMB", nTotalLimit);
		pILTTexString->RenderSubString(m_hValueSource, szFormatBuffer, g_pDrawPrim, LTVector2(fMemoryLeft, (float)(nRegionMemoryY + nCenterFontY))); 

		//global memory limits
		LTSNPrintF(szFormatBuffer, LTARRAYSIZE(szFormatBuffer), L"%d/%dMB", nGlobalAssetMem, nGlobalLimit);
		pILTTexString->RenderSubString(m_hValueSource, szFormatBuffer, g_pDrawPrim, LTVector2((float)nGraphLeft, (float)(nGlobalMemoryY + nCenterFontY))); 

		//and now handle rendering the icons
		int32 nIconStartX = vAnchor.x + knLabelColumnWidth + knColumnPadding + knIconOffset;

		RenderIcon(nIconStartX, nGlobalMemoryY, knIconSize, knIconSize, m_hNotPrefetched, GetIconAlpha(m_fNotPrefetchedTime));
		RenderIcon(nIconStartX + knIconSize, nGlobalMemoryY, knIconSize, knIconSize, m_hNotLoaded, GetIconAlpha(m_fNotLoadedTime));
		RenderIcon(nIconStartX + knIconSize * 2, nGlobalMemoryY, knIconSize, knIconSize, m_hOutOfMemory, GetIconAlpha(m_fOutOfMemoryTime));

		//all done, stop draw prim rendering
		g_pDrawPrim->EndDrawPrimBlock();
	}
}

//called to trigger an event where a resource is not properly prefetched but is accessed
void CGameStreamingMgr::OnAccessUnprefetchedResource(const char* pszResource)
{
	//reset our timing on the icon
	m_fNotPrefetchedTime = kfIconDisplayTime;

	//and log it if appropriate
	//TODO:JO
}

//called to trigger an event where a resource was not loaded when it was accessed
void CGameStreamingMgr::OnAccessUnloadedResource(const char* pszResource)
{
	//reset our timing on the icon
	m_fNotLoadedTime = kfIconDisplayTime;

	//and log it if appropriate
	//TODO:JO
}


//called to deactivate a streaming region since it is no longer relevant
void CGameStreamingMgr::DeactivateRegion(HSTREAMINGREGION hRegion)
{
	if(!hRegion)
		return;

	//disable the visibility in the region
	if(g_vtResourceStreamingUpdateVis.GetFloat() != 0.0f)
	{
		g_pLTClient->EnableRegionSectors(hRegion, false);
	}

	HREGIONASSETLIST hAsset = g_pLTClient->ResourceMgr()->GetStreamingRegionAssetList(hRegion);
	if(hAsset)
	{
		g_pLTClient->ResourceMgr()->DiscardRegionAssetList(hAsset);
	}

	// Now, notify sound that a region has been disabled so all the
	// sounds referencing the resource can be stopped.

	// Naming convention is <World Name>_<region name>

	static const uint32 knMaxNameLen = 128;

	char pBaseName[knMaxNameLen];
	char pName[knMaxNameLen];
	char pFullName[knMaxNameLen];
	int32 i;
	int32 pathpos=0;
		
	g_pLTClient->ResourceMgr()->GetStreamingRegionName(hRegion, pName, sizeof(pName));
	LTStrCpy(pBaseName, g_pMissionMgr->GetCurrentWorldName( ), knMaxNameLen);

	// I'm stripping off any path. there's probably a function to do this
	// somewhere but.. uhh.. it was quicker to rewrite it than look for it.
	// I'll come back and fix it later. -- Terry
	for (i=0; pBaseName[i] != 0; i++)
	{
		if ( (pBaseName[i] == '/') || (pBaseName[i] == '\\') )
		{
			pathpos = i+1;
		}
	}
	LTStrCpy(pFullName, &(pBaseName[pathpos]), knMaxNameLen);
	LTStrCat(pFullName, "_", knMaxNameLen);
	LTStrCat(pFullName, pName, knMaxNameLen);


	// we finally have the ID so deactivate it!
	ILTClientSoundMgr *pSoundMgr = (ILTClientSoundMgr *)g_pLTClient->SoundMgr();
	pSoundMgr->DeactivateSoundRegion(pFullName);
}

//alled to activate a streaming region when it becomes relevant
void CGameStreamingMgr::ActivateRegion(HSTREAMINGREGION hRegion)
{
	if(!hRegion)
		return;

	//enable the visibility in the region
	if(g_vtResourceStreamingUpdateVis.GetFloat() != 0.0f)
	{
		g_pLTClient->EnableRegionSectors(hRegion, true);
	}

	HREGIONASSETLIST hAsset = g_pLTClient->ResourceMgr()->GetStreamingRegionAssetList(hRegion);
	if(hAsset)
	{
		g_pLTClient->ResourceMgr()->PrefetchRegionAssetList(hAsset);
	}
}

//called to update the labels associated with the current streaming regions, and will regenerate
//labels for any region that has a NULL label
void CGameStreamingMgr::UpdateRegionLabels()
{
	if(!m_hCurrentRegionName)
	{
		CreateRegionLabel(m_hRegion, m_hCurrentRegionName);
	}

	if(!m_hLinkedRegionName)
	{
		CreateRegionLabel(m_hLinked, m_hLinkedRegionName);
	}
}

//given a region, this will generate a texture string for the name
void CGameStreamingMgr::CreateRegionLabel(HSTREAMINGREGION hRegion, HTEXTURESTRING& hLabel)
{
	static const uint32 knMaxNameLen = 128;

	//get the name of the region
	char pszRegionName[knMaxNameLen];
	g_pLTClient->ResourceMgr()->GetStreamingRegionName(hRegion, pszRegionName, LTARRAYSIZE(pszRegionName));

	//convert that over to a wide character
	wchar_t pszWideName[knMaxNameLen];

	uint32 nNumChars = LTStrLen(pszRegionName) + 1;
	for(uint32 nCurrChar = 0; nCurrChar < nNumChars; nCurrChar++)
		pszWideName[nCurrChar] = (wchar_t)pszRegionName[nCurrChar];

	//and now generate the label
	hLabel = g_pLTClient->GetTextureString()->CreateTextureString(pszWideName, m_TextFont);
}

//called to initialize all of the HUD assets
void CGameStreamingMgr::InitHUDAssets()
{
	//clear out any old assets
	TermHUDAssets();

	//setup our text strings
    m_hValueSource = g_pLTClient->GetTextureString()->CreateTextureString(L"0123456789.MB/", m_TextFont);
	m_hRegionLabel = g_pLTClient->GetTextureString()->CreateTextureString(L"Region", m_TextFont);
	m_hLinkedLabel = g_pLTClient->GetTextureString()->CreateTextureString(L"Linked", m_TextFont);
	m_hGlobalLabel = g_pLTClient->GetTextureString()->CreateTextureString(L"Global", m_TextFont);
	m_hMemoryLabel = g_pLTClient->GetTextureString()->CreateTextureString(L"Memory", m_TextFont);

	//now load up our textures
	char pszIconPath[MAX_PATH];
	LTStrCpy(pszIconPath, g_vtResourceStreamingIconPath.GetStr(), LTARRAYSIZE(pszIconPath));

	//and append a slash on the end if it doesn't exist
	uint32 nIconPathLen = LTStrLen(pszIconPath);
	if((nIconPathLen > 0) && (pszIconPath[nIconPathLen - 1] != '\\'))
		LTStrCat(pszIconPath, "\\", LTARRAYSIZE(pszIconPath));

	char pszIconName[MAX_PATH];

	LTSNPrintF(pszIconName, LTARRAYSIZE(pszIconName), "%s%s", pszIconPath, "ResNotPrefetched.dds");
	g_pLTClient->GetTextureMgr()->CreateTextureFromFile(m_hNotPrefetched, pszIconName);

	LTSNPrintF(pszIconName, LTARRAYSIZE(pszIconName), "%s%s", pszIconPath, "ResNotLoaded.dds");
	g_pLTClient->GetTextureMgr()->CreateTextureFromFile(m_hNotLoaded, pszIconName);

	LTSNPrintF(pszIconName, LTARRAYSIZE(pszIconName), "%s%s", pszIconPath, "ResOutOfMemory.dds");
	g_pLTClient->GetTextureMgr()->CreateTextureFromFile(m_hOutOfMemory, pszIconName);

	//and we are now initialized
	m_bInitializedHUDAssets = true;
}

//called to free all of the initialized HUD assets
void CGameStreamingMgr::TermHUDAssets()
{
	//free all of our texture strings
	g_pLTClient->GetTextureString()->ReleaseTextureString(m_hCurrentRegionName);
	m_hCurrentRegionName = NULL;

	g_pLTClient->GetTextureString()->ReleaseTextureString(m_hLinkedRegionName);
	m_hLinkedRegionName = NULL;

	g_pLTClient->GetTextureString()->ReleaseTextureString(m_hValueSource);
	m_hValueSource = NULL;

	g_pLTClient->GetTextureString()->ReleaseTextureString(m_hRegionLabel);
	m_hRegionLabel = NULL;

	g_pLTClient->GetTextureString()->ReleaseTextureString(m_hLinkedLabel);
	m_hLinkedLabel = NULL;

	g_pLTClient->GetTextureString()->ReleaseTextureString(m_hGlobalLabel);
	m_hGlobalLabel = NULL;

	g_pLTClient->GetTextureString()->ReleaseTextureString(m_hMemoryLabel);
	m_hMemoryLabel = NULL;

	//and free all of our individual textures
	g_pLTClient->GetTextureMgr()->ReleaseTexture(m_hNotPrefetched);
	m_hNotPrefetched = NULL;

	g_pLTClient->GetTextureMgr()->ReleaseTexture(m_hNotLoaded);
	m_hNotLoaded = NULL;

	g_pLTClient->GetTextureMgr()->ReleaseTexture(m_hOutOfMemory);
	m_hOutOfMemory = NULL;

	//and clear our initialized flag
	m_bInitializedHUDAssets = false;
}




