
#ifndef __ILTSPRITECONTROL_H__
#define __ILTSPRITECONTROL_H__

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif

/*!  Sprite control flags.  Default flags for a sprite are
\b SC_PLAY|SC_LOOP.  */

enum
{
    SC_PLAY  = (1<<0),
    SC_LOOP  = (1<<1)
};

class ILTSpriteControl
{
public:

/*! 
\param nAnims (return) Number of animations.

\return Always returns \b LT_OK.

Get the number of animations in this sprite.

Used for: Special FX.
*/
    virtual LTRESULT GetNumAnims(uint32 &nAnims) = 0;

/*! 
\param iAnim    Animation to query.
\param nFrames  (return) Number of frames in animation.

\return \b LT_INVALIDPARAMS - \em iAnim is invalid (i.e. too high).
\return \b LT_OK - Successful.

Get the number of frames in specified animation.

Used for: Special FX.
*/
    virtual LTRESULT GetNumFrames(uint32 iAnim, uint32 &nFrames) = 0;

/*! 
\param iAnim    (return) Current animation.
\param iFrame   (return) Current animation.

\return Always returns \b LT_OK.

Get the currently playing animation and frame.

Used for: Special FX.
*/
    virtual LTRESULT GetCurPos(uint32 &iAnim, uint32 &iFrame) = 0;

/*! 
\param iAnim    Index of animation to set.
\param iFrame   Animation frame to set.

\return \b LT_INVALIDPARAMS - \em iAnim is invalid (i.e. too high) and/or 
        \em iFrame is invalid (i.e. too high).
\return \b LT_OK - Successful.

Set the currently playing animation and frame.

Used for: Special FX.
*/
    virtual LTRESULT SetCurPos(uint32 iAnim, uint32 iFrame) = 0;

/*! 
\param flags    (return) Flags (see SC_ defines, above).

\return Always returns \b LT_OK.

Get the flags for this sprite.

Used for: Special FX.
*/
    virtual LTRESULT GetFlags(uint32 &flags) = 0;

/*! 
\param flags    Flags (see SC_ defines, above).

\return Always returns \b LT_OK.

Set the flags for this sprite.

Used for: Special FX.
*/
    virtual LTRESULT SetFlags(uint32 flags) = 0;

/*! 
\param msLen    (return) Length (in milliseconds).
\param iAnim    Animation to query.

\return \b LT_ERROR - \em iAnim is invalid (i.e. too high).
\return \b LT_OK - Successful.

Retrieves the length of a sprite animation in milliseconds.

Used for: Special FX.
*/
    virtual LTRESULT GetAnimLength(uint32 &msLen, const uint32 iAnim) = 0;

/*! 
\param hTex     (return) Texture handle.
\param iAnim    Animation to query.
\param iFrame   Frame to query.

\return \b LT_ERROR - \em iAnim is invalid (i.e. too high), or some other internal 
        error occurred.
\return \b LT_OK - Successful.

Retrieve an HTEXTURE for a specific frame in this sprite.

Used for: Special FX.
*/
    virtual LTRESULT GetFrameTextureHandle(HTEXTURE &hTex, const uint32 iAnim, const uint32 iFrame) = 0;
};


#endif  //! __ILTSPRITECONTROL_H__
