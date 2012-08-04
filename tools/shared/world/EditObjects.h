//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditObjects.h
//
//	PURPOSE	  : Defines all the edit object stuff.
//
//	CREATED	  : November 28 1996
//
//
//------------------------------------------------------------------

#ifndef __EDITOBJECTS_H__
#define __EDITOBJECTS_H__

// Includes....
#include "editvert.h"
#include "proplist.h"
#include "worldnode.h"
#include "classbind.h"

#ifdef DIRECTEDITOR_BUILD
#	include "modelhandle.h"
#	include "iltpreinterface.h"
#endif



class CBaseEditObj;
class CEditRegion;
class TemplateClass;
class CLTANode;
class CLTANodeBuilder;
class CLTAFile;
class CModelMgr;

#define MAX_OBJECTNAME_LEN		30
extern char *g_NameName, *g_PosName, *g_AnglesName;
extern CVector g_DummyPos;
extern LTVector g_DummyRotation;
extern CStringProp g_DummyString;

class CBaseEditObj;
typedef CMoArray<CBaseEditObj*> BaseEditObjArray;



void GetClassDefProps(ClassDef *pClass, CMoArray<PropDef*> &propList);

// If you set pNameOverride, it won't search through all the current objects
// to find a unique name.
BOOL SetupWorldNodeFromClass(
	ClassDef		*pClass, 
	TemplateClass	*pTemplate, 
	CWorldNode		*pNode, 
	CEditRegion		*pRegion,
	char			*pNameOverride=NULL);

CBaseProp*		CreatePropFromCode( int code, char *pName );

void	SetupNewProp(CBaseProp *pProp, PropDef *pVar);
BOOL	SetupWorldNode(const char *pClassName, CWorldNode *pNode, CEditRegion *pRegion);

BOOL	LoadObjectList(CAbstractIO &file, CMoArray<CBaseEditObj*> &theList);
BOOL	LoadNodeProperties(CAbstractIO &file, CWorldNode *pNode, CEditProjectMgr* pProject = NULL);
BOOL	LoadNodePropertiesLTA(CLTANode* pParseNode, CWorldNode *pNode, CPropListContainer& container, CEditProjectMgr* pProject = NULL );

void	SaveObjectList(CAbstractIO &file, CMoArray<CBaseEditObj*> &theList);
uint32	GetObjectListSize(CMoArray<CBaseEditObj*> &theList);
void	SaveObjectListLTA(CLTANodeBuilder &lb, CMoArray<CBaseEditObj*> &theList);
void	SaveNodeProperties(CAbstractIO &file, CWorldNode *pNode);
uint32	GetNodePropertiesSize(CWorldNode* pNode);
void	SaveNodePropertiesLTA(CLTAFile* pFile, CWorldNode *pNode, uint32 level);

void	CopyObjectList( CMoArray<CBaseEditObj*> &in, CMoArray<CBaseEditObj*> &out );
// Duplicates the node heirarchy between two lists of objects.  
// Note that the objects must be in exactly the same order in the list.
// Returns false on failure
bool	DuplicateObjectHeirarchy( CMoArray<CBaseEditObj*> &in, CMoArray<CBaseEditObj*> &out );


// The base class for all edit objects.
class CBaseEditObj : public CWorldNode 
#ifdef DIRECTEDITOR_BUILD
	, public ILTPreInterface	// [10/04/01 ARP] Only derive from this when building DEdit!
#endif
{

// Member Operations...

public:

							CBaseEditObj();
							CBaseEditObj( const CBaseEditObj& src );
	virtual					~CBaseEditObj();
	
	virtual CWorldNode*		AllocateSameKind()	{return new CBaseEditObj;}
	virtual void			DoCopy(CWorldNode *pOther);
					
	// Overloaded operators...
	CBaseEditObj&			operator=( const CBaseEditObj& src );

	virtual void			SetPos(const LTVector &v);		// Sets the position
	virtual void			SetOr(const LTVector &v);		// Sets the rotation

	CVector					GetUpperLeftCornerPos()		{ return GetPos(); }

// Member data...

public:

	//the region that this object belongs to
	CEditRegion*	m_pRegion;

	// Misc. data for the editor stuff to use.
	CEditVert		m_Vert;

#ifdef DIRECTEDITOR_BUILD

	// Notification that a property of this object has changed
	virtual void			OnPropertyChanged(CBaseProp* pProperty, bool bNotifyGame, const char *pModifiers);

	// Initializes/Terminates the dims
	void					InitDims();
	void					TermDims()								{ m_dimsArray.SetSize(0); }

	//this function must be called whenever the class name is updated
	virtual void			UpdateClassName()						{ UpdateObjectClassImage(); }


	// Access to the dims
	int						GetNumDims()							{ return m_dimsArray.GetSize(); }
	CVector					*GetDim(int nIndex)						{ return &m_dimsArray[nIndex]; }		
	DWORD					GetDimsFlags(int nIndex)				{ return m_dimsFlagArray[nIndex]; }

	BOOL					ShouldSearchForDims()					{ return m_bSearchForDims; }	
	void					SetSearchForDims(BOOL bSearch)			{ m_bSearchForDims=bSearch; }

	CModelHandle&			GetModelHandle()						{ return m_ModelHandle; }
	void					SetModelHandle(CModelHandle& Handle)	{ m_ModelHandle = Handle; }

	// releases the current model handle
	void					ReleaseModelHandle();	

	//updates the internal model handle
	bool					UpdateModelHandle(const char* pszFileString, CModelMgr& ModelMgr);

	//gets the radius of the object 
	float					GetVisibleRadius() const				{ return m_fVisRadius; }

	//sets the radius of the object (used for culling from the frustum)
	void					SetVisibleRadius(float fRad)			{ m_fVisRadius = fRad; }

	//updates the image used for this object
	void					UpdateObjectClassImage();

	struct DFileIdent_t*	m_pClassImageFile;

protected:

	BOOL					m_bSearchForDims;		// Indicates if the dims should be searched for or not
	CMoArray<CVector>		m_dimsArray;			// Array of the dims
	CMoArray<DWORD>			m_dimsFlagArray;		// Array of dims flags	

	float					m_fVisRadius;			// The visible radius of this object (for culling)

	CModelHandle			m_ModelHandle;			// the handle to the loaded model
	CModelMgr*				m_pModelMgr;			// the pointer to the model manager that the handle was obtained from

////////////////////////////////////
//	ILTPreInterface implementation
protected:
	
	virtual LTRESULT Parse(ConParse *pParse);
	virtual LTRESULT CPrint(const char *pMsg, ...);
	virtual LTRESULT ShowDebugWindow(bool bShow);
	virtual LTRESULT FindObject(const char *pName);
	virtual const char* GetObjectClass( const char *pObjName );
	virtual const char* GetProjectDir( );
	virtual const char* GetWorldName( );
	virtual LTRESULT GetPropGeneric( const char* pObjName, const char* pPropName, GenericProp *pProp );

	virtual LTRESULT GetObject( const char* pObjName, CBaseEditObj **ppObj );

#endif // DIRECTEDITOR_BUILD
};



#endif  // __EDITOBJECTS_H__







