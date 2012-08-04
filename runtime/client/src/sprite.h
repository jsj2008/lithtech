//------------------------------------------------------------------
//
//	FILE	  : SPRITE.H
//
//	PURPOSE	  : CSprite definition file
//
//	CREATED	  : 20th November 1996
//
//	COPYRIGHT : MONOLITH Inc 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __SPRITE_H__
#define __SPRITE_H__


class CClientMgr;
struct Sprite;
class ILTStream;
struct SpriteTracker;


// Create the sprite from a file.  Might throw an error for fatal conditions,
// or will return LTNULL for an invalid file.
Sprite* spr_Create(ILTStream *pStream);
void spr_Destroy(Sprite *pSprite);

void spr_InitTracker(SpriteTracker *pTracker, Sprite *pSprite);
void spr_UpdateTracker(SpriteTracker *pTracker, uint32 ms);


#endif




