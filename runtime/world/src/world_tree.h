#ifndef __WORLD_TREE_H__
#define __WORLD_TREE_H__

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

class WorldTreeHelper;


#define MAX_OBJ_NODE_LINKS      5
#define OBJ_NODE_LINK_ALWAYSVIS 4
#define MAX_WTNODE_CHILDREN     4
#define FRAMECODE_NOTINTREE     0xFFFFFFFF


class WorldTreeObj;
class WorldTree;
class WorldTreeNode;


typedef enum
{
    WTObj_DObject=0,    // Bounding box object (DObject).
    WTObj_Light         // Light for accurate model lighting.
} WTObjType;


typedef enum
{
    NOA_Objects=0,          // Objects
    NOA_Lights,             // Static lights
    NUM_NODEOBJ_ARRAYS
} NodeObjArray;


// Used for queries.

// IntersectSegment callback.  Return LTTRUE if you detected an intersection to
// assist with early termination.
typedef bool (*ISCallback)(WorldTreeObj *pObj, void *pUser);

typedef void (*WTObjCallback)(WorldTreeObj *pObj, void *pUser);


class FindObjInfo
{
public:

                        FindObjInfo()
                        {
                            m_pTree = NULL;
                            m_iObjArray = NOA_Objects;
                            m_Min.Init();
                            m_Max.Init();
                            m_CB = NULL;
                            m_pCBUser = NULL;
                        }

// These are automatically filled in.
public:

    WorldTree           *m_pTree;


// Fill these in when making calls.
public:
    NodeObjArray        m_iObjArray;    // Which array to index.
    LTVector            m_Min;
    LTVector            m_Max;
    WTObjCallback       m_CB;
    void                *m_pCBUser;
};


// This links a WorldTreeObj to a node.
class WTObjLink
{
public:
    LTLink          m_Link;
    WorldTreeNode   *m_pNode;
};


class WorldTreeObj
{
public:

    WorldTreeObj(WTObjType objType);
    virtual ~WorldTreeObj();

    inline WTObjType GetObjType() {
        return m_ObjType;
    }

    inline bool IsInWorldTree() {
        return m_WTFrameCode != FRAMECODE_NOTINTREE;
    }

    // This is called before the object is added to the world tree.
    // It gives objects a chance to put themselves on specific nodes rather than
    // filtering it down like normal.
    // If you return LTTRUE, then it assumes you added yourself.
    virtual bool InsertSpecial(WorldTree *pTree) {return LTFALSE;}

    // Unlink everything from the world tree.
    void RemoveFromWorldTree();

    // Get the bounding box of the object.
    const LTVector& GetBBoxMin() const { return m_MinBox; }
	const LTVector& GetBBoxMax() const { return m_MaxBox; }

	// Updates the extents of the bounding box based upon a position and dimensions
	void UpdateBBox(const LTVector& vPos, const LTVector& vDims)
	{
		m_MinBox = vPos - vDims;
		m_MaxBox = vPos + vDims;
	}

public:

	//the min and max extents of the bounding box of this object
	LTVector		m_MinBox;
	LTVector		m_MaxBox;

    WTObjLink       m_Links[MAX_OBJ_NODE_LINKS];

    // Tells what kind of object this is.
    WTObjType       m_ObjType;

    // Used in conjunction with WorldTree::m_CurFrameCode.
    // Set to FRAMECODE_NOTINTREE if the object is not in the WorldTree.
    uint32          m_WTFrameCode;
};


class WorldTreeNode
{
friend class WorldTree;

public:

							WorldTreeNode();
							~WorldTreeNode();

	//accesses the extents of the bounding box
    void					SetBBox(const LTVector& boxMin, const LTVector& boxMax);

    // Adds an object to the specified list.
    void					AddObjectToList(WTObjLink *pLink, NodeObjArray iArray);

	//determines if this node in the world tree has any children
    bool					HasChildren() const							{ return m_pChildren != NULL;}

	//accesses the specified world node
	WorldTreeNode*			GetChild(uint32 nX, uint32 nZ)				{ return &m_pChildren[nX * 2 + nZ]; }
	const WorldTreeNode*	GetChild(uint32 nX, uint32 nZ) const		{ return &m_pChildren[nX * 2 + nZ]; }

	WorldTreeNode*			GetChild(uint32 nNode)						{ return &m_pChildren[nNode]; }
	const WorldTreeNode*	GetChild(uint32 nNode) const				{ return &m_pChildren[nNode]; }

	//accessors for bounding box information
	const LTVector&			GetBBoxMin() const							{ return m_vBBoxMin; }
	const LTVector&			GetBBoxMax() const							{ return m_vBBoxMax; }
	float					GetCenterX() const							{ return m_fCenterX; }
	float					GetCenterZ() const							{ return m_fCenterZ; }

	//gets the number of objects that are included in either this object or its children
	uint32					GetNumObjectsOnOrBelow() const				{ return m_nObjectsOnOrBelow; }

	//gets the smallest of the X and Z dimensions (Y is essentially irrelevant)
	float					GetSmallestDim() const						{ return m_fSmallestDim; }
    
    // All the objects sitting on this node.
    CheapLTLink				m_Objects[NUM_NODEOBJ_ARRAYS];

public:

	//static member to handle the cleanup of a world tree link so that it can remove any dependancies
	static void				RemoveLink(WTObjLink* pLink);

private:

	// Load the tree layout, and use the node list for the children
    void					LoadLayout(ILTStream *pStream, uint8 &curByte, uint8 &curBit, WorldTreeNode* pNodes, uint32& nOffset);

    // Create child nodes from the list of passed in nodes. This will update the offset to compensate
	// for the subdivision. This assumes that the node list is large enough to hold all nodes in
	// the tree.
    void					Subdivide(WorldTreeNode* pNodes, uint32& nOffset);

	//functions for cleaning up the structure and clearing everything out
    void					Clear();
    void					Term();
    void					TermChildren();

    // Bounding box.
    LTVector				m_vBBoxMin;
    LTVector				m_vBBoxMax;

    // Centerpoint of the bounding box in the X and Z dimensions
	float					m_fCenterX;
	float					m_fCenterZ;    

    // The smallest of the X and Z dimensions.
    float					m_fSmallestDim;

	// How many objects are on or below this node? Used to stop recursion early.
    uint32					m_nObjectsOnOrBelow;

    // Parent node, NULL if none.
    WorldTreeNode			*m_pParent;

	//the children of this node
	WorldTreeNode			*m_pChildren;
};


class WorldTree
{
public:

	enum		{ MAX_NODE_LEVEL	= 16 };

    WorldTree();
	~WorldTree();

    void            InitWorldTree(WorldTreeHelper *pHelper);
    void            Term();

    // Add and remove objects from the tree.
    void            InsertObject(WorldTreeObj *pObj, NodeObjArray iArray=NOA_Objects);
    void            InsertObject2(WorldTreeObj *pObj, const LTVector& vMin, const LTVector& vMax, NodeObjArray iArray=NOA_Objects);

    // Add/remove objects to the constant visibility list
    void            InsertAlwaysVisObject(WorldTreeObj *pObj);
    void            RemoveAlwaysVisObject(WorldTreeObj *pObj);

    // Calls the specified callback for objects in the specified box.
    void			FindObjectsInBox(	const LTVector *pMin, const LTVector *pMax, 
										WTObjCallback cb, void *pCBUser, 
										NodeObjArray iArray=NOA_Objects);
    
    void			FindObjectsInBox2(FindObjInfo *pInfo);

    // Calls the specified callback for objects touching the specified point.
    void			FindObjectsOnPoint(	const LTVector *pPoint,
										WTObjCallback cb, void *pCBUser, 
										NodeObjArray iArray=NOA_Objects);

    // Calls a callback for each object in the nodes that the segment intersects.
    void			IntersectSegment(	const LTVector *pPt1, const LTVector *pPt2, 
										ISCallback cb, void *pCBUser, 
										NodeObjArray iArray=NOA_Objects);

    // Copy from the other tree.
    bool            Inherit(const WorldTree *pOther);

    // Load/save the node layout.
    bool            LoadLayout(ILTStream *pStream);

	//gets the current temporary frame code of the tree
	uint32			GetTempFrameCode() const		{ return m_nTempFrameCode; }

    WorldTreeNode*			GetRootNode()			{ return &m_RootNode; }
	const WorldTreeNode*	GetRootNode() const		{ return &m_RootNode; }

    // The list of always-visible objects
    CheapLTLink     m_AlwaysVisObjects;

private:

    // Used by Inherit to recursively copy nodes over
    void            CopyNodeLayout_R(WorldTreeNode *pDest, const WorldTreeNode *pSrc, 
									 WorldTreeNode* pNodeList, uint32& nCurrOffset);

	//the number of nodes in the tree, including the root node
	uint32			m_nNumNodes;

	//the helper for this world tree
    WorldTreeHelper *m_pHelper;

    // Gotten from m_pHelper and used during queries.
    uint32          m_nTempFrameCode;

    // Root of tree (depth value 0).        
    WorldTreeNode   m_RootNode;

	//our allocated list of nodes (not including the first one)
	WorldTreeNode*	m_pNodes;
};


#endif


