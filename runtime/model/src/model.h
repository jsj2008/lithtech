
#ifndef __MODEL_H__
#define __MODEL_H__

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __LTINTEGER_H__
#include "ltinteger.h"
#endif

#ifndef __DYNARRAY_H__
#include "dynarray.h"
#endif

#ifndef __SYSDDSTRUCTS_H__
#include "sysddstructs.h"
#endif

#ifndef __RENDEROBJECT_H__
#include "renderobject.h"
#endif

#ifndef __STDLITH_H__
#include "stdlith.h"
#endif

#include <set>

class AnimTimeRef;
class LAlloc;

//------------------------------------------------------------------ //
// Defines.
//------------------------------------------------------------------ //

#define PIECELOD_BASE			0

#define MAX_GVP_ANIMS			8

#define MAX_CHILD_MODELS		32

#define MAX_PIECE_TEXTURES		4

#define MAX_PIECES_PER_MODEL 64

#define MODEL_FILE_VERSION		25

// the minimum supported file version.
#define MIN_MODEL_FILE_VERSION  23

// the minimum supported file version for OBBs.
#define MIN_MODELOBB_FILE_VERSION  24

// Model node types.
#define MNODE_ROTATIONONLY		(1<<1)	// Only use rotation info from animation data.

#define NODEPARENT_NONE			0xFFFFFFFF

#define MODELFLAG_CACHED		(1<<0)
#define MODELFLAG_CACHED_CLIENT (1<<1)

// enable the obb in the model db.
#define MODEL_OBB 1

class ModelNode;
class ModelAnim;
class AnimKeyFrame;
class Model;
class ModelLoadRequest;
class SharedTexture;
class ModelPiece;
class CDIModelPieceLOD;
struct LTB_Header;

// pRequest is a load request with everything setup except m_pFile.
// The filename specified only has to be looked for in the same directory that the parent model is in.
// ppModel is what you should fill in with the model you loaded.
// Returns LT_OK if the model loaded ok.
// Returns LT_NOCHANGE if it didn't load the model but it's not a fatal error.
// Any other return signals an error and means to stop loading.  It will return the given error.
typedef LTRESULT (*LoadChildFn)(ModelLoadRequest *pRequest, Model **pModel);

// Default implementation.. just returns LT_NOCHANGE.
LTRESULT DefaultLoadChildFn(ModelLoadRequest *pRequest, Model **pModel);

// ------------------------------------------------------------------------
// ModelLoadRequest
// ------------------------------------------------------------------------
class ModelLoadRequest
{
public:

						ModelLoadRequest()
						{
							m_pFilename = NULL;
							m_pFile = NULL;
							m_LoadChildFn = DefaultLoadChildFn;
							m_pLoadFnUserData = NULL;
							m_bLoadChildModels = true;

							m_bTreesValid = true;
							m_bAllChildrenLoaded = true;
						}

// Stuff to be set for Model::Load.
public:

	// The file to load from..
	ILTStream			*m_pFile;

	// The model loader will call this when it wants a child model.
	LoadChildFn			m_LoadChildFn;
	void				*m_pLoadFnUserData;


// Stuff automatically set for m_LoadChildFn.
public:

	// When m_LoadChildFn is called, this is set to the filename it wants.
	const char			*m_pFilename;

	// Don't touch this.  The model loader uses it so it doesn't recursively load children.
	bool				m_bLoadChildModels;


// Return info.
public:

	// If this is FALSE, there was an error loading a ChildInfo tree.  The model
	// still loaded but at least one of the ChildInfos has an invalid tree (and can't
	// be used unless it is rebuilt with SetupChildNodes).
	bool				m_bTreesValid;

	// If this is FALSE, then one or more child models couldn't load.
	bool				m_bAllChildrenLoaded;
};

// ------------------------------------------------------------------------
// ModelString
// Cell or node in model string list
// ------------------------------------------------------------------------
struct ModelString
{
	uint32					m_AllocSize;
	struct ModelString		*m_pNext;
	char					m_String[1];
};


// ------------------------------------------------------------------------
// CDefVertexLst
// This is a frame of animation for vertex animation
// (for now all vals are floats)
// ------------------------------------------------------------------------
class CDefVertexLst {
public :

	CDefVertexLst();
	CDefVertexLst(LAlloc *alloc):m_pAlloc(alloc) {;}
	CDefVertexLst( uint32 size, LAlloc *alloc );
	~CDefVertexLst();

	// pre set the size of the array
	void setSize( uint32 size , LAlloc *alloc );

	// size of the list
	uint32 size() const                          { return m_vVertexList.GetSize();}
	// memory size of data.
	uint32 mem_size() const						 { return m_vVertexList.GetSize() * sizeof(defVert) ; }

	// get value at index (pointer to actual data )
	// the value may not be continuous, so allways use getValue to get the next elem.
	const float* getValue( uint32 index ) const;

	// kludge/ accessor to raw data.
	float* getArray()                  { return (float*)m_vVertexList.GetArray(); }
	const float* getArray() const      { return (const float*)m_vVertexList.GetArray(); }

private :
	// internal type
	struct defVert {
		float pos[3];
	};

	// data array
	CMoArray<defVert, NoCache> m_vVertexList ;
	LAlloc                    *m_pAlloc ;
};


// ------------------------------------------------------------------------
// Anim Channels
//
// Anim channels are an array of sequencial data that maps to the keytimes
// associated with the animation these channels belong to.
// All these classes are private, viewable only by animnode.
// ------------------------------------------------------------------------
class IAnimPosChannel
{
public:
	virtual ~IAnimPosChannel() {}

	virtual uint32 GetDataSize() const = 0;
	virtual void GetData(const uint8* pData, uint32 index, LTVector& vPos ) const = 0;
};

class IAnimQuatChannel
{
public:
	virtual ~IAnimQuatChannel() {}

	virtual uint32 GetDataSize() const = 0;
	virtual void GetData(const uint8* pData, uint32 index, LTRotation& rRot ) const = 0;
};


// ------------------------------------------------------------------------
// animation pos channel with one value (default)
// ------------------------------------------------------------------------
class NULLPOSChannel : public IAnimPosChannel
{
public:

	static const IAnimPosChannel*	GetSingleton();

	virtual uint32 GetDataSize() const	{ return 0; }

    virtual void GetData(const uint8* pData, uint32 index, LTVector& vPos ) const
    {
		vPos.Init();
    }
};

class NULLQUATChannel : public IAnimQuatChannel
{
public:

	static const IAnimQuatChannel*	GetSingleton();

	virtual uint32 GetDataSize() const	{ return 0;	}

    virtual void GetData(const uint8* pData, uint32 index, LTRotation& rRot ) const
    {
		rRot.Init();
    }
};

// ------------------------------------------------------------------------
// full channels
// ------------------------------------------------------------------------
class POSChannel : public IAnimPosChannel
{
public:

	static const IAnimPosChannel*	GetSingleton();

	virtual uint32 GetDataSize() const	{ return sizeof(LTVector);	}

    virtual void GetData(const uint8* pData, uint32 index, LTVector& vPos ) const
	{
        memcpy(&vPos, pData + sizeof(LTVector) * index, sizeof(LTVector)) ;
    }
};

class SinglePOSChannel : public POSChannel
{
public:

	static const IAnimPosChannel*	GetSingleton();

    virtual void GetData(const uint8* pData, uint32 index, LTVector& vPos ) const
	{
		POSChannel::GetData(pData, 0, vPos);
    }
};

class QUATChannel : public IAnimQuatChannel
{
public:

	static const IAnimQuatChannel*	GetSingleton();

	virtual uint32 GetDataSize() const	{ return sizeof(LTRotation);	}

    virtual void GetData(const uint8* pData, uint32 index, LTRotation& rRot) const
    {
        memcpy(&rRot, pData + sizeof(LTRotation) * index, sizeof(LTRotation) );
    }
};

class SingleQUATChannel : public QUATChannel
{
public:

	static const IAnimQuatChannel*	GetSingleton();

    virtual void GetData(const uint8* pData, uint32 index, LTRotation& rRot ) const
	{
		QUATChannel::GetData(pData, 0, rRot);
    }
};

// ------------------------------------------------------------------------
// 16 bit channels w/out rle.
// ------------------------------------------------------------------------
class POS16Channel : public IAnimPosChannel
{
public:

	static const IAnimPosChannel*	GetSingleton();

	virtual uint32 GetDataSize() const	{ return sizeof(int16) * 3; }

    virtual void GetData(const uint8* pData, uint32 index, LTVector& vPos ) const
    {
        const float  kScale_1_11_4 = 	(1.0f/16.0f);

        const int16 *in_vec = (const int16*)(pData + index * sizeof(int16) * 3);

        vPos.x = float(in_vec[0]) * kScale_1_11_4;
        vPos.y = float(in_vec[1]) * kScale_1_11_4;
        vPos.z = float(in_vec[2]) * kScale_1_11_4;
    }
};

class SinglePOS16Channel : public POS16Channel
{
public:

	static const IAnimPosChannel*	GetSingleton();

    virtual void GetData(const uint8* pData, uint32 index, LTVector& vPos ) const
	{
		POS16Channel::GetData(pData, 0, vPos);
    }
};


class QUAT16Channel : public IAnimQuatChannel
{
public:

	static const IAnimQuatChannel*	GetSingleton();

	virtual uint32 GetDataSize() const	{ return sizeof(int16) * 4; }

    virtual void GetData(const uint8* pData, uint32 index, LTRotation& rRot) const
    {
		const float inv_dec = 1.0f / float(0x7fff);

        const int16 *in_vec = (const int16*)(pData + index * sizeof(int16) * 4);

        rRot.m_Quat[0] = (float)in_vec [0] * inv_dec ;
        rRot.m_Quat[1] = (float)in_vec [1] * inv_dec ;
        rRot.m_Quat[2] = (float)in_vec [2] * inv_dec ;
        rRot.m_Quat[3] = (float)in_vec [3] * inv_dec ;
    }
};

class SingleQUAT16Channel : public QUAT16Channel
{
public:

	static const IAnimQuatChannel*	GetSingleton();

    virtual void GetData(const uint8* pData, uint32 index, LTRotation& rRot ) const
	{
		QUAT16Channel::GetData(pData, 0, rRot);
    }
};

// ------------------------------------------------------------------------
// Anim Node with variable type animation channels.
// ------------------------------------------------------------------------
class AnimNode
{
friend class ModelAnim;

//standard construction
public:

						AnimNode();
						~AnimNode();

	void				Term();

//loading
public:

	// Loads the animation data for this node for this animation from disk. As parameters it
	//takes the file, the compression method, the node that the animation node correlates to,
	//and also the block of memory that has been allocated to store animation data in. It is
	//free to store a pointer to the block for reading from, and is in charge of moving the pointer
	//past its animation data.
	bool				Load(ILTStream &file, ModelAnim* pAnim, uint32 compression, uint8*& pAnimData);

//accessors for animation data
public:

	CDefVertexLst*		GetVertexData( uint32 frame )
	{
		ASSERT(m_pVertexChannel);
		return &m_pVertexChannel[frame];
	}

	void GetData(uint32 frame, LTVector & pos , LTRotation & rot ) const
	{
		ASSERT(m_pPosChannel);
		ASSERT(m_pQuatChannel);

		m_pPosChannel->GetData(m_pPosData, frame, pos);
		m_pQuatChannel->GetData(m_pQuatData, frame, rot);
	}

//internal utility functions
private:

	void SetupPosChannel(const IAnimPosChannel* pInterpreter, ILTStream& file, uint32 nElements, uint8*& pData);
	void SetupQuatChannel(const IAnimQuatChannel* pInterpreter, ILTStream& file, uint32 nElements, uint8*& pData);

	// animation channels.
	const IAnimPosChannel	*m_pPosChannel;
	const IAnimQuatChannel	*m_pQuatChannel;
	const uint8				*m_pPosData;
	const uint8				*m_pQuatData;
	CDefVertexLst			*m_pVertexChannel ;

};



// ------------------------------------------------------------------------
// AnimKeyFrame
// time -> value pair
// value is call back and/or string associated with time.
// ------------------------------------------------------------------------
class AnimKeyFrame
{
public:
					AnimKeyFrame() : m_Time(0), m_pString(NULL)		{}

	// the time in milliseconds that this keyframe occurs on
	uint32			m_Time;

	// A string of information about this key..
	const char*		m_pString;

};

// ------------------------------------------------------------------------
// ModelAnim
// Collection of animations associated with a model.
// ------------------------------------------------------------------------
class ModelAnim
{
friend class Model;
public:
					ModelAnim(Model *pModel);
					~ModelAnim();

	void			Term();

	const char*		GetName() const					{return m_pName;}

	Model*			GetModel()						{return m_pModel;}
	LAlloc*			GetAlloc();

	uint32			NumKeyFrames() const			{return m_KeyFrames.GetSize();}

	//accesses the individual animation nodes
	const AnimNode*	GetAnimNode(uint32 nNode) const	{ assert(m_pAnimNodes); return &m_pAnimNodes[nNode]; }
	AnimNode*		GetAnimNode(uint32 nNode)		{ assert(m_pAnimNodes); return &m_pAnimNodes[nNode]; }

	// Returns how long (in milliseconds) the animation is.
	uint32			GetAnimTime() const;

public:

	bool			Load(ILTStream &file, uint8*& pAnimData);

public:

	// Indexable array of pointers to anim nodes.
	AnimNode		*m_pAnimNodes;

	CMoArray<AnimKeyFrame, NoCache>	m_KeyFrames;

	// The time we interpolate into this animation
	uint32			m_InterpolationMS;

private:

	//this will recursively load animation nodes, traversing the model node heirarchy
	bool			LoadAnimNodesRecurse(ILTStream& file, uint32 nCompressionType, const ModelNode* pNode, uint8*& pAnimData);

	// Deletes the root node if it's not our default root node.
	void			DeleteRootNode();


private:

	// Always must be valid.  Use SetModel() to change.
	Model			*m_pModel;

	const char		*m_pName;
};


// A (skeletal) Model contains one ChildInfo for each child model it is bound to.
// The ChildModel structure contains the mapping from the child's skeleton
// to the parent's geometry along with the weights from the skeleton to the parent.
// NOTE: the parent itself has one of these to itself because it is used for all
//       skeletal animation.
class ChildInfo
{
public:

					ChildInfo();
					~ChildInfo();
	void			Term();

public:

	// How far into the parent's m_Anims list this model's anims start.
	uint32			m_AnimOffset;

	// The child model itself.  This will be NULL if the child model couldn't be loaded.
	Model			*m_pModel;
};



// The Model::m_Anims list contains these.
class AnimInfo
{
public:
					AnimInfo();

	// Save the anim info (dims and the animation itself).
	bool			Load(ILTStream &file, uint8*& pAnimData);

public:

	ModelAnim		*m_pAnim;


// Note: the data in here is stored in the parent model for each child model animation.
// (So if there's a base model and it has 2 parent models, each parent model has separate
// user dims and translation).
public:

	// USER dimensions.. settable in ModelEdit.
	LTVector			m_vDims;

	// Translation for this animation.
	LTVector			m_vTranslation;
};


// ------------------------------------------------------------------------
// ModelSocket
// ------------------------------------------------------------------------
class ModelSocket : public LTransform
{
public:
				ModelSocket();

	const char*	GetName() const	 {return m_pName;}

public:
	const char*		m_pName;
	uint32			m_iNode;	// Which node it's attached to.

};

// ------------------------------------------------------------------------
// ModelNode
// hierarchical organization of model. coordinate system for model
// ------------------------------------------------------------------------
class ModelNode
{
friend class Model;

public:
					ModelNode(Model *pModel);
					~ModelNode();

	void			Term();
	void			Clear();

	uint32			NumChildren() const			{return m_Children.GetSize();}
	ModelNode*		GetChild(uint32 index)		{return m_Children[index];}

	uint32			GetNodeIndex() const		{return m_NodeIndex;}
	uint32			GetParentNodeIndex() const	{return m_iParentNode;}

	const char*		GetName() const				{return m_pName;}

	Model*			GetModel()					{return m_pModel;}
	LAlloc*			GetAlloc();

	// Calculates the number of nodes in this subtree (including this one).
	uint32			CalcNumNodes() const;

	// Fills in Model::m_FlatNodeList and sets its transform index.
	bool			FillNodeList(uint32 &curNodeIndex);

	// Sets m_iParentNode and recurses.
	void			SetParent_R(uint32 iNode);

	// Loads the model from the compiled file
	bool			Load(ILTStream &file, const LTMatrix& mParentInvMat);

	// Access the global transform.
	const LTMatrix&		GetGlobalTransform() const			{return m_mGlobalTransform;}
	const LTMatrix&		GetInvGlobalTransform()	const		{return m_mInvGlobalTransform;}
	const LTMatrix&		GetFromParentTransform() const		{return m_mFromParentTransform;}

	// this should be SetGlobalTransform( const LTMatrix & mMat ) ...
	void	SetGlobalTransform(const LTMatrix& mMat, const LTMatrix& mInvParentMat)
	{
		m_mGlobalTransform		= mMat;
		m_mInvGlobalTransform	= ~mMat;
		m_mFromParentTransform	= mInvParentMat * mMat;
	}

public:

	// How far out along the parent's coordinate system is this node?
	// (~parent->m_GlobalTransform * m_GlobalTransform)
	LTVector		m_vOffsetFromParent;

	// The node index in all the lists.
	uint16			m_NodeIndex;
	uint8			m_Flags;
	uint8			m_Padding; // Structure padding..

	CMoArray<ModelNode*, NoCache>	m_Children;

	// NODEPARENT_NONE if none.
	uint32			m_iParentNode;


private:

	// For Model::m_DefaultRootNode.
					ModelNode();

	// The base global transform for this node.  This is setup from the first frame of
	// the first animation exported for this model, which is considered to be the
	// 'base' transform for the model (and it's what the skeletal->envelope relations
	// are setup from).  This transform is also used to relate child and parent model
	// skeletons so the parent model's skeleton can be moved around for larger characters.
	LTMatrix		m_mGlobalTransform;
	LTMatrix		m_mInvGlobalTransform;

	//This matrix specifies from the binding pose the transform from the parent's position/or
	//to the child
	LTMatrix		m_mFromParentTransform;

	Model			*m_pModel;
	const char		*m_pName;
};

// ----------------------------------------------------------------
//  base class for things that models can render
// ----------------------------------------------------------------
class CDIModelDrawable : public CRenderObject
{
public :

	CDIModelDrawable()
		: m_pUsedNodeList(NULL),
		  m_UsedNodeListSize(0),
		  m_pVertexShader(NULL),
		  m_pPixelShader(NULL)
	{
	}

	virtual ~CDIModelDrawable()
	{
		if (m_pUsedNodeList != NULL)
		{
			delete [] m_pUsedNodeList;
			m_pUsedNodeList = NULL;
			m_UsedNodeListSize = 0;
		}
	}

	virtual uint32			GetVertexCount() 									{ return 0; }
	virtual uint32			GetPolyCount()   									{ return 0; }

	virtual bool 			Load(ILTStream & file, LTB_Header &);

	// t.f fix for models version 21 and less. This should be removed for models ver 21<
	virtual void 			CalcUsedNodes( Model * ) { }

	void					CreateUsedNodeList( uint32 size )
	{
		if( m_pUsedNodeList != NULL )
			delete[] m_pUsedNodeList ;
		LT_MEM_TRACK_ALLOC( m_pUsedNodeList = new uint8[ size ] ,LT_MEM_TYPE_MODEL);
		m_UsedNodeListSize = size ;
	}

	uint8*					m_pUsedNodeList; // most distal nodes used by this mesh.
	uint8				 	m_UsedNodeListSize;

protected:

	LTVertexShader*			m_pVertexShader;	// temporary pointer to shader used during rendering
	LTPixelShader*			m_pPixelShader;		// temporary pointer to shader used during rendering
};

//  ----------------------------------------------------------------
//  Render Object Handle for RigidMesh
//  ----------------------------------------------------------------
class CDIRigidMesh : public CDIModelDrawable
{
public :
	CDIRigidMesh()			{ m_Type = CRenderObject::eRigidMesh; }
	virtual					~CDIRigidMesh() { }
};

//  ----------------------------------------------------------------
//  Render Object handle for Skeletally deformed mesh
//  ----------------------------------------------------------------
class CDISkelMesh  : public CDIModelDrawable
{
public :
	CDISkelMesh()			{ m_Type = CRenderObject::eSkelMesh; }
	virtual					~CDISkelMesh() { }
};

//  ----------------------------------------------------------------
//  Render Object handle for Skeletally deformed mesh
//  ----------------------------------------------------------------
class CDIVAMesh  : public CDIModelDrawable
{
public :
	CDIVAMesh()				{ m_Type = CRenderObject::eVAMesh; }
	virtual					~CDIVAMesh() { }
};


// ----------------------------------------------------------------
// ModelPiece
// ModelPiece manages the renderobject(s) that represents the geometry
// for the piece.
// ModelPiece maintains an LOD interface for managing that aspect of
// model piece duties.
// ----------------------------------------------------------------
class ModelPiece
{
public :

	ModelPiece(Model* pModel);
	~ModelPiece();

	const char*		GetName() const		{ return m_pName; }

	// Load
	bool			Load(ILTStream &file, LTB_Header& LTBHeader, uint32 nFileVersion);

	// appearance of piece
	uint32			m_nNumTextures;
	int32			m_iTextures[MAX_PIECE_TEXTURES];
	int32			m_iRenderStyle;
	uint8			m_nRenderPriority;

	// LOD interface
	uint32			NumLODs() const					{ return m_nLODs; }

	// Get lod based on index
	CDIModelDrawable* GetLOD(uint32 iLOD);
    CDIModelDrawable* GetLODFromDist( int32 nBias, float dist );
	uint32			  GetLODIndexFromDist( float fdist );

	// Creates a Device Dependent RenderOjbect.
	static CDIModelDrawable *CreateModelRenderObject( uint32 type );

private:
	Model*			m_pModel;	// "parent" container. Always valid
	const char*		m_pName;

	CDIModelDrawable **m_pRenderObjects; // lod levels

	void Term();  // object terminator/ deconstructor

	// LOD Info
	float			*m_pLODDists;	// array of lod distances
	uint32			 m_nLODs ;		// size of array
};


// Weight sets are created in order to tell the engine how to blend animations together.
// There is one weight for each node.
class WeightSet
{
public:
	WeightSet(Model *pModel);
	~WeightSet();

	bool			InitWeights(uint32 nWeights);

	bool			Load(ILTStream &file);
	bool			Save(ILTStream &file);

	const char*		GetName() const	{return m_pName;}

	inline Model*	GetModel()	{return m_pModel;}
	LAlloc*			GetAlloc();

public:
	const char*		m_pName;
	CMoArray<float>	m_Weights;
	Model			*m_pModel;
};

#if(MODEL_OBB)

// ------------------------------------------------------------------------
// model obb
// Model Oriented Bounding Box type.
// ------------------------------------------------------------------------
//class ModelOBB
//{
//public:
	//LTVector		m_Pos;		// local pos or offset.
	//LTVector		m_Size ;	// box size
	//LTVector		m_Basis[3]; // orientation.
	//uint32			m_iNode ;	// which coordinate frame do I belong to?
//
	//bool RayIntersect( const LTVector &origin, const LTVector &dir, float &t1 );
	//bool OBBOverlap( const ModelOBB &other );
//};

#endif // MODEL_OBB


// ----------------------------------------------------------------
// All a model's strings are stored in here, since there are
// tons of redundant strings (like "").
class ModelStringList
{
public:

					ModelStringList(LAlloc *pAlloc=&g_DefAlloc);
					~ModelStringList();

	// Free all strings.
	void			Term();

	// Add a new string.  Checks for duplicates.
	const char*		AddString(const char *pStr);

	// Change the allocator.  You can NOT change it if strings have already been
	// allocated into here.
	bool			SetAlloc(LAlloc *pAlloc);


protected:

	inline LAlloc*	GetAlloc()	{return m_pAlloc;}


public:

	ModelString		*m_StringList;


protected:

	LAlloc			*m_pAlloc;
};

// ------------------------------------------------------------------------
// ModelMgr
// Manages current instances of model in engine.
// ------------------------------------------------------------------------
class CModelMgr {
		friend class Model ;
public :
	CModelMgr();
	~CModelMgr();

	Model*			Find( const char *filename );
	Model*			Find( uint16 file_id );

	// call this when a file is loaded.
	bool			Add( Model *pModel );

	// apply this func for each model.
	void			ForEach( void (*fn)(const Model &pModel, void *user_data), void *user_data);

	void			UncacheServerModels();
	void			UncacheClientModels();

private :

bool			Remove( Model *pModel );
	std::set<Model*> m_Models;
};



// ------------------------------------------------------------------------
// Model
// Container that holds geometric, material and animation information.
// The data contained represents data for character models as well as props
// and things.
// ------------------------------------------------------------------------
class Model
{

public:
					Model(LAlloc *pAlloc=&g_DefAlloc, LAlloc *pDefAlloc=&g_DefAlloc);
					~Model();


	uint32			GetFileVersion() const { return m_FileVersion ; }
	void			Term();


	// Deletes all the animations.
	void			TermAnims();

	// Deletes and removes references to the child models.
	void			TermChildModels();

	// Reference counting (not thread-safe)
	uint32			AddRef() { m_RefCount++; return m_RefCount; }
	uint32			Release() { uint32 tmp = m_RefCount-- ; if(m_RefCount == 0) delete this ; return tmp;}
	uint32			GetRefCount() const { return m_RefCount; }   // for debugging.

	// Access the root node.
	ModelNode*		GetRootNode() {return m_pRootNode;}

	// The way the lists are built, the root node is always node 0.
	static bool		IsRootNode(uint32 iNode) {return iNode == 0;}

	// Get the allocators this model uses.
	inline LAlloc*	GetAlloc()		{return m_pAlloc;}
	inline LAlloc*	GetDefAlloc()	{return m_pDefAlloc;}

	uint32			NumAnims() const {return m_Anims.GetSize();}

	// Just gets the anim, skips past the AnimInfo.
	ModelAnim*		GetAnim(uint32 index) {return m_Anims[index].m_pAnim;}

	AnimInfo*		GetAnimInfo(uint32 index) {return &m_Anims[index];}

	// anim Weight sets..
	uint32			NumWeightSets() const	{ return m_WeightSets.GetSize(); }
	WeightSet*		GetWeightSet(uint32 i)	{ return m_WeightSets[i]; }

	// Precalculate the lists of nodes (and figures out how many there are).
	// This should be called whenever a node is added or removed.
	bool			PrecalcNodeLists(bool bRebuild=true);

	ModelNode*		FindNode(const char *pName, uint32 *index=NULL);
	uint32			NumNodes() const {return m_FlatNodeList.GetSize();}
	ModelNode*		GetNode(uint32 i) {return m_FlatNodeList[i];}

	uint32			NumPieces() const {return m_Pieces.GetSize();}
	ModelPiece*		GetPiece(uint32 i) {return m_Pieces[i];}
	ModelPiece*		FindPiece(const char *pName, uint32 *index=NULL);
	WeightSet*		FindWeightSet(const char *pName, uint32 *index=NULL);

	// Add a string to the model's list.
	const char*		AddString(const char* pStr);

	// Find the parent of a given node.  Starts recursing at pRoot.
	// Fills in index if it's non-NULL.
	ModelNode*		FindParent(ModelNode *pNode, ModelNode *pRoot, uint32 *index=NULL);

	// Find an animation by name.
	ModelAnim*		FindAnim(const char *pName, uint32 *index=NULL, AnimInfo **ppInfo=NULL);

	// Find an animation with the given model.
	AnimInfo*		FindAnimInfo(const char *pAnimName, Model *pOwner, uint32 *index=NULL);

	// Load/save.
	bool			LoadString(ILTStream &file, const char* &pStr);

	bool			LoadAnimBindings(ModelLoadRequest *pRequest, ILTStream &file, bool bAllowUpdates);
	bool			LoadSockets(ILTStream &file);
	bool			LoadWeightSets(ILTStream &file);
	LTRESULT		Load(ModelLoadRequest *pRequest, const char* pFilename);

	// Figures out how many animations are in the child models.
	uint32			CalcNumChildModelAnims();

	// Returns how many animations in our m_Anims list come from this model.
	uint32			CalcNumParentAnims() const;

	// Parses the command string.
	void			ParseCommandString();

	// Access the filename.
	const char*		GetFilename() const ;
	bool			SetFilename(const char *pFilename);
	void			FreeFilename(); // Free m_pFilename.

// Functions to access child models.
// NOTE: there is ALWAYS at least one child model and the first child model in the
// list is always m_SelfChildModel.
public:

	uint32			NumChildModels() const {return m_nChildModels;}
	ChildInfo*		GetChildModel(uint32 index) {assert(index < m_nChildModels); return m_ChildModels[index];}

	// Returns if the child model's tree is valid.
	bool			VerifyChildModelTree(Model *pChild, ModelNode* &pErrNode);

	// Used by ModelEdit.. shouldn't be used at runtime.  Adds a new ChildModel
	// with the specified info.
	ChildInfo*		AddChildModel( Model *pChild,
								   const char *pFilename,
								   char *pErrStr, uint32 nErrStrLen);

	// Find childmodels
	ChildInfo*		FindChildModel(Model *pModel, uint32 &out_index);

	// Just initializes the model and sets our m_ChildModel[index] = pChildModel.
	bool			InitChildInfo(uint32 index, ChildInfo *pChildModel, Model *pModel, const char *pFilename);

	// Sets ModelNode::m_vOffsetFromParent (and calls it on the children).
	void			SetNodeParentOffsets();

private:

	// These are used during precalculation.
	bool			BuildFlatNodeList(bool bRebuild=true);


	// Not always set.. the engine uses this to determine if it has already
	// been loaded.  Use Get/SetFilename.
	char			*m_pFilename;


public:

	// file id associated with this data.
	uint32			m_FileID;		// Used by server.
	uint32			m_Flags;		// MODELFLAG flags above.  Used by various things.


// Geometry data.

	// The node list, generated by PREORDER traversal of the node tree.
	CMoArray<ModelNode*, NoCache>	m_FlatNodeList;

	// Geometry pieces.. these are where the verts and tris are stored.
	CMoArray<ModelPiece*, NoCache>	m_Pieces;

	// Weight sets..
	CMoArray<WeightSet*, NoCache>	m_WeightSets;

	// The command string for the model.  This is used for sequences of commands
	// that get stored with the model (like LOD ranges...)

    // ??? is this useful after the model has been loaded couldn't we just have tokens somewhere
    // that tells what the model's render state is
	const char		*m_CommandString;

	ModelStringList	m_StringList;


// Stuff contained in the command string.
public:

	float			m_VisRadius;		// Defaults to DEFAULT_MODEL_VIS_RADIUS or can be
										// overridden in the command string.
	// Shadow stuff.
	bool			m_bShadowEnable;

// Sockets.
public:

	uint32			NumSockets() const {return m_Sockets.GetSize();}
	ModelSocket*	GetSocket(uint32 index) {return m_Sockets[index];}
	ModelSocket*	FindSocket(const char *pName, uint32 *index=NULL);

// Animation data.
public:

	// Note: with skeletal models, some of these anims may not be from this model,
	// it should never assume that they are.
	CMoArray<AnimInfo, NoCache>		m_Anims;


// Misc.
public:

	// m_pAlloc = who's doing our BLOCK allocations.  Most allocations can go through
	// here, but some must go thru m_pDefAlloc if we can't predict ahead of time how
	// many allocations there will be.
	LAlloc							*m_pAlloc;
	LAlloc							*m_pDefAlloc;

	// At runtime, models are loaded using this.
	LAllocSimpleBlock				m_BlockAlloc;

#if(MODEL_OBB)
// obb
public :
	uint32			GetNumOBB();
	// GetCopyOfOBBSet copies the current set in to preallocated array.
	void			GetCopyOfOBBSet( ModelOBB *RetModelOBB );
#endif

// Child model list.
private:

	ChildInfo*		GetSelfChildModel()		{ return m_ChildModels[0]; }

	// The list of child models.
	// The model itself (m_SelfChildModel) is ALWAYS first.
	ChildInfo*						m_ChildModels[MAX_CHILD_MODELS];
	uint32							m_nChildModels;

	// Saves an allocation.
	ChildInfo						m_SelfChildModel;


private:

	// Set when it starts reading so it can track what file version it's reading in.
	uint32							m_FileVersion;

	//the data for the animations
	uint8*							m_pAnimData;

	// Reference count.. shouldn't be freed unless this is 0.
	uint32							m_RefCount;

	// Default root node.. setup so it can be subclassed.
	ModelNode						m_DefaultRootNode;
	ModelNode						*m_pRootNode;

	// socket list.
	CMoArray<ModelSocket*,NoCache>	m_Sockets;

#if(MODEL_OBB)
 	// Oriented Bounding boxes for this model.
	uint32							m_NumOBBs ; // number of obbs
	ModelOBB						*m_ModelOBBs ;// array of obbs.
#endif

};


//------------------------------------------------------------------ //
// Inlines.
//------------------------------------------------------------------ //

inline LAlloc* ModelAnim::GetAlloc()
{
	return GetModel()->GetAlloc();
}

inline LAlloc* ModelNode::GetAlloc()
{
	return GetModel()->GetAlloc();
}

inline CDIModelDrawable* ModelPiece::GetLOD(uint32 i)
{
	if(i < m_nLODs)
		return m_pRenderObjects[i];

	// Invalid LOD index.
	assert(false);
	return NULL;
}


// get the lod from dist, but bias the lod picked based on min/max values.
inline CDIModelDrawable* ModelPiece::GetLODFromDist( int32 nBias, float dist )
{
	// last valid index
    uint32 num_lods = m_nLODs - 1;
    int32 nLOD = 0;

	if( dist > 0.0f )
    {
		// return the last one if we fall off the scale
        nLOD = num_lods;

		//see if we fall within an LOD range, if so, use that LOD
        for( uint32 i = 0 ; i < num_lods ; ++i )
        {
            if( dist < m_pLODDists[i+1] )
            {
                nLOD = i;
				break;
            }
        }
    }

	//add in our bias to the lod, and make sure it is within range
	nLOD = LTCLAMP(nLOD + nBias, 0, num_lods);

    // return our lod! yay! we saved the world!
    return m_pRenderObjects[nLOD];
}

// ------------------------------------------------------------------------
// GetLODIndexFromDist( dist_from_model )
// returns lod-index for that distance value.
// ------------------------------------------------------------------------
inline uint32  ModelPiece::GetLODIndexFromDist( float dist )
{
	if( dist >= 0 )
	{
		uint32 num_lods = m_nLODs -1 ;
		for(uint32 i = 0 ; i < num_lods  ; i++ )
		{
			if( dist < m_pLODDists[i+1] )
			{
				return i;
			}
		}
		// return the last one if we fell out of scale
		return num_lods ;
	}
	else // return the first lod
	{
		return 0;
	}
}

inline LAlloc* WeightSet::GetAlloc()
{
	return GetModel()->GetAlloc();
}

// ------------------------------------------------------------------------
// Unique ModelMgr
// -----------------------------------------------------------------------
extern CModelMgr g_ModelMgr ;

#endif  // __MODEL_H__








