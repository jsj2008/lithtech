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

#include "cpp_engineobjects_de.h"
#include "Destructable.h"

class ScaleSprite : public BaseClass
{
	public :

		ScaleSprite();
		~ScaleSprite();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
        DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
	private :

		DBOOL InitialUpdate();
		DBOOL Update();
		void  ReadProp(ObjectCreateStruct *pStruct);
		void  PostPropRead(ObjectCreateStruct *pStruct);

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwSaveFlags);
		void	CacheFiles();

		DVector m_vScale;				// Size (relative) of sprite
		DVector	m_vColor;				// Color of the sprite
		DFLOAT	m_fAlpha;				// Sprite Alpha
		DBOOL	m_bFlushWithWorld;		// Is the sprite world aligned?
		DBOOL	m_bRotatable;			// Is the sprite rotatable?
		DDWORD	m_dwAdditionalFlags;	// Additional sprite flags

		HSTRING	m_hstrDamagedFile;
		HSTRING	m_hstrDestroyedFile;

		void SetDestroyed();
		void SetDamaged();
};

#endif // __SCALE_SPRITE_H__
