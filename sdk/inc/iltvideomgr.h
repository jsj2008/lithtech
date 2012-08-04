#ifndef __ILTVIDEOMGR_H__
#define __ILTVIDEOMGR_H__

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif


class VideoInst;
typedef VideoInst* HVIDEO;

/*!  
The ILTVideoMgr interface.  

Define a holder to get this interface like this:
\code
define_holder(ILTVideoMgr, your_var);
\endcode

Note: all routines in here will return \b LT_NOTINITIALIZED if the
engine can't initialize the system's video modules or the runtime DLLs
are missing.  
*/

class ILTVideoMgr : public IBase {
public:
    interface_version(ILTVideoMgr, 0);	

/*!
\param pFilename	Video filename.
\param flags		Video playback flags (see \b PLAYBACK_ defines in ltcodes.h).
\param hVideo		(return) Handle to video.

\return \b LT_NOTINITIALIZED - Video and/or renderer module not initialized. Can generally be fixed by making sure that binkw32.dll is loaded by placing it in the working directory.
\return \b LT_INVALIDPARAMS - \em pFilename is NULL.
\return \b LT_OUTOFMEMORY - Could not load video file.
\return \b LT_MISSINGFILE - \em pFilename is invalid (can't find file).
\return \b LT_ERROR - Internal DirectDraw error while initializing screen.
\return \b LT_OK - Successful.

Used for: Special FX.
*/
	virtual LTRESULT StartOnScreenVideo(const char *pFilename, uint32 flags, 
		HVIDEO &hVideo) = 0;

/*!
\param hVideo	Handle to video.

\return \b LT_NOTINITIALIZED - Video module not initialized or some other internal rendering internal error. Can generally be fixed by making sure that binkw32.dll is loaded by placing it in the working directory.
\return \b LT_INVALIDPARAMS - \em hVideo is invalid.
\return \b LT_OK - Successful.

For on-screen videos, this draws the current video to the screen.

Used for: Special FX.
*/
	virtual LTRESULT UpdateVideo(HVIDEO hVideo) = 0;

/*!
\param hVideo	Handle to video.

\return \b LT_NOTINITIALIZED - Video module not initialized. Can generally be fixed by making sure that binkw32.dll is loaded by placing it in the working directory.
\return \b LT_INVALIDPARAMS - \em hVideo is invalid.
\return \b LT_FINISHED - Finished playing.
\return \b LT_OK - Still playing.

When it's done playing, it will sit at the last frame.  You \em must call 
StopVideo() to free a video's resources.

\see StopVideo()

Used for: Special FX.
*/
	virtual LTRESULT GetVideoStatus(HVIDEO hVideo) = 0;

/*!
\param hVideo	Handle to video.

\return \b LT_NOTINITIALIZED - Video module not initialized. Can generally be fixed by making sure that binkw32.dll is loaded by placing it in the working directory.
\return \b LT_INVALIDPARAMS - \em hVideo is invalid.
\return \b LT_OK - Successful.

Stop playing a video.  For a texture video, this restores the original 
textures on any polygons to which it is bound.

Used for: Special FX.
*/
	virtual LTRESULT StopVideo(HVIDEO hVideo) = 0;
};


#endif  //! __ILTVIDEOMGR_H__

