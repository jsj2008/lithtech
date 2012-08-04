
#ifndef __SPRINKLES_H__
#define __SPRINKLES_H__


#include "ltengineobjects.h"
#include "SharedFXStructs.h"


LINKTO_MODULE( Sprinkles );

	class Sprinkles : public BaseClass
	{
	public:

						Sprinkles();
		virtual			~Sprinkles();

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		void			OnPreCreate(ObjectCreateStruct *pStruct);
		void			OnInitialUpdate();
		uint32			OnUpdate();


	public:

		SPRINKLESCREATESTRUCT m_SprinkleStruct;
	};


#endif

