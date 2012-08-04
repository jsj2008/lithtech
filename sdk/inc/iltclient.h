#ifndef __ILTCLIENT_H__
#define __ILTCLIENT_H__

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __ILTCSBASE_H__
#include "iltcsbase.h"
#endif

#ifndef __ILTINFO_H__
#include "iltinfo.h"
#endif

#ifdef LITHTECH_ESD
#include "iltesd.h"
#endif // LITHTECH_ESD

#include <vector>

//@{
/*!
\name Particle system flags

Each flag slows the system down a little more,
except for \b PS_NEVERDIE, which speeds it up.

Used for: Special FX.
*/

enum
{
//! Do they bounce?
    PS_BOUNCE =       (1<<0),

//! The particles never die (and it doesn't have to check).
    PS_NEVERDIE =     (1<<2),

//! The engine leaves the system alone; you must update and move them.
    PS_DUMB =         (1<<3),

//! Should the rotation associated with a particle be used?
	PS_USEROTATION =  (1<<4),

//! Should the particle system be lit dynamically?
	PS_LIGHT =		  (1<<5),

//! Should the particle system consider collisions with objects?
	PS_COLLIDE =	  (1<<6),

//! Are the particles in world space? If flag is not set, they are in object space.
	PS_WORLDSPACE =	  (1<<7)
};
//@}

/*!
The particle structure for particle systems.

Used for: Special FX.
*/
struct LTParticle
{
    LTVector    m_Pos;				//! Current position of the particle
    LTVector    m_Velocity;			//! Current velocity of the particle
	float       m_Size;				//! Particle size

    LTVector    m_Color;			//! Color of the particle: RGB 0-255
    float       m_Alpha;			//! Alpha of the particle: 0-1

	float		m_fAngle;			//! Angle in radians of this particle
	float		m_fAngularVelocity;	//! Velocity of the angular change in radians per second

    float       m_Lifetime;         //! Current lifetime left
    float       m_TotalLifetime;    //! Total lifetime (i.e. initial value)

	uint32		m_nUserData;		//! 32 bits of user data...

    LTParticle  *m_pNext;			//! Linked list information
    LTParticle  *m_pPrev;
};

//@{
/*!
\name Poly Grid flags

Specifies specific control for polygrids

Used for: Special FX.
*/

enum
{

//! When rendering, this polygrid should be rendered after solid objects, but before translucent objects. This can be used to fix issues with shallow polygrids that cover large areas and cause draw order issues
	PG_RENDERBEFORETRANSLUCENTS	=	(1<<0),

//! Apply fresnel blending on the environment map. This looks very nice in some situations but is slower and will not work with lighting
    PG_FRESNEL =					(1<<1),

//! Don't apply backface culling on the polygrid. This is intended for polygrids that you can see from above and below
	PG_NOBACKFACECULL =				(1<<2),

//! This flag indicates that the sprite surface of the polygrid is entirely a normal map, and if not rendering with bump mapping, to not use any texture
	PG_NORMALMAPSPRITE =			(1<<3)
};
//@}

/*!
The line point structure for line systems.

Used for: Special FX.
*/
struct LTLinePt
{
    LTVector    m_Pos;              //! Position (relative to system).

    float       r, g, b, a;         //! Color/Alpha (0-1).
};

/*!
The line structure for line systems.

Used for: Special FX.
*/
struct LTLine
{
    LTLinePt    m_Points[2];        //! Endpoints.
};

DEFINE_HANDLE_TYPE(HLTLINE)

/*!  Music info */

enum
{
    MUSIC_IMMEDIATE =         0,
    MUSIC_NEXTMEASURE =       1,
    MUSIC_NEXTSONG =          2,
    MUSIC_QUEUE =             3
};

class CSoundInstance;

class ILTSpriteControl;
class ILTLightAnim;
class ILTTexInterface;

class ILTClientPhysics;
class ILTVideoMgr;
class ILTModel;
class ILTTransform;
class ILTCursor;
class ILTClientSoundMgr;
class ILTDirectMusicMgr;
class ILTBenchmarkMgr;
class ILTDrawPrim;
class ILTFontManager;
class ILTWidgetManager;
struct InitSoundInfo;
struct ObjectCreateStruct;
class ILTMessage_Read;
class LTGraphicsCaps;

#ifdef LITHTECH_ESD
class ILTRealAudioMgr;
class ILTRealConsoleMgr;
class ILTRealVideoMgr;
#endif // LITHTECH_ESD



/*!
ILTClient interface. This is what the ClientShells and special
effect plugins use to generate the interface and client objects for
the game.

Define a holder to get this interface like this:
\code
define_holder(ILTClient, your_var);
\endcode
*/

class ILTClient : public ILTCSBase
{
public:
    interface_version_derived(ILTClient, ILTCSBase, 4);

//@}

/*!
File management.
*/

/*!
\param  pDirName    Directory path.
\return Non-circular, \b LTNULL terminated linked list of file/directory names.

Get a list of files/directories in a directory (pass in "" to start
with).  Use FreeFileList() when you're done with each list you get.

\see FreeFileList()

Used for: Misc.
*/
    FileEntry *(*GetFileList)(const char *pDirName);

/*!
\param  pHead   Head of list acquired with GetFileList().

Frees up memory allocated by GetFileList().

\see GetFileList()

Used for: Misc.
*/
    void (*FreeFileList)(FileEntry *pHead);

/*!
\param  pFilename       The configuration file.
\param  nNumValues		The number of values to write out, 0 will write all of them
\param  pValues			The names of each value to be saved out. There should be nNumValues of them.
\return \b LT_ERROR if it can't open the file for writing.

Write configuration files.

Used for: Misc.
*/
    LTRESULT (*WriteConfigFileFields)(const char *pFilename, uint32 nNumValues, const char** pValues);

/*!
\param  pFilename       The configuration file.
\return \b LT_ERROR if it can't open the file for writing.

Write configuration files.

Used for: Misc.
*/
    LTRESULT (*WriteConfigFile)(const char *pFilename);


/*!
\param  pFilename       The configuration file.
\return \b LT_ERROR if the file doesn't exist or if it can't open the file for writing.

Read and parse a configuration file.

Used for: Misc.
*/
    LTRESULT (*ReadConfigFile)(const char *pFilename);


/*!
Interface stuff.
*/

/*!
\param  offsets     (return) Array of offsets (size of array is \b NUM_AXIS_OFFSETS).

Get the offsets from the last frame of all of the currently bound
axis. Most often this is mapped to the mouse.

Used for: Input.
*/
    void (*GetAxisOffsets)(LTFLOAT *offsets);

/*!
\param  pEffectName     Effect to play.
\param  x               Data for effect.
\param  y               Data for effect.

Play a force-feedback joystick effect.

Used for: Input.
*/
    void (*PlayJoystickEffect)(const char *pEffectName, float x, float y);

/*!
Music functions.
*/

/*!
\param  szMusicDLL      Music dll filename.
\return
\b FALSE    if the provided redbook audio manager failed to initialize
\b TRUE     otherwise

Initialize the music subsystem.  Currently the only music dll that exists is
for red book audio.  IMA is no longer supported by the engine, it has been
replaced with DirectMusic which can be used from the ILTDirectMusic interface.
If you are not using red book audio you do not need to call this function!

Used for: Music.
*/
    bool (*InitMusic)(const char *szMusicDLL);

/*!
\param  szSong      Name of song.
\return
\b FALSE    if the redbook audio manager was not properly initialized
\b FALSE    is the song could not found or loaded
\b TRUE     otherwise

Cache a song file.

This function is now only used if you are playing red book audio.  If you are
using DirectMusic use the functions in the ILTDirectMusic interface.

Used for: Music.
*/
    bool (*LoadSong)(const char *szSong);

/*!
Remove all songs from memory.

This function is now only used if you are playing red book audio.  If you are
using DirectMusic use the functions in the ILTDirectMusic interface.

Used for: Music.
*/
    void (*DestroyAllSongs)();

/*!
\param  szPlayList          Name of playlist.
\param  szTransition        Name of transition song to play before playlist starts. IMA only.
\param  bLoop               Loop the list.

\param dwBoundaryFlags Can be \b MUSIC_IMMEDIATE, \b MUSIC_MEASURE, \b
MUSIC_SONG, or \b MUSIC_QUEUE.  Valid for IMA only.

\return
\b FALSE    if the redbook audio manager was not properly initialized
\b FALSE    if the playlist could not be found or started
\b TRUE     otherwise

Play a playlist.  All songs will be played until end of list.

This function is now only used if you are playing red book audio.  If you are
using DirectMusic use the functions in the ILTDirectMusic interface.

Used for: Music.
*/
    bool (*PlayList)(const char *szPlayList, const char *szTransition,
        bool bLoop, uint32 dwBoundaryFlags);

/*!
\param dwBoundaryFlags Can be \b MUSIC_IMMEDIATE, \b
MUSIC_MEASURE, \b MUSIC_SONG, or \b MUSIC_QUEUE.  Valid for IMA only.

Stop music.

This function is now only used if you are playing red book audio.  If you are
using DirectMusic use the functions in the ILTDirectMusic interface.

Used for: Music.
*/
    void (*StopMusic)(uint32 dwBoundaryFlags);

/*!
Pause music.  Can be resumed.

This function is now only used if you are playing red book audio.  If you are
using DirectMusic use the functions in the ILTDirectMusic interface.

\return
\b FALSE    if the redbook audio manager was not properly initialized
\b FALSE    if the redbook audio manager failed to pause the current song
\b TRUE     otherwise

Used for: Music.
*/
    bool (*PauseMusic)();

/*!
Resume music.

This function is now only used if you are playing red book audio.  If you are
using DirectMusic use the functions in the ILTDirectMusic interface.

\return
\b FALSE is the redbook audio manager was not properly initialized
\b FALSE if there is no current song playing, or the redbook audio manager failed to resume
\b TRUE otherwise

Used for: Music.
*/
    bool (*ResumeMusic)();

/*!
\param  szPlayList      Name of playlist.

\param  szSong          Name of song to add to playlist.
\return
\b FALSE    if the redbook audio manager was not properly initialized
\b FALSE    if the provided playlist could not be found, or the song could not be added
\b TRUE     otherwise

Create and add to playlists.

This function is now only used if you are playing red book audio.  If you are
using DirectMusic use the functions in the ILTDirectMusic interface.

Used for: Music.
*/
    bool (*AddSongToPlayList)(const char *szPlayList, const char *szSong);

/*!
\param  szPlayList      Name of playlist.

Remove a whole playlist.

This function is now only used if you are playing red book audio.  If you are
using DirectMusic use the functions in the ILTDirectMusic interface.

Used for: Music.
*/
    void (*DeletePlayList)(const char *szPlayList);

/*!
\return Volume value (range: 0=off, 100=full).

Get music volume.

This function is now only used if you are playing red book audio.  If you are
using DirectMusic use the functions in the ILTDirectMusic interface.

\see SetMusicVolume()

Used for: Music.
*/
    short (*GetMusicVolume)();

/*!
\param  wVolume     New music volume.

Set music volume (range: 0=off, 100=full).

This function is now only used if you are playing red book audio. If you are
using DirectMusic use the functions in the ILTDirectMusic interface.

\see GetMusicVolume()

Used for: Music.
*/
    void (*SetMusicVolume)(short wVolume);

/*!
Sound functions.
*/

/*!
These PlaySound() functions allow the client shell to initiate and control
sounds without going through the server.  These sounds will only be played
on the client initiating them. These functions will fill \b m_hSound
with the handle to the client sound if you set the \b PLAYSOUND_GETHANDLE
flag.  This will also force the sound not to be automatically deleted when
the sound is done playing.  You must call KillSound(). You must get the
handle in order to use the other sound functions like SetSoundPosition().
*/

/*!
\param hSound Handle to sound.

\param fTimer Current time of sound.

\return \b LT_FINISHED when sound is not playing (may be delayed due to
collisions), or \b LT_INVALIDPARAMS when \b hSound is not available;
otherwise, returns \b LT_OK.

Get current time in seconds of sound.
fTimer is set only if \b LT_OK is returned.

Used for: Audio.
*/
    LTRESULT (*GetSoundTimer)(HLTSOUND hSound, LTFLOAT *fTimer);

/*!
\param hSound Handle to sound.

\param pSixteenBitData The data buffer if the buffer is 16 bits per
sample, or \b NULL otherwise.

\param pEightBitData The data buffer if the buffer is 8 bits per
sample, or \b NULL otherwise.

\param dwSamplesPerSecond Samples per second.

\param dwChannels Number of channels. (Samples alternate between each
channel.)

\return \b LT_ERROR when sound is in neither 8- nor 16-bit format, or
\b LT_INVALIDPARAMS if \b hSound is not available; otherwise, returns
\b LT_OK.

Get the sound data and information about the data.  This will force
the sound to be decompressed. fTimer is set only if \b LT_OK is returned.

Used for: Audio.
*/
    LTRESULT (*GetSoundData)(HLTSOUND hSound,
        int16 *&pSixteenBitData, int8 *&pEightBitData,
        uint32 *dwSamplesPerSecond, uint32 *dwChannels);

/*!
\param hSound Handle to sound.

\param dwOffset Current offset into sound buffer (in buffer
increments: bytes if 8 bit, double bytes if 16 bit).

\param dwSize Size of sound buffer (in bytes).

\return \b LT_FINISHED when the sound is not playing
(may be delayed due to collisions, or done), or \b LT_INVALIDPARAMS when
\b hSound is not available; otherwise, returns \b LT_OK.

Get current offset into sound buffer of sound.
\b fTimer is set only if \b LT_OK is returned.

Used for: Audio.
*/
    LTRESULT (*GetSoundOffset)(HLTSOUND hSound, uint32 * dwOffset, uint32 * dwSize );

/*!
\param  pSoundHandle    Handle to client only sound.
\return
\b TRUE     if the sound is either no longer playing, or not found
\b FALSE    otherwise

Check whether a sound is finished playing or whether the object it was
attached to has been removed.

Used for: Audio.
*/
    bool (*IsDone)(HLTSOUND pSoundHandle);

/*!
\return Results of SetListener().

\see SetListener()

Used for: Audio.
*/
    void (*GetListener)(bool *bListenerInClient, LTVector *pPos, LTRotation *pRot);

/*!
Intersections.
*/

    bool (*IntersectSegment)(ClientIntersectQuery *pQuery, ClientIntersectInfo *pInfo);
	bool (*IntersectSweptSphere)(const LTVector& vStart, const LTVector& vEnd, float fRadius, LTVector& vFinalPos, LTVector& vNormal);
    bool (*CastRay)(ClientIntersectQuery *pQuery, ClientIntersectInfo *pInfo);

    void (*SetObjectPosAndRotation)(HLOCALOBJ hObj,
                const LTVector *pPos, const LTRotation *pRotation);

	virtual LTRESULT GetObjectPos(HLOCALOBJ hObj, LTVector *pPos) = 0;
    virtual LTRESULT SetObjectPos(HLOCALOBJ hObj, const LTVector *pPos, bool bForce=true) = 0;

    virtual LTRESULT GetObjectRotation(HLOCALOBJ hObj, LTRotation *pRotation) = 0;

    LTRESULT (*FindObjectsInSphere)(const LTVector *pCenter, float radius,
                HLOCALOBJ *inObjects, uint32 nInObjects, uint32 *nOutObjects,
                uint32 *nFound);
    LTRESULT (*FindObjectsInBox)(const LTVector *pCenter, float radius,
        HLOCALOBJ *inObjects, uint32 nInObjects, uint32 *nOutObjects,
        uint32 *nFound);

/*!
\return Pointer to the \b ILTVideoMgr interface.

Accesses the ILTVideoMgr interface.

Used for: Interface.
*/
    virtual ILTVideoMgr *VideoMgr() = 0;

/*!
Access to the ILTDrawPrim interface.

\return The ILTDrawPrim interface.

Used for: Client Management.
*/
    virtual ILTDrawPrim *GetDrawPrim() = 0;

/*!
Access to the ILTTexInterface interface.

\return The ILTTexInterface interface.

Used for: Client Management.
*/
    virtual ILTTexInterface *GetTexInterface() = 0;

/*!
Access to the ILTWidgetManager interface.

\return The ILTWidgetManager interface.


Used for: Text and UI.
*/
    virtual ILTWidgetManager *GetWidgetManager() = 0;

/*!
Access to the ILTFontManager interface.

\return The ILTFontManager interface.

Used for: Text and UI.
*/
    virtual ILTFontManager *GetFontManager() = 0;

/*!
\return Pointer to the \b ILTCursor interface.

Accesses the ILTCursor interface.

Used for: Interface.
*/
    virtual ILTCursor *Cursor() = 0;

/*!
\return Pointer to the \b ILTDirectMusicMgr interface.

Accesses the ILTDirectMusicMgr interface.

Used for: Interface.
*/
    virtual ILTDirectMusicMgr *GetDirectMusicMgr() = 0;

/*!
\return Pointer to the \b ILTBenchmarkMgr interface.

Accesses the ILTBenchmarkMgr interface.

Used for: Interface.
*/
    virtual ILTBenchmarkMgr *GetBenchmarkMgr() = 0;

    #ifdef LITHTECH_ESD
/*!
Accesses the ILTRealAudioMgr interface.

\return Pointer to the \b ILTRealAudioMgr interface.

Used for: Interface.
*/
    ILTRealAudioMgr *GetRealAudioMgr() {return m_pRealAudioMgr;}

/*!
Accesses the ILTRealConsoleMgr interface.

\return Pointer to the \b ILTRealConsoleMgr interface.

Used for: Interface.
*/
    ILTRealConsoleMgr *GetRealConsoleMgr() {return m_pRealConsoleMgr;}

/*!
Accesses the ILTRealVideoMgr interface.

\return Pointer to the \b ILTRealVideoMgr interface.

Used for: Interface.
*/
    ILTRealVideoMgr *GetRealVideoMgr() {return m_pRealVideoMgr;}

/*!
Accesses the ILTRealVideoPlayer interface.

\return Pointer to the \b ILTRealVideoPlayer interface.

Used for: Interface.
*/
    ILTRealVideoPlayer *GetRealVideoPlayer() {return m_pRealVideoPlayer;}
    #endif // LITHTECH_ESD

/*!
\defgroup ConnectionStuff
Connection stuff
*/

/*!
\ingroup ConnectionStuff
\param  mode    (return) The current game mode, specified as one of the \b STARTGAME_ defines.

\return LT_OK.

Find out what the current game mode is (set when we started the last
game with StartGame() or through the console).

\see StartGame()

Used for: Client Management.
*/
    LTRESULT (*GetGameMode)(int *mode);

/*!
\ingroup ConnectionStuff
\param  pRequest    \b StartGameRequest structure.

\return \b LT_NOTINITIALIZED when there is no driver to match the request,
LT_ERROR when unable to sucessfully host or join a lobby session,
LT_INVALIDPARAMS when the \b m_pNetSession pointer in the StartGameRequest is NULL,
LT_SERVERERROR when there is an error loading the world (Server/host only); otherwise, returns LT_OK.

Try to start a game in the mode you specify.
The existing game will keep running until the new game successfully starts.
For instance, if there is an error starting the new world, it
will still leave the current one running and put up an error message.

Used for: Client Management.
*/
    LTRESULT (*StartGame)(StartGameRequest *pRequest);

/*!
\ingroup ConnectionStuff

\param pID (return) The local client ID.
\return \b LT_NOTCONNECTED when we're not on a server or haven't gotten a
client ID yet; otherwise, returns \b LT_OK.

Get the local client ID.

Used for: Client Management.
*/
    LTRESULT (*GetLocalClientID)(uint32 *pID);

/*!
\ingroup ConnectionStuff

Disconnect from the server we're on (if any).

\note This will destroy any client-side objects you've created.

Used for: Client Management.
*/
    void (*Disconnect)();

/*!
\return \b true when connected to server; otherwise, returns false.

Determine whether this client is currently connected to a server.
(The function checks whether the ClientMgr's m_pCurShell is valid.)

Used for: Client Management.
*/
    bool (*IsConnected)();

/*!
\ingroup ConnectionStuff

Shut down the app (not right away, but after the current update).

Used for: Client Management.
*/
    void (*Shutdown)();

/*!
\ingroup ConnectionStuff

\param pMsg The printf-styled format string to display.  This
parameter can be followed by the usual data parameters.

Shut down the app (not right away, but after the current update) and display a message.

Used for: Client Management.
*/
    void (*ShutdownWithMessage)(const char *pMsg, ...);

/*!
\param  pPoint      The point in world coordinates.
\param  pColor      (return) The color value at pPoint.
\return \b LT_NOTINWORLD if the point is outside the world.

Calculate the lighting shade (RGB, 0-255) at the point you specify.

Used for: Misc.
*/
    virtual LTRESULT GetPointShade(const LTVector *pPoint, LTVector *pColor)=0;


//! \name Renderer management
//@{

/*!
\param  flags   Flags to use when performing the flip (a combination of \b FLIPSCREEN_
                flags in ltcodes.h).
\return \b LT_OK or \b LT_NOTINITIALIZED.

Flip the screen.

Used for: Rendering.
*/
    LTRESULT (*FlipScreen)(uint32 flags);

/*!
\return \b LT_OK or \b LT_NOTINITIALIZED or \b LT_ALREADYIN3D.

Start a Start3D()/End3D() block.

\note You must be in a Start3D()/End3D() block in order to render
through any cameras.

\see End3D()

Used for: Rendering.
*/
    LTRESULT (*Start3D)();

/*!
\param  hCamera     The camera to render.
\param	fFrameTime	The frame time. This is used to update things like sprites and texture scripts. Normally this can just be GetFrameTime, or can be 0 if paused
\return \b LT_OK, \b LT_NOTINITIALIZED, or \b LT_NOTIN3D.

Render from the position of the camera into its rectangle.

Used for: Rendering.
*/
    LTRESULT (*RenderCamera)(HLOCALOBJ hCamera, float fFrameTime);

/*!
\param  pClearRect  The rectangle area to clear. \b NULL means the entire screen.
\param  flags       Flags to use when performing the clear screen (a combination of
                    \b CLEARSCREEN_ flags in ltcodes.h).
\param  pClearColor Color to clear the screen to. Passing NULL results in black.
\return \b LT_OK or \b LT_NOTINITIALIZED.

Clear the Z-Buffer and Backbuffer. In most cases you will want to clear the screen before you enter your Start3D()/End3D() block.

Used for: Rendering.
*/
    LTRESULT (*ClearScreen)(LTRect *pClearRect, uint32 flags, LTRGB* pClearColor );

/*!
\param  hCamera     The camera to render.
\param  pObjects    An array of objects to render.
\param  nObjects    The size of the \b pObjects array.
\param	fFrameTime	The frame time. This is used to update things like sprites and texture scripts. Normally this can just be GetFrameTime, or can be 0 if paused
\return \b LT_OK, \b LT_NOTINITIALIZED, or \b LT_NOTIN3D.

Renders the list of objects through the given camera.

Used for: Rendering.
*/
    LTRESULT (*RenderObjects)(HLOCALOBJ hCamera,
        HLOCALOBJ *pObjects, int nObjects, float fFrameTime);

/*!
\param  hCamera			The camera to use as a position for the generation of the cubic environment map
\param  nSize			The number of pixels on a side of the final output environment map
\param  pszFilePrefix	The filename that will be placed at the beginning of each filename, it will append the postfix and the .bmp extension
\return \b LT_OK, \b LT_NOTINITIALIZED, or \b LT_NOTIN3D.

Given the specified position of the camera it will generate 6 images in each direction to be used for a cubic
environment map

Used for: Rendering.
*/
    LTRESULT (*MakeCubicEnvMap)(HLOCALOBJ hCamera, uint32 nSize, const char* pszFilePrefix);

/*!
\return \b LT_NOTINITIALIZED when the render struct is not initialized,
or \b LT_NOTIN3D when this function is called outside a Start3D()/End3D()
code block; otherwise, returns \b LT_OK.

Starts a StartOptimized2D()/EndOptimized2D() block.

\note You must be in a StartOptimized2D()/EndOptimized2D() code block
to draw any optimized 2D surfaces.

\see EndOptimized2D()

Used for: 2D Rendering.
*/
    LTRESULT (*StartOptimized2D)();

/*!
\return \b LT_NOTINITIALIZED when the render struct is not initialized,
or \b LT_NOTIN3D when this function is called outside a Start3D()/End3D()
code block; otherwise, returns \b LT_OK.

Ends a StartOptimized2D()/EndOptimized2D() block.

\note You must be in a StartOptimized2D()/EndOptimized2D() code block
to draw any optimized 2D surfaces.

\see StartOptimized2D()

Used for: 2D Rendering.
*/
    LTRESULT (*EndOptimized2D)();

/*!
\param  blend       The blend mode.

\return \b LT_NOTINITIALIZED when the render struct is not initialized,
\b LT_NOTIN3D when this function is called outside a Start3D()/End3D() code
block, or \b LT_ERROR for general failure; otherwise, returns LT_OK.


Change the source/destination blend mode for rendering Optimized2D surfaces.
(Defaults to LTSURFACEBLEND_ALPHA.)

\note
Drawing a surface with a blend mode other than \b _ALPHA will
automatically optimize the surface.

\see GetOptimized2DBlend()

Used for: 2D Rendering.
*/
    LTRESULT (*SetOptimized2DBlend)(LTSurfaceBlend blend);

/*!
\param  blend       (return) The blend mode.

\return \b LT_NOTINITIALIZED when the render struct is not initialized,
or \b LT_ERROR for general failure; otherwise, returns \b LT_OK.

Gets the current blend mode.

\see SetOptimized2DBlend()

Used for: 2D Rendering.
*/
    LTRESULT (*GetOptimized2DBlend)(LTSurfaceBlend &blend);

/*!
\param  flags   Flags to use when performing the flip (a combination of \b END3D_
                flags in ltcodes.h).
\return \b LT_OK, \b LT_NOTINITIALIZED, or \b LT_NOTIN3D.

End a Start3D/End3D block.

\note You must be in a Start3D()/End3D() block in order to render
through any cameras.

\see Start3D()

Used for: Rendering.
*/
    LTRESULT (*End3D)(uint32 flags );

/*!
\param  hColor      (return) The current color.
\return \b LT_NOTINITIALIZED when the render struct is not initialized, or \b LT_ERROR for general failure; otherwise, returns \b LT_OK.

Get the color currently used on Optimized2D surfaces.

\see SetOptimized2DColor()

Used for: 2D Rendering.
*/
    LTRESULT (*GetOptimized2DColor)(HLTCOLOR &hColor);

/*!
\param  hColor      The color to apply.

\return \b LT_NOTINITIALIZED when the render struct is not initialized,
\b LT_NOTIN3D if this function is called outside a Start3D()/End3D() code
block, or \b LT_ERROR for general failure; otherwise, returns \b LT_OK.

Change the color used on Optimized2D surfaces (i.e. apply a color
filter to Optimized2D).

\see GetOptimized2DColor()

Used for: 2D Rendering.
*/
    LTRESULT (*SetOptimized2DColor)(HLTCOLOR hColor);

/*!

\return Pointer to RMode Linked list structure containing the valid render modes.

Get a \b NULL - terminated list of supported render modes.

\see GetRenderMode()
\see SetRenderMode()

Used for: Rendering.
*/
    RMode *(*GetRenderModes)();

/*!
\param pModes An array of \b pMode variables to relinquish.

Relinquish list of supported render modes.

Used for: Rendering.
*/
    void (*RelinquishRenderModes)(RMode *pModes);

/*!
\param  pMode   (return) The current render mode.
\return LT_OK if Successful, LT_ERROR on Failure.

Get the current render mode.

\see SetRenderMode()

Used for: Rendering.
*/
    LTRESULT (*GetRenderMode)(RMode *pMode);

/*!
\param pMode The requested render mode.

\return \b LT_KEPTSAMEMODE when the function cannot set the mode
requested but was able to restore the previous mode, or
\b LT_UNABLETORESTOREVIDEO if it cannot set the mode and cannot restore
the video mode (in which case it will give a message and shutdown
afterwards); otherwise, returns \b LT_OK.

Tells the renderer to switch to the requested render mode.

\note It may not have set the \em exact mode you requested.  You can
check with GetRenderMode() to see what the new mode is.

\see GetRenderMode()

Used for: Rendering.
*/
    LTRESULT (*SetRenderMode)(RMode *pMode);

/*!
\param flags The flags to while performing the shutdown (a combination
of the \b RSHUTDOWN_ flags in ltcodes.h).

\return LT_OK if Successful, LT_ERROR on Failure.

Shut down the renderer.  The renderer won't come back until you call
SetRenderMode().

\see SetRenderMode()

Used for: Rendering.
*/
    LTRESULT (*ShutdownRender)(uint32 flags);

/*!
Renderer Stats
*/

/*!
\param refStats         Struct that gets filled with the renderer stats
\return LT_ERROR        if called before the renderer is not initialized

Obtains a current list of stats on the renderer such as poly counts

Used for: Renderer Stats
*/

    virtual LTRESULT GetRendererStats(LTRendererStats &refStats)=0;

/*!
Render Groups
*/

/*!
\param  hObject         Object that you want to change the group of
\param  nGroup          The group you want the object to belong to
\return LT_INVALIDPARAMS if the object is invalid or the group out of range

Set the render group that the object belongs to. These groups can then be turned on or off, making
all objects that belong to them turn on and off.

Used for: Object Rendering Groups
*/

	virtual LTRESULT SetObjectRenderGroup(HOBJECT hObj, uint32 nGroup)=0;

/*!
\param  nGroup           The group that you want to turn on or off
\param  bVisible		 Whether or not it is visible
\return LT_INVALIDPARAMS if the group is out of range

Causes all objects of the specified group to either become visible or invisible (on top of their existing
visible status)

Used for: Object Rendering Groups
*/

	virtual LTRESULT SetObjectRenderGroupEnabled(uint32 nGroup, bool bEnabled)=0;

/*!
\param  bVisible		 Whether or not it is visible
\return LT_OK

Sets all object rendering groups to the specified visible status (with the exception of the default group
which is always visible)

Used for: Object Rendering Groups
*/

	virtual LTRESULT SetAllObjectRenderGroupEnabled()=0;

/*!
\param  pszSource	 The render style to map
\param	pszMapTo	 The render style to map the above render style to
\return LT_OK if both render styles are found, or LT_FAIL otherwise

When doing the glow rendering this will cause the specified render style to map to the other when doing
the glow effect

Used for: Glow rendering
*/

	virtual LTRESULT AddGlowRenderStyleMapping(const char* pszSource, const char* pszMapTo)=0;

/*!
\param  pszFilename	 The render style to default to if no mapping is found
\return LT_OK if the render style is found, or LT_FAIL otherwise

When doing the glow rendering this will set all unmapped render styles to this render style

Used for: Glow rendering
*/

	virtual LTRESULT SetGlowDefaultRenderStyle(const char* pszFilename)=0;

/*!
\param  pszFilename	 The render style to use if no glow is intended on the object is found
\return LT_OK if the render style is found, or LT_FAIL otherwise

When doing the glow rendering this will set all non glowing objects to this render style

Used for: Glow rendering
*/

	virtual LTRESULT SetNoGlowRenderStyle(const char* pszFilename)=0;

/*!
\param pFileName  Vertex shader file name.
\param VertexShaderID  Id to assign to the loaded vertex shader.
\param pVertexElements  An array of D3DVERTEXELEMENT9 vertex elements used to create the vertex shader declaration.
\param VertexElementsSize  Size in bytes of the vertex elements array.
\param bCompileShader  If bCompileShader is true, then the file is assumed to be a shader text file,
and the shader will be compiled by the renderer. If bCompileShader is false, then the shader file is
assumed to be a compiled binary.
\return \b true is the shader was loaded and false otherwise.

Load a vertex shader from file. You can either pass in a shader text file or a compiled binary
shader file by using the bCompileShader parameter.

Used for: Models.
*/
	virtual bool				AddVertexShader(const char *pFileName, int VertexShaderID,
												const uint32 *pVertexElements, uint32 VertexElementsSize,
												bool bCompileShader = false) = 0;

/*!
\param VertexShaderID  Id to assign to the loaded vertex shader.
\return \b true is the shader was removed and false otherwise.

Remove the vertex shader with the given id.

Used for: Models.
*/
	virtual void				RemoveVertexShader(int VertexShaderID) = 0;

/*!
Remove all vertex shaders.

Used for: Models.
*/
	virtual void				RemoveAllVertexShaders() = 0;

/*!
\param VertexShaderID  Id to assign to the loaded vertex shader.
\return \b pointer to a vertex shader or NULL.

Get access to the loaded vertex shader with the given id.

Used for: Models.
*/
	virtual LTVertexShader*		GetVertexShader(int VertexShaderID) = 0;

/*!
\param pFileName  Pixel shader file name.
\param PixelShaderID  Id to assign to the loaded pixel shader. The Id values -52, -51 and -50 are reserved.
\param bCompileShader  If bCompileShader is true, then the file is assumed to be a shader text file,
and the shader will be compiled by the renderer. If bCompileShader is false, then the shader file is
assumed to be a compiled binary.
\return \b true is the shader was loaded and false otherwise.

Load a pixel shader from file. You can either pass in a shader text file or a compiled binary
shader file by using the bCompileShader parameter.

Used for: Models.
*/
	virtual bool				AddPixelShader(const char *pFileName, int PixelShaderID,
											   bool bCompileShader = false) = 0;

/*!
\param PixelShaderID  Id of the loaded pixel shader.
\return \b true is the shader was removed and false otherwise.

Remove the pixel shader with the given id.

Used for: Models.
*/
	virtual void				RemovePixelShader(int PixelShaderID) = 0;

/*!
Remove all pixel shaders.

Used for: Models.
*/
	virtual void				RemoveAllPixelShaders() = 0;

/*!
\param PixelShaderID  Id of the loaded pixel shader.
\return \b pointer to a pixel shader or NULL.

Get access to the loaded pixel shader with the given id.

Used for: Models.
*/
	virtual LTPixelShader*		GetPixelShader(int PixelShaderID) = 0;

/*!
Fonts.
*/

/*!
\param  pFontName       Requested font name.
\param  width           Requested font width.
\param  height          Requested font height.
\param  bItalic         Request italicized font?
\param  bUnderline      Request underlined font?
\param  bBold           Request bolded font?
\return Handle to the created font (\b HLTFONT).

Create the closest font possible to the one asked for.

Used for: 2D Rendering.
*/
    HLTFONT (*CreateFont)(const char *pFontName, int width, int height,
        bool bItalic, bool bUnderline, bool bBold);

/*!
\param  hFont   Handle of font to delete.

Delete a previously created font. This must be called to avoid
memory leaks.

Used for: 2D Rendering.
*/
    void (*DeleteFont)(HLTFONT hFont);

/*!
\param hFont Handle of font.

\param pixels Number of pixels to insert.

\return \b LT_ERROR when the font is invalid; otherwise, returns \b
LT_OK.

Set the intercharacter spacing. Intercharacter spacing is added to
each character, including break characters, when the system writes a
line of text.

\see GetFontExtraSpace()

Used for: 2D Rendering.
*/
    LTRESULT (*SetFontExtraSpace)(HLTFONT hFont, int pixels);

/*!
\param  hFont   Handle of font.
\param  pixels  (return) Number of pixels currently inserted.
\return \b LT_ERROR when the font is invalid; otherwise, returns \b
LT_OK.

Get the intercharacter spacing. Intercharacter spacing is added to
each character, including break characters, when the system writes a
line of text.

\see SetFontExtraSpace()

Used for: 2D Rendering.
*/
    LTRESULT (*GetFontExtraSpace)(HLTFONT hFont, int &pixels);


/*!
Access to client console variables.
*/

/*!
\param  pName       Name of console program.
\param  fn          Associated function.

\return \b LT_OK or \b LT_ALREADYEXISTS.

Register a console program.  pName is just stored so it should either
be static or allocated.  When the clientshell DLL is unloaded, it gets
rid of any registered programs.

Used for: Misc.
*/
    LTRESULT (*RegisterConsoleProgram)(const char *pName, ConsoleProgramFn fn);

/*!
\param  pName   Name of console program.

\return \b LT_OK or \b LT_NOTFOUND.

Unregister a console program.

Used for: Misc.
*/
    LTRESULT (*UnregisterConsoleProgram)(const char *pName);

/*!
\param  pName   Name of console variable.
\return \b NULL if the parameter doesn't exist.

Get console variable.

Used for: Misc.
*/
    HCONSOLEVAR (*GetConsoleVar)(const char *pName);

/*!
Access to server console mirror variables.
*/

/*!
\param  pName       Name of console variable.
\param  val         (return) Console variable value.
\return \b LT_NOTFOUND if the variable isn't found.

Get server console variable float value.

Used for: Misc.
*/
    virtual LTRESULT GetSConValueFloat(const char *pName, float &val)=0;

/*!
\param  pName       Name of console variable.
\param  valBuf      (return) Buffer to hold console variable string.
\param  bufLen      Length of buffer.
\return \b LT_NOTFOUND if the variable isn't found.

Get server console variable string value.

Used for: Misc.
*/
    virtual LTRESULT GetSConValueString(const char *pName,
        char *valBuf, uint32 bufLen)=0;


/*!
Colors.
*/

/*!
\param  r               Red component (0.0 to 1.0).
\param  g               Green component (0.0 to 1.0).
\param  b               Blue component (0.0 to 1.0).
\param  bTransparent    Transparency on/off. Only used when the function specifies so.
\return Handle to the created color.

Creates a color.  You should free up this color with DeleteColor()
when you're done.

\note Anywhere you can pass a color, if you pass in NULL, it will use black.

Used for: 2D Rendering.
*/
    HLTCOLOR (*CreateColor)(float r, float g, float b,
        bool bTransparent);

/*!
\param  hColor      Handle to the color to delete.

Delete a previously created \b HLTCOLOR. This function must be called to
avoid memory leaks.

\note Alternatively, you can use the SetupColor1 and SetupColor2 and
not worry about deletion.

Used for: 2D Rendering.
*/
    void (*DeleteColor)(HLTCOLOR hColor);

/*!
\param  r               Red component (0.0 to 1.0).
\param  g               Green component (0.0 to 1.0).
\param  b               Blue component (0.0 to 1.0).
\param  bTransparent    Transparency on/off. Only used when the function specifies so.
\return Handle to the created color.

This function is just for convenience.  You don't have to create or
delete these colors, and they're always around.

Used for: 2D Rendering.
*/
    HLTCOLOR (*SetupColor1)(float r, float g, float b,
        bool bTransparent);

/*!
\param  r               Red component (0.0 to 1.0).
\param  g               Green component (0.0 to 1.0).
\param  b               Blue component (0.0 to 1.0).
\param  bTransparent    Transparency on/off. Only used when the function specifies so.
\return Handle to the created color.

This function is just for convenience.  You don't have to create or
delete these colors, and they're always around.

Used for: 2D Rendering.
*/
    HLTCOLOR (*SetupColor2)(float r, float g, float b,
        bool bTransparent);

/*!
Surface management.

\note All the rectangles you specify in here do not include the right
and bottom edges. For example, if you draw a rectangle (0, 1, 0, 1), it will
draw 1 pixel instead of 4 pixels.

\par Try to use the screen surface as little as possible.  Using the
screen surface stalls \em all asynchronous rendering performance and
can cut frame rate in half on some cards.  That is not to say you
shouldn't use them in the interface; just don't use them for things
that are always there like frag counts. The only functions that don't
stall asynchronous performance (feel free to use these regularly) are:

- DrawSurfaceToSurface
- DrawSurfaceToSurfaceTransparent
- ScaleSurfaceToSurface
- ScaleSurfaceToSurfaceTransparent
*/

/*!
\param hSurface The surface to process.

\param hColor The border color.

\param pRect (return) The resulting rectangle, expressed in
coordinates of the surface.

\return \b LT_INVALIDPARAMS when the input parameters are \b NULL,
\b LT_NOTINITIALIZED when the render struct is not initialized, or \b LT_ERROR for
general failure; otherwise, returns \b LT_OK.

Goes around the edges of the surface and returns the smallest inside
rectangle allowing for a border of \b hColor.  For example, if \b
hSurface had a black border of 2 pixels on the left and right and a
black border of 3 pixels on the top and bottom, \b pRect would be set
to (2,3,2,3).

Used for: 2D Rendering.
*/
    LTRESULT (*GetBorderSize)(HSURFACE hSurface, HLTCOLOR hColor,
        LTRect *pRect);

/*!
\param hSurface The surface to process.

\param hTransparentColor The color to optimize with.

\return \b LT_INVALIDPARAMS when the input surface is \b NULL, or \b
LT_ERROR when the render struct cannot optimize the surface;
otherwise, returns \b LT_OK.

Optimize any surfaces you use while rendering 3D stuff in the
background.  Then, when the 3D rendering is done, draw them in a
StartOptimized2D/EndOptimized2D block.  Call this function each time
you change the contents.

\see StartOptimized2D()
\see EndOptimized2D()

Used for: 2D Rendering.
*/
    LTRESULT (*OptimizeSurface)(HSURFACE hSurface,
        HLTCOLOR hTransparentColor);

/*!
\param hSurface The surface to process.

\return \b LT_INVALIDPARAMS when the surface is \b NULL; otherwise,
returns \b LT_OK.

Unoptimize a surface.

Used for: 2D Rendering.
*/
    LTRESULT (*UnoptimizeSurface)(HSURFACE hSurface);

/*!
\return Handle to the screen surface.

Get the screen surface handle.

Used for: 2D Rendering.
*/
    HSURFACE (*GetScreenSurface)();

/*!
\param  pBitmapName     Filename of bitmap.
\return \b LT_INVALIDPARAMS when the parameters are \b NULL, or \b
LT_ERROR when the file cannot be found otherwise, returns \b LT_OK.

Loads an 8-bit PCX surface and returns an 8 bit heightmap

Used for: 2D Rendering.
*/
    LTRESULT (*CreateHeightmapFromBitmap)(const char *pBitmapName, uint32* pWidth, uint32* pHeight, uint8** ppData);

/*!
\param  pBitmapName     Filename of bitmap.
\return Handle to the newly-created surface.

Create a surface, sized to the dimensions of the bitmap.  The bitmap
is an 8-bit PCX file. (You can use any palette you want.)

Used for: 2D Rendering.
*/
    HSURFACE (*CreateSurfaceFromBitmap)(char *pBitmapName);

/*!
\param  pData     The data returned from CreateHeightmapFromBitmap.
\return \b LT_INVALIDPARAMS when the parameters is \b NULL, or \b LT_OK

Frees the memory returned from CreateHeightmapFromBitmap

Used for: 2D Rendering.
*/
    LTRESULT (*FreeHeightmap)(uint8* pData);


/*!
\param  hFont           Font to use.
\param  hString         Source string.
\param  hForeColor      Color of text in surface.
\param  hBackColor      Color of background in surface.
\param  extraPixelsX    Left/right border thickness.
\param  extraPixelsY    Top/bottom border thickness.
\return Handle to the newly-created surface.

Create a surface just large enough for the string. You can make the
surface a little larger with extraPixelsX and extraPixelsY.

Used for: 2D Rendering.
*/
    HSURFACE (*CreateSurfaceFromString)(HLTFONT hFont, HSTRING hString,
        HLTCOLOR hForeColor, HLTCOLOR hBackColor,
        int extraPixelsX, int extraPixelsY);

/*!
\param  width   Width of new surface.
\param  height  Height of new surface.
\return Handle to the newly-created surface.

Create a blank surface.

Used for: 2D Rendering.
*/
    HSURFACE (*CreateSurface)(uint32 width, uint32 height);

/*!
\param hSurface Handle of surface to delete.

\return \b LT_ERROR when either the surface or the global render
struct is \b NULL; otherwise, returns \b LT_OK.

Delete a surface.

Used for: 2D Rendering.
*/
    LTRESULT (*DeleteSurface)(HSURFACE hSurface);

/*!
\param  hSurface    Handle of surface to query.
\return Void pointer to user data.

Get the user-defined surface data associated with each surface. This
data can contain any information and can be formatted in any way the
user sees fit.

\see SetSurfaceUserData()

Used for: 2D Rendering.
*/
    void *(*GetSurfaceUserData)(HSURFACE hSurface);

/*!
\param  hSurface    Handle of surface.
\param  pUserData   User data to attach.

Attach whatever user data you want to a surface. This data can contain
any information and can be formatted in any way the user sees fit.

\see GetSurfaceUserData()

Used for: 2D Rendering.
*/
    void (*SetSurfaceUserData)(HSURFACE hSurface, void *pUserData);

/*!
\param hSurface Handle of surface.

\param x Pixel x coordinate.

\param y Pixel y coordinate.

\param color (return) Color of pixel at (x,y).

\return \b LT_INVALIDPARAMS when the input surface is \b NULL or x and y
coordinates are outside the surface, or \b LT_ERROR when the surface data
cannot be locked; otherwise, returns \b LT_OK.

Get pixel color (\em slow).

\see SetPixel()

Used for: 2D Rendering.
*/
    LTRESULT (*GetPixel)(HSURFACE hSurface, uint32 x, uint32 y,
        HLTCOLOR *color);

/*!
\param hSurface Handle of surface.

\param x Pixel x coordinate.

\param y Pixel y coordinate.

\param color Color to set.

\return \b LT_INVALIDPARAMS when the input surface is \b NULL or x and y
coordinates are outside the surface, or \b LT_ERROR when the surface data
cannot be locked; otherwise, returns \b LT_OK.

Set pixel color (\em slow).

\see GetPixel()

Used for: 2D Rendering.
*/
    LTRESULT (*SetPixel)(HSURFACE hSurface, uint32 x, uint32 y,
        HLTCOLOR color);

/*!
\param  hFont       Font to use for calculation.
\param  hString     The string.
\param  sizeX       (return) Calculated width.
\param  sizeY       (return) Calculated height.

Get the dimensions that this string would take up if it were used to
make a surface.

Used for: 2D Rendering.
*/
    void (*GetStringDimensions)(HLTFONT hFont, HSTRING hString,
        int *sizeX, int *sizeY);

/*!
\param  hDest       Destination surface.
\param  hFont       Font to use.
\param  hString     String to draw.
\param  pRect       Location to draw the string to (in \b hDest coordinates).
\param  hForeColor  Text color.
\param  hBackColor  Background color.

Opaquely draw the string into the rectangle.

Used for: 2D Rendering.
*/
    void (*DrawStringToSurface)(HSURFACE hDest, HLTFONT hFont,
        HSTRING hString, LTRect *pRect, HLTCOLOR hForeColor,
        HLTCOLOR hBackColor);

/*!
\param  hSurf       The surface to query.
\param  pWidth      (return) Width of surface.
\param  pHeight     (return) Height of surface.

Get the dimensions of a surface. You can pass in \b NULL for \b pWidth
and \b pHeight.

Used for: 2D Rendering.
*/
    void (*GetSurfaceDims)(HSURFACE hSurf, uint32 *pWidth,
        uint32 *pHeight);

/*!
\param hDest Destination surface.

\param pSourceBitmapName Filename of bitmap to draw.

\param pSrcRect Area from bitmap to draw (in bitmap coordinates).
A value of \b NULL means to draw the whole thing.

\param destX Destination x position (in \b hDest coordinates).

\param destY Destination y position (in \b hDest coordinates).

\return \b false when the surface is \b NULL or the bitmap file cannot be
loaded; otherwise, returns \b true.

Draw a bitmap (or part of one) to a surface. Opaque.

Used for: 2D Rendering.
*/
    bool (*DrawBitmapToSurface)(HSURFACE hDest,
        const char *pSourceBitmapName, const LTRect *pSrcRect, int destX, int destY);

/*!
\param hDest Destination surface.

\param hSrc Source surface.

\param hMask Mask surface.

\param pSrcRect Area from \b hSrc to draw (in \b hSrc coordinates).
\b NULL = whole thing.

\param destX Destination x position (in \b hDest coordinates).

\param destY Destination y position (in \b hDest coordinates).

\param hColor Transparency color.

\return \b LT_NOTINITIALIZED when there is no render struct, or \b
LT_ERROR for general failure (problems with the surfaces, etc.);
otherwise, returns \b LT_OK.

Draws \b hSrc to \b hDest as it would a normal transparent blit, but tiles
the surface of \b hMask into the nontransparent pixels of \b hSrc (airbrushed
text). You can't have a mask texture larger than 256x256, and the mask
size must be a power of 2.

Used for: 2D Rendering.
*/
    LTRESULT (*DrawSurfaceMasked)(HSURFACE hDest, HSURFACE hSrc,
        HSURFACE hMask, LTRect *pSrcRect, int destX, int destY,
        HLTCOLOR hColor);

/*!
\param hDest Destination surface.

\param hSrc Source surface.

\param pSrcRect Area from \b hSrc to draw (in \b hSrc coordinates).
NULL means the whole thing.

\param destX Destination x position (in \b hDest coordinates).

\param destY Destination y position (in \b hDest coordinates).

\param hTransColor Transparency color.

\param hFillColor Fill color.

\return \b LT_NOTINITIALIZED when there is no render struct, or \b LT_ERROR
for general failure (problems with the surfaces, etc.); otherwise,
returns \b LT_OK.

Draws \b hSrc onto \b hDest using a solid color mask (i.e. fills in
the non-transparent source pixels with the color you specify).

Used for: 2D Rendering.
*/
    LTRESULT (*DrawSurfaceSolidColor)(HSURFACE hDest, HSURFACE hSrc,
        LTRect *pSrcRect, int destX, int destY, HLTCOLOR hTransColor,
        HLTCOLOR hFillColor);

/*!
\param hDest Destination surface.

\param hSrc Source surface.

\param pSrcRect Area from \b hSrc to draw (in \b hSrc coordinates).
\b NULL means the whole thing.

\param destX Destination x position (in \b hDest coordinates).

\param destY Destination y position (in \b hDest coordinates).

\return \b LT_NOTINITIALIZED when there is no render struct, or \b
LT_ERROR for general failure (problems with the surfaces, etc.);
otherwise, returns \b LT_OK.

Draws the source surface (or part of one) onto the destination surface.

Used for: 2D Rendering.
*/
    LTRESULT (*DrawSurfaceToSurface)(HSURFACE hDest, HSURFACE hSrc,
        LTRect *pSrcRect, int destX, int destY);

/*!
\param  hDest               Destination surface.
\param  hSrc                Source surface.

\param pSrcRect Area from \b hSrc to draw (in \b hSrc coordinates).
\b NULL means the whole thing.

\param  destX               Destination x position (in \b hDest coordinates).
\param  destY               Destination y position (in \b hDest coordinates).
\param  hColor              Transparency color.

\return \b LT_NOTINITIALIZED when there is no render struct, or \b LT_ERROR for general
failure (problems with the surfaces, etc.); otherwise, returns \b LT_OK.

Draw the source surface onto the destination surface with transparency.

Used for: 2D Rendering.
*/
    LTRESULT (*DrawSurfaceToSurfaceTransparent)(HSURFACE hDest,
        HSURFACE hSrc, LTRect *pSrcRect, int destX, int destY,
        HLTCOLOR hColor);

/*!
\param  hDest               Destination surface.
\param  hSrc                Source surface.
\param  pDestRect           Area of \b hDest to fill (in \b hDest coordinates).  \b NULL means the whole thing.
\param  pSrcRect            Area from \b hSrc to draw (in \b hSrc coordinates).  \b NULL means the whole thing.

\return \b LT_NOTINITIALIZED when there is no render struct, or \b
LT_ERROR for general failure (problems with the surfaces, etc.);
otherwise, returns \b LT_OK.

Draw the source surface onto the destination surface, but scale
\b pSrcRect to fit \b pDestRect.

Used for: 2D Rendering.
*/
    LTRESULT (*ScaleSurfaceToSurface)(HSURFACE hDest, HSURFACE hSrc,
        LTRect *pDestRect, LTRect *pSrcRect);

/*!
\param  hDest               Destination surface.
\param  hSrc                Source surface.
\param  pDestRect           Area of \b hDest to fill (in \b hDest coordinates).  \b NULL
means the whole thing.
\param  pSrcRect            Area from \b hSrc to draw (in \b hSrc coordinates).
\b NULL means the whole thing.
\param  hColor              Transparency color.

\return \b LT_NOTINITIALIZED when there is no render struct, or \b LT_ERROR for general
failure (problems with the surfaces, etc.); otherwise, returns \b LT_OK.

Draw the source surface onto the destination surface, but scale \b pSrcRect to
fit \b pDestRect with transparency.

Used for: 2D Rendering.
*/
    LTRESULT (*ScaleSurfaceToSurfaceTransparent)(HSURFACE hDest,
        HSURFACE hSrc, LTRect *pDestRect, LTRect *pSrcRect,
        HLTCOLOR hColor);

/*!
\param  hDest               Destination surface.
\param  hSrc                Source surface.
\param  pDestRect           Area of \b hDest to fill (in \b hDest coordinates).  \b
NULL means the whole thing.
\param  pSrcRect            Area from \b hSrc to draw (in \b hSrc coordinates).  \b
NULL means the whole thing.
\param  hTransColor         Transparency color.
\param  hFillColor          Fill color.

\return \b LT_NOTINITIALIZED when there is no render struct, or \b LT_ERROR for general
failure (problems with the surfaces, etc.); otherwise, returns \b LT_OK.

Draw the source surface onto the destination surface, but scale \b pSrcRect to fit \b pDestRect
using a solid color mask.

Used for: 2D Rendering.
*/
    LTRESULT (*ScaleSurfaceToSurfaceSolidColor)(HSURFACE hDest,
        HSURFACE hSrc, LTRect *pDestRect, LTRect *pSrcRect,
        HLTCOLOR hTransColor, HLTCOLOR hFillColor);

/*!
\param hDest Destination surface.

\param hSrc Source surface.

\param pCoords Array of warp points that define source-destination
mapping.

\param nCoords Number of elements in the \b pCoords array.

\return \b LT_NOTINITIALIZED when the render struct is not initialized,
or \b LT_INVALIDPARAMS when surfaces are \b NULL or coordinates are bad;
otherwise, returns \b LT_OK.

(Affine) warp the source polygon into the destination polygon.  The source
coordinates are \em clamped to be inside the polygon of the source surface, and
the warp is clipped against the destination polygon (don't specify
coordinates outside the source polygon, but feel free to specify them
outside the destination polygon).  The polygon you specify should be
convex.  The minimum number of coordinates is 3 and the maximum is 10.

Used for: 2D Rendering.
*/
    LTRESULT (*WarpSurfaceToSurface)(HSURFACE hDest, HSURFACE hSrc,
        LTWarpPt *pCoords, int nCoords);

/*!
\param hDest Destination surface.
\param hSrc Source surface.
\param pCoords Array of warp points that define source-destination mapping.
\param nCoords Number of elements in the \b pCoords array.
\param hColor Transparency color.

\return \b LT_NOTINITIALIZED when the render struct is not initialized,
or \b LT_INVALIDPARAMS when surfaces are \b NULL or coordinates are bad;
otherwise, returns \b LT_OK.

(Affine) warp the source poly into the destination polygon with transparency.
The source coordinates are
\em clamped to be inside the polygon of the source surface,
and the warp is clipped against the destination polygon (don't
specify coordinates outside the source polygon, but feel free to
specify them outside the destination polygon).  The polygon you
specify should be convex.  The minimum number of coordinates is 3 and
the maximum is 10.

Used for: 2D Rendering.
*/
    LTRESULT (*WarpSurfaceToSurfaceTransparent)(HSURFACE hDest,
        HSURFACE hSrc, LTWarpPt *pCoords, int nCoords, HLTCOLOR hColor);

/*!
\param hDest Destination surface.
\param hSrc Source surface.
\param pCoords Array of warp points that define source-destination mapping.
\param nCoords Number of elements in the \b pCoords array.
\param hTransColor Transparency color.
\param hFillColor Fill color.

\return \b LT_NOTINITIALIZED when the render struct is not initialized,
or \b LT_INVALIDPARAMS when surfaces are \b NULL or coordinates are bad;
otherwise, returns \b LT_OK.

(Affine) warp the source poly into the destination polygon using a solid
color mask.  The source coordinates are \em clamped to be inside the
polygon of the source surface, and the warp is clipped against the
destination polygon (don't specify coordinates outside the source
polygon, but feel free to specify them outside the destination
polygon).  The polygon you specify should be convex.  The minimum
number of coordinates is 3 and the maximum is 10.

Used for: 2D Rendering.
*/
    LTRESULT (*WarpSurfaceToSurfaceSolidColor)(HSURFACE hDest,
        HSURFACE hSrc, LTWarpPt *pCoords, int nCoords,
        HLTCOLOR hTransColor, HLTCOLOR hFillColor);

/*!
\param hDest Destination surface.
\param hSrc Source surface.
\param pOrigin Position to treat as origin (in \b hDest
coordinates). A value of \b NULL means to use the centerpoint as origin.
\param destX Destination x position (in \b hDest coordinates).
\param destY Destination y position (in \b hDest coordinates).
\param angle Angle by which to rotate \b hSrc.
\param scaleX Scale to horizontally expand/contract \b hSrc.
\param scaleY Scale to vertically expand/contract \b hSrc.

\return \b LT_NOTINITIALIZED when the render struct is not initialized, or
\b LT_INVALIDPARAMS when surfaces are \b NULL or coordinates are bad;
otherwise, returns \b LT_OK.

Transform the source surface onto the destination surface.

Used for: 2D Rendering.
*/
    LTRESULT (*TransformSurfaceToSurface)(HSURFACE hDest, HSURFACE hSrc,
        LTFloatPt *pOrigin, int destX, int destY, float angle,
        float scaleX, float scaleY);

/*!
\param hDest Destination surface.
\param hSrc Source surface.
\param pOrigin Position to treat as origin (in hDest
coordinates). A value of \b NULL means to use the centerpoint as origin.
\param destX Destination x position (in \b hDest coordinates).
\param destY Destination y position (in \b hDest coordinates).
\param angle Angle to rotate \b hSrc.
\param scaleX Scale to horizontally expand/contract \b hSrc.
\param scaleY Scale to vertically expand/contract \b hSrc.
\param \b hColor Transparency color.

\return \b LT_NOTINITIALIZED when render struct not initialized,
or \b LT_INVALIDPARAMS when surfaces are \b NULL or coordinates are bad;
otherwise, returns \b LT_OK.

Transform the source surface onto the destination surface with transparency

Used for: 2D Rendering.
*/
    LTRESULT (*TransformSurfaceToSurfaceTransparent)(HSURFACE hDest,
        HSURFACE hSrc, LTFloatPt *pOrigin, int destX, int destY,
        float angle, float scaleX, float scaleY, HLTCOLOR hColor);

/*!
\param  hDest               Destination surface.
\param  pRect               Destination area (in \b hDest coordinates).
\param  hColor              Fill color.

\return \b LT_NOTINITIALIZED when the render struct is not initialized,
\b LT_INVALIDPARAMS when a destination surface is invalid, or \b LT_ERROR for
general failure; otherwise, returns \b LT_OK.

Draw a filled rectangle into the surface.

Used for: 2D Rendering.
*/
    LTRESULT (*FillRect)(HSURFACE hDest, LTRect *pRect, HLTCOLOR hColor);

/*!
\param hSurface Surface to query.
\param alpha (return) Alpha value (0.0=transparent, 1.0=opaque).

\return \b LT_INVALIDPARAMS when input surface is \b NULL; otherwise,
returns \b LT_OK.

Get surface alpha.

\note Alpha is only used on optimized surfaces, and only when they are
blitted to the screen.

\see SetSurfaceAlpha()

Used for: 2D Rendering.
*/
    LTRESULT (*GetSurfaceAlpha)(HSURFACE hSurface, float &alpha);

/*!
\param hSurface Surface to modify.
\param alpha Alpha value to set (0.0=transparent, 1.0=opaque).

\return \b LT_INVALIDPARAMS when surface is \b NULL; otherwise, returns \b LT_OK.

Set surface alpha.

\note Alpha is only used on optimized surfaces, and only when they are
blitted to the screen.

\see GetSurfaceAlpha()

Used for: 2D Rendering.
*/
    LTRESULT (*SetSurfaceAlpha)(HSURFACE hSurface, float alpha);

/*!
Helpers.
*/

/*!
\param  pCounter    The counter that the engine will update.

Start a counter.

\note Use this to time sections of code.  Timing is done in
microseconds (millionths of a second).

Start a counter.

\note Use this to time sections of code.  Timing is done in
microseconds (millionths of a second).

\see EndCounter()

Used for: Misc.
*/
    void (*StartCounter)(LTCounter *pCounter);

/*!
\param  pCounter    The counter that the engine will stop.
\return The number of ticks since StartCounter() was called.

Stop a counter.

\note Use this to time sections of code.  Timing is done in
microseconds (millionths of a second).

\see StartCounter()

Used for: Misc.
*/
    uint32 (*EndCounter)(LTCounter *pCounter);

/*!
\param bOn \b true means to turn the input on, and \b false means to turn it
off. The default is \b true.

Turn the input state on or off.  This is for when the client is
interacting with menus and you don't want mouse movement or keystrokes
to be sent to the server.

Used for: Input.
*/
    void (*SetInputState)(bool bOn);

/*!
Clear all the keyboard, command, and axis offset input.

Used for: Input.
*/
    LTRESULT (*ClearInput)();

/*!
\param  nDevice     The device to poll.

\return A list of DeviceBindings for a given device.  You must call
FreeDeviceBindings() to free the list.

\see FreeDeviceBindings()

Used for: Input.
*/
    DeviceBinding *(*GetDeviceBindings)(uint32 nDevice);

/*!
\param  pBindings   The head of the bindings list.

Free the list of DeviceBindings returned by
GetDeviceBindings().

\see GetDeviceBindings()

Used for: Input.
*/
    void (*FreeDeviceBindings)(DeviceBinding* pBindings);

/*!
\param  pDeviceName		The name of the input device.
\param  pTriggerName	The name of the trigger to clear (device object name)

Clear a binding for a device.
\see AddBinding()

Used for: Input.
*/
	bool (*ClearBinding)(const char *pDeviceName, const char *pTriggerName);

/*!
\param  pDeviceName		The name of the input device.
\param  pTriggerName	The name of the trigger to clear (device object name)
\param  pActionName		The name of action to bind to the trigger.
\param  rangeLow		The low range if used (zero if not used)
\param  rangeHigh		The high range if used (zero if not used)

Add a binding for a device (set ranges to 0 to not use ranges).
Note that this does not clear any existing binding.
\see ClearBinding()

Used for: Input.
*/
	bool (*AddBinding)(const char *pDeviceName, const char *pTriggerName, const char *pActionName,
							float rangeLow, float rangeHigh);


/*!
\param  nDevices        Bit-field of devices to track (\b DEVICETYPE_ defines
                        from ltbasedefs.h).
\param  nBufferSize     Number of device events that could occur between calls
                        to TrackDevice(), not to exceed \b MAX_INPUT_BUFFER_SIZE.

\return
\b LT_ERROR     if the input services have not been initialized
\b LT_ERROR     if the buffer size is larger than MAX_INPUT_BUFFER_SIZE
\b LT_ERROR     if none of the requested devices could be acquired
\b LT_TRUE      otherwise

Start tracking input devices.  Between calls to StartDeviceTrack() and
EndDeviceTrack(), no command states will be set through the normal
input.

Used for: Input.
*/
    LTRESULT (*StartDeviceTrack)(uint32 nDevices,
        uint32 nBufferSize);

/*!
\param  pInputArray     (return) Buffer of tracking structures.
\param  pnInOut         (input/return) Supply the size of pInputArray. Returns with number
                        of events filled in.
\return \b LT_INPUTBUFFEROVERFLOW if there were more events that occurred than the
        original buffer size.

Track Input Devices.

\see        StartDeviceTrack()

Used for: Input.
*/
    LTRESULT (*TrackDevice)(DeviceInput* pInputArray,
        uint32* pnInOut);

/*!
\return
\b LT_ERROR     if input services were not properly initialized
\b LT_OK        otherwise

Stop tracking input devices.

Used for: Input.
*/
    LTRESULT (*EndDeviceTrack)();

/*!
\param  nDeviceFlags    Bit-field of device flags (\b DEVICETYPE_ defines in ltbasedefs.h).
\return A DeviceObject list.

Retrieve a list of device objects (like axes, buttons, etc.) for one
or more devices.  You must free the list with FreeDeviceObjects().

\see FreeDeviceObjects()

Used for: Input.
*/
    DeviceObject *(*GetDeviceObjects)(uint32 nDeviceFlags);

/*!
\param  pList       Head of the DeviceObject list.

Free a DeviceObject list allocated with GetDeviceObjects().

\see GetDeviceObjects()

Used for: Input.
*/
    void (*FreeDeviceObjects)(DeviceObject* pList);

/*!
\param  nDeviceType     Device type (\b DEVICETYPE_ defines in ltbasedefs.h).
\param  pStrBuffer      (return) Buffer to hold name.
\param  nBufferSize     Size of pStrBuffer.

\return \b LT_OK or \b LT_NOTFOUND.

Get the name of the first input device of the given type.

Used for: Input.
*/
    LTRESULT (*GetDeviceName)(uint32 nDeviceType,
        char* pStrBuffer, uint32 nBufferSize);

/*!
\param  pszDeviceName	Device name to access.
\param  nDeviceObjectId Object Id returned in DeviceBinding or DeviceObject.
\param  pszDeviceObjectName Name to be filled in by function.
\param  nDeviceObjectNameLen Max length in chars of pszDeviceObjectName.

\return \b LT_OK if successful.

Get the name of the device object by id.

Used for: Input.
*/
    virtual LTRESULT GetDeviceObjectName( char const* pszDeviceName, uint32 nDeviceObjectId,
        char* pszDeviceObjectName, uint32 nDeviceObjectNameLen ) = 0;

/*!
\param  strDeviceName   Device name.
\param  pIsEnabled  (return) \b true if enabled, or \b false if disabled.

\return Always \b LT_OK.

Find out if the specified device is enabled yet.

\see EnableDevice()

Used for: Input.
*/
    LTRESULT (*IsDeviceEnabled)(const char* strDeviceName,
        bool* pIsEnabled);

/*!
\param  strDeviceName   Device name.

\return \b LT_OK or \b LT_ERROR.

Attempt to enable specified device.

\see IsDeviceEnabled()

Used for: Input.
*/
    LTRESULT (*EnableDevice)(const char* strDeviceName);

/*!
\return the number of devices

Determine the number of raw devices (e.g. gamepads, joysticks)
attached to the system.

Used for: Input.
*/
    uint32 (*GetNumRawDevices)();

/*!
\param deviceNum  device index, from zero to numDevices, the value
returned from GetNumRawDevices.
\param pData  pointer to buffer in which to store the data.
\return \b LT_OK or \b LT_ERROR

Retrieve the raw analog data for a device, indexed from zero to
numDevices, which is the value returned from GetNumRawDevices.

You must pass in a pointer to a buffer of sufficient size to hold the
device data, as specified by GetRawDeviceDataSize.

\see GetNumRawDevices
\see GetRawDeviceDataSize
Used for: Input.
*/
    LTRESULT (*GetRawDeviceData)(uint32 deviceNum,
        void* pData);

/*!
\param deviceNum  device index, from zero to numDevices, the value
returned from GetNumRawDevices.
\return the number of bytes of data that will be returned

Retreive the amount of data available from a call to GetRawDeviceData.
\see GetRawDeviceData
Used for: Input.
*/
    uint32 (*GetRawDeviceDataSize)(uint32 deviceNum);

/*!
\param deviceNum  device index, from zero to numDevices, the value
returned from GetNumRawDevices.
\return the number of bytes of actuator data that this device accepts

Retrieve the amount of data that a call to SetRawDeviceActuatorData
will accept.
\see SetRawDeviceActuatorData
Used for: Input.
*/
    uint32 (*GetRawDeviceActuatorDataSize)(uint32 deviceNum);

/*!
\param deviceNum  device index, from zero to numDevices, the value
returned from GetNumRawDevices.
\param pData  pointer to buffer containing actuator data
\return \b LT_OK or \b LT_ERROR

Sends actuator data to the input device.
\see GetRawDeviceActuatorDataSize
Used for: Input.
*/
   LTRESULT (*SetRawDeviceActuatorData)(uint32 deviceNum,
        void* pData);

/*!
\return Number of seconds since the game was started.

Get the time since the game was started.  This accesses the GAME timer
on the server. Since this value comes from the server, it will be
intermittent, so only use it for things that must be synced with the
server. This timer will not update when there isn't a connection to
the server.

Used for: Misc.
*/
    float (*GetGameTime)();

/*!
\return Number of seconds since the previous frame.

Get the time since the frame was started.  This accesses the GAME
timer on the server. Since this value comes from the server, it will
be intermittent, so only use it for things that must be synced with
the server. This timer will not update when there isn't a connection
to the server.

Used for: Misc.
*/
    float (*GetGameFrameTime)();

/*!
\param  pMsg    The printf-styled format string to display.  This parameter can be followed
                by the usual format parameters.

Used to output a TRACE message to the Debug Output window.  Newlines
must be explicitly used.

Used for: Misc.
*/
    void (*DebugOut)(const char *pMsg, ...);

/*!
\param  pDef    (return) Sky definition structure (filled in by function).
\return Always returns \b LT_OK.

Get the sky definition.

Used for: Special FX.
*/
    LTRESULT (*GetSkyDef)(SkyDef *pDef);

/*!
\param  commandNum      Command ID to query.
\return A boolean indicating whether the given command is toggled on or off.

Query a command for on/off status.

Used for: Input.
*/
    bool (*IsCommandOn)(int commandNum);

/*!
\param  pString     The string command to execute.

Same as typing a string into the console.

Used for: Misc.
*/
    void (*RunConsoleString)(const char *pString);

/*!
\return Handle to client object (NULL if currently doesn't exist).

Get your client object.

Used for: Misc.
*/
    HLOCALOBJ (*GetClientObject)();

/*!
\param  pScale      Light scale RGB value (range: clamped to 0.0-1.0).

Get the global light scale that is applied to all rendering.

\see SetGlobalLightScale()
\see OffsetGlobalLightScale()

Used for: Misc.
*/
    void (*GetGlobalLightScale)(LTVector *pScale);

/*!
\param  pScale      Light scale RGB value (range: clamped to 0.0-1.0).

Set the global light scale that is applied to all rendering.

\see GetGlobalLightScale()
\see OffsetGlobalLightScale()

Used for: Misc.
*/
    void (*SetGlobalLightScale)(const LTVector *pScale);

/*!
\param  pOffset     Light scale RGB offset (range: clamped to 0.0-1.0).

Change the global light scale that is applied to all rendering by an offset.

\see GetGlobalLightScale()
\see SetGlobalLightScale()

Used for: Misc.
*/
    void (*OffsetGlobalLightScale)(const LTVector *pOffset);

/*!
\param  dir     Direction vector.
\return Always returns \b LT_OK

Get global light direction.

\see SetGlobalLightDir()

\see SetGlobalLightDir()

Used for: Misc.
*/
    virtual LTRESULT GetGlobalLightDir(LTVector &dir)=0;

/*!
\param  dir     Direction vector.
\return Always returns \b LT_OK

Set global light direction.

\see GetGlobalLightDir()

\see GetGlobalLightDir()

Used for: Misc.
*/
    virtual LTRESULT SetGlobalLightDir(const LTVector &dir)=0;

/*!
\param  color   Light color.
\return Always returns \b LT_OK

Get global light color.

\see SetGlobalLightColor()

Used for: Misc.
*/
    virtual LTRESULT GetGlobalLightColor(LTVector &color)=0;

/*!
\param  color   Light color.

\return Always returns \b LT_OK

Set global light color.

\see GetGlobalLightColor()

Used for: Misc.
*/
    virtual LTRESULT SetGlobalLightColor(const LTVector& color)=0;

/*!
\param  color   Amount of ambient light to contribute to objects in the global light
\return Always returns \b LT_OK

Get the amount of global light that will be converted to ambient lighting

\see SetGlobalLightAmbientColor()

Used for: Misc.
*/
    virtual LTRESULT GetGlobalLightConvertToAmbient(float& fConvertToAmbient)=0;

/*!
\param  color   Amount of ambient light to contribute to objects in the global light

\return Always returns \b LT_OK

Sets the amoun of light that will be converted to ambient lighting, so for example 0.2
will convert 20% of the directional lighting on the model to ambient. This prevent
such harsh shadows on areas that go out of the light

\see GetGlobalLightAmbientColor()

Used for: Misc.
*/
    virtual LTRESULT SetGlobalLightConvertToAmbient(float fConvertToAmbient)=0;

/*!
Messaging.
*/

/*!
New message functions.  These functions don't free the message, so
call LMessage::Release() after sending.

\param  pMsg         \b Message to send to the server
\param  flags       \b MESSAGE_ Flags defined in \b ltcodes.h
\return \b LT_INVALIDPARAMS if the message is invalid, or the server shell is invalid; otherwise, returns \b LT_OK.

Send a message to the server shell.  The server shell receives the message through the OnMessage() function.

\see ILTCommon::CreateMessage()
\see OnMessage()

Used for: Messaging.
*/
    virtual LTRESULT SendToServer(ILTMessage_Read *pMsg, uint32 flags)=0;

/*!
\param	pAddr	Buffer to receive the address of the server
\param	pPort	uint16 to receive the port of the server
\return \bLT_INVALIDPARAMS if one of the parameters is invalid, \b LT_NOTCONNECTED if not connected to a server; otherwise, returns \b LT_OK

Used for: Networking.
*/
	virtual LTRESULT GetServerIPAddress(uint8 pAddr[4], uint16 *pPort) = 0;

/*!
\param  pStruct     Description of object.
\return a valid local client object handle on successful creation and addition of the
LTObject to the local client world, \b LTNULL on failure

Create a client-side object.

Used for: Object.
*/
    HLOCALOBJ (*CreateObject)(ObjectCreateStruct *pStruct);

/*!
\param  hObj    Object to process (parent object of attachments).
\return
\b LT_INVALIDPARAMS     if the object handle is invalid or NULL
\b LT_OK                otherwise

Update the position and rotation of the attachments on the object.
Attachments are always automatically updated when the object is
rendered.

Used for: Object.
*/
    virtual LTRESULT ProcessAttachments(HOBJECT hObj)=0;

    LTRESULT (*GetObjectScale)(HLOCALOBJ hObj, LTVector *pScale);
    LTRESULT (*SetObjectScale)(HLOCALOBJ hObj, const LTVector *pScale);

    void (*SetObjectRotation)(HLOCALOBJ hObj, const LTRotation *pRotation);

/*!
\param  hObject     Object to query.
\param  r           (return) Red value (range: 0.0-1.0).
\param  g           (return) Green value (range: 0.0-1.0).
\param  b           (return) Blue value (range: 0.0-1.0).
\param  a           (return) Alpha value (range: 0.0-1.0).

Get the color and alpha of the object.

\see SetObjectColor()

Used for: Object.
*/
    void (*GetObjectColor)(HLOCALOBJ hObject,
        float *r, float *g, float *b, float *a);

/*!
\param  hObject     Object to modify.
\param  r           Red value (range: 0.0-1.0).
\param  g           Green value (range: 0.0-1.0).
\param  b           Blue value (range: 0.0-1.0).
\param  a           Alpha value (range: 0.0-1.0).

Set the color and alpha of the object.

\note This function overrides a model's render style's material diffuse color.

\see GetObjectColor()

Used for: Object.
*/
    void (*SetObjectColor)(HLOCALOBJ hObject,
        float r, float g, float b, float a);

/*!
\param  hObj        Object to query.
\return Void pointer to user data.

User data for the object. This data is completely defined by the user
and has no necessary bearing on the rest of the system.

\see SetObjectUserData()

Used for: Object.  */
    void *(*GetObjectUserData)(HLOCALOBJ hObj);

/*!
\param  hObj        Object to modify.
\param  pData       User data.

Set user data for the object. This data is completely defined by the user
and has no necessary bearing on the rest of the system.

\see GetObjectUserData()

Used for: Object.
*/
    void (*SetObjectUserData)(HLOCALOBJ hObj, void *pData);

/*!
Camera functions.
*/

/*!
\param  hCamera     Camera to use for calculation.
\param  sx          Screen x coord.
\param  sy          Screen y coord.
\param  pOut        (return) Calculated 3D result (in world coordinates).
\return \b LT_OUTSIDE if the screen coordinates aren't inside the rectangle of the camera.

Get the 3D coordinates of a screen coordinate given a camera.  The 3D
coordinate is one unit out along the forward vector.


Used for: Rendering.
*/
    LTRESULT (*Get3DCameraPt)(HLOCALOBJ hCamera,
        int sx, int sy, LTVector *pOut);

/*!
\param hObj Camera to query.
\param pX (return) X FOV (range: \f$\frac{\pi}{100}\f$ to \f$ (99 \times \frac{\pi}{100}) \f$).
\param pY (return) Y FOV (range: \f$\frac{\pi}{100}\f$ to \f$ (99 \times \frac{\pi}{100}) \f$).

Get the FOV of a camera.  It defaults to (\f$\frac{\pi}{2}\f$, \f$\frac{\pi}{2}\f$).

\see SetCameraFOV()

Used for: Rendering.
*/
    void (*GetCameraFOV)(HLOCALOBJ hObj, float *pX, float *pY);

/*!
\param  hObj        Camera to modify.

\param  pX          (return) X FOV (range: clamped to \f$\frac{\pi}{100}\f$ to \f$ (99 \times \frac{\pi}{100}) \f$).
\param  pY          (return) Y FOV (range: clamped to \f$\frac{\pi}{100}\f$ to \f$ (99 \times \frac{\pi}{100}) \f$).

Set the FOV of a camera.  It defaults to (\f$\frac{\pi}{2}\f$, \f$\frac{\pi}{2}\f$).

\see GetCameraFOV()

Used for: Rendering.
*/
    void (*SetCameraFOV)(HLOCALOBJ hObj, float fovX, float fovY);

/*!
\param hObj Camera to query.
\param bFullscreen (return) \b TRUE means a fullscreen camera.  \b FALSE means a non-fullscreen camera.
\param left (return) Left edge of camera rectangle (in screen coordinates).
\param top (return) Top edge of camera rectangle (in screen coordinates).
\param right (return) Right edge of camera rectangle (in screen coordinates).
\param bottom (return) Bottom edge of camera rectangle (in screen coordinates).

Get the rectangle defined for this camera.

\see SetCameraRect()

\see SetCameraRect()

Used for: Rendering.
*/
    void (*GetCameraRect)(HLOCALOBJ hObj, bool *bFullscreen,
        int *left, int *top, int *right, int *bottom);

/*!
\param  hObj            Camera to modify.
\param  bFullscreen     \b true if fullscreen camera, \b false if non-fullscreen camera.
\param  left            Left edge of camera rectangle (in screen coordinates).
\param  top         Top edge of camera rectangle (in screen coordinates).
\param  right           Right edge of camera rectangle (in screen coordinates).
\param  bottom          Bottom edge of camera rectangle (in screen coordinates).

Set the rectangle of the camera on the screen.  If bFullscreen is \b true,
then it ignores the rectangle and draws the camera full-screen.  If
the rectangle extends over the screen boundaries, then it is clipped.

\see GetCameraRect()

Used for: Rendering.
*/
    void (*SetCameraRect)(HLOCALOBJ hObj, bool bFullscreen,
        int left, int top, int right, int bottom);

/*!
\param  hCamera     Camera to query.
\param  pAdd        (return) Additive value (range: 0.0-1.0).
\return \b false if \b hCamera is not a camera.

Get the camera light additive value.

\see SetCameraLightAdd()

\see SetCameraLightAdd()

Used for: Rendering.
*/
    bool (*GetCameraLightAdd)(HLOCALOBJ hCamera, LTVector *pAdd);

/*!
\param  hCamera     Camera to modify.
\param  pAdd        Additive value to set (range: 0.0-1.0).
\return \b false if \b hCamera is not a camera.

Set the camera light additive value.  Light add is applied \em after
scaling, so if the light is fully bright and scaling is zero, you'll
only see whiteness.  When the light add is nonzero, it draws a poly
over the screen, so don't use it all the time.

\see GetCameraLightAdd()

Used for: Rendering.
*/
    bool (*SetCameraLightAdd)(HLOCALOBJ hCamera, const LTVector *pAdd);

/*!
Particle system manipulation.
*/

/*!
- \b gravityAccel default is -500
- \b flags default is 0
- \b particleRadius default is 300
- color scale defaults to 1.0
- Particle colors are 0-255.

All particle positions are \em relative to the position and rotation of
the particle system.  In many cases, your code can be very simple and
fast if you just move and rotate the particle system and not the
particles.
*/

/*!
\param  hObj            Particle system to modify.
\param  pTextureName    Name of particle texture (.dtx or .spr).
\param  gravityAccel    Gravity acceleration value.
\param  flags           Flags (see PS_ defines, above).
\param  particleRadius  Particle system radius (used for visibility culling).

\return \b LT_INVALIDPARAMS - Either \em hObj is invalid (NULL, or
            not an \b OT_PARTICLESYSTEM) or \em pTextureName is a NULL pointer,
\return \b LT_NOTFOUND - \em pTextureName is invalid (file not found).
\return \b LT_OK - Successful.

Set the parameters of the particle system.

Used for: Special FX.
*/
    LTRESULT (*SetupParticleSystem)(HLOCALOBJ hObj,
        const char *pTextureName, float gravityAccel, uint32 flags,
        float particleRadius);

/*!
\param  hObj        Particle system to which this particle will be added.
\param  pPos        Position of particle (in local obj coordinates).
\param  pVelocity   Velocity vector.
\param  pColor      Color (0-255).
\param  lifeTime    Lifetime.

\return If successful, returns a pointer to the particle added.
\return \b LTNULL - \em hObj is invalid (NULL, or not an \b OT_PARTICLESYSTEM).

Adds a particle to a particle system.

Used for: Special FX.
*/
    virtual LTParticle* AddParticle(HLOCALOBJ hObj, const LTVector *pPos,
        const LTVector *pVelocity, const LTVector *pColor, float lifeTime)=0;

/*!
\param  hObj            Particle system to which these particles will be added.
\param  nParticles      Number of particles to add.
\param  pMinOffset      Minimum of position range (in local obj coordinates).
\param  pMaxOffset      Maximum of position range (in local obj coordinates).
\param  pMinVelocity    Minimum of velocity range.
\param  pMaxVelocity    Maximum of velocity range.
\param  pMinColor       Minimum of color range (0-255).
\param  pMaxColor       Maximum of color range (0-255).
\param  minLifeTime     Minimum of lifetime range.
\param  maxLifeTime     Maximum of lifetime range.

Add several particles to a particle system.  LithTech will randomize
between the ranges given. This is more efficient than calling AddParticle(),
if only because of the function pointer overhead with AddParticle().

Used for: Special FX.
*/
    void (*AddParticles)(HLOCALOBJ hObj, uint32 nParticles,
        const LTVector *pMinOffset, const LTVector *pMaxOffset,
        const LTVector *pMinVelocity, const LTVector *pMaxVelocity,
        const LTVector *pMinColor, const LTVector *pMaxColor,
        float minLifetime, float maxLifetime);

/*!
\param hObj     Particle system to query.
\param pHead    (return) Head of list.
\param pTail    (return) Tail of list.

\return \b true - Successful.
\return \b false - \em hObj is invalid (NULL, or not an \b OT_PARTICLESYSTEM).

Get a pointer to the linked list of particles. This is so that you can
iterate through them and move them all. The engine will usually do
this faster, but it is limited in scope.

Used for: Special FX.
*/
    bool (*GetParticles)(HLOCALOBJ hObj,
        LTParticle **pHead, LTParticle **pTail);

/*!
\param  hSystem         Particle system to modify.
\param  pParticle       Particle to remove.

Remove a particle.

Used for: Special FX.
*/
    void (*RemoveParticle)(HLOCALOBJ hSystem, LTParticle *pParticle);

/*!
\param  hSystem         Particle system to optimize.

\return \b LT_INVALIDPARAMS - \em hSystem is invalid (NULL, or not an
            \b OT_PARTICLESYSTEM).
\return \b LT_OK - No problems.

This is an optimization you can make to help the engine minimize its
boundaries on a particle system.  If you create particles in various
places and they go away, you can use this every so often to
recalculate where the particles are.

Used for: Special FX.
*/
    LTRESULT (*OptimizeParticles)(HLOCALOBJ hSystem);

/*!
\param  hSystem         Particle system to sort.

\param	vDir			Direction vector to sort on (sorts from near to far)

\param  nNumIters		Number of times to pass through the list for sorting

\return \b LT_INVALIDPARAMS - \em hSystem is invalid (NULL, or not an
            \b OT_PARTICLESYSTEM).
\return \b LT_OK - No problems.

This function will take a particle system and a vector indicating how the particles
should be sorted. It will then run through for the specified number of iterations
and sort the particles from nearest to farthest.

Used for: Special FX.
*/
    LTRESULT (*SortParticles)(HLOCALOBJ hSystem, const LTVector& vDir, uint32 nNumIters);

/*!
\param hObj			Particle system to modify.
\param EffectID		Effect Shader ID.

\return \b LT_INVALIDPARAMS - \em hObj is invalid (NULL, or not an \b OT_PARTICLESYSTEM).
\return \b LT_OK - Successful.

Set the Effect Shader ID.

Used for: Special FX.
*/
	LTRESULT (*SetParticleSystemEffectShaderID)(HLOCALOBJ hObj, uint32 EffectShaderID);

/*!
Line system manipulation.  As with particle systems, the lines
are centered around the origin of the object.  Don't place the object
at the origin and place lines far off to the side; it is more efficient
to keep the lines as close to the center as possible.
*/

/*!
\param  hObj    Line system to query.
\param  hPrev   Line to reference.  Use \b NULL to get first line in system.

\return Next line in the list.
\return \b LTNULL - End of the list or \em hObj is invalid (NULL, or not an
            \b OT_LINESYSTEM).

Get the next line in the line system.

\note If you call RemoveLine() on the current \b HLTLINE, \em do \em not
pass that into GetNextLine(). Instead, call GetNextLine() first, while
the \b HLTLINE is still valid.

Used for: Special FX.
*/
    HLTLINE (*GetNextLine)(HLOCALOBJ hObj, HLTLINE hPrev);

/*!
\param  hLine   Line to query.
\param  pLine   (return) Line info.

Get the line info from an \b HLTLINE.

\see SetLineInfo()

Used for: Special FX.
*/
    void (*GetLineInfo)(HLTLINE hLine, LTLine *pLine);

/*!
\param  hLine   Line to modify.
\param  pLine   Line info to set.

Set the line info for an HLTLINE.

\see GetLineInfo()

Used for: Special FX.
*/
    void (*SetLineInfo)(HLTLINE hLine, LTLine *pLine);

/*!
\param  hObj    Line system to modify.
\param  pLine   Description of line to add.

\return If successful, returns a handle to the line added.
\return \b LTNULL - \em hObj is invalid (NULL, or not an \b OT_LINESYSTEM).

Add a line to the end of the list of the line system.

Used for: Special FX.
*/
    HLTLINE (*AddLine)(HLOCALOBJ hObj, LTLine *pLine);

/*!
\param  hObj    Line system to modify.
\param  hLine   Line to remove.

Remove a line from a line system.

Used for: Special FX.
*/
    void (*RemoveLine)(HLOCALOBJ hObj, HLTLINE hLine);

/*!
\param  hObj            VolumeEffect to set up.
\param  info            Initialization structure for this VolumeEffect.

\return \b false - \em hObj is invalid (NULL, or not an \b OT_VOLUMEEFFECT),
                    or part of the initialization structure is invalid.
\return \b true - Successful.

A volume effect is a generic way of defining a custom effect.  An example
of it's use is snow, which is implemeted as a dynamic particle volume
effect.

Used for: Special FX.
*/
	bool (*SetupVolumeEffect)(HLOCALOBJ hObj, VolumeEffectInfo& info);

/*!
\param hObj			VolumeEffect to modify.
\param EffectID		Effect Shader ID.

\return \b LT_INVALIDPARAMS - \em hObj is invalid (NULL, or not an \b OT_VOLUMEEFFECT).
\return \b LT_OK - Successful.

Set the Effect Shader ID.

Used for: Special FX.
*/
	LTRESULT (*SetVolumeEffectEffectShaderID)(HLOCALOBJ hObj, uint32 EffectShaderID);

/*!
\param  pos             Center of AABB.
\param  dims            Dimensions of AABB.
\param  indices         Array that will be filled in with the indices of the blockers in the AABB.

This retrieves the indices of all particle blockers that are within
the AABB specified by pos and dims.

Used for: Special FX.
*/
	bool (*GetParticleBlockersInAABB)( const LTVector& pos, const LTVector& dims, std::vector<uint32>& indices );

/*!
\param  index           Index of blocker to retrieve.
\param  normal          Normal portion of blocker plane equation.
\param  dist            Distance portion of blocker plane equation.
\param  numEdges        Number of edges in blocker poly.
\param  edgeNormals     Normals in XZ plane of each of the blocker edges.

\return \b false - index is out of range.
\return \b true - Successful.

This retrieves the information for a particle blocker polygon.  This form
gets the information needed for fast vertical particle intersection, which
is the polygon edge normals in the XZ plane.

Used for: Special FX.
*/
	bool (*GetParticleBlockerEdgesXZ)(uint32 index, LTPlane& blockerPlane, uint32& numEdges, LTPlane*& edgePlanes);

/*!
Poly grid manipulation.
*/

/*!
\param  hObj            Polygrid to set up.
\param  width           Width of polygrid.
\param  height          Height of polygrid.
\param  nPGFlags		PolyGrid specific flags, can be a bitwise combination
						of the PG_ flags.
\param  pEnabledVerts	A list of booleans that is width*height large, and specifies
						which vertices are valid for rendering. This may be NULL,
						in which case all vertices are valid.

\return \b false - \em hObj is invalid (NULL, or not an \b OT_POLYGRID),
                    or either width or height is invalid (<2 or >65000).
\return \b true - Successful.

A poly grid is basically a heightmapped grid of pixels that are drawn
as polygons.  Each pixel can have a value from -128 to 127.  The
value of the pixel defines its height and is a lookup into the color table
for the vertex color.  You can scale and rotate the poly grid using
SetObjectScale() and SetObjectRotation().

Used for: Special FX.
*/
    bool (*SetupPolyGrid)(HLOCALOBJ hObj, uint32 width, uint32 height, uint32 nPGFlags, bool* pEnabledVerts);

/*!
\param  hObj            Polygrid to modify.
\param  pFilename       Texture filename (.spr).

\return \b LT_ERROR - \em hObj is invalid (NULL, or not an \b OT_POLYGRID).
\return \b LT_MISSINGSPRITEFILE - \em pFilename is invalid (can't find file).
\return \b LT_INVALIDSPRITEFILE - Invalid/corrupt texture (sprite).
\return \b LT_OK - Successful.

Set the texture.  The texture \em must be a sprite file.  It \em
cannot be a .dtx file.

Used for: Special FX.
*/
    LTRESULT (*SetPolyGridTexture)(HLOCALOBJ hObj, const char *pFilename);

/*!
\param hObj         Polygrid to modify.
\param pFilename    Environment map filename (.dtx file).  If \b NULL,
                    disable the environment map.

\return \b LT_INVALIDPARAMS - \em hObj is invalid (NULL, or not an \b OT_POLYGRID).
\return \b LT_NOTFOUND - \em pFilename is invalid (can't find file).
\return \b LT_OK - Successful.

Set the environment map for the PolyGrid.

Used for: Special FX.
*/
    LTRESULT (*SetPolyGridEnvMap)(HLOCALOBJ hObj, const char *pFilename);

/*!
\param  hObj			 Polygrid to query.
\param  xPan			 (return) Texture horizontal pan.
\param  yPan			 (return) Texture vertical pan.
\param  xScale			 (return) Texture horizontal scale.
\param  yScale			 (return) Texture vertical scale.
\param  fBaseReflection	 (return) Base amount of reflection on the polygrid
\param  fVolumeIOR		 (return) The index of refraction for the volume being simulated (used only for fresnel, default is 1.33)

\return \b LT_ERROR - \em hObj is invalid (NULL, or not an \b OT_POLYGRID).
\return \b LT_INVALIDPARAMS - Either \em xPan, \em yPan, \em xScale,
                \em yScale is NULL.
\return \b LT_OK - Successful.

Get the texture pan and scale for a PolyGrid.

\see SetPolyGridTextureInfo()

Used for: Special FX.
*/
    LTRESULT (*GetPolyGridTextureInfo)(HLOCALOBJ hObj, float *xPan,
        float *yPan, float *xScale, float *yScale, float *fBaseReflection, float *fVolumeIOR);

/*!
\param  hObj			 Polygrid to modify.
\param  xPan			 Texture horizontal pan. (default = 0.0)
\param  yPan			 Texture vertical pan. (default = 0.0)
\param  xScale			 Texture horizontal scale. (default = 1.0)
\param  yScale			 Texture vertical scale. (default = 1.0)
\param  fBaseReflection	 Base amount of reflection on the polygrid (default = 0.5)
\param  fVolumeIOR		 The index of refraction for the volume being simulated (default = 1.33)

\return \b LT_ERROR - \em hObj is invalid (NULL, or not an \b OT_POLYGRID).
\return \b LT_OK - Successful.

Set the texture pan and scale for a PolyGrid.  These are \em not auto-pan or
auto-scale values, and must be adjusted whenever you want to animate them.

\see GetPolyGridTextureInfo()

Used for: Special FX.
*/
    LTRESULT (*SetPolyGridTextureInfo)(HLOCALOBJ hObj, float xPan,
        float yPan, float xScale, float yScale, float fBaseReflection, float fVolumeIOR);

/*!
\param  hObj        Polygrid to query.
\param  pBytes      (return) Pointer to heightmap data (size of data block
                    = \em pWidth * \em pHeight bytes.
\param  pWidth      (return) Width (number of horizontal vertices).
\param  pHeight     (return) Height (number of vertical vertices).
\param  pColorTable (return) Pointer to color table (a 256-element array).

\return \b LT_INVALIDPARAMS - \em hObj is invalid (NULL, or not an \b OT_POLYGRID),
                or if \em any of the other parameters are NULL.
\return \b LT_OK - Successful.

Use this function to get a pointer to a polygrid's data, its dimensions, and
its color table.  The buffers at \em pBytes and \em pColorTable can be modified
to directly affect the PolyGrid.  Heightmap values range from -128 to 127, with 0
representing center (i.e. center of \em hObj).  Color table values range from 0 to
255, and directly map to the heightmap values (e.g. CT0 maps to HM-127, CT1 maps to
HM-126, ...).

Note that you can both set a PolyGrid's alpha \em globally with SetObjectColor()
and \em per \em vertex via the alpha component of in the color table.  Additionally,
you can use an alpha mask in the texture.

Used for: Special FX.
*/
    LTRESULT (*GetPolyGridInfo)(HLOCALOBJ hObj, char **pBytes, uint32 *pWidth, uint32 *pHeight, PGColor **pColorTable);


/*!
\param hObj			Polygrid to modify.
\param EffectID		Effect Shader ID.

\return \b LT_INVALIDPARAMS - \em hObj is invalid (NULL, or not an \b OT_POLYGRID).
\return \b LT_OK - Successful.

Set the Effect Shader ID.

Used for: Special FX.
*/
	LTRESULT (*SetPolyGridEffectShaderID)(HLOCALOBJ hObj, uint32 EffectShaderID);
/*!
Network startup/join/host functions.
*/

/*!
\param  pDriver     Name of driver.  Use \b NULL for the default net driver. (currently unsupported)
\param  dwFlags     Flags (currently unsupported).
\return \b LT_OK (always).

Clear any running shells from the client manager; terminate any running network drivers;
start drivers "dplay2" and "internet".

Call this function before calling any other network functions.

The parameters are ignored.

Used for: Networking.
*/
    LTRESULT (*InitNetworking)(const char *pDriver, uint32 dwFlags);

/*!
\param  pListHead   (return) List of \b NetServices.
\return \b LT_OK (always)

Get a list of net services (TCP/IP, modem, etc).

You should call FreeServiceList() to deallocate the returned list when you are done with it.

\see FreeServiceList()

Used for: Networking.
*/
    LTRESULT (*GetServiceList)(NetService* &pListHead);

/*!
\param  pListHead   Head of list of \b NetServices to deallocate.
\return LT_OK (always)

Deallocates the list headed by the parameter \b pListHead.

Call this function when you are finished using the list returned by
GetServiceList().

\see GetServiceList()

Used for: Networking.
*/
    LTRESULT (*FreeServiceList)(NetService *pListHead);

/*!
\param  hNetService     Description of net service.
\return \b LT_NOTINITIALIZED if it can't find the service, or \b LT_ERROR if the service is found but can't be selected; otherwise, returns \b LT_OK.

Find and select the given service as the one to use.

Used for: Networking.
*/
    LTRESULT (*SelectService)(HNETSERVICE hNetService);

/*!
\param pListHead (return) List of \b NetSessions
\param pInfo A string containing IP addresses to check for sessions.
\return \b LT_NOTINITIALIZED if the NetMgr has no main driver (failed to call InitNetworking?) or there is no query socket, or \b LT_ERROR if it can't bind to port when starting the query process; otherwise, returns \b LT_OK.

Fills \b pListHead with enumerated sessions.

\b pInfo is a list of particular IP addresses to check, separated by semicolons.
A record containing a single asterisk (*) means to check the local network
for any broadcast sessions.  A colon followed by a number designates a port to use.
If the first character of an address is a numeral (between '0' and '9') it is treated
as a numeric IP address; otherwise it's treated as a host name.

Internally, the TCP driver will use the StartQuery/UpdateQuery/EndQuery functions
to look for sessions.  It will keep looking for connections for 3.5 seconds
(constant \b QUERY_TIME), sending out queries every 1.0 seconds (constant \b QUERY_SEND_INTERVAL).

You should call FreeSessionList() to deallocate the returned list when you are done with it.

\see FreeSessionList()

Used for: Networking.
*/
    LTRESULT (*GetSessionList)(NetSession* &pListHead, const char *pInfo);

/*!
\param  pListHead       List of \b NetSessions to free.
\return LT_OK (always)

Deallocates the list headed by the parameter \b pListHead.

Call this function when you are finished using the list returned by
GetSessionList() or GetQueryResults().

\see GetSessionList()
\see GetQueryResults()

Used for: Networking.
*/
    LTRESULT (*FreeSessionList)(NetSession *pListHead);

    LTRESULT (*AddInternetDriver)();
    LTRESULT (*RemoveInternetDriver)();

/*!
\defgroup queryfns Query functions.
Alternate method of getting session lists.  These only work for services with
the \b NETSERVICE_TCPIP flag.  These functions return immediately so you can
update a UI in the background without having to "freeze" the UI while it
queries hosts.

(GetSessionList() uses these functions to do its work; it just doesn't return
control between updates.)
*/

/*!
\ingroup queryfns
\param pInfo A string containing IP addresses to check for sessions.
\return \b LT_NOTINITIALIZED if there is no main driver, or \b LT_ERROR if it can't bind to port; otherwise, returns \b LT_OK.

\b pInfo is formatted just as described for GetSessionList().

\see GetSessionList()

Used for: Networking.
*/
    virtual LTRESULT StartQuery(const char *pInfo)=0;


/*!
\ingroup queryfns
\return \b LT_NOTINITIALIZED if there is no main driver or there is no query socket; otherwise, returns \b LT_OK.

Search for sessions that match the query set up through a prior call to StartQuery().

Call this as often as possible.

Results of the query are stored in the main driver connection.  Use GetQueryResults() to retrieve those results.

\see StartQuery()
\see GetQueryResults()

Used for: Networking.
*/
    virtual LTRESULT UpdateQuery()=0;

/*!
\ingroup queryfns

\param pListHead (return) List of \b NetSessions found through the query.
\return \b LT_NOTINITIALIZED if there is no main driver; otherwise, returns \b LT_OK.

Get the list of current results from the query.

Each time you call this function, a new session list is allocated which must be
freed through a call to FreeSessionList() when you are done with it.

\see FreeSessionList()

Used for: Networking.
*/
    virtual LTRESULT GetQueryResults(NetSession* &pListHead)=0;

/*!
\ingroup queryfns
\return \b LT_NOTINITIALIZED if there is no main driver; otherwise, returns \b LT_OK.

End the current query set up through a prior call to StartQuery().

\see StartQuery()

Used for: Networking.
*/
    virtual LTRESULT EndQuery()=0;

/*!
\param  sDriver     String containing name of driver.
\return \b LT_ERROR if it tried to add a driver and that failed, or to indicate that the session was not lobby launched; otherwise, return \b LT_OK (indicated a lobby launched session).

Determine if we were lobby-launched.

Passing \b NULL for \b sDriver will cause the "dplay2" driver to be used.

If a driver of the given name cannot be found, IsLobbyLaunched() will try to add it.

Used for: Networking.
*/
    LTRESULT (*IsLobbyLaunched)(const char* sDriver);

/*!
\param sDriver String containing name of driver.
\param ppLobbyLaunchData (return) Pointer to a pointer to whatever is going to contain the lobby data.

\return \b LT_INVALIDPARAMS if ppLobbyLaunchData is invalid, \b LT_ERROR if it tries to add a driver and fails or the call to the driver to fill in the info fails; otherwise returns \b LT_OK.

Get the lobby launch info if available.

Passing \b NULL for \b sDriver will cause the "dplay2" driver to be used.

\b ppLobbyLaunchData should point to the type of struct which the specified driver knows how to fill in with lobby data.
In the case of DPlay2, \b ppLobbyLaunchData should point to a \b DPLCONNECTION** (DPlay-defined) struct.

Used for: Networking.
*/
    LTRESULT (*GetLobbyLaunchInfo)(const char* sDriver, void** ppLobbyLaunchData);

/*!
\param pVersionInfo (return) \b LTVERSIONINFOEXT struct to be filled in by the function
\return \b LT_INVALIDPARAMS if \b pVersionInfo is \b NULL or if it's not the expected size (i.e., different versions of the struct); otherwise, returns \b LT_OK.

Get the extended version info.

Used for: Misc.
*/
    LTRESULT (*GetVersionInfoExt)(LTVERSIONINFOEXT* pVersionInfo);

/*!
\param pPerformanceInfo (return) \b LTPERFORMANCEINFO struct to be filled in by the function
\return LT_INVALIDPARAMS if \b pPerformanceInfo is \b NULL or if it's not the expected size (i.e., different versions of the struct); otherwise, returns \b LT_OK.

Get run-time performance info for last frame.

The renderer is queried for some of the performance info.

Used for: Misc.
*/
    LTRESULT (*GetPerformanceInfo)(LTPERFORMANCEINFO* pPerformanceInfo);

/*!
\param bEnable Enable/Disable access to the console.
\return LT_OK on success.

Enable/Disable access to the console.

Used for: Misc.
*/
    LTRESULT (*SetConsoleEnable)(bool bEnable);

/*!
\param bLocal Results of the query
\return LT_OK on success

Returns whether or not the client is local to the server

Used for: Misc.
*/
	virtual LTRESULT IsLocalToServer(bool *bLocal) = 0;

/*!
Light manipulation.
*/

/*!
\param  hObj    Light to query.
\param  r       (return) Red value (0-1).
\param  g       (return) Green value (0-1).
\param  b       (return) Blue value (0-1).

Get the color of a light. When you create a light, its color defaults to (0,0,0).

\see SetLightColor()

Used for: Special FX.
*/
    void (*GetLightColor)(HLOCALOBJ hObj, float *r, float *g, float *b);

/*!
\param  hObj    Light to modify.
\param  r       Red value (0-1).
\param  g       Green value (0-1).
\param  b       Blue value (0-1).

Set the color of a light.

\see GetLightColor()

Used for: Special FX.
*/
    void (*SetLightColor)(HLOCALOBJ hObj, float r, float g, float b);

/*!
\param  hObj    Light to query.

\return If \em hObj is an \b OT_LIGHT, returns the radius of the light.
        Otherwise returns 1.0.

Get the radius of a light. When you create a light, its radius defaults to 100.

\see SetLightRadius()

Used for: Special FX.
*/
    float (*GetLightRadius)(HLOCALOBJ hObj);

/*!
\param  hObj    Light to modify.
\param  radius  Radius to set.

Set the radius of a light.

\see GetLightRadius()

Used for: Special FX.
*/
    void (*SetLightRadius)(HLOCALOBJ hObj, float radius);

/*!
\param	nID		ID of the lightgroup.
\param	pColor	Return value destination for the lightgoup's color.

\return \b LT_NOTFOUND - The provided lightgroup was not found.
\return \b LT_NOTINWORLD - No world information is available.
\return \b LT_OK - Successful.

Get the current color of a lightgroup.

Used for: Lightgroup control
*/
	virtual LTRESULT GetLightGroupColor(uint32 nID, LTVector *pColor) const = 0;

/*!
\param	nID		ID of the lightgroup.
\param	vColor	Desired color for the lightgroup.

\return \b LT_NOTFOUND - The provided lightgroup was not found.
\return \b LT_NOTINWORLD - No world information is available.
\return \b LT_ERROR - Unable to complete operation.
\return \b LT_OK - Successful.

Set the current color of a lightgroup.  Setting the color to (0,0,0) will turn
off the lightgroup.

\note When LT_ERROR is returned, the current lightgroup color may not match the rendered light color

Used for: Lightgroup control
*/
	virtual LTRESULT SetLightGroupColor(uint32 nID, const LTVector &vColor) = 0;

/*!
\param	nID		  ID of the occluder.
\param	pEnabled  Return value destination for the occluder's state.

\return \b LT_NOTFOUND - The provided occluder was not found.
\return \b LT_NOTINWORLD - No world information is available.
\return \b LT_OK - Successful.

Get the current state of an occluder.

Used for: Occluder control
*/
	virtual LTRESULT GetOccluderEnabled(uint32 nID, bool *pEnabled) const = 0;

/*!
\param	nID		  ID of the occluder.
\param	bEnabled  Desired state for the occluder.

\return \b LT_NOTFOUND - The provided occluder was not found.
\return \b LT_NOTINWORLD - No world information is available.
\return \b LT_ERROR - Unable to complete operation.
\return \b LT_OK - Successful.

Set the current status of an occluder.

Used for: Occluder control
*/
	virtual LTRESULT SetOccluderEnabled(uint32 nID, bool bEnabled) = 0;

/*!
Texture Effect Variable manipulation
*/

/*!
\param	nID		ID of the texture effect stage.
\param	nVar	The variable that is currently being set.
\param  fVal	The value to set the variable to

\return \b LT_NOTINWORLD - No world information is available.
\return \b LT_ERROR - Unable to complete operation.
\return \b LT_OK - Successful.

Set the specified variable of the specified texture effect. This can be used
so that gamecode can control how some effects are updated or evaluated.

Used for: Texture Effect Variable manipulation
*/
	virtual LTRESULT SetTextureEffectVar(uint32 nID, uint32 nVar, float fVal) = 0;


/*!
Sprite manipulation.
*/

/*!
\param  hObj        Sprite to modify.
\param  hPoly       Polygon to clip to.  Use INVALID_POLY to un-clip the sprite.

\return \b LT_ERROR - \em hObj is invalid (NULL, or not an \b OT_SPRITE).
\return \b LT_OK - Successful.

Clips the sprite onto a polygon.

Used for: Special FX.
*/
    LTRESULT (*ClipSprite)(HLOCALOBJ hObj, HPOLY hPoly);

/*!
\param  hObj        Sprite to query.
\param  pControl    (return) Control interface.

\return \b LT_INVALIDPARAMS - \em hObj is invalid (NULL, or not an \b OT_SPRITE).
\return \b LT_OK - Successful.

Get the sprite control interface for a sprite instance.

Used for: Special FX.
*/
    virtual LTRESULT GetSpriteControl(HLOCALOBJ hObj, ILTSpriteControl* &pControl)=0;

/*!
\param  hObj        Sprite to modify.
\param  EffectID    Effect shader ID.

\return \b LT_INVALIDPARAMS - \em hObj is invalid (NULL, or not an \b OT_SPRITE).
\return \b LT_OK - Successful.

Set the Effect Shader ID.

Used for: Special FX.
*/
    virtual LTRESULT SetSpriteEffectShaderID(HLOCALOBJ hObj, uint32 EffectShaderID)=0;

/*!
\param  hCanvas     Canvas to query.
\param  fn          (return) Associated function.
\param  pUserData   (return) Associated user data.

\return \b LT_INVALIDPARAMS - \em hCanvas is invalid (NULL, or not an \b OT_CANVAS).
\return \b LT_OK - Successful.

Get the function and data associated with canvas drawing.

\see SetCanvasFn()

Used for: Special FX.
*/
    virtual LTRESULT GetCanvasFn(HOBJECT hCanvas,
        CanvasDrawFn &fn, void* &pUserData)=0;

/*!
\param  hCanvas     Canvas to modify.
\param  fn          Associated function.
\param  pUserData   Associated user data.

\return \b LT_INVALIDPARAMS - \em hCanvas is invalid (NULL, or not an \b OT_CANVAS).
\return \b LT_OK - Successful.

Set the function associated with canvas drawing. This function gets
called once during the rendering loop. During the callback you may
draw polygon fans with the DrawPrimitive() call.

\see GetCanvasFn()

Used for: Special FX.
*/
    virtual LTRESULT SetCanvasFn(HOBJECT hCanvas,
        CanvasDrawFn fn, void* pUserData)=0;

/*!
\param  hCanvas     Canvas to query.
\param  radius      (return) Radius of canvas.

\return \b LT_INVALIDPARAMS - \em hCanvas is invalid (NULL, or not an \b OT_CANVAS).
\return \b LT_OK - Successful.

Get the radius of the canvas. This is used for visibility.

\see SetCanvasRadius()

Used for: Special FX.
*/
    virtual LTRESULT GetCanvasRadius(HOBJECT hCanvas, float &radius)=0;

/*!
\param  hCanvas     Canvas to modify.
\param  radius      Radius of the canvas.

\return \b LT_INVALIDPARAMS - \em hCanvas is invalid (NULL, or not an \b OT_CANVAS).
\return \b LT_OK - Successful.

Set the radius of the canvas. This is used for visibility.

\see GetCanvasRadius()

Used for: Special FX.
*/
    virtual LTRESULT SetCanvasRadius(HOBJECT hCanvas, float radius)=0;

/*!
\param  hPoly       Polygon to query.
\param  pFlags      (return) Texture flags.

\return \b LT_ERROR when no world is loaded or \b hPoly is invalid; otherwise, returns \b LT_OK.

Get the texture flags from a poly.

Used for: Misc.
*/
    LTRESULT (*GetPolyTextureFlags)(HPOLY hPoly, uint32 *pFlags);

/*!
\param  hPoly       Polygon to find maximum extents of.
\param	vPoint		Center point of sphere to be found constrained to polygon
\param  fRadius (return) Maximum radius that can be used from the specified point

Given a polygon and a point, it will find the largest possible radius that can be used
and still be contained within the polygon. This is useful for bullet holes, blood splats, etc

Used for: Misc.
*/
    virtual LTRESULT GetMaxRadiusInPoly(
        const HPOLY hPoly, const LTVector& vPos, float& fMaxRadius)=0;

/*!
Render hooks.
*/

/*!
\param  fn      Hook function.
\param  pUser   User data.
\return \b LT_OK

When this is set, the renderer will call your hook function before
drawing each model.  So as not to slow the renderer, either keep the
function very fast or set it to \b NULL.

Used for: Rendering.
*/
    LTRESULT (*SetModelHook)(ModelHookFn fn, void *pUser);

/*!
Engine hooks.
*/

/*!
\param pName Name of engine hook.

\param pData User data.

\return \b LT_OK when it understands the string; otherwise, returns an error.

This is here so we can avoid adding API functions if necessary and for
some system-dependent or misc. stuff.  Pass in a string describing
what you want and it fills in pData.

\par Hooks:
\li \b HWND Returns main window handle.

Used for: Misc.
*/
    LTRESULT (*GetEngineHook)(char *pName, void **pData);

/*!
\param pCaps Name of engine hook.

\return \b LT_OK when it succeeds, returns an error if not.

This is here so we can query the video caps on the API level.

Used for: Misc.
*/
    LTRESULT (*QueryGraphicDevice)(LTGraphicsCaps* pCaps);

protected:
    #ifdef LITHTECH_ESD
    ILTRealAudioMgr     *m_pRealAudioMgr;
    ILTRealConsoleMgr   *m_pRealConsoleMgr;
    ILTRealVideoMgr     *m_pRealVideoMgr;
    ILTRealVideoPlayer  *m_pRealVideoPlayer;
    #endif //! LITHTECH_ESD
};

#endif  //! __ILTCLIENT_H__
