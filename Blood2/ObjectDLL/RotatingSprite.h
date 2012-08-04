// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingSprite.cpp
//
// PURPOSE : A version of DetailSprite that can rotate,
//
//
// ----------------------------------------------------------------------- //

#ifndef __ROTATINGSPRITE_H__
#define __ROTATINGSPRITE_H__

#include "cpp_engineobjects_de.h"
#include "DetailSprite.h"
#include "Rotating.h"

class RotatingSprite : public DetailSprite
{
	public :

		RotatingSprite();
		virtual ~RotatingSprite() {}

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

		Rotating		m_Rotating;
};

#endif // __ROTATINGSPRITE_H__
