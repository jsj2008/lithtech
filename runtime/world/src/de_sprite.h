#ifndef __DE_SPRITE_H__
#define __DE_SPRITE_H__
// This module defines the Sprite structures.


class SharedTexture;
struct FileIdentifier;

struct SpriteEntry
{
    SharedTexture *m_pTex;
};


struct SpriteAnim
{
    char            m_sName[32];
    SpriteEntry     *m_Frames;
    uint32          m_nFrames;
    uint32          m_MsAnimLength;
    uint32          m_MsFrameRate;
    LTBOOL          m_bKeyed;
    uint32          m_ColourKey;
    LTBOOL          m_bTranslucent;
};


struct Sprite
{
	Sprite()
 	{
 		m_pFileIdent = LTNULL;
 		m_bTagged	 = false;
 		dl_TieOff(&m_Link);
 	}
 
     LTLink			m_Link;
     SpriteAnim		*m_Anims;
     uint32			m_nAnims;
 	FileIdentifier* m_pFileIdent;
 	bool			m_bTagged;
};


// Tracks sprite frames.
struct SpriteTracker
{
    Sprite          *m_pSprite;
    SpriteAnim      *m_pCurAnim;
    SpriteEntry     *m_pCurFrame;           
    uint32          m_MsCurTime;
    uint32          m_Flags;
};


#endif  // __DE_SPRITE_H__
