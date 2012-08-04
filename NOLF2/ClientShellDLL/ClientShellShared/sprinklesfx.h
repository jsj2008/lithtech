
#ifndef __SPRINKLESFX_H__
#define __SPRINKLESFX_H__


#include "SpecialFX.h"
#include "SharedFXStructs.h"



	class FXType;


	typedef enum
	{
		FXObj_Particle,
		FXObj_Model
	} FXObjType;


	class FXObj
	{
	public:

		BOOL		IsValid()	{return !!m_pParticle;}


	public:

		union
		{
            LTParticle  *m_pParticle;
			HOBJECT		m_hModel;
		};

        LTVector     m_Angles;
        LTVector     m_AnglesVel;

        LTVector     m_Velocity;
        LTLink       m_Link;
	};


	// These abstract the differences between model and particle sprinkles.
	class FXObjControl
	{
	public:

		virtual void	DebugPrint(FXType *pType, FXObj *pObj)=0;
        virtual LTVector GetObjPos(FXType *pType, FXObj *pObj)=0;
        virtual void    SetObjPos(FXType *pType, FXObj *pObj, const LTVector &pos)=0;

		virtual void	SetObjAlpha(FXObj *pObj, float alpha)=0;
        virtual void    SetObjColor(FXObj *pObj, const LTVector &color)=0;

		virtual void	ShowObj(FXObj *pObj)=0;
		virtual void	HideObj(FXObj *pObj)=0;
	};


	class FXType
	{
	public:

					FXType();
					~FXType();


		void		AddFreeObj(FXObj *pObj);
		FXObj*		PopFreeObj();


	public:

		FXObjType		m_ObjType;
		FXObjControl	*m_pControl;

		// Maximum angular velocity (goes from -X to X).
        LTVector     m_AnglesVel;

		float		m_SpawnRadius;	// Radius around the player they spawn in from.
		float		m_Speed;		// How fast they travel.

        LTVector     m_ColorMin;
        LTVector     m_ColorMax;

		// Particle system.
		HOBJECT		m_hObject;

		// Particles.
		FXObj		*m_pObjects;
        uint32      m_nObjects;

        LTLink       m_ActiveList;
        LTLink       m_InactiveList;
	};


	class SprinklesFX : public CSpecialFX
	{
	public:

        virtual LTBOOL   Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL   Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL   Update();

		virtual uint32 GetSFXID() { return SFX_SPRINKLES_ID; }

	public:

		FXType			m_FXTypes[MAX_SPRINKLE_TYPES];
        uint32          m_nFXTypes;
	};


#endif


