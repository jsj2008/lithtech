// This module defines the various object structures used by DE.

#ifndef __DE_OBJECTS_H__
#define __DE_OBJECTS_H__

// ------------------------------------------------------------- //
// Types and defines.
// ------------------------------------------------------------- //

class SharedTexture;
class LineSystem;
class SpriteInstance;
class ObjectMgr;
class SObjData;
class TransformMaker;
class LTObject;
class WorldModelInstance;
class ModelInstance;
class DynamicLight;
class CameraInstance;
class LTParticleSystem;
class LTPolyGrid;
class LineSystem;
class ContainerInstance;
class Canvas;
class LTVolumeEffect;
class WorldBsp;
class WorldData;
class Node;
class LTAnimTracker;
class Model;
struct Sprite;
struct ObjectCreateStruct;

#ifndef __BDEFS_H__
#include "bdefs.h"
#endif

#ifndef __DE_WORLD_H__
#include "de_world.h"
#endif

#ifndef __LTANIMTRACKER_H__
#include "ltanimtracker.h"
#endif

#ifndef __DE_SPRITE_H__
#include "de_sprite.h"
#endif

#ifndef __MODEL_H__
#include "model.h"
#endif

#ifndef __ILTSPRITECONTROL_H__
#include "iltspritecontrol.h"
#endif

#ifndef __TRANSFORMMAKER_H__
#include "transformmaker.h"
#endif

#define INVALID_OBJECTID ((unsigned short)-1)

#define INVALID_SERIALIZEID 0xFFFF

// Internal object flags (most are only used by the server).
#define IFLAG_OBJECTGOINGAWAY   (1<<0)  // This object is going away..
#define IFLAG_MOVING            (1<<1)  // Used while moving objects so it doesn't recursively push objects around.
#define IFLAG_INWORLD           (1<<2)  // Set in AddObjectToWorld(), cleared in RemoveObjectFromWorld().
#define IFLAG_INACTIVE          (1<<3)  // Completely inactive (no physics updates or touch notifies).
#define IFLAG_INACTIVE_TOUCH    (1<<4)  // Inactive (no updates or physics), but gets touch notifies.
#define IFLAG_NOTMOVEABLE		(1<<5)	// Determines whether or not this object is moveable
#define IFLAG_APPLYPHYSICS      (1<<6)  // Object has physics applied
#define IFLAG_HASCLIENTREF      (1<<7)  // This object has a client ref pointing to it.
#define IFLAG_INSKY             (1<<8)  // Is this object in the sky?
#define IFLAG_MAINWORLDMODEL	(1<<9)	// Is this object the main world model?

// Just a helper to see if an object is inactive.
#define IFLAG_INACTIVE_MASK (IFLAG_INACTIVE|IFLAG_INACTIVE_TOUCH)

class CRenderStyle;

// The total number of user render groups available
#define MAX_OBJECT_RENDER_GROUPS			256

// ------------------------------------------------------------- //
// Structures.
// ------------------------------------------------------------- //

class Attachment
{
public:
	LTransform          m_Offset;       // transform offset.
	uint16				m_nChildID;		// the child object of this attachment.
	uint16				m_nParentID;	// the parent object of this attachment.
	uint32              m_iSocket;      // Model node index (if the parent is not a model, this is -1).
    Attachment          *m_pNext;
};


struct ClientData
{
    // Position interpolation info
	float			m_fLastUpdatePosTime; // Client time of the last update from the server
    LTVector        m_LastUpdatePosServer;  // Last position given by server
    LTVector        m_LastUpdateVelServer;  // Last velocity given by server
    LTLink          m_MovingLink;   // Link into the main list of moving objects.

    // Rotation interpolation info
    float           m_fRotAccumulatedTime;
    LTRotation      m_rLastUpdateRotServer;
    LTLink          m_RotatingLink;     // Link into the main list of rotating objects.

    // Line system to track where moving objects go (for debugging)
    HLOCALOBJ       m_hLineSystem;

    uint32			m_ClientFlags;      // Client-side object flags..

    void            *m_pUserData;       // User data..
};


// All objects sitting in the BSP are LTObjects.
class LTObject : public WorldTreeObj
{
// Virtuals.
public:

    LTObject();
    LTObject(char objectType);
    virtual ~LTObject();

    virtual void Init(ObjectMgr *pMgr, ObjectCreateStruct *pStruct);
    void Clear();


    // Is this a client or a server object?
    ClientServerType	GetCSType() const { return (ClientServerType)!!sd; }

// Overridables.
public:

    // Sets up the matrix with the object's position, rotation, and scale.
    void SetupTransform(LTMatrix &mat);

    // This is so main world models can't be moved.
    bool IsMoveable() const			{ return (m_InternalFlags & IFLAG_NOTMOVEABLE) == 0; }

    // Returns LTTRUE if it's a WorldModelInstance with WIF_MAINWORLD.
    bool IsMainWorldModel() const	{ return (m_InternalFlags & IFLAG_MAINWORLDMODEL) != 0; }

// Helpers.
public:

    // Cast to certain types of objects.
    inline ModelInstance*       ToModel()           {ASSERT(m_ObjectType==OT_MODEL); return (ModelInstance*)this;}
    inline SpriteInstance*      ToSprite()          {ASSERT(m_ObjectType==OT_SPRITE); return (SpriteInstance*)this;}
    inline WorldModelInstance*  ToWorldModel()      {ASSERT(HasWorldModel()); return (WorldModelInstance*)this;}
    inline DynamicLight*        ToDynamicLight()    {ASSERT(m_ObjectType==OT_LIGHT); return (DynamicLight*)this;}
    inline CameraInstance*      ToCamera()          {ASSERT(m_ObjectType==OT_CAMERA); return (CameraInstance*)this;}
    inline LTParticleSystem*    ToParticleSystem()  {ASSERT(m_ObjectType==OT_PARTICLESYSTEM); return (LTParticleSystem*)this;}
    inline LTPolyGrid*          ToPolyGrid()        {ASSERT(m_ObjectType==OT_POLYGRID); return (LTPolyGrid*)this;}
    inline LineSystem*          ToLineSystem()      {ASSERT(m_ObjectType==OT_LINESYSTEM); return (LineSystem*)this;}
    inline ContainerInstance*   ToContainer()       {ASSERT(m_ObjectType==OT_CONTAINER); return (ContainerInstance*)this;}
    inline Canvas*              ToCanvas()          {ASSERT(m_ObjectType==OT_CANVAS); return (Canvas*)this;}
	inline LTVolumeEffect*		ToVolumeEffect()	{ASSERT(m_ObjectType==OT_VOLUMEEFFECT); return (LTVolumeEffect*)this;}

    // Is this object derived from WorldModel?
    bool HasWorldModel() const 
	{
        return m_ObjectType==OT_WORLDMODEL || m_ObjectType==OT_CONTAINER;
    }

	// Is this object derived from Model?
	bool IsModel() const 
	{
		return m_ObjectType==OT_MODEL;
	}

    // Returns true if the object is scaled (ie: its scale is not (1,1,1)).
    bool IsScaled() const
	{
        return	(fabs(m_Scale.x - 1.0f) > 0.01f) || 
				(fabs(m_Scale.y - 1.0f) > 0.01f) || 
				(fabs(m_Scale.z - 1.0f) > 0.01f);
    }

	// Returns true if the object should be treated as translucent
	bool IsTranslucent() const
	{
		return	(m_ColorA < 255) ||										//our alpha isn't completely solid
				(m_Flags2 & (FLAG2_ADDITIVE | FLAG2_FORCETRANSLUCENT));	// or they have a flag set forcing translucency
	}

	// Determines if this object is paused or not
	bool IsPaused() const
	{
		return !!(m_Flags & FLAG_PAUSED);
	}

    // Get/Set position and dims.  These automatically update m_MinBox and m_MaxBox.
    const LTVector &GetPos() const					{ return m_Pos; }
    void			SetPos(const LTVector &pos)		{ m_Pos = pos; m_MinBox = m_Pos - m_Dims; m_MaxBox = m_Pos + m_Dims; }

	void			SetDims(const LTVector &dims)	{ m_Dims = dims; UpdateBBox(GetPos(), m_Dims); m_Radius = m_Dims.Mag() + 0.1f; }
	const LTVector& GetDims() const					{ return m_Dims; }

    float			GetRadius() const				{ return m_Radius; }
    float			GetRadiusSquared() const		{ return m_Radius * m_Radius; }

	// Get the global force override...
	const LTVector &GetGlobalForceOverride() const	{ return m_GlobalForceOverride; }

public:

    ObjectMgr       *m_pObjectMgr;      // The object's mommy

    LTLink          m_Link;     // Link for the client.
    uint32			m_Flags;    // The object's flags  (from FLAGS_).
    uint32			m_Flags2;   // The object's flags2 (from FLAGS2_).
    uint32			m_UserFlags;// User flags.

    uint8			m_ColorR;   // RGBA color info.
    uint8			m_ColorG;
    uint8			m_ColorB;
    uint8			m_ColorA;

    Attachment      *m_Attachments;		// Objects attached to this one.

    LTVector        m_Scale;    // Scale..

    uint16			m_ObjectID; // The object ID (and its index into CClientStruct::m_ObjectInfos).
                                // -1 for client-created objects.

    uint16			m_SerializeID;  // Used while loading/saving.  Set to INVALID_SERIALIZEID if not loading/saving.

    uint8			m_ObjectType;   // Object type (an OT_ number).
    uint8			m_BPriority;        // Blocking priority.

	uint8			m_nRenderGroup;	//the rendering group that this object belongs to. It defaults to the default group which can never be turned off. It can be overridden to specify a group which can then be turned on or off

	void			*m_pUserData;	// User data..
	float			m_Radius;		// Radius approximation. Calculated from the dims of the object
	LTVector		m_Dims;		// Dimensions of this object.

	// Physics stuff.
    float           m_Mass;					// mass, kg
    LTVector        m_Pos;					// position
    LTRotation      m_Rotation;				// orientation as a unit quaternion
    LTVector        m_Velocity;				// velocity, meters/sec
	LTVector		m_GlobalForceOverride;	// global force override

    //static/dynamic friction coeff
    float			m_FrictionCoefficient;
    LTVector		m_Acceleration;

    // The list of objects that are standing on this one.
    LTLink			m_ObjectsStandingOn;
    LTLink			m_StandingOnLink;

	// The list of references to this object
	CheapLTLink		m_RefList;

    // How much pain (squared) can this object take before the engine tells it?
    float			m_ForceIgnoreLimitSqr;

    // Each object can be 'standing on' one object at a time.
    LTObject		*m_pStandingOn;
    const Node		*m_pNodeStandingOn;

    uint32			m_InternalFlags;    // Internal flags used by the server (IFLAG_).

    ClientData      cd;     // Client-specific data.
    SObjData        *sd;    // Server object data.

	// Notify the object references that the object is about to be deleted
	void NotifyObjRefList_Delete();

    //********************************************************************
    //The stuff below relies on the BSP world rep.
    //

    // WorldTreeObj overrides.
    virtual bool InsertSpecial(WorldTree *pTree);

    //
    //Stuff above here relies on the BSP world rep.
    //********************************************************************
};


// WorldModels.
class WorldModelInstance : public LTObject
{
// Overrides.
public:

                        WorldModelInstance();
                        WorldModelInstance(char objectType);
    virtual             ~WorldModelInstance();

    virtual void        Init(ObjectMgr *pMgr, ObjectCreateStruct *pStruct);
    void                Clear();

    virtual bool		InsertSpecial(WorldTree *pTree);

    // Makes an HPOLY given a node.  This has to check both the BSPs in
    // m_pWorldData to see which one the Node is in.
    HPOLY               MakeHPoly(const Node *pNode);


public:

    // Sets m_pOriginalBsp, m_pWorldBsp, and m_pValidBsp.
    void				InitWorldData(const WorldBsp *pOriginalBsp, const WorldBsp *pWorldBsp);

    const WorldBsp*		GetOriginalBsp() const	{return m_pOriginalBsp;}
    const WorldBsp*		GetWorldBsp() const		{return m_pWorldBsp;}
    const WorldBsp*		GetValidBsp() const		{return m_pValidBsp;}


// Vars.
public:

    // These are taken directly from WorldData, OR, if this is a TerrainSection,
    // they point at the TerrainSection.
    const WorldBsp          *m_pOriginalBsp;
    const WorldBsp          *m_pWorldBsp;
    const WorldBsp			*m_pValidBsp;

    // Identity except for moving BSPs.
    LTMatrix                m_Transform;
    LTMatrix                m_BackTransform;
};


// Containers (derived from WorldModel).
class ContainerInstance : public WorldModelInstance
{
// Overrides.
public:

                        ContainerInstance();
    virtual             ~ContainerInstance();

    virtual void        Init(ObjectMgr *pMgr, ObjectCreateStruct *pStruct);


// Vars.
public:
    unsigned short      m_ContainerCode;
};


// Cameras.
class CameraInstance : public LTObject
{
// Overrides.
public:

                CameraInstance();
    virtual     ~CameraInstance();

public:
    int         m_Left, m_Top, m_Right, m_Bottom;
    float       m_xFov, m_yFov;
    int         m_bFullScreen;
    LTVector    m_LightAdd;     // Brighten everything up (values 0-1).  Does nothing when
                                // this is 0,0,0.  This just draws a poly over
                                // the screen which is SLOW.
};


// Dynamic lights.
class DynamicLight : public LTObject
{
// Overrides.
public:

                    DynamicLight();
    virtual         ~DynamicLight();

    float   GetRadius() const
    {
        return m_LightRadius;
    }

// Vars.
public:
    float       m_LightRadius;
};

// ------------------------------------------------------------------------
// Model instances.
// model database instance container for LTObject.
// ------------------------------------------------------------------------
class ModelInstance : public LTObject
{
public:
		enum {	INVALID_MODEL_INFO_INDEX	= 0xFFFF };
		enum { REMOVE=0, ADD=1, INVALID=2  } ; 

// Overrides.
public:

    ModelInstance();
    virtual ~ModelInstance();

	// Update the ModelInstance.
	void				ServerUpdate( uint32 msFrameTime ); // use this on the server
	void				ClientUpdate( uint32 msFrameTime ); // use this on the client... 

    virtual float       GetRadius()      { return GetModelDB()->m_VisRadius * MAX(m_Scale.x, MAX(m_Scale.y, m_Scale.z)); }
	
	// Model Interface Methods.
    
	// if failure occurs with the piece index is out of range. or the lod spec does not fit the request
    bool                GetLODValFromDist( uint32 piece_index, float distance, uint32 &lod );

	// returns false if piece_index is out of range or there is no model
	bool		        GetNumLOD( uint32 piece_index, uint32 &num_lod );

	//Allocates all the node control data after a model has been setup
	void				SetupNodeInfo();

	//Frees up all allocated data for the node control functions
	void				FreeNodeInfo();


	//counts the number of anim trackers associated with this object
	uint32				GetNumAnimTrackers() const;

	// this updates only nodes on an evaluation path. (i.e a sub tree of hierarchy)
	bool				UpdateCachedTransformsWithPath();
	// this ignores flags, and just updates. 
	bool				ForceUpdateCachedTransforms();

	// node control

	//adds a node control function for a single node
	void				AddNodeControlFn( HMODELNODE hNode, NodeControlFn pFn, void *pUserData);

	//adds a node control function for the entire model (all nodes will have this function called)
	void				AddNodeControlFn( NodeControlFn pFn, void *pUserData);

	//removes a node control function from the specified node. If the user data is NULL, it will
	//remove all that match the function and will not check the user data
	void				RemoveNodeControlFn( HMODELNODE hNode, NodeControlFn pFn, void* pUserData);

	//removes a node control function from all nodes. If the user data is NULL, it will
	//remove all that match the function and will not check the user data
	void				RemoveNodeControlFn( NodeControlFn pFn, void* pUserData);
	
	//determines if a node control function is associated with the specified node
	bool				HasNodeControlFn( HMODELNODE hNode );

	//applies the series of node control functions to the specified matrix
	void				ApplyNodeControl( const NodeControlData& Data );

	//retreives the number of nodes. assumes a valid model database
	uint32				NumNodes()										{assert(GetModelDB()); return GetModelDB()->NumNodes(); }

	// node access
	uint32				NodeGetRootIndex();
	uint32				NodeGetNumChildren( uint32 inode );
	uint32				NodeGetChild( uint32 inode, uint32 index );
	uint32				NodeGetParent( uint32 inode );

	// transform access returns false if no cached transforms.
	bool				GetCachedTransform( uint32 iNode, LTMatrix &transform );
	
	// get transforms used for rendering, these are device dependant matrices.
	DDMatrix*			GetRenderingTransforms();
	
	// returns false if iNode/iSock is out of range.
	// both kinds one for ltransform, one for ltmatrix
	bool				GetNodeTransform( uint32 iNode, LTransform &tf, bool bInWorldSpace=true );
	bool				GetNodeTransform( uint32 iNode, LTMatrix &mat, bool bInWorldSpace=true );
	bool				GetSocketTransform( uint32 iSock, LTransform &tf, bool bInWorldSpace = true );
	bool				GetSocketTransform( uint32 iSock, LTMatrix *mat, bool bInWorldSpace= true  );

	//accessors for determining whether or not the specified node needs to be evaluated
	bool				ShouldEvaluateNode(uint32 nNode)					{ assert(nNode < NumNodes()); return m_CachedTransformInfo[ nNode ].m_bNeedEvaluation; }
	void				SetShouldEvaluateNode(uint32 nNode, bool bVal)		{ assert(nNode < NumNodes()); m_CachedTransformInfo[ nNode ].m_bNeedEvaluation = bVal; }

	//accessors for determining if this node is already evaluated
	bool				IsNodeEvaluated(uint32 nNode)						{ assert(nNode < NumNodes()); return m_CachedTransformInfo[ nNode ].m_bEvaluated; }
	void				SetNodeEvaluated(uint32 nNode, bool bVal)			{ assert(nNode < NumNodes()); m_CachedTransformInfo[ nNode ].m_bEvaluated = bVal; }

	//accessors for determining if this node has already evaluated its rendering transform
	bool				IsNodeEvaluatedRendering(uint32 nNode)				{ assert(nNode < NumNodes()); return m_CachedTransformInfo[ nNode ].m_bEvaluatedRendering; }
	void				SetNodeEvaluatedRendering(uint32 nNode, bool bVal)	{ assert(nNode < NumNodes()); m_CachedTransformInfo[ nNode ].m_bEvaluatedRendering = bVal; }

	// set up the evaluation path for this node.
	void				SetupNodePath( uint32 iNode );

	// set up the evaluation path for this renderable thing's used nodes. The thing will have a list of 
	// leaf nodes. 
	void				SetupLODNodePath( CDIModelDrawable *pLOD );

	void				SetStringKeyCallback( StringKeyCallback   SKCB ) { m_StringKeyCallBack =SKCB; }

	// add child models 
	bool				AddChildModelDB( Model * );

	//this will mark all nodes as needing to be re-evaluated
	void				ResetCachedTransformNodeStates();

	// Helpers.
public:

	// BindModelDB associate a model with this model instance. 
	void				BindModelDB( Model * );
	// disassociate the model from this ltobject, returns just unbound model.
	void				UnbindModelDB();

	// if a pointer to model is going to be kept for long, add-ref please.
    Model*              GetModelDB()                            { return m_AnimTracker.GetModel(); }

    bool                IsPieceHidden(uint32 index) const       { ASSERT(index < MAX_PIECES_PER_MODEL); return !!(m_HiddenPieces[index / 32] & (1 << (index % 32))); }

    bool                SetRenderStyle(uint32 iIndex, CRenderStyle* pRenderStyle);
    bool                GetRenderStyle(uint32 iIndex, CRenderStyle** ppRenderStyle);

	void				InitAnimTrackers( uint16 flags = 0 );
	LTAnimTracker*		GetTracker(ANIMTRACKERID TrackerID);
	LTAnimTracker*		GetTracker(ANIMTRACKERID TrackerID, LTAnimTracker** &pPrev);

public:
 
	SharedTexture*		m_pSkins[MAX_MODEL_TEXTURES];


    LTAnimTracker       m_AnimTracker;                          // The model always has at least one animation tracker.
    LTAnimTracker*      m_AnimTrackers;                         // The list of animations pit's playing - this is linked with their m_Link::m_pNext (null terminated).

    uint32              m_HiddenPieces[MAX_PIECES_PER_MODEL / 32]; // 1 bit for each hidden piece.

    CRenderStyle*       m_pRenderStyles[MAX_MODEL_RENDERSTYLES];

    Sprite*             m_pSprites[MAX_MODEL_TEXTURES];         // It might be using a sprite to update the texture frames.
    SpriteTracker       m_SpriteTrackers[MAX_MODEL_TEXTURES];

    LTVector            m_LastDirLightPos;                      // Last position at which this instance was directionally lit (used for static sunlight)
    float               m_LastDirLightAmount;                   // Previous amount of directional lighting applied to the model (-1 if never previously lit)
	uint16				m_nRenderInfoIndex;						// Index to use during rendering of the global model info cache (used for client side rendering only, this will be INVALID_MODEL_INFO_INDEX outside of rendering)
	uint16				m_nRenderInfoParentIndex;				// Index of parent's render information. Used in rendering to work around attachments
	uint8				m_nNumSprites;							// The number of sprites this model has loaded on it. Primarily for optimizing away the need to update sprites

#if(MODEL_OBB)
	// Oriented Bounding Box (OBB) Methods.
  	void				EnableCollisionObjects(); // get collision data from modeldb
  	void				DisableCollisionObjects();
	void				UpdateCollisionObjects( ModelOBB * );
	void				UpdateCollisionObject( uint32 obb_index, ModelOBB & return_value );
  	bool				IsCollisionObjectsEnabled()			  { return	m_NumOBBs > 0 ; }
  	uint32				NumCollisionObjects()				  { return  m_NumOBBs  ;    }
  	const ModelOBB *	GetCollisionObject( uint32 i )  const { return  &m_ModelOBBs[i];}
  
	// pass in an array of modelobb pointers the size equal to NumCollisionObjects(). There is no checking for size in function.
  	void				GetCollisionObjects( ModelOBB *);
  	
#endif // MODEL_OBB

protected : 

	// Cached Transforms 
	void				EnableTransformCache();
	void				DisableTransformCache();
	
	LTMatrix			*m_CachedTransforms;
	DDMatrix			*m_RenderingTransforms ; 

	// state of every node in tranform cache 
	struct SCachedTransformInfo
	{
		SCachedTransformInfo()
		{
			m_bNeedEvaluation		= false;
			m_bEvaluated			= false;
			m_bEvaluatedRendering	= false;
		}

		//does this node need to be evaluated
		bool		m_bNeedEvaluation;

		//has this node already been evaluated and the matrix can be used as is
		bool		m_bEvaluated;

		//has the rendering transform been evaluated
		bool		m_bEvaluatedRendering;
	};
	
	SCachedTransformInfo   *m_CachedTransformInfo;

	StringKeyCallback   m_StringKeyCallBack;

	struct SNodeControlInfo
	{
		//the function to be used
		NodeControlFn		m_ControlFn;

		//the user data associated with the callback
		void*				m_pUserData;

		//the next callback in the list
		SNodeControlInfo*	m_pNext;
	};

	//all the information that is stored per node
	struct SNodeInfo
	{
		//a pointer to the first node in a list of node control infos if any are acting upon this
		//node
		SNodeControlInfo*	m_pNodeControls;
	};

	//the list of starting node controls for each node
	SNodeInfo			*m_pNodeInfo;
	uint32				m_nNumNodeInfos;

	// update the movement encoding hint node. 
	// this is done only on the client.s
	void DoMoveHint( LTAnimTracker *pTracker );

#if(MODEL_OBB)
	// Oriented bounding box stuff from modeldb
  	uint32				m_NumOBBs ; 
  	ModelOBB			*m_ModelOBBs; // base/cached transformed 
#endif

};

 

// A single particle.
struct PSParticle
{
    LTVector    m_Pos;				// Current position of the particle
    LTVector    m_Velocity;			// Current velocity of the particle
	float       m_Size;				// Particle size

    LTVector    m_Color;			// 0-255
    float       m_Alpha;			// 0-1
    
	float		m_fAngle;			// Angle in radians of this particle
	float		m_fAngularVelocity;	// Velocity of the angular change in radians per second

    float       m_Lifetime;         // Current lifetime left
    float       m_TotalLifetime;    // Total lifetime (i.e. initial value)

	uint32		m_nUserData;		// 32 bits of user data...

    PSParticle  *m_pNext;			// Linked list information
    PSParticle  *m_pPrev;
};


// A particle system.
class LTParticleSystem : public LTObject
{
// Overrides.
public:

                        LTParticleSystem();
    virtual             ~LTParticleSystem();

    virtual void        Init(ObjectMgr *pMgr, ObjectCreateStruct *pStruct);


// Vars.
public:

    PSParticle      m_ParticleHead;     // Lists of particles.

    StructBank      *m_pParticleBank;   // Where the particles come from.
    SharedTexture   *m_pCurTexture;     // Current texture for particles.

    // Color for the particles for the software renderer.
    unsigned char   m_SoftwareR;
    unsigned char   m_SoftwareG;
    unsigned char   m_SoftwareB;
    unsigned char   m_Padding; // Structure alignment padding..

    Sprite          *m_pSprite;         // It might be using a sprite to update the frames.
    SpriteTracker   m_SpriteTracker;

    LTVector        m_SystemCenter;     // The sphere enclosing the whole particle system.
    float           m_SystemRadius;

    LTVector        m_OldCenter;        // Used in update loops to see if it grew.
    float           m_OldRadius;

    int             m_nParticles;           // Total number of particles in the system.
    int             m_nChangedParticles;    // Number of new or changed particles since the last update.

    LTVector        m_MinPos, m_MaxPos; // Min and max particle positions.

    float           m_GravityAccel;     // Gravity acceleration.
    float           m_ParticleRadius;   // The size of each particle.

    int             m_nSrcBlend;        // Src blend mode LTBLEND_
    int             m_nDestBlend;       // Dest blend mode LTBLEND_
    uint32  m_psFlags;
	uint32			m_nEffectShaderID;
};


// A line system.
struct LSLinePoint
{
    LTVector    m_Pos;
    float       r, g, b, a;
};

struct LSLine
{
    LSLinePoint     m_Points[2];
    LineSystem      *m_pSystem;
    LSLine          *m_pPrev, *m_pNext;
};

class LineSystem : public LTObject
{
// Overrides.
public:

                        LineSystem();
    virtual             ~LineSystem();

    virtual void        Init(ObjectMgr *pMgr, ObjectCreateStruct *pStruct);

// Vars.
public:
    StructBank  *m_pLineBank;   // Where the lines come from.
    int         m_bChanged;         // Set when lines get added or moved.
    LSLine      m_LineHead;

    LTVector    m_MinPos, m_MaxPos; // Min and max positions (relative to object position).

    LTVector    m_SystemCenter;     // Centerpoint (relative to object position).
    float       m_SystemRadius;
};


// Implemented in SpriteControlImpl.cpp.
class SpriteControlImpl : public ILTSpriteControl
{
public:

    virtual LTRESULT    GetNumAnims(uint32 &nAnims);
    virtual LTRESULT    GetNumFrames(uint32 iAnim, uint32 &nFrames);

    virtual LTRESULT    GetCurPos(uint32 &iAnim, uint32 &iFrame);
    virtual LTRESULT    SetCurPos(uint32 iAnim, uint32 iFrame);

    virtual LTRESULT    GetFlags(uint32 &flags);
    virtual LTRESULT    SetFlags(uint32 flags);

    virtual LTRESULT    GetAnimLength(uint32 &msLen, const uint32 iFrame);
    virtual LTRESULT    GetFrameTextureHandle(HTEXTURE &hTex, const uint32 iAnim, const uint32 iFrame);

    SpriteInstance      *m_pSprite;
};


// Sprite instance.
class SpriteInstance : public LTObject
{
// Overrides.
public:

                    SpriteInstance();
    virtual         ~SpriteInstance();

    virtual float   GetRadius()
    {
        float baseMax;

        // Should make this a better number!
        //baseMax = (float)sqrt(256*256 + 256*256);
        baseMax = (float)363.0f;
        return baseMax * MAX(m_Scale.x, m_Scale.y);
    }


public:

    Sprite*         GetSprite() {return m_SpriteTracker.m_pSprite;}


// Vars.
public:

 
	SpriteTracker		m_SpriteTracker;
	HPOLY				m_ClipperPoly;	// Poly index if this sprite is clipped.
										// INVALID_HPOLY if not clipped.
	SpriteControlImpl	m_SCImpl;
	uint32				m_nEffectShaderID;
};


// PolyGrids!
class LTPolyGrid : public LTObject
{
// Overrides.
public:

                    LTPolyGrid();
    virtual         ~LTPolyGrid();

// Vars.
public:
    char            *m_Data;    // The grid data.
    unsigned short  *m_Indices; // The precalculated index list.

    Sprite          *m_pSprite;         // It might be using a sprite to update the texture frames.
    SpriteTracker   m_SpriteTracker;

    SharedTexture   *m_pEnvMap;

    float           m_xPan, m_yPan;
    float           m_xScale, m_yScale;
	float			m_fBaseReflection;
	float			m_fFresnelVolumeIOR;	//The index of refraction for the volume, used for generating fresnel data
	uint32			m_nEffectShaderID;

	uint32*			m_pValidMask;			//the pointer to the valid mask (NULL if none is used)
	uint32			m_nValidMaskWidth;		//width in uint32's of the valid mask

    uint32			m_nTris;
    uint32			m_nIndices;

	//polygrid specific flags
	uint32			m_nPGFlags;

    LTLink          m_LeafLinks;    // All the leaves it's in (LeafLinks).

    uint32  m_Width, m_Height;
    PGColor         m_ColorTable[256];
};


class Canvas : public LTObject
{
public:

                    Canvas();

    virtual float   GetRadius()     {return m_CanvasRadius;}

public:

    CanvasDrawFn    m_Fn;
    void            *m_pFnUserData;

    // Visibility radius (not clipping radius).
    float           m_CanvasRadius;
};


// A rendering effect within a volume.
class LTVolumeEffect : public LTObject
{
public:
                    LTVolumeEffect();
    virtual         ~LTVolumeEffect();

	virtual void    Init(ObjectMgr *pMgr, ObjectCreateStruct *pStruct);

	VolumeEffectInfo::EffectType m_EffectType;	// effect type for this volume

	// dynamic particle specific information
	SharedTexture* m_DPTexture;					// texture to be used on this particle system
	VolumeEffectDPUpdateFn m_DPUpdateFn;		// function that fills in the vertex buffer
	VolumeEffectInfo::DynamicParticlePrimitive m_DPPrimitive;	// primitive type to be used
	void* m_DPUserData;							// user data pointer passed back to the update function
	VolumeEffectInfo::DynamicParticleLighting m_DPLighting;		// lighting type to be used
	uint32 m_DPLightConstant;					// constant light value to be used if lighting is kConstant
	bool m_DPSaturate;							// true if saturated lighting (MODULATE2X) should be used
	uint32 m_nEffectShaderID;
};


// Record type.
// Used to catalog things that get added to the Register.  The Register is a list of things with unique id's..
struct LTRecord
{
    unsigned char   m_nRecordType;
    void *          m_pRecordData;
};

#define RECORDTYPE_LTOBJECT (1<<0)      // Record is a LTObject.
#define RECORDTYPE_SOUND    (1<<1)      // Record is a Sound.

// Setup the transformation for a WorldModel.
void obj_SetupWorldModelTransform(WorldModelInstance *pWorldModel);


#endif  // __DE_OBJECTS_H__

