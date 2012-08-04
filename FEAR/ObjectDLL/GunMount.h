// ----------------------------------------------------------------------- //
//
// MODULE  : GunMount.h
//
// PURPOSE : This object allows a gun to be attached to objects and controlled
//				through triggers.
//
// CREATED : 5/22/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GUNMOUNT_H__
#define __GUNMOUNT_H__

//
// Includes...
//
#include "Character.h"

LINKTO_MODULE( GunMount );

class GunMount : public GameBase
{
	public: // Methods...

		GunMount( );
		~GunMount( );

	protected: // Methods... 

        uint32	EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );

	private:

		DECLARE_MSG_HANDLER( GunMount, HandleFireMsg );
		DECLARE_MSG_HANDLER( GunMount, HandleVisibleMsg );

		void	InitialUpdate( );
		bool	ReadProp( const GenericPropList *pProps );
        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void	Update( );

		bool	CreateWeapon( );

		// Fires the projectile.
		bool	FireWeapon( );
		bool	GetFirePos( LTVector& vPos );

		// Checks if current animation is finished.
		bool	IsAnimationComplete( );

		// State machine functions.
		bool	StartNotFiring( );
		bool	StartPreFiring( );
		void	UpdatePreFiring( );
		bool	StartFiring( );
		void	UpdateFiring( );
		bool	StartPostFiring( );
		void	UpdatePostFiring( );

		// Changes visibility on gun.
		void	SetVisible( bool bVisible );

	protected:


	private:

		enum FiringState
		{
			eNotFiring,
			ePreFiring,
			eFiring,
			ePostFiring,
		};

		std::string	m_sWeapon;
		CArsenal	m_Arsenal;

		FiringState	m_eFiringState;
		int32		m_nRoundsToFire;

		bool		m_bVisible;
};

class GunMountPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList( 
			const char *szRezPath,
			const char *szPropName,
			char **aszStrings,
			uint32 *pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLen );

		virtual LTRESULT PreHook_Dims(
			const char* szRezPath,
			const char* szPropName, 
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims,
			const char* pszObjName, 
			ILTPreInterface *pInterface);
};

#endif // __GUNMOUNT_H__
