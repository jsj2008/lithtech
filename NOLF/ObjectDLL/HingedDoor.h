// ----------------------------------------------------------------------- //
//
// MODULE  : HingedDoor.h
//
// PURPOSE : A HingedDoor object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

#ifndef __HINGED_DOOR_H__
#define __HINGED_DOOR_H__

#include "RotatingDoor.h"

class HingedDoor : public RotatingDoor
{
	public:

		HingedDoor();

	protected:

        uint32  EngineMessageFn(uint32 messageID, void *pData, float fData);

		virtual void SetOpening();
        virtual void SetClosed(LTBOOL bInitialize=LTFALSE);
		virtual void SetLightAnimOpen();
		virtual float GetRotatingLightAnimPercent();

	private :

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
        void    Save(HMESSAGEWRITE hWrite, uint8 nType);
        void    Load(HMESSAGEREAD hRead, uint8 nType);

        LTBOOL   m_bOpeningNormal;   // Tells if it's opening in its normal direction or
									// the opposite.
        LTBOOL   m_bOpenAway;
        LTVector m_vOriginalOpenDir;
        LTVector m_vOriginalOpenAngles;
};


// Used by preprocessor lighting stuff.
void SetupTransform_HingedDoor(ILTPreLight *pInterface,
	HPREOBJECT hObject,
	float fPercent,
    LTVector &vOutPos,
    LTRotation &rOutRotation);


#endif // __HINGED_DOOR_H__