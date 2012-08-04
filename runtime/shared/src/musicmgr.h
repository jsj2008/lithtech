
#ifndef __MUSICMGR_H__
#define __MUSICMGR_H__

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif


//-------------------------------------------------------------------------------------
//
// MusicMgr
//
// Defines the interface for playing music.
// 
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
//
// music_caps
//
// Caps bits for music.
//
//-------------------------------------------------------------------------------------
enum _music_caps
{   
    MUSIC_VOLUMECONTROL = 0x0001,   // Volume control allowed.
    MUSIC_INSTRUMENTSET = 0x0002,   // Instrument set supported.
    MUSIC_MOTIFSAVAIL   = 0x0004    // Motifs available.
};

//-------------------------------------------------------------------------------------
//
// SMusicMgr
//
// Function interface for playing sounds through Sound DLL.
//
//-------------------------------------------------------------------------------------
struct SMusicMgr
{
    // Initialize the music engine...
    bool  (*Init)(SMusicMgr *pMusicMgr);

    // Terminate the music engine...
    void    (*Term)();

    // Set the data directory...
    bool  (*SetDataDirectory)(const char *szDataDir);

    // Set instrument set for music engine, must have caps for this...
    bool  (*InitInstruments)(const char *szDLSFile, const char *szStyleFile);

// Song access stuff...

    // Preload the music data for song...
    void *  (*CreateSong)(const char *pFileName);

    // Get a song pointer...
    void *  (*GetSong)(const char *pFileName);

    // Destroy the music data for a song...
    void    (*DestroySong)(void *pSong);

// Song play stuff...

    // Play preloaded music...
    bool  (*PlayBreak)(void *pSong, uint32 dwPlayBoundaryFlags);

    // Create a playlist...
    void *  (*CreatePlayList)(const char *szPlayListName);

    // Get a playlist...
    void *  (*GetPlayList)(const char *szPlayListName);

    // Add song to playlist...
    bool  (*AddSongToList)(void *pPlayList, void *pSong);

    // Get music data for a song in a playlist...
    void *  (*GetSongFromList)(void *pPlayList, unsigned long dwIndex);

    // Remove a playlist...
    void    (*RemoveList)(void *pPlayList);

    // Play the playlist.  bLoop=1 will play playlist forever.  
    bool  (*PlayList)(void *pPlayList, void *pTransition, bool bLoop, uint32 dwPlayBoundaryFlags);

    // Play a motif on top of current song.  Must have caps for this...
    bool  (*PlayMotif)(const char *szMotifName, bool bLoop);

    // Play a motif on top of current song.  Must have caps for this...
    bool  (*StopMotif)(const char *szMotifName);

    // Destroy the music data for all songs...
    void    (*DestroyAllSongs)();

    // Stop the current song...
    void    (*Stop)(uint32 dwPlayBoundaryFlags);

    // Pause the current song...
    bool  (*Pause)(uint32 dwPlayBoundaryFlags);

    // Resume the current song...
    bool  (*Resume)();

    // Get the volume.  Must have caps for this.  100-full volume, 0-no volume...
    uint16  (*GetVolume)();

    // Set the volume.  Must have caps for this.  100-full volume, 0-no volume...
    void    (*SetVolume)(unsigned short vol);

    // App must provide a console print function, so music dll can send messages to console.
    void    (*ConsolePrint)(const char *pMsg, ...);

    // Music dll setup correctly.  Set by music dll.  0-not valid, !0-valid.
    bool  m_bValid;

    // Music enabled.  Set by dll to 1.  App can set to 0 or 1.  0-not valid, !0-valid.
    bool  m_bEnabled;

    // Handle to window playing music.  Set by app.
    void *  m_hWnd;

    // Handle to Instance of app.  Set by app.
    void *  m_hInstance;

    // Capability bits.  Set by music dll.
    uint32  m_ulCaps;

};
typedef SMusicMgr *LPSMusicMgr;


// To make a DirectEngine sound DLL, make a DLL with the function 
// "MusicDLLSetup" that looks like this.  This function should init all the
// function pointers in the structure.
typedef void (*MusicDLLSetupFn)(SMusicMgr *pMusicMgr);


#endif // __MUSICMGR_H__

