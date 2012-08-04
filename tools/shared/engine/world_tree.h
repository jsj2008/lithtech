
/*

This file contains the structure that subdivides all of the world data.  All
searches for objects and geometry start at the root of this tree.  

The tree is currently a quadtree but it's relatively easy to change it to 
a different structure like an octree.

The tree uses the concept of a Vis Container to allow for different visibility
schemes.  If an object moves into a node with a vis container object, it is added to
that object's visibility structure.  When the renderer encounters the node with
the vis container object, it stops recursing through the WorldTree, and asks
the vis container to output any visible objects.

*/


#ifndef __WORLD_TREE_H__
#define __WORLD_TREE_H__

    #ifndef __VISIT_PVS_H__
//	#include "visit_pvs.h"
    #endif

    #ifndef __WORLDTREEHELPER_H__
	#include "worldtreehelper.h"
    #endif


	#define MAX_OBJ_NODE_LINKS		5
	#define OBJ_NODE_LINK_ALWAYSVIS 4
	#define MAX_WTNODE_CHILDREN		4
	#define FRAMECODE_NOTINTREE		0xFFFFFFFF
	

	class WorldTreeObj;
	class WorldTree;
	class WorldTreeNode;


	// This can be used to generalize the (X,Z) nature of a quadtree. If it's changed to
	// an octree, then any code using these will work.
	#define NUM_WORLDTREE_DIMS	2
	extern uint32 g_WorldTreeDims[NUM_WORLDTREE_DIMS]; // For an (X,Z) worldtree, this will be (0,2).


	// Plane identifiers. These are guaranteed to go in order of g_WorldTreeDims and +,-.
	#define WTPLANE_LEFT	0	// +x
	#define WTPLANE_RIGHT	1	// -x
	#define WTPLANE_BACK	2	// +z
	#define WTPLANE_FRONT	3	// -z
	#define NUM_WTPLANES	4

	


	typedef enum
	{
		WTObj_DObject=0,	// Bounding box object (DObject).
		WTObj_Light			// Light for accurate model lighting.
	} WTObjType;


	typedef enum
	{
		NOA_Objects=0,			// Objects
		NOA_Lights,				// Static lights
		NOA_VisContainers,		// Vis containers
		NOA_TerrainSections,	// Terrain sections
		NUM_NODEOBJ_ARRAYS
	} NodeObjArray;


	// A NodePath represents a path down the WorldTree to a specific node.
	// Each level is encoded as 2 bits in m_Path.  4 bytes allows for a maximum
	// of 16 levels in the tree.
	class NodePath
	{
	public:
		inline void	Clear()
		{
			m_Path[0] = m_Path[1] = m_Path[2] = m_Path[3] = 0;
			m_iLevel = 0;
		}

	public:
		uint8	m_Path[4];
		uint32	m_iLevel;
	};

	#define MAX_NODE_LEVEL	16


	// Read/write NodePaths.
	inline void LTStream_Read(ILTStream *pStream, NodePath &path)
	{
		*pStream >> path.m_Path[0];
		*pStream >> path.m_Path[1];
		*pStream >> path.m_Path[2];
		*pStream >> path.m_Path[3];
		*pStream >> path.m_iLevel;
	}

	inline void LTStream_Write(ILTStream *pStream, NodePath &path)
	{
		*pStream << path.m_Path[0];
		*pStream << path.m_Path[1];
		*pStream << path.m_Path[2];
		*pStream << path.m_Path[3];
		*pStream << path.m_iLevel;
	}


	// Used for queries.
	
	// IntersectSegment callback.  Return LTTRUE if you detected an intersection to
	// assist with early termination.
	typedef LTBOOL (*ISCallback)(WorldTreeObj *pObj, void *pUser);

	typedef void (*WTObjCallback)(WorldTreeObj *pObj, void *pUser);
	typedef LTBOOL (*CreateNodesCB)(WorldTree *pWorldTreeTree, WorldTreeNode *pNode, void *pUser);

	// Used in the VisQueryRequest.  Returns LTTRUE if it should process the node.
	typedef LTBOOL (*NodeFilterFn)(WorldTreeNode *pNode);
	

	
	class FindObjInfo
	{
	public:

							FindObjInfo()
							{
								m_pTree = LTNULL;
								m_iObjArray = NOA_Objects;
								m_Min.Init();
								m_Max.Init();
								m_Viewpoint.Init();
								m_CB = LTNULL;
								m_pCBUser = LTNULL;
							}

	// These are automatically filled in.
	public:

		WorldTree			*m_pTree;
	
	
	// Fill these in when making calls.
	public:
		NodeObjArray		m_iObjArray;	// Which array to index.
		LTVector			m_Min;
		LTVector			m_Max;
		LTVector			m_Viewpoint; // Used in VisQuery.
		WTObjCallback		m_CB;
		void				*m_pCBUser;
	};


/*	class VisQueryRequest
	{
	public:

							VisQueryRequest()
							{
								m_Viewpoint.Init();
								m_ViewRadius = 0.0f;
								m_ObjectCB = DummyIterateObject;
								m_AddObjects = LTNULL;
								m_pUserData = LTNULL;
								m_LeafCB = DummyIterateLeaf;
								m_iObjArray = NOA_Objects;
								m_PortalTest = DummyPortalTest;
								m_NodeFilterFn = LTNULL;
							}


	// Set before calling VisQuery.
	public:
	
		NodeObjArray		m_iObjArray;
		LTVector			m_Viewpoint;
		float				m_ViewRadius;
		WTObjCallback		m_ObjectCB;
		AddObjectsFn		m_AddObjects;
		void				*m_pUserData;
		IterateLeafCB		m_LeafCB;
		PortalTestFn		m_PortalTest;
		NodeFilterFn		m_NodeFilterFn;
	
	// Set automatically.
	public:
		
		LTVector			m_Min;
		LTVector			m_Max;
		WorldTree			*m_pTree;

	};
  */
	
	// This links a WorldTreeObj to a node.
	class WTObjLink
	{
	public:
		LTLink			m_Link;
		WorldTreeNode	*m_pNode;
	};


	class WorldTreeObj
	{
	public:

						WorldTreeObj(WTObjType objType);
		virtual			~WorldTreeObj();

		inline WTObjType	GetObjType()	{return m_ObjType;}
		inline LTBOOL		IsInWorldTree()	{return m_WTFrameCode != FRAMECODE_NOTINTREE;}

		// This is called before the object is added to the world tree.
		// It gives objects a chance to put themselves on specific nodes rather than
		// filtering it down like normal.
		// If you return LTTRUE, then it assumes you added yourself.
		virtual LTBOOL	InsertSpecial(WorldTree *pTree)	{return LTFALSE;}
	
		// Unlink everything from the world tree.
		virtual void	RemoveFromWorldTree();

		// Get the bounding box of the object.
		virtual void	GetBBox(LTVector *pMin, LTVector *pMax)=0;


	// Visibility stuff.
	public:

		// Returns LTTRUE if the object can contain objects for visibility.
		virtual LTBOOL	IsVisContainer()	{return LTFALSE;}

		// If you're a vis container, you should have a list of DLinks that is
		// MAX_OBJ_NODE_LINKS in length for WorldTreeNode::m_VisContainers.
		virtual LTLink*	GetVisContainerLink(uint32 iLink)	{ASSERT(LTFALSE); return LTNULL;}

		// Add the object to this object's internal visibility structure.  When the renderer gets
		// to any nodes that this object is sitting on, it will stop recursing through the WorldTree
		// and call upon vis containers to give it more objects.
		virtual void	AddToVis(WorldTreeObj *pObj)	{ASSERT(LTFALSE);}

		// Do the callback for all the visible objects.
//		virtual void	DoVisQuery(VisQueryRequest *pRequest)	{ASSERT(LTFALSE);}


	public:

		WTObjLink		m_Links[MAX_OBJ_NODE_LINKS];

		// Tells what kind of object this is.
		WTObjType		m_ObjType;

		// Used in conjunction with WorldTree::m_CurFrameCode.
		// Set to FRAMECODE_NOTINTREE if the object is not in the WorldTree.
		uint32			m_WTFrameCode;
	};


	class WorldTreeNode
	{
	friend class WorldTree;
	
	public:

						WorldTreeNode();
						~WorldTreeNode();

		void			Clear();
		void			Term();
		void			TermChildren();
	
		// Sets m_BBoxMin, m_BBoxMax, m_vCenter, and m_MaxSize.
		void			SetBBox(LTVector boxMin, LTVector boxMax);
		void			GetBBox(LTVector *pBoxMin, LTVector *pBoxMax);

		inline LTBOOL	HasChildren() {return !!m_Children[0][0];}
		
		// Sets up the specified bounding plane.  iPlane must be one of the 
		// WTPLANE_ defines.  The planes point towards the INSIDE of the node.
		void			SetupPlane(uint32 iPlane, LTPlane *pPlane);

		// Returns the number of nodes in this node's subtree (including the node itself).
		uint32			NumSubtreeNodes();

		// Load/save the tree layout.
		LTBOOL			LoadLayout(ILTStream *pStream, uint8 &curByte, uint8 &curBit);
		void			SaveLayout(ILTStream *pStream, uint8 &curByte, uint8 &curBit);

		// Create child nodes.
		LTBOOL			Subdivide();

		// Adds an object to the specified list.
		void			AddObjectToList(WTObjLink *pLink, NodeObjArray iArray);


	protected:

		// Called by WorldTree::GetNodePath.
		LTBOOL			GetNodePath(WorldTreeNode *pNode, NodePath *pPath);

	
	public:
		
		// All the objects sitting on this node.
		CheapLTLink			m_Objects[NUM_NODEOBJ_ARRAYS];

		// Bounding box.
		LTVector			m_BBoxMin;
		LTVector			m_BBoxMax;

		// Centerpoint of the bounding box.
		LTVector			m_vCenter;

		// Maximum size in any dimension.
		float				m_MinSize;

		// Distance from m_vCenter to m_BBoxMax.
		float				m_Radius;

		// How many objects are on or below this node?
		// Used to stop recursion early.
		uint32				m_nObjectsOnOrBelow;

		// Parent node, LTNULL if none.
		WorldTreeNode		*m_pParent;

		union
		{
			// Referenced as (+x, +z).  If m_Children[0][0] is set, then
			// all of them are set.  Use HasChildren() to determine this.
			WorldTreeNode	*m_Children[2][2];
			
			// For simpler loops..
			WorldTreeNode	*m_ChildrenA[MAX_WTNODE_CHILDREN];
		};
	};


	class WorldTree
	{
	public:

						WorldTree();

		uint32			GetCurFrameCode();

		void			Init(WorldTreeHelper *pHelper);
		void			Term();

		// Get a node given a NodePath.
		// LTNULL is returned if the path is invalid.
		virtual WorldTreeNode*	FindNode(NodePath *pPath);

		// Gets the path for a given node.  This is a VERY slow operation consisting of
		// a search through the tree for the specified node.  Returns LTFALSE if the 
		// node cannot be found.
		LTBOOL			GetNodePath(WorldTreeNode *pNode, NodePath *pPath);
		
		// Add and remove objects from the tree.
		void			InsertObject(WorldTreeObj *pObj, NodeObjArray iArray=NOA_Objects);
		void			InsertObject2(WorldTreeObj *pObj, LTVector *pMin, LTVector *pMax, NodeObjArray iArray=NOA_Objects);
		void			RemoveObject(WorldTreeObj *pObj);

		// Add/remove objects to the constant visibility list
		void			InsertAlwaysVisObject(WorldTreeObj *pObj);
		void			RemoveAlwaysVisObject(WorldTreeObj *pObj);

		// Calls the specified callback for objects in the specified box.
		virtual void	FindObjectsInBox(LTVector *pMin, LTVector *pMax, 
			WTObjCallback cb, void *pCBUser, NodeObjArray iArray=NOA_Objects);
		
		virtual void	FindObjectsInBox2(FindObjInfo *pInfo);

		// Calls the specified callback for objects touching the specified point.
		virtual void	FindObjectsOnPoint(LTVector *pPoint,
			WTObjCallback cb, void *pCBUser, NodeObjArray iArray=NOA_Objects);

		// Calls a callback for each object in the nodes that the segment intersects.
		virtual void	IntersectSegment(LTVector *pPt1, LTVector *pPt2, 
			ISCallback cb, void *pCBUser, NodeObjArray iArray=NOA_Objects);

		// Finds objects inside the specified bounding box.  This differs from FindObjectsInBox
		// in that if it runs across a vis container, it asks the vis container to handle objects
		// in and under its node.
//		virtual void	DoVisQuery(VisQueryRequest *pRequest);

		// Copy from the other tree.
		LTBOOL			Inherit(WorldTree *pOther);

		// Recurses and creates nodes in the tree.  
		LTBOOL			CreateNodes(
			LTVector *pMin, LTVector *pMax, 
			CreateNodesCB cb, void *pUser);

		// Load/save the node layout.
		LTBOOL			LoadLayout(ILTStream *pStream);
		LTBOOL			SaveLayout(ILTStream *pStream);

		inline WorldTreeNode*	GetRootNode()	{return &m_RootNode;}

		
	// Overrides.
	public:

		virtual void	GetBBox(LTVector *pMin, LTVector *pMax) {}

	
	protected:

		LTBOOL			RecurseAndCreateNodes(WorldTreeNode *pNode,
			LTVector *pMin, LTVector *pMax, CreateNodesCB cb, void *pUser,
			uint32 depth);

		// Used by Inherit.
		LTBOOL			CopyNodeLayout_R(WorldTreeNode *pDest, WorldTreeNode *pSrc);


	public:

		WorldTreeHelper	*m_pHelper;

		// Gotten from m_pHelper and used during queries.
		uint32			m_TempFrameCode;

		// Root of tree (depth value 0).		
		WorldTreeNode	m_RootNode;

		// These are maintained for debugging info..
		uint32			m_nNodes;
		uint32			m_nDepth;	// Max depth.

		// Depth of tree that terrains are at.
		uint32			m_TerrainDepth;

		// The list of always-visible objects
		CheapLTLink		m_AlwaysVisObjects;
	};


#endif


