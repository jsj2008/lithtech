

#ifndef __CLIENT_DE_H__
#define __CLIENT_DE_H__


	#include "basedefs_de.h"
	#include "clientshell_de.h"
	#include "common_de.h"
	#include "CSBase.h"

													

	// Particle system flags. Each one slows it down a little more
	// except for PS_NEVERDIE which speeds it up..
	#define PS_BOUNCE		1	// Do they bounce?
	#define PS_SHADOWS		2	// Enable shadows.
	#define PS_NEVERDIE		4	// The particles never die (and it doesn't have to check).
	#define PS_DUMB			8	// The engine leaves the system alone.. you must
								// update and move them.


	class SpriteControl;


	
	// The particle structure for particle systems.
	typedef struct DEParticle_t
	{
		DVector m_Velocity;
		DVector	m_Color;	// Particle colors are 0-255.
		struct DEParticle_t *m_pNext;
	} DEParticle;

	
	// The line structure for line system.
	typedef struct DELinePt_t
	{
		DVector		m_Pos;
		float		r, g, b, a;	// Values 0-1.
	} DELinePt;

	typedef struct DELine_t
	{
		DELinePt	m_Points[2];
	} DELine;

	DEFINE_HANDLE_TYPE(HDELINE)

	// ------------------------------------------------------------------ //
	// Client-side object flags.
	// ------------------------------------------------------------------ //

	// The engine calls ClientShell::OnObjectRemove() when the object goes away.
	#define CF_NOTIFYREMOVE		(1<<0)
	#define CF_NOTIFYMODELKEYS	(1<<1)	// Calls ClientShellDE::OnModelKey when it hits model keys.
	#define CF_DONTSETDIMS		(1<<2)	// The engine automatically sets the object's dims to its
										// animation dims.  This tells it not to.

	


	//  Music info

	#define MUSIC_IMMEDIATE			0
	#define MUSIC_NEXTMEASURE		1
	#define MUSIC_NEXTSONG			2
	#define MUSIC_QUEUE				3


	// Used in AddSurfaceEffect.
	typedef struct SurfaceEffectDesc_t
	{
		char	*m_pName;  // You don't have to allocate this..
		
		// If you return NULL from here, the effect won't be initialized.
		void*	(*InitEffect)(struct SurfaceData_t *pSurfaceData, int argc, char **argv);
		int		(*UpdateEffect)(struct SurfaceData_t *pSurfaceData, void *pData);	// pData is what you returned from InitEffect.
												// Return 1 if you changed the surface.
		void	(*TermEffect)(void *pData);
	} SurfaceEffectDesc;


	// ------------------------------------------------------------------ //
	// Model hook stuff.
	// ------------------------------------------------------------------ //

	typedef void (*ModelHookFn)(struct ModelHookData_t *pData, void *pUser);



	class CPhysicsLT;


	// ------------------------------------------------------------------ //
	// ClientDE interface. This is what the ClientShells and special
	// effect plugins use to do the interface and client objects for the game.
	// ------------------------------------------------------------------ //

	class ClientDE : public CSBase
	{
	friend class CClientMgr;

	protected:

			virtual			~ClientDE() {}

	
	public:
		
			
		
		// Access to the smaller interfaces.

			CommonLT*	Common()	{return m_pCommonLT;}
			CPhysicsLT*	Physics()	{return m_pPhysicsLT;}

		
		// Connection stuff.

			// Tries to start the game in the mode you specify.
			// This will keep the existing game running until it successfully starts
			// the new game.. ie: if there's an error starting the new world, it
			// will still leave the current one running and put up an error message.
			DRESULT		(*StartGame)(StartGameRequest *pRequest);

			// Find out what the current game mode is (how we started the last
			// game with StartGame or thru the console).  Fills in mode with 
			// one of the STARTGAME_ defines above.
			DRESULT		(*GetGameMode)(int *mode);

			// Gets the local client ID.
			// Returns DE_NOTCONNECTED if we're not on a server or haven't
			// gotten a client ID yet.
			DRESULT		(*GetLocalClientID)(DDWORD *pID);

			DBOOL		(*IsConnected)();	// Are we on a server currently?

			// Disconnect from the server we're on (if any).
			// NOTE: this will destroy any client-side objects you've created!
			void		(*Disconnect)();

			// Shuts down the app (not right away, but after the current update).
			void		(*Shutdown)();

			// Shuts down the app (not right away, but after the current update).
			void		(*ShutdownWithMessage)( char *pMsg, ... );

			// OBSOLETE: use the CommonDE one.
			virtual DRESULT GetPointStatus(DVector *pPoint)=0;

			// Get the shade (RGB, 0-255) at the point you specify.
			// Returns DE_NOTINWORLD if the point is outside the world.
			virtual DRESULT	GetPointShade(DVector *pPoint, DVector *pColor)=0;
			
		
		// Renderer management.

			// Flip the screen.  Flags are a combination of FLIPSCREEN_ flags in de_codes.
			// Returns LT_OK or LT_NOTINITIALIZED.
			DRESULT		(*FlipScreen)(DDWORD flags);

			// Clear the backbuffer.  Pass in NULL to clear the whole screen.
			// Flags is a combination of the CLEARSCREEN_ flags in de_codes.h.
			// Returns LT_OK or LT_NOTINITIALIZED.
			DRESULT		(*ClearScreen)(DRect *pClearRect, DDWORD flags);

			// You must be in a Start3D()/End3D() block in order to render
			// through any cameras.
			// Returns LT_OK or LT_NOTINITIALIZED or LT_ALREADYIN3D.
			DRESULT		(*Start3D)();
			
				// Render from the camera's position into its rectangle.
				// Returns LT_OK or LT_NOTINITIALIZED or LT_NOTIN3D.
				DRESULT		(*RenderCamera)(HLOCALOBJ hCamera);

				// Renders the list of objects through the given camera.
				// Returns LT_OK or LT_NOTINITIALIZED or LT_NOTIN3D.
				DRESULT		(*RenderObjects)(HLOCALOBJ hCamera, HLOCALOBJ *pObjects, int nObjects);

				// You must be in a StartOptimized2D()/EndOptimized2D() block to draw any optimized 2D surfaces.
				DRESULT		(*StartOptimized2D)();
				DRESULT		(*EndOptimized2D)();
			
			// Returns LT_OK or LT_NOTINITIALIZED or LT_NOTIN3D.
			DRESULT		(*End3D)();

			// Get a (NULL-terminated) list of supported render modes.
			RMode*		(*GetRenderModes)();
			void		(*RelinquishRenderModes)(RMode *pModes);
			
			// Fills in the current render mode.
			DRESULT		(*GetRenderMode)(RMode *pMode);

			// Goes into the requested render mode.
			// Returns LT_OK if successful.  Note: it may not have set the _exact_ mode you
			// requested.  You can check with GetRenderMode() to see what the new mode is.
			// Returns LT_KEPTSAMEMODE if it couldn't set the mode requested, but
			// was able to restore the previous mode.
			// Returns DE_UNABLETORESTOREVIDEO if it couldn't set the mode and couldn't
			// restore the video mode (in which case it will give a message and shutdown
			// afterwards).
			DRESULT		(*SetRenderMode)(RMode *pMode);
			
			// Shutdown the renderer.  flags is a combination of the RSHUTDOWN_ flags in de_codes.h.
			// The renderer won't come back until you call SetRenderMode().
			DRESULT		(*ShutdownRender)(DDWORD flags);


		// File management.

			// Open a file up.  Pass in the relative filename.
			// Free the file by calling DStream::Release().
			virtual DRESULT	OpenFile(char *pFilename, DStream **pStream)=0;
		
			// Get a list of files/directories in a directory (pass in "" to start with).
			// The list is a noncircular linked list with DNULL terminating it.
			FileEntry*	(*GetFileList)(char *pDirName);
			
			// Use FreeFileList when you're done with each list you get.
			void		(*FreeFileList)(FileEntry *pHead);

			// Get the world info string from the World Info dialog in DEdit.
			// Returns DE_NOTFOUND if it can't open the file and DE_INVALIDWORLDFILE
			// if the file is an invalid version.
			DRESULT		(*GetWorldInfoString)(char *pFilename, char *pInfoString, DDWORD maxLen, DDWORD *pActualLen);

			// Read/write configuration files.  Returns LT_ERROR if the file doesn't exist
			// or if it can't open the file for writing.
			DRESULT		(*ReadConfigFile)(char *pFilename);
			DRESULT		(*WriteConfigFile)(char *pFilename);


		// Interface stuff.

			// offsets is NUM_AXIS_OFFSETS large.
			void		(*GetAxisOffsets)(DFLOAT *offsets);

			void		(*PlayJoystickEffect)(char *pEffectName, float x, float y);
		

		// Music functions.

			DBOOL		(*InitMusic)( char *szMusicDLL );

			// Sets the directory to find music data files.
			// IMA only.
			DBOOL		(*SetMusicDirectory)( char *szMusicDirectory );

			// Downloads DLS.
			// szDLSFile is the Downloadable Synthizer file containing the waves for 
			// instruments.  szStyleFile is a style file, that is a compainion to the 
			// DLS file.  It must contain bands that define the program changes for
			// each instrument in DLS...
			// IMA only.
			DBOOL		(*InitInstruments)( char *szDLSFile, char *szStyleFile );

			// Cache in a song file.
			DBOOL		(*LoadSong)(char *szSong);

			// Remove all the songs from memory...
			void		(*DestroyAllSongs)();

			// Plays a playlist.  All songs will be played until end of list.  
			// Arguments:
			//		szPlayList - Name of playlist.
			//		szTransition - Name of transition song to play before playlist starts.  IMA only.
			//		bLoop - Loop the list.
			//		dwBoundaryFlags - Can be MUSIC_IMMEDIATE, MUSIC_MEASURE, MUSIC_SONG, MUSIC_QUEUE.  Valid for IMA only.
			DBOOL		(*PlayList)( char *szPlayList, char *szTransition, DBOOL bLoop, DDWORD dwBoundaryFlags );

			// Plays a musical break.
			// IMA only.
			// Arguments:
			//		szSong - Name of song.
			//		dwBoundaryFlags - Can be MUSIC_IMMEDIATE, MUSIC_MEASURE, MUSIC_SONG, MUSIC_QUEUE.  Valid for IMA only.
			DBOOL		(*PlayBreak)( char *szSong, DDWORD dwBoundaryFlags );

			// Stops music.  
			// Arguments:
			//		dwBoundaryFlags - Can be MUSIC_IMMEDIATE, MUSIC_MEASURE, MUSIC_SONG, MUSIC_QUEUE.  Valid for IMA only.
			void		(*StopMusic)( DDWORD dwBoundaryFlags );

			// Pause music.  Can be resumed...
			DBOOL		(*PauseMusic)();

			// Resume music...
			DBOOL		(*ResumeMusic)();

			// Create and add to playlists....
			// Arguments:
			//		szPlayList - Name of playlist.
			//		szSong - Name of song to add.
			DBOOL		(*AddSongToPlayList)(char *szPlayList, char *szSong);

			// Removes a whole playlist...
			// Arguments:
			//		szPlayList - Name of playlist.
			void		(*DeletePlayList)(char *szPlayList);

			// Plays a motif.
			// Ima only.
			// Arguments:
			//		szMotif - name of motif.
			//		bLoop - Loop the motif.
			DBOOL		(*PlayMotif)(char *szMotif, DBOOL bLoop);

			// Stops a motif for IMA.
			// IMA only.
			// Arguments:
			//		szMotif - name of motif.
			void		(*StopMotif)(char *szMotif);

			// Volume range is 0-off, 100-full.
			short		(*GetMusicVolume)();
			void		(*SetMusicVolume)( short wVolume );


		// Sound functions.

			// Gets a list of the 3d sound providers available on system.  Choose one pass it in the 
			// InitSoundInfo structure.  Use bVerify to tell the engine not to report providers that 
			// aren't completely supported on the system.  This takes longer and causes speaker popping.  
			// Games should only need  to do this when a different provider is chosen.  EAX support
			// can only be checked when the provider is opened, so without the bVerify, the 
			// SOUND3DPROVIDER_CAPS_REVERB cannot be set.  InitSound will report this info in the
			// m_dwResults flags.
			// Be sure to release both lists with ReleaseSound3DProviderList.
			DRESULT			(*GetSound3DProviderLists)( Sound3DProvider *&pSound3DProviderList, DBOOL bVerify );
			void			(*ReleaseSound3DProviderList)( Sound3DProvider *pSound3DProviderList );

			// Initializes the sound driver.
			DRESULT			(*InitSound)( InitSoundInfo *pSoundInfo );

			// Volume controls between 0 - 100
			unsigned short	( *GetSoundVolume )();
			void			( *SetSoundVolume )( unsigned short nVolume );

			// Controls the reverb properties.
			// Inputs:
			DRESULT			( *SetReverbProperties )( ReverbProperties *pReverbProperties );
			DRESULT			( *GetReverbProperties )( ReverbProperties *pReverbProperties );

			// These PlaySound function allows the client shell to initiate and control sounds
			// without going through the server.  These sounds will only be played 
			// on the client initiating them. These functions will fill m_hSound
			// with the handle to the client sound if you set the PLAYSOUND_GETHANDLE flag.  This will
			// also force the sound not to be automatically deleted when the sound is done playing.  You
			// must call KillSound.  You must get the handle in order to use the other sound functions
			// like SetSoundPosition.

				// Play a sound with full control
				// Arguments:
				//		pPlaySoundInfo - sound control structure
				// Return:
				//		Handle to the client only sound.
				DRESULT	(*PlaySound)( PlaySoundInfo *pPlaySoundInfo );

				// Update position and orientation of a client only sound.
				// Arguments:
				//		hSound - Handle to client only sound.
				//		pPos - New position of sound. Can be NULL.
				// Return:
				//		LT_OK - Successful.
				//		LT_INVALIDPARAM - Invalid parameters.
				//		LT_ERROR - Unable find hSound
				DRESULT	(*SetSoundPosition)( HSOUNDDE hSound, DVector *pPos );

				// Get current position and orientation of a client only sound.
				// Arguments:
				//		hSound - Handle to client only sound.
				//		pPos - Destination of position. Can be NULL.
				//		pOrientation - Destination of orientation. Can be NULL.
				// Return:
				//		LT_OK - Successful.
				//		LT_INVALIDPARAM - Invalid parameters.
				//		LT_ERROR - Unable to find hSound.
				DRESULT	(*GetSoundPosition)( HSOUNDDE hSound, DVector *pPos );

				// Pause/resume sounds.
				void		(*PauseSounds)( );
				void		(*ResumeSounds)( );

				// Get total length in seconds of sound.
				// Arguments:
				//		hSound - Handle to sound.
				//		fDuration - Duration of sound.
				// Returns:
				//		DE_OK if successful.
				//		DE_INVALIDPARAMS if hSound not available.
				DRESULT	(*GetSoundDuration)( HSOUNDDE hSound, DFLOAT *fDuration );

				// Check if sound finished playing or if object it was attached to was removed.
				// Arguments:
				//		pSoundHandle - handle to client only sound
				DBOOL	(*IsDone)( HSOUNDDE pSoundHandle );

				// Kill a sound.
				// Arguments:
				//		pSoundHandle - handle to client only sound
				void	(*KillSound)( HSOUNDDE pSoundHandle );

				// Set the listener status, position and orientation.  If bListenerInClient is TRUE, then
				// pPos and pRot are ignored and can be set to NULL.
				void	(*SetListener)( DBOOL bListenerInClient, DVector *pPos, DRotation *pRot );


		// Video functions.  These functions use Smacker movies.

			// DE_OK, VIDEO_ERROR.
			// flags is a combination of the PLAYBACK_ flags in de_codes.h.
			DRESULT		(*StartVideo)(char *pFilename, DDWORD flags);
			DRESULT		(*StopVideo)();

			// Draws the current video playback to the screen.
			DRESULT		(*UpdateVideo)();
			
			// VIDEO_PLAYING, VIDEO_NOTPLAYING.
			DRESULT		(*IsVideoPlaying)();


		// String functions.  Strings are reference counted objects that cannot
		// be manipulated.  When you create one with FormatString or CreateString,
		// the reference count is 1.  When you copy a string with CopyString, the
		// reference count is incremented.  When you free one with FreeString,
		// it decrements the reference count.. when the reference count goes to
		// zero, it deletes the string.  If you forget to free up any strings, 
		// DirectEngine will spit out a message telling you about it..

			// In Windows, messageCode comes from your resource DLL.  The messages
			// need to be formatted with %1!s! %2!s! (the number is the argument 
			// number and the !s! says its a string).  You can also use !f! and !d!
			// for floating point and whole number.
			HSTRING		(*FormatString)(int messageCode, ...);
			
			// Copy a string.. much more efficient than CreateString().
			HSTRING		(*CopyString)(HSTRING hString);
			HSTRING		(*CreateString)(char *pString);
			void		(*FreeString)(HSTRING hString);

			DBOOL		(*CompareStrings)(HSTRING hString1, HSTRING hString2);
			DBOOL		(*CompareStringsUpper)(HSTRING hString1, HSTRING hString2);

			// Get the string's data.. you really should only use this for strings
			// that you stored off and want to pass to the engine for a filename 
			// or something..  Most strings could be in some format other than ANSI.
			char*		(*GetStringData)(HSTRING hString);


		// Intersections.

			// Intersect a line segment.. (used to be raycast, but line segments are WAY faster).
			// Returns TRUE and fills in pInfo if it hit something.
			DBOOL		(*IntersectSegment)(ClientIntersectQuery *pQuery, ClientIntersectInfo *pInfo);

			// Same as IntersectSegment, except for it casts a ray from pQuery->m_From
			// in the direction of pQuery->m_Dir.
			DBOOL		(*CastRay)(ClientIntersectQuery *pQuery, ClientIntersectInfo *pInfo);

			// Find objects in a sphere.  This is only based on their centerpoints, not
			// on their dimensions or anything.  inObjects is filled in and nOutObjects 
			// is set to the number of objects filled in.  nFound might be larger if
			// it found more objects than nInObjects.
			DRESULT		(*FindObjectsInSphere)(DVector *pCenter, float radius, 
				HLOCALOBJ *inObjects, DDWORD nInObjects, DDWORD *nOutObjects, DDWORD *nFound);


		// Fonts..

			// Creates the closest font it can to the one you asked for.
			HDEFONT		(*CreateFont)(char *pFontName, int width, int height, 
				DBOOL bItalic, DBOOL bUnderline, DBOOL bBold);

			void		(*DeleteFont)(HDEFONT hFont);


		// Colors..

			// r, g, and b go from 0 to 1.
			// bTransparent is only used when the function specifies so.
			// Note: anywhere you can pass a color, if you pass in NULL, it'll use black.
			HDECOLOR	(*CreateColor)(float r, float g, float b, DBOOL bTransparent);
			void		(*DeleteColor)(HDECOLOR hColor);

			// Just for convenience.  You don't have to create/delete these colors,
			// and they're always around!
			HDECOLOR	(*SetupColor1)(float r, float g, float b, DBOOL bTransparent);
			HDECOLOR	(*SetupColor2)(float r, float g, float b, DBOOL bTransparent);

		
		// Surface management.
		// Note:  All the rectangles you specify in here do not include the right and bottom
		//        edges.  ie: If you draw a rectangle (0, 1, 0, 1), it'll draw 1 pixel
		//        instead of 4 pixels. 

		// Note:  Try to use the screen surface as little as possible.  Using the screen
		//        surface stalls ALL asynchronous rendering performance and can cut framerate
		//        in half on some cards.  That's not to say don't use them in the interface,
		//        just don't use them for things that are always there like frag counts, etc..
		//        The only functions that don't stall async performance (feel free to use
		//        these regularly) are:
		//           DrawSurfaceToSurface
		//           DrawSurfaceToSurfaceTransparent
		//           ScaleSurfaceToSurface
		//           ScaleSurfaceToSurfaceTransparent

			// This goes around the edges of the surface and returns the smallest inside
			// rectangle allowing for a border of hColor.  For example, if hSurface 
			// had a black border of 2 pixels on the left and right and a black border 
			// of 3 pixels on the top and bottom, pRect would be set to (2,3,2,3).
			DRESULT		(*GetBorderSize)(HSURFACE hSurface, HDECOLOR hColor, DRect *pRect);

			// Any surfaces you use while rendering 3D stuff at the same time should be optimized
			// with this function and drawn in an StartOptimized2D/EndOptimized2D block.
			// You need to call OptimizeSurface each time you change its contents.
			DRESULT		(*OptimizeSurface)(HSURFACE hSurface, HDECOLOR hTransparentColor);
			DRESULT		(*UnoptimizeSurface)(HSURFACE hSurface);
		
			HSURFACE	(*GetScreenSurface)();

			// Creates a surface sized to the dimensions of the bitmap.
			// The bitmap is an 8-bit (you can use any palette you want..) PCX file.
			// So pBitmapName might look like "interface/bitmaps/menu1.pcx".
			HSURFACE	(*CreateSurfaceFromBitmap)(char *pBitmapName);
			
			// Creates a surface just large enough for the string.
			// You can make the surface a little larger with extraPixelsX and extraPixelsY.
			HSURFACE	(*CreateSurfaceFromString)(HDEFONT hFont, HSTRING hString,
				HDECOLOR hForeColor, HDECOLOR hBackColor, 
				int extraPixelsX, int extraPixelsY);
			
			// Create a plain old surface.
			HSURFACE	(*CreateSurface)(DDWORD width, DDWORD height);
			
			DRESULT		(*DeleteSurface)(HSURFACE hSurface);

			// Attach whatever user data you want to a surface.
			void*		(*GetSurfaceUserData)(HSURFACE hSurface);
			void		(*SetSurfaceUserData)(HSURFACE hSurface, void *pUserData);

			// Access the pixels (SLOW).
			DRESULT		(*GetPixel)(HSURFACE hSurface, DDWORD x, DDWORD y, HDECOLOR *color);
			DRESULT		(*SetPixel)(HSURFACE hSurface, DDWORD x, DDWORD y, HDECOLOR color);
			
			// Gets the dimensions that this string would take up.
			void		(*GetStringDimensions)(HDEFONT hFont, HSTRING hString, int *sizeX, int *sizeY);

			// Draws the string into the rectangle..
			void		(*DrawStringToSurface)(HSURFACE hDest, HDEFONT hFont, HSTRING hString, 
				DRect *pRect, HDECOLOR hForeColor, HDECOLOR hBackColor);

			// You can pass in NULL for pWidth and pHeight if you want.
			void		(*GetSurfaceDims)(HSURFACE hSurf, DDWORD *pWidth, DDWORD *pHeight);
			
			// Draw a bitmap to a surface..
			DBOOL		(*DrawBitmapToSurface)(HSURFACE hDest, char *pSourceBitmapName, 
				DRect *pSrcRect, int destX, int destY);

			// Draws hSrc to hDest like a normal transparent blit, but tiles hMask's surface
			// into the nontransparent pixels of hSrc (can you say airbrushed text?)
			// You can't have a mask texture larger than 256x256, and the mask must be
			// a power of 2.
			DRESULT		(*DrawSurfaceMasked)(HSURFACE hDest, HSURFACE hSrc, HSURFACE hMask,
				DRect *pSrcRect, int destX, int destY, HDECOLOR hColor);

			// Draws hSrc onto hDest, but fills in the nontransparent pixels with the
			// color you specify.
			DRESULT		(*DrawSurfaceSolidColor)(HSURFACE hDest, HSURFACE hSrc,
				DRect *pSrcRect, int destX, int destY, HDECOLOR hTransColor, HDECOLOR hFillColor);
			
			// Draws the source surface onto the dest surface.  You can specify a rectangle 
			// in the source surface and the destination coordinates on the destination
			// surface.. it doesn't scale the bitmap..  If you pass in NULL for the source
			// rect, it'll just use the whole thing.
			DRESULT		(*DrawSurfaceToSurface)(HSURFACE hDest, HSURFACE hSrc, 
				DRect *pSrcRect, int destX, int destY);

			DRESULT		(*DrawSurfaceToSurfaceTransparent)(HSURFACE hDest, HSURFACE hSrc, 
				DRect *pSrcRect, int destX, int destY, HDECOLOR hColor);

			// Scales the source surface rectangle into the dest rectangle..
			DRESULT		(*ScaleSurfaceToSurface)(HSURFACE hDest, HSURFACE hSrc,
				DRect *pDestRect, DRect *pSrcRect);

			DRESULT		(*ScaleSurfaceToSurfaceTransparent)(HSURFACE hDest, HSURFACE hSrc,
				DRect *pDestRect, DRect *pSrcRect, HDECOLOR hColor);

			DRESULT		(*ScaleSurfaceToSurfaceSolidColor)(HSURFACE hDest, HSURFACE hSrc,
				DRect *pDestRect, DRect *pSrcRect, HDECOLOR hTransColor, HDECOLOR hFillColor);

			// (Affine) warps the source poly into the dest poly.  The source
			// coordinates are CLAMPED to be inside the source surface's rectangle, and
			// the warp is clipped against the destination rectangle (ie: don't specify
			// coordinates outside the source rectangle, but feel free to specify them
			// outside the destination rectangle).  The polygon you specify should be
			// convex.  The minimum number of coordinates is 3 and the maximum
			// is 10.
			DRESULT		(*WarpSurfaceToSurface)(HSURFACE hDest, HSURFACE hSrc, 
				DWarpPt *pCoords, int nCoords);

			DRESULT		(*WarpSurfaceToSurfaceTransparent)(HSURFACE hDest, HSURFACE hSrc, 
				DWarpPt *pCoords, int nCoords, HDECOLOR hColor);

			DRESULT		(*WarpSurfaceToSurfaceSolidColor)(HSURFACE hDest, HSURFACE hSrc, 
				DWarpPt *pCoords, int nCoords, HDECOLOR hTransColor, HDECOLOR hFillColor);

			// Transform the source surface onto the dest surface.  The origin is
			// in the destination surface's coordinates.  If you specify NULL, it will
			// use the centerpoint as the origin.
			DRESULT		(*TransformSurfaceToSurface)(HSURFACE hDest, HSURFACE hSrc,
				DFloatPt *pOrigin, int destX, int destY, float angle,
				float scaleX, float scaleY);

			DRESULT		(*TransformSurfaceToSurfaceTransparent)(HSURFACE hDest, HSURFACE hSrc,
				DFloatPt *pOrigin, int destX, int destY, float angle,
				float scaleX, float scaleY, HDECOLOR hColor);

			// Draw a filled rectangle into the surface.
			DRESULT		(*FillRect)(HSURFACE hDest, DRect *pRect, HDECOLOR hColor);



		// Access to client console variables...

			// Register a console program.  pName is just stored so it should either be
			// static or allocated.  When the client shell DLL is unloaded, it gets 
			// rid of any registered programs.
			// Returns DE_OK or DE_ALREADYEXISTS.
			DRESULT			(*RegisterConsoleProgram)(char *pName, ConsoleProgramFn fn);
			
			// Returns DE_OK or DE_NOTFOUND.
			DRESULT			(*UnregisterConsoleProgram)(char *pName);

			// Returns NULL if the parameter doesn't exist.
			HCONSOLEVAR		(*GetConsoleVar)(char *pName);
			
			// Gets the value of a parameter .. returns 0/NULL if you pass in NULL.
			float			(*GetVarValueFloat)(HCONSOLEVAR hVar);
			char*			(*GetVarValueString)(HCONSOLEVAR hVar);

		// Access to server console mirror variables...

			// The 'new' accessors for server console variables.  Returns DE_NOTFOUND
			// if the variable isn't found.
			virtual DRESULT	GetSConValueFloat(char *pName, float &val)=0;
			virtual DRESULT	GetSConValueString(char *pName, char *valBuf, DDWORD bufLen)=0;

			// Gets the value of a parameter .. returns 0/NULL if you pass in NULL.
			// OBSOLETE (will be removed soon).  Use the GetSCon functions.
			virtual float	GetServerConVarValueFloat(char *pName)=0;
			virtual char*	GetServerConVarValueString(char *pName)=0;
					

		// Helpers..

			// Use these to time sections of code.  Timing is done in microseconds
			// (1,000,000 counts per second).
			void		(*StartCounter)(struct DCounter_t *pCounter);
			DDWORD		(*EndCounter)(struct DCounter_t *pCounter);

			// Setup the panning sky stuff.  Pass in DNULL for the filename if you want
			// to disable the panning sky.  Returns DE_NOTFOUND if it can't find
			// the texture.
			// index tells which global panning thing you want to change.  It
			// is one of the GLOBALPAN_ defines in de_codes.h.
			DRESULT		(*SetGlobalPanTexture)(DDWORD index, char *pFilename);
			DRESULT		(*SetGlobalPanInfo)(DDWORD index, float xOffset, float zOffset, float xScale, float zScale);

			// Register a surface effect.  You should register all your effects in OnEngineInitialized
			// (before you start a world).
			DRESULT		(*AddSurfaceEffect)(SurfaceEffectDesc *pDesc);

			// Turn the input state on or off.  This is for times when the client
			// is interacting with menus and you don't want their mouse movement or
			// keystrokes to get sent to the server.  This defaults to ON.
			void		(*SetInputState)(DBOOL bOn);

			// Clears all the keyboard, command, and axis offset input.
			DRESULT		(*ClearInput)();

			// Returns a list of DeviceBindings for a given device.  You must call FreeDeviceBindings()
			// to free the list.
			DeviceBinding*	(*GetDeviceBindings)(DDWORD nDevice);
			void			(*FreeDeviceBindings)(DeviceBinding* pBindings);

			// Track Input Devices.  Between calls to StartDeviceTrack() and EndDeviceTrack() no command
			// states will be set through the normal input.  Pass in the devices to track (DEVICETYPE_
			// defines) and a buffer size for all input devices.  The buffer size is the number of events
			// that could occur between calls to TrackDevice(), not to exceed MAX_INPUT_BUFFER_SIZE.
			// Supply TrackDevice() with an array of DeviceInput structures, and the number of structures
			// in the array.  When TrackDevice() returns, the pnInOut variable will contain the number of
			// events that have occurred (the number of filled-in DeviceInput structures).  If there were
			// more events that occurred than the original buffer size allowed for, TrackDevice will return
			// LT_INPUTBUFFEROVERFLOW.
			DRESULT		(*StartDeviceTrack)(DDWORD nDevices, DDWORD nBufferSize);
			DRESULT		(*TrackDevice)(DeviceInput* pInputArray, DDWORD* pnInOut);
			DRESULT		(*EndDeviceTrack)();

			// Retrieve a list of device objects (like axes, buttons, etc.) for one or more devices.
			// Pass GetDeviceObjects a combination of DEVICETYPE_ flags and it will return a DeviceObject
			// (defined in basedefs_de.h) list.
			// You must free the list with FreeDeviceObjects().
			DeviceObject*	(*GetDeviceObjects)(DDWORD nDeviceFlags);
			void			(*FreeDeviceObjects)(DeviceObject* pList);

			// Get the name of the first input device of the given type.
			// Returns either LT_OK or DE_NOTFOUND.
			DRESULT		(*GetDeviceName)(DDWORD nDeviceType, char* pStrBuffer, DDWORD nBufferSize);
			
			// Find out if the specified device is enabled yet.
			// Fills in the BOOL pointer and always returns LT_OK.
			DRESULT		(*IsDeviceEnabled)(char* strDeviceName, DBOOL* pIsEnabled);
			
			// Attempt to enable specified device.
			// Returns LT_OK or LT_ERROR.
			DRESULT		(*EnableDevice)(char* strDeviceName);

			// These access the real, accurate timer.
			float		(*GetTime)();
			float		(*GetFrameTime)();


			// These access the GAME timer (which resides on the server).
			// Since this comes from the server, it will be intermittent, so only 
			// use it for things that must be synced with server.  This timer will
			// not update when there isn't a connection to the server.
			float		(*GetGameTime)();
			float		(*GetGameFrameTime)();

			
			// Print a string to the console.
			void		(*CPrint)(char *pMsg, ...);

			// Used to output a TRACE message to the Debug Output window.  Newlines must be explicitly used.
			void	(*DebugOut)( char *pMsg, ... );

			DBOOL		(*IsCommandOn)(int commandNum);

			// Same as typing a string into the console.
			void		(*RunConsoleString)(char *pString);

			// Gives you a pointer to the client shell you created.
			LPCLIENTSHELLDE (*GetClientShell)();

			// Get your client object (NULL if you don't currently have one).
			HLOCALOBJ	(*GetClientObject)();

			// Rotate a rotation around the given vector.
			void		(*RotateAroundAxis)(DRotation *pRotation, DVector *pAxis, float amount);

			// Treat the rotation like Euler angles...
			void		(*EulerRotateX)(DRotation *pRotation, float amount);
			void		(*EulerRotateY)(DRotation *pRotation, float amount);
			void		(*EulerRotateZ)(DRotation *pRotation, float amount);

			// Align a rotation to a normal.  The vector you pass in becomes the
			// forward vector in the rotation it outputs.
			// Use pUp to set the frame of reference or set it to NULL, in which case
			// the up vector could be anything.  pUp should not be equal to pVector.
			void		(*AlignRotation)(DRotation *pRotation, DVector *pVector, DVector *pUp);

			// OBSOLETE: use CommonLT::SetupEuler.
			virtual DRESULT	SetupEuler(DRotation *pRotation, float pitch, float yaw, float roll)=0;

			// Interpolate between two rotations (with quaternions).
			DRESULT		(*InterpolateRotation)(DRotation *pDest, DRotation *pRot1, DRotation *pRot2, float t);

			// OBSOLETE.  Use CommonLT::GetRotationVectors.
			virtual DRESULT GetRotationVectors(DRotation *pRotation, DVector *pUp, DVector *pRight, DVector *pForward)=0;


			// You can set a global light scale that's applied to all rendering.
			// Colors are RGB 0-1 (values are clamped for you).	
			void		(*GetGlobalLightScale)(DVector *pScale);
			void		(*SetGlobalLightScale)(DVector *pScale);
			void		(*OffsetGlobalLightScale)(DVector *pOffset);

			// Obsolete, use Parse2.
			int			(*Parse)(char *pCommand, char **pNewCommandPos, char *argBuffer, char **argPointers, int *nArgs);
	


		// Messaging.

			virtual HMESSAGEWRITE	StartMessage(DBYTE messageID)=0;
			virtual DRESULT			EndMessage(HMESSAGEWRITE hMessage)=0; // Just calls EndMessage2 with MESSAGE_GUARANTEED.
			virtual DRESULT			EndMessage2(HMESSAGEWRITE hMessage, DDWORD flags)=0;


		// NEW message functions.  These functions don't free the message so you need to
		// call LMessage::Release after sending.

			virtual DRESULT	SendToServer(LMessage &msg, DBYTE msgID, DDWORD flags)=0;


		// Management of client-side objects.

			HLOCALOBJ	(*CreateObject)(ObjectCreateStruct *pStruct);
			DRESULT		(*DeleteObject)(HLOCALOBJ hObj);

			// Gets the objects attached to this object.  outListSize is filled in with how many
			// objects it filled into inList and outNumAttachment is the actual number of attachments
			// (which can be larger than outListSize if inListSize is too small to fit them all).
			DRESULT		(*GetAttachments)(HLOCALOBJ hObj, HLOCALOBJ *inList, DDWORD inListSize, 
				DDWORD *outListSize, DDWORD *outNumAttachments);

			// Updates the position/rotation of the attachments on the object.  Attachments are
			// always automatically updated when the object is rendered.
			virtual DRESULT	ProcessAttachments(HOBJECT hObj)=0;
			  

			// Change position and rotation.  It's more efficient to set them 
			// at the same time...
			void		(*GetObjectPos)(HLOCALOBJ hObj, DVector *pPos);
			void		(*SetObjectPos)(HLOCALOBJ hObj, DVector *pPos);

			void		(*GetObjectRotation)(HLOCALOBJ hObj, DRotation *pRotation);
			void		(*SetObjectRotation)(HLOCALOBJ hObj, DRotation *pRotation);
			void		(*SetObjectPosAndRotation)(HLOCALOBJ hObj, DVector *pPos, DRotation *pRotation);

			DDWORD		(*GetObjectType)(HLOCALOBJ hObj);
			
			// Get/Set scale.
			DRESULT		(*GetObjectScale)(HLOCALOBJ hObj, DVector *pScale);
			DRESULT		(*SetObjectScale)(HLOCALOBJ hObj, DVector *pScale);

			// RGB 0-1.
			void		(*GetObjectColor)(HLOCALOBJ hObject, float *r, float *g, float *b, float *a);
			void		(*SetObjectColor)(HLOCALOBJ hObject, float r, float g, float b, float a);

			DDWORD		(*GetObjectFlags)(HLOCALOBJ hObj);
			void		(*SetObjectFlags)(HLOCALOBJ hObj, DDWORD flags);

			// Get/Set object user flags.  Can't set user flags on an object
			// created on the server.
			DRESULT		(*GetObjectUserFlags)(HLOCALOBJ hObj, DDWORD *pFlags);
			DRESULT		(*SetObjectUserFlags)(HLOCALOBJ hObj, DDWORD flags);
			
			// Get/set the client flags (defined above).
			DDWORD		(*GetObjectClientFlags)(HLOCALOBJ hObj);
			void		(*SetObjectClientFlags)(HLOCALOBJ hObj, DDWORD flags);

			// User data for the object..
			void*		(*GetObjectUserData)(HLOCALOBJ hObj);
			void		(*SetObjectUserData)(HLOCALOBJ hObj, void *pData);


		// Camera functions.

			// Gets the 3D coordinates of a screen coordinate given a camera.  The 3D
			// coordinate is one unit out along the forward vector.  Returns DE_OUTSIDE
			// if the screen coordinates aren't inside the camera's rectangle.
			DRESULT		(*Get3DCameraPt)(HLOCALOBJ hCamera, int sx, int sy, DVector *pOut);

			// Get/Set a camera's FOV.  It defaults to (PI/2, PI/2).
			// It will clamp your values between (PI/100, 99*PI/100)
			void		(*GetCameraFOV)(HLOCALOBJ hObj, float *pX, float *pY);
			void		(*SetCameraFOV)(HLOCALOBJ hObj, float fovX, float fovY);

			void		(*GetCameraRect)(HLOCALOBJ hObj, DBOOL *bFullscreen,
				int *left, int *top, int *right, int *bottom);
			
			// Set the camera's rectangle on the screen.
			// If bFullscreen is DTRUE, then it ignores the rect and draws the
			// camera fullscreen.  If the rectangle extends over the screen
			// boundaries, then it is clipped..
			void		(*SetCameraRect)(HLOCALOBJ hObj, DBOOL bFullscreen, 
				int left, int top, int right, int bottom);

			// Get/Set the camera light add.  RGB 0-1.  Light add is applied AFTER
			// scaling, so if light is fully bright and scaling is zero, you'll just
			// see whiteness.  When the light add is nonzero, it draws a poly over 
			// the screen so don't use it all the time!
			// These return DFALSE if the object is not a camera.
			DBOOL		(*GetCameraLightAdd)(HLOCALOBJ hCamera, DVector *pAdd);
			DBOOL		(*SetCameraLightAdd)(HLOCALOBJ hCamera, DVector *pAdd);



		// Particle system manipulation.

			// gravityAccel default is -500
			// flags default is 0
			// particleRadius default is 300
			// color scale defauls to 1.0
			// Particle colors are 0-255.
			
			// All particle positions are RELATIVE to the particle system's 
			// position and rotation.  In many cases, you can have your code be very simple
			// and fast if you just move and rotate the particle system and not the particles.
			
			// Change the system's parameters.
			DRESULT		(*SetupParticleSystem)(HLOCALOBJ hObj, char *pTextureName, float gravityAccel, DDWORD flags, float particleRadius);

			// The software version uses a single color for all the particles in each
			// system specified here (default 1).  RGB (0-1).
			DRESULT		(*SetSoftwarePSColor)(HLOCALOBJ hObj, float r, float g, float b);

			virtual DEParticle*	AddParticle(HLOCALOBJ hObj, DVector *pPos, DVector *pVelocity, DVector *pColor, float lifeTime)=0;
			
			void		(*AddParticles)(HLOCALOBJ hObj, DDWORD nParticles,
				DVector *pMinOffset, DVector *pMaxOffset,
				DVector *pMinVelocity, DVector *pMaxVelocity, 
				DVector *pMinColor, DVector *pMaxColor,
				float minLifetime, float maxLifetime);

			DBOOL		(*GetParticles)(HLOCALOBJ hObj, DEParticle **pHead, DEParticle **pTail);

			// Get/Set particle positions.  hSystem is NOT checked to be valid here
			// for speed so make sure it's valid!
			void		(*GetParticlePos)(HLOCALOBJ hSystem, DEParticle *pParticle, DVector *pPos);
			void		(*SetParticlePos)(HLOCALOBJ hSystem, DEParticle *pParticle, DVector *pPos);

			// Remove a particle.
			void		(*RemoveParticle)(HLOCALOBJ hSystem, DEParticle *pParticle);

			// This is an optimization you can make to help the engine minimize its boundaries on
			// a particle system.  If you create particles in various places and they go away, you
			// can use this every so often to recalculate where the particles are.
			DRESULT		(*OptimizeParticles)(HLOCALOBJ hSystem);
			


		// Line system manipulation.
		// As with particle systems, the lines are centered around the object's origin.
		// Don't just place the object at the origin and put lines way off to the side,
		// it's more efficient to keep the lines as close to the center as possible.

			// Set hPrev to NULL to start, then pass in the return value, etc..
			// Returns DNULL for last line.
			// ** If you call RemoveLine on the current HDELINE, DO NOT
			// ** pass that into GetNextLine - call GetNextLine first
			// ** while the HDELINE is still valid!
			HDELINE		(*GetNextLine)(HLOCALOBJ hObj, HDELINE hPrev);

			void		(*GetLineInfo)(HDELINE hLine, DELine *pLine);
			void		(*SetLineInfo)(HDELINE hLine, DELine *pLine);

			// Adds a line to the end of the line system's list.
			HDELINE		(*AddLine)(HLOCALOBJ hObj, DELine *pLine);
			void		(*RemoveLine)(HLOCALOBJ hObj, HDELINE hLine);


		// Poly grid manipulation.

			// A poly grid is basically a heightmapped grid of pixels that are drawn
			// as polygons.  Each pixel can have a value from -127 to 127.  The pixel's value 
			// defines its height and is a lookup into the color table for the vertex color.
			// You can scale and rotate the poly grid using SetObjectScale and SetObjectRotation.
			// bHalfTriangles will cause it to look VERY triangulated, but draw way faster.
			DBOOL		(*SetupPolyGrid)(HLOCALOBJ hObj, DDWORD width, DDWORD height, DBOOL bHalfTrianges);

			// Set the texture.  The texture MUST be a sprite file.  It CANNOT be a .dtx file!
			DRESULT		(*SetPolyGridTexture)(HLOCALOBJ hObj, char *pFilename);

			// Set the environment map for the PolyGrid.  This MUST be a DTX file.
			// Specify NULL if you want to disable the environment map.  Returns 
			// DE_NOTFOUND if it can't find the specified map.
			DRESULT		(*SetPolyGridEnvMap)(HLOCALOBJ hObj, char *pFilename);

			// Get/Set the texture pan and scale for a PolyGrid.
			// Defaults are 0.0 for xPan and yPan, and 1.0 for xScale and yScale.
			DRESULT		(*GetPolyGridTextureInfo)(HLOCALOBJ hObj, float *xPan, float *yPan, float *xScale, float *yScale);
			DRESULT		(*SetPolyGridTextureInfo)(HLOCALOBJ hObj, float xPan, float yPan, float xScale, float yScale);
			
			// You can set it to be transparent or not.  It defaults to not being transparent.
			DRESULT		(*GetPolyGridInfo)(HLOCALOBJ hObj, char **pBytes, DDWORD *pWidth, DDWORD *pHeight, PGColor **pColorTable);

			// Set pMin and pMax to the dimensions of the box you want the polygrid to fit in.
			// pPos and pScale will be filled in with the recommended position and scale.
			DRESULT		(*FitPolyGrid)(HLOCALOBJ hObj, DVector *pMin, DVector *pMax, DVector *pPos, DVector *pScale);


		// Light manipulation.

			// Get/Set a light's color (RGB, 0.0f to 1.0f).
			// When you create a light, its color defaults to (0,0,0).
			void	(*GetLightColor)(HLOCALOBJ hObj, float *r, float *g, float *b);
			void	(*SetLightColor)(HLOCALOBJ hObj, float r, float g, float b);

			// Get/Set a light's radius.
			// When you create a light, its radius defaults to 100.
			float	(*GetLightRadius)(HLOCALOBJ hObj);
			void	(*SetLightRadius)(HLOCALOBJ hObj, float radius);


		// Container manipulation.

			// Get a container's code.
			// Returns DFALSE if the object is not a container.
			DBOOL	(*GetContainerCode)(HLOCALOBJ hObj, D_WORD *pCode);

			// Find out what containers contain the given point.
			// Returns the number of containers filled in.
			DDWORD	(*GetPointContainers)(DVector *pPoint, HLOCALOBJ *pList, DDWORD maxListSize);


		// Sprite manipulation.

			// This clips the sprite on the poly.
			// Returns DE_OK or DE_ERROR if not a sprite.
			// Pass in INVALID_HPOLY to un-clip the sprite.
			DRESULT	(*ClipSprite)(HLOCALOBJ hObj, HPOLY hPoly);

			// Get the sprite control interface for a sprite.  Returns DE_INVALIDPARAMS
			// if the object is not a sprite.
			virtual DRESULT GetSpriteControl(HLOCALOBJ hObj, SpriteControl* &pControl)=0;


		// Client-side models..
		
			// Iterate through the model's nodes.  Returns DE_FINISHED when there are no more.
			// hCurNode = INVALID_MODEL_NODE;
			// while(interface->GetNextModelNode(hModel, hCurNode, &hCurNode) == DE_OK)
			// { ... }
			DRESULT	(*GetNextModelNode)(HLOCALOBJ hObject, HMODELNODE hNode, HMODELNODE *pNext);

			// Get a model node's name.
			DRESULT	(*GetModelNodeName)(HLOCALOBJ hObject, HMODELNODE hNode, char *pName, DDWORD maxLen);

			// Hide/unhide a node on the model (they're all unhidden by default).
			// Returns DE_OK, DE_ERROR, LT_NOCHANGE, or DE_NODENOTFOUND.
			DRESULT	(*GetModelNodeHideStatus)(HOBJECT hObj, char *pNodeName, /* out */DBOOL *bHidden);
			DRESULT	(*SetModelNodeHideStatus)(HOBJECT hObj, char *pNodeName, DBOOL bHidden);

			// Get the current (global) transformation for a model node.
			// Returns DFALSE if the node does not exist or if the object
			// you pass in is not a model.
			DBOOL	(*GetModelNodeTransform)(HOBJECT hObj, char *pNodeName,	
				DVector *pPos, DRotation *pRot);

			// Returns the animation the model is currently on.  (DDWORD)-1 if none.
			DDWORD	(*GetModelAnimation)(HLOCALOBJ hObj);
			void	(*SetModelAnimation)(HLOCALOBJ hObj, DDWORD iAnim);

			// Get an animation index from a model.
			// Returns -1 if the animation doesn't exist (or if the object isn't a model).
			HMODELANIM	(*GetAnimIndex)(HOBJECT hObj, char *pAnimName);

			// Starts the current animation over.
			DRESULT	(*ResetModelAnimation)(HLOCALOBJ hObj);

			// Tells what the playback state of the model is (a combination of the
			// MS_ bits defined in basedefs_de.h).
			DDWORD	(*GetModelPlaybackState)(HLOCALOBJ hObj);

			// Get/Set the looping state of the model.  The default state is TRUE.
			DBOOL	(*GetModelLooping)(HLOCALOBJ hObj);
			void	(*SetModelLooping)(HLOCALOBJ hObj, DBOOL bLoop);


		// (Geometry) surface functions.

			DBOOL	(*GetSurfaceBounds)(SurfaceData *pSurface, DVector *pMin, DVector *pMax);

			// Hide/Unhide a poly.
			DRESULT	(*SetPolyHideStatus)(HPOLY hPoly, DBOOL bHide);

			// Just used for internal debugging.. never use this.
			DRESULT	(*GetPolyIndex)(HPOLY hPoly, DDWORD *pIndex);

			// Get the texture flags from a poly.  Returns DE_OK
			// or DE_ERROR if no world is loaded or hPoly is invalid.
			DRESULT	(*GetPolyTextureFlags)(HPOLY hPoly, DDWORD *pFlags);

		
		// Render hooks.

			// When this is set, the renderer will call your hook function before drawing each
			// model.  Either keep the function very fast or set it to NULL so you don't slow
			// the renderer down.
			DRESULT	(*SetModelHook)(ModelHookFn fn, void *pUser);


		// Engine hooks.

			// This is here so we can avoid adding API functions if necessary and for
			// some system-dependent or misc. stuff.  Pass in a string describing what
			// you want and it fills in pData.
			// Returns DE_OK if it understands the string or an error otherwise.
			// Strings:
			//    HWND: Returns main window handle.
			DRESULT	(*GetEngineHook)(char *pName, void **pData);


		// Network startup/join/host functions.

			// Call this function before calling any other
			// network functions. pDriver can be NULL to use the default net driver.
			// No flags are currently supported.
			DRESULT (*InitNetworking)(char *pDriver, DDWORD dwFlags);

			// Gets a list of net services (tcp/ip, modem, etc).
			DRESULT (*GetServiceList)(NetService* &pListHead);

			// Call this function when you are finished using the list returned by
			// GetServiceList().
			DRESULT (*FreeServiceList)(NetService *pListHead);

			// Selects the given service as the one to use.
			DRESULT (*SelectService)(HNETSERVICE hNetService);

			// Gets a list (and count) of enumerated sessions.
			// See driver flags for a description of pInfo.
			DRESULT (*GetSessionList)(NetSession* &pListHead, char *pInfo);

			// Call this function when you are finished using the list returned by
			// GetSessionList().
			DRESULT (*FreeSessionList)(NetSession *pListHead);


			// Alternate mode of getting session lists.  These only work for services with
			// the NETSERVICE_TCPIP flag.  These functions return immediately so you can update
			// a UI in the background without having to 'freeze' the UI while it queries hosts.

				// Start querying.  pInfo contains the address list formatted just like GetSessionLists.
				virtual DRESULT	StartQuery(char *pInfo)=0;
				
				// Update the query.  Call this as often as possible.
				virtual DRESULT UpdateQuery()=0;

				// Get the current list of results from the query.  Each time you call this,
				// a new session list is allocated and you need to free it with FreeSessionList.
				virtual DRESULT GetQueryResults(NetSession* &pListHead)=0;

				// End the current query.
				virtual DRESULT EndQuery()=0;


			// Determines if we werw lobby launched.
			DRESULT (*IsLobbyLaunched)(char* sDriver);

			// Gets the lobby launch info if available.
			DRESULT (*GetLobbyLaunchInfo)(char* sDriver, void** ppLobbyLaunchData);

			// Gets the tcp/ip address of the main driver if available.
			DRESULT (*GetTcpIpAddress)(char* sAddress, DDWORD dwBufferSize);

	protected:
		
			CommonLT	*m_pCommonLT;
			CPhysicsLT	*m_pPhysicsLT;
	};


	// For backward compatibility.. never use this.
	#define CClientDE ClientDE


#endif  // __CLIENT_DE_H__


