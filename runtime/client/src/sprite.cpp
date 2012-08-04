//------------------------------------------------------------------
//	FILE	  : SPRITE.CPP
//	PURPOSE	  : CSprite implementation file
//	CREATED	  : 20th November 1996
//	COPYRIGHT : MONOLITH Inc 1996 All Rights Reserved
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "sprite.h"
#include "clientmgr.h"
#include "sysfile.h"
#include "de_objects.h"



Sprite* spr_Create(ILTStream *pStream)
{
	uint32 nFrames, nFrameRate, bTransparent, bTranslucent, colourKey;
	uint32 i, iAnim;
	char s[1024];
	Sprite *pSprite;
	SpriteAnim *pAnim;
	uint16 strLen;
	FileRef ref;


	pSprite = LTNULL;

	// Setup the Sprite.
	LT_MEM_TRACK_ALLOC(pSprite = (Sprite*)dalloc(sizeof(Sprite)),LT_MEM_TYPE_SPRITE);
	memset(pSprite, 0, sizeof(Sprite));
	pSprite->m_Link.m_pData = pSprite;

	// Read in the animations
	pSprite->m_nAnims = 1;  // Sprites only get one animation currently...
	
	LT_MEM_TRACK_ALLOC(pSprite->m_Anims = (SpriteAnim*)dalloc(sizeof(SpriteAnim) * pSprite->m_nAnims),LT_MEM_TYPE_SPRITE);
	memset(pSprite->m_Anims, 0, sizeof(SpriteAnim) * pSprite->m_nAnims);
	
	for(iAnim=0; iAnim < pSprite->m_nAnims; iAnim++)
	{
		pAnim = &pSprite->m_Anims[iAnim];

		STREAM_READ(nFrames);
		STREAM_READ(nFrameRate);
		STREAM_READ(bTransparent);
		STREAM_READ(bTranslucent);
		STREAM_READ(colourKey);

		// Allocate array for resource ID's
		LT_MEM_TRACK_ALLOC(pAnim->m_Frames = (SpriteEntry*)dalloc(sizeof(SpriteEntry) * nFrames),LT_MEM_TYPE_SPRITE);

		// Record the name of the animation
		LTStrCpy(pAnim->m_sName, "Untitled", sizeof(pAnim->m_sName));
		
		// Set the number of frames in this animation to zero
		pAnim->m_nFrames = nFrames;
		pAnim->m_MsFrameRate = nFrameRate;
		pAnim->m_MsAnimLength = (1000 / nFrameRate) * nFrames;

		pAnim->m_bKeyed = (uint8)bTransparent;
		pAnim->m_bTranslucent = (uint8)bTranslucent;
		pAnim->m_ColourKey = colourKey;


		// Read in the frames for the animation.
		for(i=0; i < nFrames; i++)
		{
			// Read in frame file name
			STREAM_READ(strLen);
			if(strLen > 1000)
			{
				spr_Destroy(pSprite);
				return LTNULL;
			}

			pStream->Read(s, strLen);
			s[strLen] = 0;

			ref.m_FileType = FILE_CLIENTFILE;
			ref.m_pFilename = s;
			pAnim->m_Frames[i].m_pTex = g_pClientMgr->AddSharedTexture(&ref);
		}
	}

	if(pStream->ErrorStatus() != LT_OK)
	{
		spr_Destroy(pSprite);
		return LTNULL;
	}
	else
	{
		return pSprite;
	}
}


void spr_Destroy(Sprite *pSprite)
{
	uint32 i;

    if(pSprite->m_pFileIdent)
	{
		pSprite->m_pFileIdent->m_pData = LTNULL;
	}
			
	// Remove ourselves from the m_Sprites lists
	dl_Remove(&pSprite->m_Link); 
	dl_TieOff(&pSprite->m_Link);

	if(pSprite->m_Anims)
	{
		for(i=0; i < pSprite->m_nAnims; i++)
		{
			if(pSprite->m_Anims[i].m_Frames)
            {
				dfree(pSprite->m_Anims[i].m_Frames);
            }
		}

		dfree(pSprite->m_Anims);
	}

	dfree(pSprite);
}


void spr_InitTracker(SpriteTracker *pTracker, Sprite *pSprite)
{
	pTracker->m_pSprite = pSprite;
	pTracker->m_pCurAnim = &pSprite->m_Anims[0];
	
	if(pTracker->m_pCurAnim->m_nFrames > 0)
		pTracker->m_pCurFrame = &pTracker->m_pCurAnim->m_Frames[0];
	else
		pTracker->m_pCurFrame = LTNULL;

	pTracker->m_MsCurTime = 0;
	pTracker->m_Flags = SC_PLAY | SC_LOOP;
}


void spr_UpdateTracker(SpriteTracker *pTracker, uint32 msDelta)
{
	uint32 len;
	uint32 iFrame;

	if(pTracker->m_pCurAnim)
	{
		len = pTracker->m_pCurAnim->m_MsAnimLength;
		if(len)
		{
			if(pTracker->m_Flags & SC_PLAY)
			{
				pTracker->m_MsCurTime += msDelta;
			}
			
			if(pTracker->m_MsCurTime >= pTracker->m_pCurAnim->m_MsAnimLength && !(pTracker->m_Flags & SC_LOOP))
			{
				pTracker->m_MsCurTime = pTracker->m_pCurAnim->m_MsAnimLength - 1;
			}
			else
			{
				pTracker->m_MsCurTime %= pTracker->m_pCurAnim->m_MsAnimLength;
			}

			// Figure out current frame
			iFrame = (pTracker->m_MsCurTime / (1000 / pTracker->m_pCurAnim->m_MsFrameRate)) % pTracker->m_pCurAnim->m_nFrames;
			pTracker->m_pCurFrame = &pTracker->m_pCurAnim->m_Frames[iFrame];
		}
		else
		{
			pTracker->m_pCurFrame = LTNULL;
		}
	}
}





