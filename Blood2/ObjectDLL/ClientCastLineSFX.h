// ----------------------------------------------------------------------- //
//
// MODULE  : CClientCastLineSFX.h
//
// PURPOSE : CClientCastLineSFX - Definition
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_CAST_LINE_SFX_H__
#define __CLIENT_CAST_LINE_SFX_H__

#include "ClientSFX.h"


class CClientCastLineSFX : public CClientSFX
{
	public :

		void Setup(DVector vStartColor, DVector vEndColor,
				   DFLOAT fStartAlpha, DFLOAT fEndAlpha, HOBJECT hCastTo=DNULL );
};

#endif // __CLIENT_CAST_LINE_SFX_H__
