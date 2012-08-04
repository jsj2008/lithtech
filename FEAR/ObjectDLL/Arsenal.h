// ----------------------------------------------------------------------- //
//
// MODULE  : Arsenal.h
//
// PURPOSE : Arsenal aggregate object - Definition
//
// CREATED : 9/25/97
//			 5/01/03 - Renamed to Arsenal from Weapons.
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ARSENAL_H__
#define __ARSENAL_H__

//
// Includes...
//

	#include "iaggregate.h"
	#include "ltbasedefs.h"
	#include "WeaponDB.h"


//
// Defines...
//
	
	#define SELECTION_NONE "<none>"

	#define ADD_WEAPON( Num, Group ) \
			ADD_STRINGPROP_FLAG( Weapon##Num, SELECTION_NONE, Group | PF_STATICLIST, "This is a list of weapons defined in the weapons attribute file.  If a weapon is selected the object will have the weapon in it's arsenal." ) \
			ADD_STRINGPROP_FLAG( Socket##Num, SELECTION_NONE, Group | PF_STATICLIST, "This is an optional name of a socket that the weapon will attach to if the weapon has a model.  If the object or weapon is not a model, if no socket is specified or the socket can't be found, the weapon will not be attached to the object." )

	#define ADD_ARSENAL_AGGREGATE( Group ) \
			PROP_DEFINEGROUP( Arsenal, Group, "A group of properties that define the weapons arsenal for this object." ) \
			ADD_WEAPON( 0, Group ) \
			ADD_WEAPON( 1, Group ) \
			ADD_WEAPON( 2, Group ) \
			ADD_WEAPON( 3, Group ) \

	#define DEFAULT_ACTIVE_WEAPONS	4	// Should be the same as the number of ADD_WEAPON() calls above...


	// Use weapon's default ammo id.  NOTE: This shares the same namespace as
	// the WeaponMgr::m_AmmoList indexes: 0 - WeaponMgr::m_AmmoList->GetLength()).
	// Also, 255 is reserved by the WeaponMgr...

	#define AMMO_DEFAULT_ID			254

//
// Forwards...
//

	class CWeapon;
	class CProjectile;
	class CHHWeaponModel;
	class CPlayerObj;
	

// Class containing data to associate an active weapon with a weapon Id and an attached model if appropriate...

class CActiveWeapon
{
public :	// Methods...

	CActiveWeapon()
	:	m_hWeapon			( NULL ),
		m_hAmmo				( NULL ),
		m_pWeapon			( NULL ),
		m_sSocket			( ),
		m_hHHWeaponModel	( NULL ),
		m_hMeleeWeaponModel	( NULL )
	{ }

	~CActiveWeapon() { Term( ); }

	void		Term();
	void		Detach();

	void		CreateSpawnString( char* szSpawn, uint32 dwBufLen );
	void		CreateAttachString( char* szAttach, uint32 dwBufLen );

	inline const char*	GetSocketName() const	{ return m_sSocket.c_str(); }
	inline HOBJECT		GetModelObject() const	{ return m_hHHWeaponModel; }
	inline HOBJECT		GetDualModelObject() const { return m_hDualWeaponModel;	}
	inline HOBJECT		GetMeleeModelObject() const { return m_hMeleeWeaponModel; }
	inline HOBJECT		GetBlockModelObject() const { return m_hBlockWeaponModel; }
	inline HWEAPON		GetWeaponRecord() const	{ return m_hWeapon; }
	inline HAMMO		GetAmmoRecord() const	{ return m_hAmmo; }
	inline HWEAPON		GetWeaponData(bool bUseAIStats) const	{ return g_pWeaponDB->GetWeaponData(m_hWeapon, bUseAIStats); }
	inline HAMMO		GetAmmoData(bool bUseAIStats) const		{ return g_pWeaponDB->GetAmmoData(m_hAmmo, bUseAIStats); }

	inline CWeapon*		GetWeapon() const		{ return m_pWeapon; }

	void		Save( ILTMessage_Write *pMsg );
	void		Load( ILTMessage_Read *pMsg );

	
private :	// Members...

	friend class CArsenal;
	
	HWEAPON				m_hWeapon;
	HAMMO				m_hAmmo;

	CWeapon				*m_pWeapon;
	std::string			m_sSocket;
	LTObjRefNotifier	m_hHHWeaponModel;

	std::string			m_sDualWeaponSocket;
	LTObjRefNotifier	m_hDualWeaponModel;

	LTObjRef			m_hMeleeWeaponModel;
	LTObjRef			m_hBlockWeaponModel;
};


// Class Definition

class CArsenal : public IAggregate, public ILTObjRefReceiver
{
	struct WeaponAttachmentProperties
	{
		HWEAPON				m_hWeapon;
		HAMMO				m_hAmmo;
		std::string			m_sSocket;
	};

	public :

		CArsenal();
		~CArsenal();

		bool			Init( HOBJECT hObject );

		void			ObtainWeapon( HWEAPON hWeapon, HAMMO hAmmo = NULL, int32 nAmmo = -1, bool bNotifyClient = false, int32 nHealth = -1, bool bUseWeaponChangeAnims = false);
		void			RemoveWeapon( HWEAPON hWeapon );

		void			ObtainMod( HWEAPON hWeapon, HMOD hAmmo, bool bNotifyClient = false, bool bDisplayMsg = true );


		bool			ChangeWeapon( HWEAPON hWeapon, HAMMO hAmmo = NULL, CActiveWeapon *pWeaponToChange = NULL );
		bool			ChangeWeapon( CActiveWeapon *pActiveWeapon );
		void			DeselectCurWeapon();

		int32			AddAmmo( HAMMO hAmmo, int32 nAmount ); //returns amount ofz ammo actually added
		bool			SetAmmo( HAMMO hAmmo, int32 nAmmo = -1 );
		void			DecrementAmmo( HAMMO hAmmo );
		int32			GetAmmoCount( HAMMO hAmmo );
		int32			GetWeaponAmmoCount( HWEAPON hWeapon );
		HWEAPON			GetCurWeaponRecord() const { return m_hCurWeapon; }

		inline uint8	NumActiveWeapons() const { ASSERT( m_vecpActiveWeapons.size() == ( uint8 )m_vecpActiveWeapons.size()); return ( uint8 )m_vecpActiveWeapons.size(); }
		CActiveWeapon*	GetActiveWeapon( uint32 nIndex ) { return (nIndex < m_vecpActiveWeapons.size() ? m_vecpActiveWeapons[nIndex] : NULL); }

		CWeapon*		GetCurWeapon();
		CWeapon*		GetWeapon( HWEAPON hWeapon );

		HOBJECT			GetObject() const { return m_hObject; }

		bool			IsValidWeaponRecord( HWEAPON hWeapon );

		void			Reset();

		void			RemoveAllActiveWeapons();

		bool			HasActiveWeapons() const { return !m_vecpActiveWeapons.empty();	}

		void			RemoveActiveWeapon( const CActiveWeapon *pActiveWeapon );

		// This is used solely by the CWeapon class so it can
		// shoot vector based projectiles without instantiating
		// a new object.
		CProjectile*	GetVecProjectile() { return m_pVecProjectile; }

		
		// Engine calls to the aggregate...

		uint32			EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float lData);

		bool			ReadProp(LPBASECLASS pObject, const GenericPropList *pProps);

		void			Save(ILTMessage_Write *pMsg, uint8 nType);
		void			Load(ILTMessage_Read *pMsg, uint8 nType);

		void			OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );


		int				EnumerateActiveWeapons( CActiveWeapon **apActiveWeapons, const int cMaxWeapons );
		void			GetInfiniteAmmo( );

		CActiveWeapon*	ActivateWeapon( const char *szWeapon, const char *szSocket );

		void			HideWeapons( bool bHide );
		


		//handle players picking up weapons
		bool			AddWeapon( HOBJECT hSender, HWEAPON hWeapon, 
									HAMMO hAmmo, int32 nAmmo, uint32 nHealth, uint8 nInventorySlot);

		//is there any active weapon that can use the specified ammo type?
		bool			CanUseAmmo(HAMMO hAmmo);

		// is the weapon active
		bool			HasWeapon(HWEAPON hWeapon);
		bool			HasMod(HWEAPON hWeapon, HMOD hMod);

		// Handles accessing weapons added through worldedit, as they are 
		// added by an external entity such as the AI.  The AI is currently
		// the only using of this functionality.
		void			ClearWeaponAttachmentProperties() { return m_ReadPropWeaponList.clear(); }
		int				GetWeaponAttachmentPropertiesCount() const { return m_ReadPropWeaponList.size(); }
		bool			GetWeaponAttachmentProperties( uint32 index, HWEAPON& hOutWeapon, HAMMO& hOutAmmo, const char** pszOutSocketName );

private :

		// Private Member functions

		// Removes all the pending objects from the current
		// "projectiles needing messages" list.
		bool			CleanProjectileMessageList();


		// Member Variables

		LTObjRef		m_hObject;			// The owning object of this aggregate
		CPlayerObj*		m_pPlayer;			// if the owner is a player, cache that pointer

		HWEAPON			m_hCurWeapon;

		CWeapon**		m_pWeapons;
		int*			m_pAmmo;

		// This is used solely by the CWeapon class so it can
		// shoot vector based projectiles without instantiating
		// a new object.
		CProjectile*	m_pVecProjectile;	// Projectile class used with vector weapons

		void			DeleteWeapons();
		void			CreateAllWeapons();
		void			CreateWeapon( uint32 nWeaponIndex );
		bool			IsValidWeaponIndex( uint32 nWeaponIndex );

		void			HandlePotentialWeaponChange( CPlayerObj *pPlayer, HWEAPON hWeapon, HAMMO hAmmo, bool bHaveIt );

		bool			IsBetterWeapon( HWEAPON hWeapon );

		CActiveWeapon*	GetActiveWeapon( const char *pSocketName );

		typedef std::vector<CActiveWeapon*, LTAllocator<CActiveWeapon*, LT_MEM_TYPE_OBJECTSHELL> >	ActiveWeaponPtrArray;
		ActiveWeaponPtrArray	m_vecpActiveWeapons;

		typedef std::vector< WeaponAttachmentProperties, LTAllocator<WeaponAttachmentProperties, LT_MEM_TYPE_OBJECTSHELL> > WeaponAttachmentPropertiesListType;
		WeaponAttachmentPropertiesListType m_ReadPropWeaponList;

		bool			ActivateWeapon( CActiveWeapon *pActiveWeapon );
		void			ActivateWeapons( );
};

inline CWeapon* CArsenal::GetCurWeapon()
{
	CWeapon* pRet = NULL;
	if( IsValidWeaponRecord( m_hCurWeapon ))
	{
		uint32 nWeaponIndex = g_pWeaponDB->GetRecordIndex( m_hCurWeapon );
		pRet = m_pWeapons[nWeaponIndex];
	}
	return pRet;
}

inline bool CArsenal::IsValidWeaponIndex( uint32 nWeaponIndex )
{
	return (nWeaponIndex < g_pWeaponDB->GetNumWeapons());
}

inline CWeapon* CArsenal::GetWeapon( HWEAPON hWeapon )
{
	CWeapon* pRet = NULL;

	if( IsValidWeaponRecord( hWeapon ))
	{
		uint32 nWeaponIndex = g_pWeaponDB->GetRecordIndex( hWeapon );
		pRet = m_pWeapons[nWeaponIndex];
	}

	return pRet;
}

extern CBankedList<CWeapon> s_bankCWeapon;



class CArsenalPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList(const char* szRezPath,
												const char* szPropName,
												char** aszStrings,
												uint32* pcStrings,
												const uint32 cMaxStrings,
												const uint32 cMaxStringLength);

	private :

		static bool					sm_bInitted;
};

#endif //__ARSENAL_H__
