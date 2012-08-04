//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditRegion.h
//
//	PURPOSE	  : Defines the CEditRegion class.
//
//	CREATED	  : October 5 1996
//
//
//------------------------------------------------------------------

#ifndef __EDITREGION_H__
#define __EDITREGION_H__


// Includes....
#include "editvert.h"
#include "editbrush.h"
#include "editobjects.h"
#include "worldnode.h"
#include "navigator.h"
#include "prefabmgr.h"

#ifdef DIRECTEDITOR_BUILD
#	include "editprojectmgr.h"
#	include "undo_mgr.h"
#endif


// Defines....
class CEditPoly;
class CEditBrush;
class CLTANode;
class CLTAFile;

typedef enum
{
	REGIONLOAD_INVALIDFILE,
	REGIONLOAD_OUTOFMEMORY,
	REGIONLOAD_INVALIDFILEVERSION,
	REGIONLOAD_OK
} RegionLoadStatus;


#define REGION_LTA_VERSION		2


class CEditRegion
{
public:

							CEditRegion();
							~CEditRegion();
	
	void					Term();


// Loading/saving.
public:

	RegionLoadStatus		LoadFile( const char* filename, CEditProjectMgr* pProject, uint32& nVersion, bool& bBinary );

	RegionLoadStatus		LoadLTA( const char* filename, CEditProjectMgr* pProject, uint32& nVersion );
	void					SaveLTA( CLTAFile* pFile );

	RegionLoadStatus		LoadTBW( const char* filename, CEditProjectMgr* pProject, uint32& nVersion );
	void					SaveTBW( CAbstractIO& OutFile );

	void					SaveHeaderLTA(CLTAFile* pFile, uint32 level, DWORD versionCode );
	BOOL					RecurseAndLoadNodeLTA( CLTANode* pParseNode, CWorldNode *pNode, CEditProjectMgr* pProject );
	void					RecurseAndSaveNodeLTA( CLTAFile* pFile, CWorldNode *pNode, uint32 level );

	bool					RecurseAndLoadNodeTBW( CAbstractIO& InFile, CWorldNode *pNode, CEditProjectMgr* pProject );
	void					RecurseAndSaveNodeTBW( CAbstractIO& OutFile, CWorldNode *pNode );

	//called after a level is loaded and set up to update any version incompatibilities.
	bool					PostLoadUpdateVersion(uint32 nVersion, bool& bModified);

// Geometry helpers.
public:

	// Returns NULL if nodes are OK.  Returns a node pointer for the
	// first one with an error.
	CWorldNode*				CheckNodes(CWorldNode *pRoot=NULL);

	uint32					GetTotalNumPolies();
	uint32					GetTotalNumPoints();
	uint32					NumBrushSelections();

	
	void					CleanupGeometry(CLinkedList<CEditBrush*> *pBrushes=NULL);
	void					FixInvalidBrushes(CLinkedList<CEditBrush*> *pBrushes=NULL);
	void					RemoveCollinearVertices(CLinkedList<CEditBrush*> *pBrushes=NULL);
	void					RemoveDuplicatePoints(CLinkedList<CEditBrush*> *pBrushes=NULL);
	void					UpdatePlanes(CLinkedList<CEditBrush*> *pBrushes=NULL);
	void					FixInaccurateGeometry(CLinkedList<CEditBrush*> *pBrushes=NULL, float fThreshold = 0.01);
				
	
	void					CopyRegion( CEditRegion *pOther );
	
	void					RemoveUnusedBrushes();

	#ifdef DIRECTEDITOR_BUILD
		void					UpdateTextureIDs( CEditProjectMgr *pProj );

		// Renames all occurences of pSrcTextureName to pDestTextureName (case-insensitive).
		// Returns the number of renamed textures.
		uint32					RenameTexture(const char *pSrcTextureName, const char *pDestTextureName, bool bScaleOPQs=true);

		void					UpdateAllObjectProperties( const char *pModifiers );
		
	#endif

	void					UpdateTextureStrings();
	
	void					GetBoxIntersectBrushes( CEditBrush *pBrush, CMoArray<CEditBrush*> &intersects );
						
	// This function takes a world node and adds it and its children nodes
	// to an array.  It is useful if you want to work with a flattened array
	// of nodes rather than a tree of nodes.
	void					FlattenTreeToArray(CWorldNode *pRoot, CMoArray<CWorldNode*> &array);

	// Offset (move them in the world) the selected nodes by a vector
	void					OffsetSelectedNodes(CVector vOffset);

	// Scale the edit region by a floating point value
	void					ScaleBy(float fScaleFactor);

// Node functions.
public:

	CWorldNode*				AddNullNode(CWorldNode *pParentNode);
	void					AddNodeToRoot( CWorldNode *pNode );

	void					AttachNode( CWorldNode *pChild, CWorldNode *pParent=NULL );
	void					DetachNode( CWorldNode *pNode, BOOL bAttachToParent=TRUE );
	
	void					ClearSelections();
	void					ClearPathNodes();

	CWorldNode*				FindNodeByID(DWORD id);
	CWorldNode*				FindNodeByName(char *pName);
	
	// Returns the number of objects found.
	uint32					FindObjectsByName(const char *pName, CBaseEditObj **objectList, uint32 maxObjects);

	CBaseEditObj*			GetFirstSelectedObject();

	void					DoSelectionOperation( CWorldNode *pNode, BOOL bMultiSelect, BOOL bDoSubtree );

	void					RecurseAndSelect( CWorldNode *pNode, BOOL bUpdateSelectionArray=TRUE);
	void					RecurseAndUnselect( CWorldNode *pNode, BOOL bUpdateSelectionArray=TRUE);
	void					RecurseAndInverseSelect( CWorldNode *pNode, BOOL bUpdateSelectionArray=TRUE);
	void					RecurseAndInverseHide( CWorldNode *pNode );
	void					RecurseAndInverseUnhide( CWorldNode *pNode );

	void					SelectNode( CWorldNode *pNode, BOOL bUpdateSelectionArray=TRUE);
	void					UnselectNode( CWorldNode *pNode, BOOL bUpdateSelectionArray=TRUE);

	void					FreezeNode( CWorldNode *pNode, bool bFreeze, bool bDeselect, bool bRecurse );

	// Updates the selection array
	void					UpdateSelectionArray();	

	// Generates a unique name for the specified object
	// Return value:  TRUE if a new name was assigned
	BOOL					GenerateUniqueNameForNode(CWorldNode *pNode);
	

#ifdef DIRECTEDITOR_BUILD
	// Generates unique names for the selected nodes.
	// pOldNamesArray	- Filled with the names of the objects that have been changed
	// pNewNamesArray	- Filled with the new names of the objects that have been changed
	// pActionList		- Stores the undo information
	// bStoreUndoOnly	- Set this to TRUE to only store the UNDO information about the nodes.
	void					GenerateUniqueNamesForSelected(CStringArray *pOldNamesArray=NULL, CStringArray *pNewNamesArray=NULL,
														   PreActionList *pActionList=NULL, BOOL bStoreUndoOnly=FALSE);

	// Updates properties for referencing objects in the world.
	//
	// bSelectedOnly		- Indicates that only the selected objects should be modified
	// lpszOldName			- The original name of the object
	// lpszNewName			- The updated name for the object
	// pPropertyNameArray	- Filled in with the object::property names that are changed so that a report dialog can be made
	// pOriginalValues		- The original values for a property that has changed
	// pUpdatedValues		- The updated values for the property that has changed
	// pActionList			- Stores the undo information
	// bStoreUndoOnly		- Set this to TRUE to only store the UNDO information about the nodes.
	void					UpdateObjectsReferenceProps(BOOL bSelectedOnly, const char *lpszOldName, const char *lpszNewName,
														CStringArray *pPropertyNameArray=NULL, CStringArray *pOriginalValues=NULL, CStringArray *pUpdatedValues=NULL,
														PreActionList *pActionList=NULL, BOOL bStoreUndoOnly=FALSE);

	// Updates properties that reference a node that has its name
	// changed. For example, when an objects name has changed, properties
	// that reference the object must also be updated.
	//	
	// pNode				- The node that is to be updated
	// lpszOldName			- The original name of the object
	// lpszNewName			- The updated name for the object
	// pPropertyNameArray	- Filled in with the object::property names that are changed so that a report dialog can be made
	// pOriginalValues		- The original values for a property that has changed
	// pUpdatedValues		- The updated values for the property that has changed
	// pActionList			- Stores the undo information
	// bStoreUndoOnly		- Set this to TRUE to only store the UNDO information about the nodes.
	//
	// Returns:		TRUE	- The node had updated properties
	//				FALSE	- The node did not need to be updated
	BOOL					UpdateNodeRefProps(CWorldNode *pNode, const char *lpszOldObjectName, const char *lpszNewObjectName,
											   CStringArray *pPropertyNameArray=NULL, CStringArray *pOriginalValues=NULL, CStringArray *pUpdatedValues=NULL,
											   PreActionList *pActionList=NULL, BOOL bStoreUndoOnly=FALSE);

	//called in order to have all objects in the region update the icon that they are using for
	//the class icon representation. This should be called when the icon directory changes
	BOOL					UpdateObjectClassIcons();

#endif

protected:
	// Updates the selection array recursively
	void					RecurseUpdateSelectionArray(CWorldNode *pParentNode);

// Accessors.
public:

	// Return the root node
	CWorldNode*				GetRootNode()				{ return &m_RootNode; }

	// Return the array of navigator positions
	CNavigatorPosArray*		GetNavigatorPosArray()		{ return &m_NavigatorPosArray; }

	// Return the selected nodes
	uint32					GetNumSelections() const	{ return m_Selections.GetSize(); }
	CWorldNode*				GetSelection(int nIndex)	{ return m_Selections[nIndex]; }

	// Return the position of the green marker
	const LTVector&			GetMarker() const			{ return m_vMarker; }

	//called to add an object to the object list
	void					AddObject(CBaseEditObj* pObject);

	//called to remove an object from the object list
	void					RemoveObject(CBaseEditObj* pObject);

	//called when a brush is added
	void					AddBrush(CEditBrush* pBrush);

	//called when a brush is removed
	void					RemoveBrush(CEditBrush* pBrush);

	//called when a brush is modified, whether it be moved, or the vertices changed, etc.
	void					UpdateBrushGeometry(CEditBrush* pBrush);

	//updates everything about a brush, general case.
	void					UpdateBrush(CEditBrush* pBrush);

	//called when a node is updated. It will forward it to the appropriate updater
	void					UpdateNode(CWorldNode* pNode);

	//this will remove a node from the path list and clear out any related flags
	void					RemoveNodeFromPath(CWorldNode* pNode);

	//this will add a node to the list of nodes for paths
	void					AddNodeToPath(CWorldNode* pNode);

#ifdef DIRECTEDITOR_BUILD
	// Returns the node which is marked as the "active parent" which means that
	// new nodes get added to this parent.  The root node is returned if an
	// active parent cannot be found.
	CWorldNode*				GetActiveParentNode(CWorldNode *pStartNode=NULL);
	void					SetActiveParentNode(CWorldNode *pNode);

	CWorldNode*				m_pActiveParentNode;
#endif

	CPrefabMgr*				GetPrefabMgr()		{return &m_PrefabMgr;}

// World node (and CWorldNodeView) information.
public:

	// Path nodes.
	CMoArray<CWorldNode*>	m_PathNodes;			
	
	// The root world node.
	CWorldNode				m_RootNode;

	// Current selection.
	CMoArray<CWorldNode*>	m_Selections;

	// Array of navigator positions
	CNavigatorPosArray		m_NavigatorPosArray;
	
	// Last navigator position (used for the next/prev navigator command)
	int						m_nLastNavigatorPos;

	// Marker
	CVector					m_vMarker;

public:
	
// NOTE:	These lists of objects and brushes are not where things are deleted from.
//			They're always deleted from the WorldNode tree!
	
	// All the objects.
	CMoArray<CBaseEditObj*>		m_Objects;
	
	// All the brushes.
	CLinkedList<CEditBrush*>	m_Brushes;


	// Used while loading for fast indexing of brushes..
	CMoArray<CEditBrush*>		m_QuickBrushIndexer;
		

	// Used to hold texture names when loading polies.
	CStringHolder				*m_pStringHolder;

	CStringHolder				m_DefaultStringHolder;
	
	// Variable length info string.
	char						*m_pInfoString;

	// Helper for debugging file problems..
	int							m_LoadErrorCode;	
	
private:

	CPrefabMgr				m_PrefabMgr;
};


#endif  // __EDITREGION_H__


