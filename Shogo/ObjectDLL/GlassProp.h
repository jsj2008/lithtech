// ----------------------------------------------------------------------- //
//
// MODULE  : GlassProp.h
//
// PURPOSE : Model GlassProp - Definition
//
// CREATED : 1/26/98
//
// ----------------------------------------------------------------------- //

#ifndef __GLASS_PROP_H__
#define __GLASS_PROP_H__

#include "Prop.h"

class GlassProp : public Prop
{
	public :

 		GlassProp();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
	
	private :

		void	ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		DVector m_vDims;
};

#endif // __GLASS_PROP_H__
