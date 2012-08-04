// ----------------------------------------------------------------------- //
//
// MODULE  : ScaleSprite.cpp
//
// PURPOSE : ScaleSprite class - implementation
//
// CREATED : 12/07/97
//
// ----------------------------------------------------------------------- //

#ifndef __SCALE_SPRITE_H__
#define __SCALE_SPRITE_H__

#include "GameBase.h"
#include "Destructible.h"
#include "SFXFuncs.h"

class ScaleSprite : public GameBase
{
	public :

		ScaleSprite();
		~ScaleSprite();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private :

        LTBOOL InitialUpdate();
        LTBOOL Update();
		void  ReadProp(ObjectCreateStruct *pStruct);
		void  PostPropRead(ObjectCreateStruct *pStruct);

        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwSaveFlags);
		void	CacheFiles();

        LTVector m_vScale;               // Size (relative) of sprite
        LTVector m_vColor;               // Color of the sprite
        LTFLOAT  m_fAlpha;               // Sprite Alpha
        LTBOOL   m_bFlushWithWorld;      // Is the sprite world aligned?
        LTBOOL   m_bRotatable;           // Is the sprite rotatable?
        uint32  m_dwAdditionalFlags;    // Additional sprite flags
        LTBOOL   m_bStartOn;             // Start visible?

		HSTRING	m_hstrDamagedFile;
		HSTRING	m_hstrDestroyedFile;

		LENSFLARE	m_LensInfo;			// Lens flare info

		void SetDestroyed();
		void SetDamaged();
};

#endif // __SCALE_SPRITE_H__