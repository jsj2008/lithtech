
#ifndef __VIDEOMGR_H__
#define __VIDEOMGR_H__

class Surface;
class SharedTexture;

LTRESULT VMSurfaceLeechFn(Nexus *pNexus, Leech *pLeech, int msg, void *pUserData);
LTRESULT VMTextureLeechFn(Nexus *pNexus, Leech *pLeech, int msg, void *pUserData);
extern LeechDef g_VMSurfaceLeechDef;
extern LeechDef g_VMTextureLeechDef;

class VideoInst
{
protected:

	virtual			~VideoInst() {}


public:

	virtual void	Release() {}

	virtual void	OnRenderInit()=0;
	virtual void	OnRenderTerm()=0;

	// Update the video playback.  
	// Returns LT_OK *if a new frame is ready*.
	// Returns LT_INPROGRESS if the old frame's contents still hold.
	// Returns LT_FINISHED if the video is done playing.
	virtual LTRESULT Update()=0;

	// If this is an on-screen video, this draws the video to the screen.
	virtual LTRESULT DrawVideo()=0;

	// LT_OK = still playing
	// LT_FINISHED = done playing
	virtual LTRESULT GetVideoStatus()=0;

public:

	CMLLNode		m_Link;
};


class VideoMgr
{
public:

	VideoMgr() {}

	// Create a specific type of video and adds it to m_Videos.
	virtual LTRESULT CreateScreenVideo(const char *pFilename, uint32 flags, VideoInst* &pVideo)=0;
	
	// Updates all the videos.. updates textures using texture videos.
	virtual void	UpdateVideos();

	// Calls through to all the videos.
	virtual void	OnRenderInit();
	virtual void	OnRenderTerm();

public:

	// Currently playing videos.
	CMultiLinkList<VideoInst*>	m_Videos;
	

};

// Create and initialize a video manager.  Returns NULL if it can't be initialized
// (ie: if the smacker DLLs are missing).
extern VideoMgr* CreateVideoMgr(const char *pName);


#endif

