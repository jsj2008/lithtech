//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : PropList.h
//
//	PURPOSE	  : Defines CPropList and related classes.
//
//	CREATED	  : December 1 1996
//
//
//------------------------------------------------------------------

#ifndef __PROPLIST_H__
	#define __PROPLIST_H__


	class CEditProjectMgr;
	class CLTANode;
	class CLTANodeBuilder;
	class CLTAFile;


	// Property types.
	#include "ltserverobj.h"
	#include "ltproperty.h"


	#define MAX_PROPNAME_LEN		30
	#define MAX_STRINGPROP_LEN		MAX_GP_STRING_LEN
	#define MAX_EDITPROP_LEN		80



	typedef void (*StringListFillFn)( void *pData, CMoArray<char*> &fillIn );




	// One class for each property type...
	class CBaseProp
	{
		public:
	
								CBaseProp( const char *pName );

			virtual void		LoadData( CAbstractIO &file )	{}
			virtual void		LoadDataLTA( CLTANode* pNode )	{}
			virtual void		SaveData( CAbstractIO &file )	{}
			virtual uint32		DataSize( ) const { return 0; }
			virtual void		SaveDataLTA( CLTAFile* pFile, uint32 level )	{}
			virtual void		SaveDataLTA( CLTANodeBuilder &lb )	{}
			virtual void		Copy( CBaseProp *pOther );
			virtual CBaseProp*	CreateSameKind()				{ return new CBaseProp(NULL); }
			virtual void		SetupString(char *pStr) {}
			virtual void		SetupGenericProp(GenericProp *pProp) {}

			// Access to member variables
			int					GetType()	{ return m_Type; }
			long				GetFlags()	{ return m_PropFlags; }
			char				*GetName()	{ return m_Name; }

			// This is now virtual (someone forgot!) - David C.
			virtual bool				SameAs(CBaseProp* pProp) 
			{
				if( m_Type == pProp->m_Type )
					if( m_PropFlags == pProp->m_PropFlags )
						if( _stricmp(m_Name, pProp->m_Name) == 0 )
							return true;
				return false;
			}
				

			// Member data
		public:

			// This is the property type from serverobj_de.h.
			int				m_Type;
			unsigned long	m_PropFlags;
			char			m_Name[MAX_PROPNAME_LEN];
			PropDef*		m_pPropDef;
	};

	class CStringProp : public CBaseProp
	{
		public:
						
						CStringProp(  const char *pName  );
			
			void		LoadData( CAbstractIO &file );
			void		LoadDataLTA( CLTANode* pNode );
			void		SaveData( CAbstractIO &file );
			uint32		DataSize( ) const;
			void		SaveDataLTA( CLTAFile* pFile, uint32 level );
			void		SaveDataLTA( CLTANodeBuilder &lb );
			void		Copy( CBaseProp *pOther );
			virtual		CBaseProp*	CreateSameKind()				{ return new CStringProp(NULL); }
			void		SetupString(char *pStr);
			virtual void		SetupGenericProp(GenericProp *pProp);

			void		SetString(const char *lpszString);
			char		*GetString()								{ return m_String; }

			char		m_String[MAX_STRINGPROP_LEN+1];

			bool	SameAs(CBaseProp* pProp) 
			{
				if( CBaseProp::SameAs(pProp) )
				{
					if( _stricmp(m_String, ((CStringProp*)pProp)->m_String) == 0 )
					{
						return true;
					}
				}
				return false;
			}

	};

	class CVectorProp : public CBaseProp
	{
		public:

						CVectorProp(  const char *pName  );
			
			void		LoadData( CAbstractIO &file );
			void		LoadDataLTA( CLTANode* pNode );
			void		SaveData( CAbstractIO &file );
			uint32		DataSize( ) const;
			void		SaveDataLTA( CLTAFile* pFile, uint32 level );
			void		SaveDataLTA( CLTANodeBuilder &lb );
			void		Copy( CBaseProp *pOther );
			virtual		CBaseProp*	CreateSameKind()				{ return new CVectorProp(NULL); }
			void		SetupString(char *pStr);
			virtual void		SetupGenericProp(GenericProp *pProp);

			void		SetVector(CVector v)					{ m_Vector=v; }
			CVector		m_Vector;

			bool	SameAs(CBaseProp* pProp) 
			{
				if( CBaseProp::SameAs(pProp) )
				{
					if( m_Vector == ((CVectorProp*)pProp)->m_Vector )
					{
						return true;
					}
				}
				return false;
			}

	};

	class CRotationProp : public CBaseProp
	{
	public:

					CRotationProp(  const char *pName  );

		void		LoadData( CAbstractIO &file );
		void		LoadDataLTA( CLTANode* pNode );
		void		SaveData( CAbstractIO &file );
		uint32		DataSize( ) const;
		void		SaveDataLTA( CLTAFile* pFile, uint32 level );
		void		SaveDataLTA( CLTANodeBuilder &lb );
		void		Copy( CBaseProp *pOther );
		virtual CBaseProp*	CreateSameKind() { return new CRotationProp(NULL); }
		void		SetupString(char *pStr);
		virtual void		SetupGenericProp(GenericProp *pProp);

		void		InitEulerAngles()				{m_EulerAngles.Init();}
		LTVector&	GetEulerAngles()				{return m_EulerAngles;}
		void		SetEulerAngles(LTVector vec)	{m_EulerAngles = vec;}

		bool	SameAs(CBaseProp* pProp) 
		{
			if( CBaseProp::SameAs(pProp) )
			{
				if( GetEulerAngles() == ((CRotationProp*)pProp)->GetEulerAngles() )
				{
					return true;
				}
			}
			return false;
		}


	// They used to be stored as DRotations..
	private:
		DVector		m_EulerAngles;
	};

	class CColorProp : public CVectorProp
	{
		public:

			CColorProp(  const char *pName  );

			void				SetColor(CVector vColor)		{ SetVector(vColor); }
			virtual CBaseProp*	CreateSameKind()				{ return new CColorProp(NULL); }

			bool	SameAs(CBaseProp* pProp) 
			{
				if( CVectorProp::SameAs(pProp) )
				{
					return true;
				}
				return false;
			}
	};

	class CRealProp : public CBaseProp
	{
		public:

						CRealProp( const char *pName );

			void		LoadData( CAbstractIO &file );
			void		LoadDataLTA( CLTANode* pNode );
			void		SaveData( CAbstractIO &file );
			uint32		DataSize( ) const;
			void		SaveDataLTA( CLTAFile* pFile, uint32 level );
			void		SaveDataLTA( CLTANodeBuilder &lb );
			void		Copy( CBaseProp *pOther );
			void		SetupString(char *pStr);
			virtual void		SetupGenericProp(GenericProp *pProp);

			virtual		CBaseProp*	CreateSameKind()				{ return new CRealProp(NULL); }

			void		SetValue(float fValue)					{ m_Value=fValue; }
			float		m_Value;

			bool	SameAs(CBaseProp* pProp) 
			{
				if( CBaseProp::SameAs(pProp) )
				{
					if( m_Value == ((CRealProp*)pProp)->m_Value )
						return true;
				}
				return false;
			}

	};

	class CBoolProp : public CBaseProp
	{
		public:

						CBoolProp( const char *pName );

			void		LoadData( CAbstractIO &file );
			void		LoadDataLTA( CLTANode* pNode );
			void		SaveData( CAbstractIO &file );
			uint32		DataSize( ) const;
			void		SaveDataLTA( CLTAFile* pFile, uint32 level );
			void		SaveDataLTA( CLTANodeBuilder &lb );
			void		Copy( CBaseProp *pOther );
			void		SetupString(char *pStr);
			virtual void		SetupGenericProp(GenericProp *pProp);

			void		SetValue(bool bValue)					{ m_Value=bValue; }
			virtual		CBaseProp*	CreateSameKind()				{ return new CBoolProp(NULL); }

			char		m_Value;

			bool		SameAs(CBaseProp* pProp) 
			{
				if( CBaseProp::SameAs(pProp) )
				{
					if( m_Value == ((CBoolProp*)pProp)->m_Value )
						return true;
				}
				return false;
			}

	};


	// The property list.  Every object has one of these.
	class CPropList
	{
		public:

									CPropList();
									~CPropList();
			
			void					Term();

			CBaseProp*				GetProp(const char *pName, BOOL bAssert=FALSE, uint32 *pIndex=NULL);
			CBaseProp*				GetPropPartial(const char *pName, BOOL bAssert=FALSE, uint32 *pIndex=NULL);
			CBaseProp*				GetMatchingProp(const char *pName, int type);
			// Note : This returns the first prop that matches ALL of the flags.  (Not just any of them)
			CBaseProp*				GetPropByFlagsAndType(uint32 flags, int type, uint32 *index=NULL);
			
			// Copies property list values from one list to another.  This removes non-matching values.
			void					CopyValues(CPropList *pSource);

			// Copies the matching values (same name and type) from one list to another.
			void					CopyMatchingValues(CPropList *pSourceList);

			// Returns the number of properties
			int						GetSize()			{ return m_Props.GetSize(); }

			// Returns a property at a specific index
			CBaseProp				*GetAt(int nIndex)	{ return m_Props[nIndex]; }

			bool					SameAs(CPropList* pPropList);

			//determines if the property lists are the same, but ignores properties listed in the filter
			bool					SameAs(CPropList* pPropList, uint32 nNumFilters, const char** pszFilter);

		public:

			CMoArray<CBaseProp*>	m_Props;

	};


class CPropListContainer
{
public:
			CPropListContainer();
	void	Init();
	uint32	AddPropList(CPropList* propList, uint32 nNumFilters, const char** pszFilters);
	uint32	AddPropList(CPropList* propList, bool alwaysAdd=false);

	void	SaveLTA( CLTAFile* pFile, uint32 level );
	void	SaveTBW( CAbstractIO& OutFile );

	CPropList* GetElem(uint32 index) {return m_propList[index];} 
	void	UnloadFromMemory();
private:
	CMoArray<CPropList*> m_propList;
};


#endif  // __PROPLIST_H__





