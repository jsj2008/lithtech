//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditProjectMgr.h
//
//	PURPOSE	  : Defines the CEditProjectMgr class, derived from
//              CProjectMgr .. it adds on tracking of different
//              file types, classified into worlds and sprites and stuff.
//
//	CREATED	  : December 16 1996
//
//
//------------------------------------------------------------------

#ifndef __EDITPROJECTMGR_H__
	#define __EDITPROJECTMGR_H__


	// Includes....
	#include "dibmgr.h"
	#include "proplist.h"
	#include "classbind.h"
	#include "d_filemgr.h"
	// #include "objectimporter.h"
	
	#define CLASSFILENAME_LEN			40
	#define MAX_TEMPLATECLASSNAME_LEN	32

	class CClass;



	class CProjectClass
	{
		public:

										~CProjectClass();
			
			ClassDef					*m_pClass;
			CMoArray<CProjectClass*>	m_Children;
			
			// The props from BuildClassDefPropList.
			CMoArray<PropDef*>			m_Props;

	};

	class TemplateClass
	{
	public:
		char m_ClassName[MAX_TEMPLATECLASSNAME_LEN];
		char m_ParentClassName[MAX_TEMPLATECLASSNAME_LEN];		
	};

	class CObjectImporter;
	class CEditProjectMgr
	{
		public:

										CEditProjectMgr();
										~CEditProjectMgr();


			// Call when a new project is being opened.
			void						OnOpenProject(const char *pDir);
			void						Term();
			
			
			// Updates the class and class file arrays.
			void						UpdateClasses();


			// Load and save any other info...
			void						LoadProjectFile( CAbstractIO &file );
			void						SaveProjectFile( CAbstractIO &file );

			// Find a class definition..
			ClassDef*					FindClassDef(const char *pName);
			
			PropDef*					FindPropInList(CMoArray<PropDef*> &propList, char *pPropName, DWORD *pIndex=NULL);

			// These functions are ALWAYS used when setting up an object's properties.
			// These functions take property overriding and flags into account to build
			// the final property list.
			void						RecurseAndBuildClassDefPropList(
				CProjectClass *pProjectClass);

			void						BuildClassDefPropList(
				ClassDef *pClass, 
				CMoArray<PropDef*> &propList);

			// Fast access to the results from BuildClassDefPropList. Use this whenever
			// possible.
			CMoArray<PropDef*>*			GetClassDefProps(
				ClassDef *pClass);

			CProjectClass*				FindClassInTree( const char *pName );
			CProjectClass*				RecurseAndFindClass( CMoArray<CProjectClass*> &classes, const char *pName );
			TemplateClass*				FindTemplateClass(const char *pName);

			// Fills sDestBaseClass in with the base class of sClassName.  FALSE is returned if
			// sClassName could not be found.
			BOOL						GetBaseClassName(CString sClassName, CString &sDestBaseClass);

		// Accessors.
		public:

			CDibMgr*					GetDibMgr()					{ return &m_DibMgr; }

			CMoArray<CProjectClass*>&	GetClasses()				{ return m_Classes; }
			CMoArray<TemplateClass*>&	GetTemplateClasses()		{ return m_TemplateClasses; }
		
			CObjectImporter*			GetTemplateObjectImporter()	{ return m_pTemplateObjectImporter; }

			CString						GetBaseProjectDir()			{ return m_BaseProjectDir; }

		private:

			void						LoadBinaries();
			void						LoadTemplateClasses();
			void						UnloadBinaries();
			void						UpdateBUTFile();

			void						MakeClassTree();
			CProjectClass*				FindClassInArray(ClassDef *pClass, CMoArray<CProjectClass*> *pArray);


		public:

			CDibMgr						m_DibMgr;
			DFILEMGR					m_hFileMgr;


			// Class stuff.
			CMoArray<CProjectClass*>	m_Classes;
			CMoArray<TemplateClass*>	m_TemplateClasses;

			// This is the ClassDef list, copied from the object DLL (the object DLL
			// is closed after copying the classdef list over).
			ClassDef					*m_ClassDefs;
			DWORD						m_nClassDefs;

			CString						m_BaseProjectDir;
			BOOL						m_bProjectDirectorySet;

			HCLASSMODULE				m_hModule;

		protected:
			// Used to import the template classes
			CObjectImporter				*m_pTemplateObjectImporter;
	};


#endif  // __EDITPROJECTMGR_H__
