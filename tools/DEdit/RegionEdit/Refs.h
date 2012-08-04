//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : Refs.h
//
//	PURPOSE	  : Defines all the C<X>Ref classes.
//
//	CREATED	  : November 12 1996
//
//
//------------------------------------------------------------------

#ifndef __REFS_H__
#define __REFS_H__


	// Includes....
	#include "editvert.h"


	// Defines....
	class CEditBrush;
	class CEditPoly;
	class CBaseEditObj;

	
	// CVertRef
	class CVertRef
	{
		public:
	
							CVertRef()										{ Term(); }
							CVertRef( CEditBrush *pBrush, uint32 iVert )	{ Init(pBrush, iVert); }
							CVertRef( const CVertRef &other ) : m_pBrush(other.m_pBrush), m_iVert(other.m_iVert) {} 
			
			void			Init( CEditBrush *pBrush, uint32 iVert )		{ m_pBrush=pBrush; m_iVert=iVert; }
			void			Term()											{ m_pBrush=NULL;  m_iVert=NULL; }
			BOOL			IsValid() const									{ return m_pBrush != NULL; }

			CEditVert&		operator () ();
			CVertRef&		operator = ( const CVertRef &other )			{ m_pBrush=other.m_pBrush; m_iVert=other.m_iVert; return *this; }
			bool			operator == ( const CVertRef &other ) const		{ return (m_pBrush==other.m_pBrush) && (m_iVert==other.m_iVert); }
			bool			operator != ( const CVertRef &other ) const		{ return (m_pBrush!=other.m_pBrush) || (m_iVert!=other.m_iVert); }


		public:

			CEditBrush		*m_pBrush;
			uint32			m_iVert;

	};
	


	// CEdgeRef
	class CEdgeRef
	{
		public:

							CEdgeRef()						{ Term(); }
							CEdgeRef( CEditBrush *pBrush, uint32 v1, uint32 v2 );
							CEdgeRef( const CEdgeRef &other) : m_pBrush(other.m_pBrush), m_Vert1(other.m_Vert1), m_Vert2(other.m_Vert2) {}
			
			BOOL			Init( CEditBrush *pBrush, uint32 v1, uint32 v2 );
			void			Term()							{ m_pBrush = NULL; }
			BOOL			IsValid() const					{ return !!m_pBrush; }
			
			CEditVert&		Vert1();
			CEditVert&		Vert2();
			
			CEdgeRef&		operator = ( const CEdgeRef &other );
			bool			operator == ( const CEdgeRef &other ) const;
			bool			operator != ( const CEdgeRef &other ) const;
			
			void			Sort();


		public:
					
			CEditBrush	*m_pBrush;
			uint32		m_Vert1, m_Vert2;

	};



	// CPolyRef
	class CPolyRef
	{
		public:

							CPolyRef()							{ Term(); }
							CPolyRef( CEditBrush *pBrush, uint32 iPoly );
							CPolyRef( const CPolyRef &other) : m_pBrush(other.m_pBrush), m_iPoly(other.m_iPoly) {}

			BOOL			Init( CEditBrush *pBrush, uint32 iPoly );
			void			Term()								{ m_pBrush = NULL; }
			BOOL			IsValid() const						{ return !!m_pBrush; }

			CVertRef		Vert( uint32 i );

			CPolyRef&		operator = ( const CPolyRef &other );
			bool			operator == ( const CPolyRef &other ) const;
			bool			operator != ( const CPolyRef &other ) const;
			CEditPoly*		operator () ();

		
		public:

			CEditBrush		*m_pBrush;
			uint32			m_iPoly;

	};



	// BRUSHREF

	class CBrushRef
	{
		public:

							CBrushRef()					{ m_pBrush = NULL; }
							CBrushRef(const CBrushRef &other) : m_pBrush(other.m_pBrush) {}
							
			BOOL			Init( CEditBrush *pBrush )	{ m_pBrush = pBrush; return TRUE; }
			void			Term()						{ m_pBrush = NULL; }
			BOOL			IsValid() const				{ return m_pBrush != NULL; }

			CBrushRef&		operator = ( const CBrushRef &other );
			bool			operator == ( const CBrushRef &other ) const;
			bool			operator != ( const CBrushRef &other ) const;
			CEditBrush*		&operator () ();
	
			
		public:
			
			CEditBrush		*m_pBrush;

	};



	// OBJECTREF

	class CObjectRef
	{
		public:

							CObjectRef()						{ m_pObject = NULL; }
							CObjectRef(const CObjectRef &other) : m_pObject(other.m_pObject) {}

			BOOL			Init( CBaseEditObj *pObj )			{ m_pObject = pObj; return TRUE; }
			void			Term()								{ m_pObject = NULL; }
			BOOL			IsValid() const						{ return m_pObject != NULL; }

			CObjectRef&		operator = ( const CObjectRef &other )		{ m_pObject=other.m_pObject; return *this; }
			bool			operator == ( const CObjectRef &other )	const { return (m_pObject == other.m_pObject); }
			bool			operator != ( const CObjectRef &other )	const { return m_pObject != other.m_pObject; }
			CBaseEditObj*	&operator () ()						{ return m_pObject; }


		public:
			
			CBaseEditObj	*m_pObject;

	};






	// TYPEDEFS

	typedef CMoArray<CVertRef>		CVertRefArray;
	typedef CMoArray<CEdgeRef>		CEdgeRefArray;
	typedef CMoArray<CPolyRef>		CPolyRefArray;
	typedef CMoArray<CBrushRef>		CBrushRefArray;
	typedef CMoArray<CObjectRef>	CObjectRefArray;



	/// EDGEREF INLINES

	inline CEdgeRef::CEdgeRef( CEditBrush *pBrush, uint32 v1, uint32 v2 )
	{
		Init( pBrush, v1, v2 );
	}


	inline BOOL CEdgeRef::Init( CEditBrush *pBrush, uint32 v1, uint32 v2 )
	{
		m_pBrush = pBrush;
		m_Vert1 = v1;
		m_Vert2 = v2;

		Sort();
		return TRUE;
	}


	inline void CEdgeRef::Sort()
	{
		if( m_Vert1 > m_Vert2 )
		{
			uint32 temp = m_Vert2;
			m_Vert2 = m_Vert1;
			m_Vert1 = temp;
		}
	}


	inline CEdgeRef& CEdgeRef::operator = ( const CEdgeRef &other )
	{
		Init( other.m_pBrush, other.m_Vert1, other.m_Vert2 );
		return *this;
	}


	inline bool CEdgeRef::operator == ( const CEdgeRef &other ) const
	{
		return (other.m_pBrush==m_pBrush) && (other.m_Vert1==m_Vert1) && (other.m_Vert2==m_Vert2);
	}


	inline bool CEdgeRef::operator != ( const CEdgeRef &other ) const
	{
		return (other.m_pBrush!=m_pBrush) || (other.m_Vert1!=m_Vert1) || (other.m_Vert2!=m_Vert2);
	}


	/// POLYREF INLINES

	inline CPolyRef::CPolyRef( CEditBrush *pBrush, uint32 iPoly )
	{
		Init( pBrush, iPoly );
	}


	inline BOOL CPolyRef::Init( CEditBrush *pBrush, uint32 iPoly )
	{		
		m_pBrush = pBrush;
		m_iPoly = iPoly;

		return TRUE;
	}


	inline CPolyRef& CPolyRef::operator = ( const CPolyRef &other )
	{
		Init( other.m_pBrush, other.m_iPoly );
		return *this;
	}


	inline bool CPolyRef::operator == ( const CPolyRef &other ) const
	{
		return (m_pBrush == other.m_pBrush) && (m_iPoly == other.m_iPoly);
	}


	inline bool CPolyRef::operator != ( const CPolyRef &other ) const
	{
		return (m_pBrush != other.m_pBrush) || (m_iPoly != other.m_iPoly);
	}


	// BRUSHREF INLINES

	inline CBrushRef& CBrushRef::operator = ( const CBrushRef &other )
	{
		m_pBrush = other.m_pBrush;
		return *this;
	}


	inline bool CBrushRef::operator == ( const CBrushRef &other ) const
	{
		return (m_pBrush == other.m_pBrush);
	}


	inline bool CBrushRef::operator != ( const CBrushRef &other ) const
	{
		return (m_pBrush != other.m_pBrush);
	}


	inline CEditBrush* &CBrushRef::operator () ()
	{
		return m_pBrush;
	}
	

#endif



