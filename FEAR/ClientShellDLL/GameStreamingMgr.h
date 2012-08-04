//---------------------------------------------------------------------------------
// GameStreamingMgr.h
// 
// This is a manager that handles updating the resource system in response to game
// events. This is responsible for managing the currently active streaming regions
// that the player is involved in.
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
//---------------------------------------------------------------------------------
#ifndef __GAMESTREAMINGMGR_H__
#define __GAMESTREAMINGMGR_H__

#ifndef __ILTRESOURCEMGR_H__
#	include "iltresourcemgr.h"
#endif

class CGameStreamingMgr
{
public:

	~CGameStreamingMgr();

	//singleton access
	static CGameStreamingMgr&	Singleton();

	//called to initialize the streaming system at an application wide level
	void	Init();

	//called to clean up the streaming system at an application wide level
	void	Term();

	//called after a level has been loaded to allow the streaming to perform any necessary operations
	void	OnLevelLoaded();

	//called to update the current streaming data based upon the point provided to the callback
	void	UpdateRegions(const LTVector& vPos);

	//called to clear out any currently associated regions
	void	ClearCurrentRegions();

	//called to display the HUD to the screen
	void	DisplayStreamingHUD(float fElapsedTime);

	//---------------------------------------------
	// Event notifiers
	
	//called to trigger an event where a resource is not properly prefetched but is accessed
	void	OnAccessUnprefetchedResource(const char* pszResource);

	//called to trigger an event where a resource was not loaded when it was accessed
	void	OnAccessUnloadedResource(const char* pszResource);
	
private:

	//singleton access only
	CGameStreamingMgr();
	PREVENT_OBJECT_COPYING(CGameStreamingMgr);

	//------------------------------------------
	// Streaming Support

	//called to deactivate a streaming region since it is no longer relevant
	void	DeactivateRegion(HSTREAMINGREGION hRegion);

	//alled to activate a streaming region when it becomes relevant
	void	ActivateRegion(HSTREAMINGREGION hRegion);

	//the streaming region that we are currently within
	HSTREAMINGREGION	m_hRegion;

	//the streaming region that we are currently linked to (need to prefetch before we get there)
	HSTREAMINGREGION	m_hLinked;

	//------------------------------------------
	// HUD Support

	//called to update the labels associated with the current streaming regions, and will regenerate
	//labels for any region that has a NULL label
	void				UpdateRegionLabels();

	//given a region, this will generate a texture string for the name
	void				CreateRegionLabel(HSTREAMINGREGION hRegion, HTEXTURESTRING& hLabel);

	//called to initialize all of the HUD assets
	void				InitHUDAssets();

	//called to free all of the initialized HUD assets
	void				TermHUDAssets();

	//have we initialized the HUD assets?
	bool				m_bInitializedHUDAssets;

	//the time associated with each icon. These will tick down until zero at which point they will
	//no longer be visible
	float				m_fNotPrefetchedTime;
	float				m_fNotLoadedTime;
	float				m_fOutOfMemoryTime;

	//the font that we are using for our text
	CFontInfo			m_TextFont; 

	//the texture string associated with the current streaming region name
	HTEXTURESTRING		m_hCurrentRegionName;

	//the texture string associated with the linked streaming region name
	HTEXTURESTRING		m_hLinkedRegionName;

	//the texture string used for displaying of numbers and memory counts
	HTEXTURESTRING		m_hValueSource;

	//the texture strings used for different labels on the HUD
	HTEXTURESTRING		m_hRegionLabel;
	HTEXTURESTRING		m_hLinkedLabel;
	HTEXTURESTRING		m_hGlobalLabel;
	HTEXTURESTRING		m_hMemoryLabel;

	//the texture used for the different icons on the HUD
	HTEXTURE			m_hNotPrefetched;
	HTEXTURE			m_hNotLoaded;
	HTEXTURE			m_hOutOfMemory;
};

#endif
