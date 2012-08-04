//----------------------------------------------------------------------------
//              
//	MODULE:		RelationMgr.h
//              
//	PURPOSE:	CRelationMgr declaration
//              
//	CREATED:	30.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __RELATIONMGR_H__
#define __RELATIONMGR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include <set>

// Forward declarations
class CRelationMgr;
class CRelationButeMgr;
class CObjectRelationMgr;
class CCollectiveRelationMgr;

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:	CRelationMgr
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
class CRelationMgr : public ILTObjRefReceiver
{
	public:

		// Public members
		typedef std::vector<CCollectiveRelationMgr*> _listCollectives;
		typedef std::vector<CObjectRelationMgr*> _listpObjectRelationMgr;
		typedef std::map<std::string, CObjectRelationMgr*> _mapStringToORM;

		struct ObjectToORMMapEntry
		{
			CObjectRelationMgr* m_pORM;
			LTObjRefNotifier	m_hObjRef;
		};

		typedef std::map< HOBJECT, ObjectToORMMapEntry > _mapObjectToORM;

		// Ctors/Dtors/etc
		CRelationMgr();
		~CRelationMgr();

		LTBOOL	Init();
		void	Term();

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		void PerWorldInit();
		void PerWorldTerm();

		// Methods:
		static CRelationMgr* GetGlobalRelationMgr();
		CRelationButeMgr* GetButeMgr();

		CCollectiveRelationMgr* FindCollective( const char* const szName );
		CObjectRelationMgr* GetObjectRelationMgr( const char* const szName );
		CObjectRelationMgr* GetObjectRelationMgr( HOBJECT HInstance );

		void AddObjectRelationMgr(CObjectRelationMgr* pObjectRelationMgr, HOBJECT hObject );
		void RemoveObjectRelationMgr(CObjectRelationMgr* pObjectRelationMgr );
		void RemoveObjectRelationMgr( _listpObjectRelationMgr::iterator );

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

	protected:
		// Protected members
		void LinkObjectRelationMgr(CObjectRelationMgr* pORM, HOBJECT hObject);
		void UnlinkObjectRelationMgr(const CObjectRelationMgr* const pORM);
		bool IsObjectRelationMgrLinked(const CObjectRelationMgr* const pORM) const;

	private:
		// Private members

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		CRelationMgr(const CRelationMgr& rhs) {}
		CRelationMgr& operator=(const CRelationMgr& rhs ) {}

		static CRelationMgr* m_pSingleInstance;

		void AddCollective(CCollectiveRelationMgr* pCollective);
		void RemoveCollective(CCollectiveRelationMgr* pCollective);

		void FreeAllResources();
		void DeleteAndRemove(_listCollectives::iterator);

		// Pointer to the single instance of the RelationButeMgr
		CRelationButeMgr*			m_pRelationButeMgr;
	
		// Searching for a ObjectRelationMgr by object Name
		_mapStringToORM				m_mapStringToORM;

		// Searching for an ObjectRelationMgr instance by handle
		_mapObjectToORM				m_mapObjectToORM;
		
		// List of all ObjectRelationMgrs in existance
		_listpObjectRelationMgr		m_listpObjectRelationMgrs;
		
		// List of all collectives in existance
		_listCollectives			m_listCollectives;
};

#endif // __RELATIONMGR_H__
