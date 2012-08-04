/****************************************************************************
;
;   MODULE:     ILTDirectMusic (.H)
;
;   PURPOSE:    Define types and classes that need to be exposed to the game 
;               for the Lithtech implementation of directmusic.
;
;   HISTORY:    Aug-1-1999 [BLB]
;               Nov-16-2000 Updated documentation sections [BLB}
;
***************************************************************************/

#ifndef __ILTDIRECTMUSIC_H__
#define __ILTDIRECTMUSIC_H__

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

/*!  
Types used to describe when to start playing a segment or motif.
This includes doing a start, a stop, or anything else that reqires some 
type of change. 
*/

enum LTDMEnactTypes 
{
    LTDMEnactInvalid = 0,
    LTDMEnactDefault,
    LTDMEnactImmediately,
    LTDMEnactNextBeat,
    LTDMEnactNextMeasure,
    LTDMEnactNextGrid,
    LTDMEnactNextSegment,
	LTDMEnactNextMarker
};

#define LTDMEnactImmediatly LTDMEnactImmediately

/*! 
The ILTDirectMusicMgr interface provides functions for using Driect Music.

Define a holder to get this interface like this:
\code
define_holder(ILTDirectMusicMgr, your_var);
\endcode
*/

class ILTDirectMusicMgr : public IBase {
public:
    interface_version(ILTDirectMusicMgr, 0);


/*! 
\return LT_OK if successful or LT_ERROR if not successful.

Initialize the LTDirectMusic interface. This should be called at the start
of the program to initialize the interface. It is typically called in the
user’s implementation of the OnEngineInitialized function.

Used for: Music.
*/
    virtual LTRESULT Init() = 0;



/*! 
\return Always returns LT_OK.

Terminate the LTDirectMusic interface. This should be called at the end
of the program to clean up and terminate the interface. It is typically
called in the user’s implementation of the OnEngineTerm function.

Used for: Music.
*/
    virtual LTRESULT Term() = 0;

/*! 
\param sWorkingDirectory The name of the default directory containing 
the music content including the control file.
\param sControlFileName The name of the music control file to read in 
for this level. If no path is specified it is read from the default 
directory.
\param sDefine1 First optional define parameter which can be used to pass
information into the control file.  The control file can then use
a #ifdef to check for a define. For example you might want to define
"HIGH_QUALITY" or "LOW_WUALITY" to control the quality level of the music.
\param sDefine2 Second optional define parameter.
\param sDefine3 Third optional define parameter.
\return LT_OK if successful or LT_ERROR if not successful.

Called at the start of each level to initialize the music for that level.
The directory for all of the music files and the control file is specified in
the sWorkingDirectory parameter. The name of the control file to use is
specified in the sControlFileName parameter. Up to three optional
defines can also be used to pass information to the control file. These
defines are used when loading the control file to optionally include or
exclude portions of the file. This function is typically called from the
user’s implementation of the DoLoadWorld function.

Used for: Music.
*/
    virtual LTRESULT InitLevel(const char* sWorkingDirectory, 
        const char* sControlFileName, const char* sDefine1 = NULL,
       const char* sDefine2 = NULL, const char* sDefine3 = NULL) = 0;



/*! 
\return Always returns LT_OK.

Called at the end of each level to terminate the music for that level.
This function is typically called from the users implementation of the
OnExitWorld function.

Used for: Music.
*/
    virtual LTRESULT TermLevel() = 0;




/*! 
\return LT_OK if successful or LT_ERROR if not successful.

Called to begin playing music . This function is typically called from the
users implementation of the FirstUpdate function. Optionally, a trigger
in the level can be used to activate it.

Used for: Music.
*/
    virtual LTRESULT Play() = 0;




/*! 
\param nStart Optional parameter which describes when the music should stop
playing.
\return LT_OK if successful or LT_ERROR if not successful.

Called to stop the music from playing. This function is usually
called from a trigger in the level.

Used for: Music.
*/
    virtual LTRESULT Stop(const LTDMEnactTypes nStart = LTDMEnactDefault) = 0;


/*! 
\param nStart Optional parameter which describes when the music should stop
playing.
\return LT_OK if successful or LT_ERROR if not successful.

Pause music playback. 

Used for: Music.
*/
    virtual LTRESULT Pause(const LTDMEnactTypes nStart = LTDMEnactDefault) = 0;




/*! 
\return LT_OK if successful or LT_ERROR if not successful.

Resume music playback after a pause. 

Used for: Music.
*/
    virtual LTRESULT UnPause() = 0;



/*! 
\param nVolume Contains the volume to set. Negative values make 
the music volume lower and positive values make it higher. The number is in fractions 
of a decibel so fairly large numbers are needed to cause a noticeable effect.
\return LT_OK if successful or LT_ERROR if not successful.

Set the current volume adjustment. This function is usually called from a trigger in 
the level.

Used for: Music.
*/
    virtual LTRESULT SetVolume(const long nVolume) = 0;




/*! 
\param nNewIntensity Contains the intensity number to change to. This value must be
greater than zero.
\param nStart This is an optional parameter that determines when the intensity will
begin to change.
\return LT_OK if successful or LT_ERROR if not successful.

Change the current intensity that is being played. If a transition is defined in the
control file, the transition will be played before the new intensity starts.

Used for: Music.
*/
    virtual LTRESULT ChangeIntensity(const int nNewIntensity, 
        const LTDMEnactTypes nStart = LTDMEnactInvalid) = 0;




/*! 
\param sSecondarySegment Contains the name of the secondary segment to begin playing.
\param nStart This parameter defines when, relative to the current primary segment,
the secondary segment will begin playing.
\return LT_OK if successful or LT_ERROR if not successful.

Begin playing the secondary segment with the name specified. 

Used for: Music.
    */
    virtual LTRESULT PlaySecondary(const char* sSecondarySegment, 
        const LTDMEnactTypes nStart = LTDMEnactDefault) = 0;



/*! 
\param sSecondarySegment Contains the name of the secondary segment to stop playing. 
If the name is NULL then all secondary segments that are currently playing will stop.
\param nStart This parameter determines when playback will stop.
\return LT_OK if successful or LT_ERROR if not successful.

Stop playing all secondary segments with the name specified. If the name is NULL, then 
stop playing all secondary segments. 


Used for: Music.
*/
    virtual LTRESULT StopSecondary(const char* sSecondarySegment = NULL, 
        const LTDMEnactTypes nStart = LTDMEnactDefault) = 0;




/*! 
\param sStyleName The name of the style which contains the motif we want to play.
\param sMotifName The name of the motif to play.
\param nStart This parameter defines when, relative to the current primary segment,
the motif will begin playing.
\return LT_OK if successful or LT_ERROR if not successful.

Begin playing a motif.


Used for: Music.
*/
    virtual LTRESULT PlayMotif(const char* sStyleName, const char* sMotifName,
        const LTDMEnactTypes nStart = LTDMEnactDefault) = 0;




/*! 
\param sStyleName The name of the style which contains the motif we want to stop.  
If this name is NULL all styles will be checked.
\param sMotifName The name of the motif to stop. If this name is NULL all motifs 
that are playing will be stopped.
\param nStart This parameter determines when playback will stop.
\return LT_OK if successful or LT_ERROR if not successful.

Stop all motifs with the specified name in the specified style.  If the Style name is 
NULL then all styles will be searched.  If the Motif names is NULL all motifs will be stopped.


Used for: Music.
*/
    virtual LTRESULT StopMotif(const char* sStyleName, 
        const char* sMotifName = NULL, 
        const LTDMEnactTypes nStart = LTDMEnactDefault) = 0;




/*! 
\return The current intensity that is being played.

Used for: Music.
*/
    virtual int GetCurIntensity() = 0;




/*! 
\param sName Contains the string name to convert to an enact type.
\return The enact type which is represented by the string that was passed in. If
NULL is passed in, or an unrecognized string is passed in then LTDMEnactInvalid is
returned.

Convert a string to an enact type.  This is a very useful function to use when
converting user strings in triggers into enact types to pass into the LTDirectMusic
API.  The following conversions are done :

    "Invalid" = LTDMEnactInvalid

    "Default" = LTDMEnactDefault

    "Immediatly" = LTDMEnactImmediately

    "Immediate" = return LTDMEnactImmediately

    "NextBeat" = return LTDMEnactNextBeat

    "NextMeasure" = return LTDMEnactNextMeasure

    "NextGrid" = return LTDMEnactNextGrid

    "NextSegment" = return LTDMEnactNextSegment

	"NextMarker" = return LTDMEnactNextMarker

    "Beat" = return LTDMEnactNextBeat

    "Measure" = return LTDMEnactNextMeasure

    "Grid" = return LTDMEnactNextGrid

    "Segment" = return LTDMEnactNextSegment

	"Marker" = return LTDMEnactNextMarker


Used for: Music.
*/
    virtual LTDMEnactTypes StringToEnactType(const char* sName) = 0;




/*!
\param nType Enact type to convert to a string.
\param sName Pointer to a string buffer to contain the convert string. This
buffer must be at least 10 bytes to contain the largest possible enact value!
If an unrecognized enact type is passed in then the string will be set to an
empty string. (ie the first character will be \0)

Convert an enact type to a string. The following conversions are done :
        LTDMEnactInvalid = "Invalid"

        LTDMEnactDefault = "Default"

        LTDMEnactImmediately = "Immediate"

        LTDMEnactNextBeat = "Beat"

        LTDMEnactNextMeasure = "Measure"

        LTDMEnactNextGrid = "Grid"

        LTDMEnactNextSegment = "Segment"

		LTDMEnactNextMarker = "Marker"


Used for: Music.
*/
    virtual void EnactTypeToString(LTDMEnactTypes nType, char* sName) = 0;




/*!
\return The number of intensities defined in the current level.

\note The return value is undefined if you are not currently in a
level.

Returns the number of intensities defined in the current level.

Used for: Music.  */
    virtual int GetNumIntensities() = 0;




/*! 
\return The initial intensity value for this level.

Get the initial intensity for the current level. The return value is not defined 
if you are currently not in a level.

Used for: Music.
*/
    virtual int GetInitialIntensity() = 0;





/*!
\return The initial volume for this level.

Get the initial volume for the current level. The return value is not defined if 
you are currently not in a level.

Used for: Music.
*/
    virtual int GetInitialVolume() = 0;


/*!
\return The volume offset for this level.

Get the volume offset for the current level. The return value is not defined if 
you are currently not in a level.

Used for: Music.
*/
	virtual int GetVolumeOffset() = 0;

	
};
	
#endif //! __ILTDIRECTMUSIC_H__

