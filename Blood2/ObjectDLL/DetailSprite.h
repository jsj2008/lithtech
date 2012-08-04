// ----------------------------------------------------------------------- //
//
// MODULE  : DetailSprite.cpp
//
// PURPOSE : DetailSprite class - implementation
//
// CREATED : 2/28/98
//
// Object Use:
//
// The DetailSprite object is used to place a sprite (sign, etc) on a wall for 
// decoration.
//
// Property					Type		Description
// ========					====		===========
//
// Pos						Vector		Initial position of the sprite
// Rotation					Rotation	Rotation of the sprite
// Filename					String		Sprite filename
// ScaleX					Float		Scale of the sprite in the X direction
// ScaleY					Float		Scale of the sprite in the Y direction
// FlushWithWorld			Bool		Sprite should be moved flush with the world.
// Rotatable				Bool		Sprite should have FLAG_ROTATEABLESPRITE set.
// Chromakey				Bool		Sprite should have FLAG_SPRITECHROMAKEY set.
//
// ----------------------------------------------------------------------- //

#ifndef __DETAILSPRITE_H__
#define __DETAILSPRITE_H__

#include "cpp_engineobjects_de.h"
#include "B2BaseClass.h"

class DetailSprite : public B2BaseClass
{
	public :

		DetailSprite();
		virtual ~DetailSprite();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		DBOOL	InitialUpdate(DVector *pMovement);
		DBOOL	Update(DVector *pMovement);
		void	ReadProp(ObjectCreateStruct *pStruct);
		void	CreateAnotherSprite(DVector *pvPos, DRotation *prRot);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		DVector m_vScale;
		HSTRING	m_hstrFilename;
		DBOOL	m_bRotatable;
		DBOOL	m_bChromaKey;
		DBOOL	m_bAlignedToWorld;
		DBOOL	m_bFlushWithWorld;
		DBOOL	m_bRepeat;
		DDWORD	m_dwRepeatX;
		DDWORD	m_dwRepeatY;
		DFLOAT	m_fSpacingX;
		DFLOAT	m_fSpacingY;
};

#endif // __DETAILSPRITE_H__
