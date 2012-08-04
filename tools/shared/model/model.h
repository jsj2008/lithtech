

#ifndef __MODEL_H__
#define __MODEL_H__
#pragma warning (disable:4786)

    #ifndef __BDEFS_H__
    #include "bdefs.h"
    #endif

    #ifndef __QUATERNION_H__
    #include "ltrotation.h"
    #endif

    #ifndef __LTANIMTRACKER_H__
    #include "ltanimtracker.h"
    #endif

    #ifndef __MODELALLOCATIONS_H__
    #include "modelallocations.h"
    #endif

	// stl goodies.
	#include <map>
	#include <string>
	#include <vector>   

	// How the vertex weighting works:
	// M = skeleton transform for the given frame
	// r = vertex->skeleton relation for this vertex (vertex->skeleton)
	// v = original (global) vertex position for this vertex
	// w = envelope weight for this vertex (vertex->skeleton)
	// offset = ((M * r) - v) * w
	// offset = (w*(M * r) - w*v)
	// a = w * r  (for each vertex)...
	// b = w * v
	// offset = M * a - b
	// The full equation, with 3 bones
	// n = v + (M1 * a1 - b1) + (M2 * a2 - b2) + (M3 * a3 - b3)
	// s = v - b1 - b2 - b3
	// n = s + M1*a1 + M2*a2 + M3*a3



	//------------------------------------------------------------------ //
	// Defines.
	//------------------------------------------------------------------ //

	#define PIECELOD_BASE			0

	#define MAX_GVP_ANIMS			8

	#define MAX_PIECENAME_LEN		32
	#define MAX_SOCKETNAME_LEN		16
	#define MAX_CHILD_MODELS		32
	#define MAX_WEIGHTSETNAME_LEN	16

	#define MAX_PIECE_TEXTURES		4

	#define MODEL_FILE_VERSION		25

	// Model node types.
	#define MNODE_REMOVABLE		(1<<0)	// This node can be removed.
	#define MNODE_ROTATIONONLY	(1<<1)	// Only use rotation info from animation data.

	// Key types.
	#define KEYTYPE_POSITION	0
	#define KEYTYPE_CALLBACK	1

	#define VC_NOTUSED	((uint32)0xFFFFFFFF)
	#define SN_NOTUSED	((uint32)0xFFFFFFFF)

	#define NODEPARENT_NONE		0xFFFFFFFF
	
	#define MODELFLAG_TOUCHED	(1<<0)	// Used by client.
	#define MODELHEADER_SIZE	8
	
	// ------------------------------------------------------------------------
	// oriented bounding box def.
	// ------------------------------------------------------------------------
	struct SOBB  {
		LTVector m_Pos ;
		LTVector m_Size ; // m_Dim ; 
		LTRotation m_Orientation;

		// defaults
		SOBB():m_Pos(0.0f,0.0f,0.0f), m_Size(1.0f,1.0f,1.0f){ m_Orientation.Init();}
		// copy contstr
		SOBB(const SOBB &o ):m_Pos(o.m_Pos),m_Size(o.m_Size), m_Orientation(o.m_Orientation){}

	};

	class ModelNode;
	class ModelAnim;
	class AnimKeyFrame;
	class Model;
	class ModelLoadRequest;
	class SharedTexture;
	class TransformMaker;
	class ModelPiece;
	class PieceLOD ;
	class CDIModelPieceLOD;
	struct LTB_Header;
	
	typedef unsigned char KeyType;
	typedef void (*KeyCallback)(void *pTracker, AnimKeyFrame *pFrame);


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
	// GVPStruct 
	// render/animation set up structure. Sets up what is sent down to renderer.
	// ------------------------------------------------------------------------
	class GVPStruct
	{
	public:
					GVPStruct()
					{
						m_Vertices = NULL;
						m_VertexStride = 0;
						Mat_Identity(&m_BaseTransform);
						m_nAnims = 1;
						m_iLOD = PIECELOD_BASE;
					}

		
		// What frame we're asking for.
		AnimTimeRef	m_Anims[MAX_GVP_ANIMS];
		uint32		m_nAnims;

		// Here is where the vertices go.  There must be room in here for all the vertices in the model.
		// The vertices are output in order of the m_FlatNodeList nodes (preorder traversal).
		// m_DestVertices points at the first LTVector.  m_VertexStride tells how many
		// bytes are in between each vertex.
		void		*m_Vertices;
		uint32		m_VertexStride;

		// Global transform applied to all verts.
		LTMatrix		m_BaseTransform;

		// What LOD to get the vertex positions for.
		uint32		m_iLOD;
		float		m_CurrentLODDist ;
	};


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
								m_bLoadChildModels = TRUE;
								
								m_bTreesValid = TRUE;
								m_bSaveCountValid = TRUE;
								m_bAllChildrenLoaded = TRUE;
							}

		bool				IsValid() {return m_pFilename && m_pFile && m_LoadChildFn;}


	// Stuff to be set for Model::Load.
	public:
		
		// The file to load from..
		ILTStream				*m_pFile;
		
		// The model loader will call this when it wants a child model.
		LoadChildFn			m_LoadChildFn;
		void				*m_pLoadFnUserData;


	// Stuff automatically set for m_LoadChildFn.
	public:

		// When m_LoadChildFn is called, this is set to the filename it wants.
		char				*m_pFilename;

		// Don't touch this.  The model loader uses it so it doesn't recursively load children.
		bool				m_bLoadChildModels;


	// Return info.
	public:

		// If this is FALSE, there was an error loading a ChildInfo tree.  The model 
		// still loaded but at least one of the ChildInfos has an invalid tree (and can't
		// be used unless it is rebuilt with SetupChildNodes).
		bool				m_bTreesValid;

		// If this is FALSE, then one of the child models is out of date.
		bool				m_bSaveCountValid;

		// If this is FALSE, then one or more child models couldn't load.
		bool				m_bAllChildrenLoaded;
	};

	// ------------------------------------------------------------------------
	// ModelString
	// Cell or node in model string list
	// ------------------------------------------------------------------------
	struct ModelString_t
	{
		uint32					m_AllocSize;
		struct ModelString_t	*m_pNext;
		char					m_String[1];
	} ;
	// backwards compat
	typedef ModelString_t ModelString;

	// ------------------------------------------------------------------------
	//  ModelSphere
	// ------------------------------------------------------------------------
	struct ModelSphere_t
	{
		LTVector	m_Center;
		float	m_Radius;
	} ;
	// backwards compat
	typedef ModelSphere_t ModelSphere;

	// ------------------------------------------------------------------------
	// NewVertexWeight
	// xyz in iNode's local space + weight + matrix 
	// a vertex to matrix association. for doing skeletal deformation
	// ------------------------------------------------------------------------
	class NewVertexWeight
	{
	public:
				NewVertexWeight();
	public:
		float	m_Vec[4];
		uint32	m_iNode;
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
		CDefVertexLst( int size, LAlloc *alloc );
		~CDefVertexLst();

		// pre set the size of the array
		void setSize( int size , LAlloc *alloc );
		
		// size of the list
		int  size()                                  { return m_vVertexList.GetSize();}
		// memory size of data.
		int mem_size() 								 { return m_vVertexList.GetSize() * sizeof(defVert) ; }
		
		// get value at index (copy of data )
		void getValue( int index, float val[3] ) ;   
		
		// get value at index (pointer to actual data )
		// the value may not be continuous, so allways use getValue to get the next elem.
		float* getValue( int index );
		
		// kludge/ accessor to raw data.
		float* getArray()                  { return (float*)m_vVertexList.GetArray(); }
		
		// add values at the end of the list
		void append( float val[3] );
		void append( float x, float y, float z);

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
	// ModelVert
	// Model Vertex. encompasses weight values base pos, and normal
	// ------------------------------------------------------------------------
	class ModelVert
	{
	public:
							ModelVert();

		NewVertexWeight*	AddWeight();


	public:

		NewVertexWeight	*m_Weights;
		uint16			m_nWeights;
		uint16			m_iReplacement; // This tells what vertex in the next LOD model
										// this vertex ends up collapsing into.  This is
										// used while transitioning the model between LODs.
										// NOTE: this is not saved.  It is only used by the
										// LOD generation code.  Xbox code was using it, but
										// it is effectively dead.

		// Vertex position.
		LTVector			m_Vec;

		// The vertex normal.
		LTVector			m_Normal;
	};


	typedef struct UVPair_t
	{
		float tu, tv;
	} UVPair;


	class ModelTri
	{
	public:
		
		void			operator=(const ModelTri &other)
		{
			uint32 i;
			for(i=0; i < 3; i++)
			{
				m_Indices[i] = other.m_Indices[i];
				m_UVs[i] = other.m_UVs[i];
			}
		}
		
		unsigned short	m_Indices[3];
	
		UVPair			m_UVs[3];
        LTVector        m_Normals[3];
        LTVector        m_Colors[3];
	};

	// ------------------------------------------------------------------------
	// NodeKeyFrame
	// data for AnimNode animation data 
	// ------------------------------------------------------------------------
	class NodeKeyFrame
	{
	public:
		NodeKeyFrame() :m_pDefVertexLst(NULL)  {
			m_Translation.Init(0.0f,0.0f,0.0f);
			m_Quaternion.Init( 0.0f,0.0f,0.0f,1.0f);
		}

		void		operator=(const NodeKeyFrame &other)
		{
			m_Translation = other.m_Translation;
			m_Quaternion = other.m_Quaternion;
			// shallow copy
			m_pDefVertexLst = other.m_pDefVertexLst ;
		}
		
		void		ConvertToMatrix(LTMatrix &mat)
		{
			m_Quaternion.ConvertToMatrix(mat);
			mat.m[0][3] = m_Translation.x;
			mat.m[1][3] = m_Translation.y;
			mat.m[2][3] = m_Translation.z;
		}
		
		void		ConvertFromMatrix(LTMatrix &mat)
		{
			m_Quaternion.ConvertFromMatrix(mat);
			m_Translation.x = mat.m[0][3];
			m_Translation.y = mat.m[1][3];
			m_Translation.z = mat.m[2][3];
		}

		void		Identity()
		{
			m_Quaternion.Init();
			m_Translation.Init();
		}
			
		
		LTVector		m_Translation;
		LTRotation		m_Quaternion;
		uint32			m_Time ;
	
		// vertex animation data.
		CDefVertexLst * m_pDefVertexLst ;
	};

	// ------------------------------------------------------------------------
	// AnimNode
	// represents a collection of positions/rotations in time associated with 
	// a model node.
	// ------------------------------------------------------------------------
	class AnimNode
	{
	friend class ModelAnim;

	public:

							AnimNode(ModelAnim *pAnim, AnimNode *pParent);
		virtual				~AnimNode();
		
		void				Term();
		void				Clear();

		uint32				NumChildren() {return m_Children.GetSize();}
		AnimNode*			GetChild(uint32 index) {return m_Children[index];}

		Model*				GetModel();
		LAlloc*				GetAlloc();

		// Used while precalculating lists..
		bool				FillAnimNodeList(uint32 &curNodeIndex);
		bool				SetNodePointers(ModelNode *pNode);

		uint32				NumKeyFrames();	


	// Clipboard functions.
	public:

		// (This creates the tree in the dest anim node and stores the specified keyframe in it).
		bool				CopyKeyFrame(uint32 iKeyFrame, AnimNode *pDest);

		// Adds pSrc's first keyframe after the specified frame.
		bool				PasteKeyFrame(AnimNode *pSrc, uint32 iBefore);

		// Removes the specified keyframe (and recurses).
		bool				RemoveKeyFrame(uint32 iKeyFrame);

		// Save	
		bool				Save(ILTStream &file, uint32 compression_type, uint32& nAnimDataSize);

		// if there's a peice associated with this its a vertex anim
		bool				isVertexAnim() { return m_iPiece >= 0 ; }

		// the target for a vertex animation is a peice. the piece is in class model.
		void				SetVertexAnimTarget( int ival ) { m_iPiece = ival ; }
		int					GetVertexAnimTarget()           { return m_iPiece ; }
		
		// get the size of the vertex animation for this node. returns 0 if there
		// are none.
		int					GetVertexAnimMemSize() ; 
		// get all the vert anim size for anim hierarchy.
		int					GetTotalVertexAnimMemSize();
		
	// Overridables.	
	public:

		virtual ModelAnim*	GetAnim() {return m_pAnim;}
		virtual void		SetAnim(ModelAnim *pAnim);
		virtual AnimNode*	AllocateSameKind(ModelAnim *pAnim, AnimNode *pParent);

		
	public:
		
		// Points to its model node.
		ModelNode			*m_pNode;

		NodeKeyFrame *GetKeyFrame( uint32 time );		

		// Parent animnode.  Root has NULL.
		AnimNode			*m_pParentNode;

		// Number of children is m_pNode->m_nChildren.
		CMoArray<AnimNode*,NoCache>	m_Children;

		// private 
		// Number of keyframes is m_pAnim->m_nKeyFrames.
		CMoArray<NodeKeyFrame,NoCache>	m_KeyFrames;
		uint32				   m_LastKF ;
	
	protected :
		// data pertaining to vertex anim

		// piece that this anim node is associated with if its a vertex animation
		// node. If there is no vertex animation the piece value is -1
		int32                  m_iPiece; 

		
		
	private:

		// For ModelAnim::m_DefaultRootNode.
							AnimNode();

		// Always must be valid.
		ModelAnim			*m_pAnim;
	};

	
	// ------------------------------------------------------------------------
	// AnimKeyFrame
	// time -> value pair 
	// value is call back and/or string associated with time.
	// ------------------------------------------------------------------------
	class AnimKeyFrame
	{
	public:
						AnimKeyFrame();
		
		uint32	m_Time;

		// A string of information about this key..
		char*			m_pString;
		
		KeyType			m_KeyType;
		KeyCallback		m_Callback;
		
		// User data for the callback.
		void			*m_pUser1;
	};

	// ------------------------------------------------------------------------
	// ModelAnim
	// Collection of animations associated with a model.
	// ------------------------------------------------------------------------
	class ModelAnim
	{
	friend class Model;
	public:
		enum ANIMCOMPRESSIONTYPE { 
			NONE=0, RELEVANT, RELEVANT_16, RELEVANT_ROT16_ONLY };

						ModelAnim(Model *pModel);
		virtual			~ModelAnim();

		void			Term();

		inline uint32	NumKeyframes()	{return m_KeyFrames.GetSize();}

		// Access the root node.
		AnimNode*		GetRootNode()					{return m_pRootNode;}
		void			SetRootNode(AnimNode *pNode);

		char*			GetName()	{return m_pName;}
		void			SetName(const char *pName);

		Model*			GetModel() {return m_pModel;}
		LAlloc*			GetAlloc();

		uint32			NumKeyFrames() {return m_KeyFrames.GetSize();}
	
		bool			PrecalcNodeLists(bool bRebuild=TRUE);

		// Returns how long (in milliseconds) the animation is.
		uint32			GetAnimTime();

		// Copy the animation
		bool			CopyAnim(ModelAnim *pSrc);
		bool			CopyAnimNode( AnimNode* pSrc, AnimNode* pDest );

	// 'Clipboard' functions.
	public:

		// These can be used to move keyframes around.
		ModelAnim*	CopyKeyFrame(uint32 iKeyFrame);
		ModelAnim*	CutKeyFrame(uint32 iKeyFrame);
		bool			PasteKeyFrame(ModelAnim *pFrame, uint32 iBefore);

		// Remove the specified keyframe.  There must be at least 3 keyframes in the animation
		// for this to work.
		bool			RemoveKeyFrame(uint32 iKeyFrame);

		// Load/save.
		bool			Load(ILTStream &file);
		bool			Save(ILTStream &file, uint32& nAnimDataSize);


	// Overridables.
	public:
		
		virtual void	SetModel(Model *pModel);
		virtual ModelAnim*	AllocateSameKind(Model *pModel);


	// Used by Model::PrecalculateLists.
	private:

		bool			BuildAnimNodeList(bool bRebuild=TRUE);

		
	public:

		// Indexable array of pointers to anim nodes.
		AnimNode		**m_AnimNodes;
		
		CMoArray<AnimKeyFrame, NoCache>	m_KeyFrames;

		// Weight set ModelEdit uses while stacking anims.
		uint32			m_ModelEditWeightSet;

		// The time we interpolate into this animation
		uint32			m_InterpolationMS;
		
		// The compression method we are going to save the 
		// binary data with.
		uint32			m_CompressionType ;
	private:

		// Deletes the root node if it's not our default root node.
		void			DeleteRootNode();
		
	
	private:

		// Always must be valid.  Use SetModel() to change.
		Model			*m_pModel;
		
		char			*m_pName;

		AnimNode		m_DefaultRootNode; // Default root node.
		AnimNode		*m_pRootNode;
	
		uint16			*m_pLUTWorkBuffer ; // used in saving.

	};


	// Used to represent position/rotation for some stuff.
	// basic node transform
	class ModelTransform
	{
	public:
		void		ConvertToMatrix(LTMatrix &mat)
		{
			m_Rot.ConvertToMatrix(mat);
			mat.SetTranslation(m_Pos);
			mat.Scale(m_Scale.x, m_Scale.y, m_Scale.z);
		}

		void		ConvertToMatrixNoScale(LTMatrix &mat)
		{
			m_Rot.ConvertToMatrix(mat);
			mat.SetTranslation(m_Pos);
		}
		
		void		ConvertFromMatrix(LTMatrix &mat)
		{
			m_Scale = mat.GetScale();			
			//mat.Scale(1.0f/m_Scale.x,1.0f/m_Scale.y,1.0f/m_Scale.z); 
			// check for zero scale, if so don't divide by zero ok?
			mat.Scale(
				(m_Scale.x?1.0f/m_Scale.x:0.0f),  
				(m_Scale.y?1.0f/m_Scale.y:0.0f),
				(m_Scale.z?1.0f/m_Scale.z:0.0f)
				); 
			m_Rot.ConvertFromMatrix(mat);
			mat.GetTranslation(m_Pos);
		}

		void ConvertFromMatrixNoScale(LTMatrix &mat)
		{
			m_Rot.ConvertFromMatrix(mat);
			mat.GetTranslation(m_Pos);
		}

		LTVector		m_Pos;
		LTVector		m_Scale ;
		LTRotation		m_Rot;
	};


	// Just a position/rotation.
	class CIRelation : public ModelTransform
	{
	public:

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

		uint32			GetSaveIndex()	{return m_SaveIndex;}

		bool			Load(ILTStream &file);
		bool			Save(ILTStream &file);

		// Before depending on the bounding radius, this should be called to make sure it's valid.
		bool			IsBoundRadiusValid() {return m_bBoundRadiusValid;}

		LAlloc*			GetAlloc();


	public:

		// Relation matrices... used because the original skeleton points can
		// be moved for different parent models.
		CMoArray<CIRelation>	m_Relation;

		// Tells if the animation bounding radii are valid.
		bool			m_bBoundRadiusValid;

		// How far into the parent's m_Anims list this model's anims start.
		uint32			m_AnimOffset;

		// This is what it uses while loading to do its callback to load the child model.
		char			*m_pFilename;

		// All models have a # that's incremented each time they're saved.  This is saved
		// when the ChildModel is first created and if it's wrong (the child model file
		// has been changed), the animation dims must be recalculated.
		uint32			m_SaveIndex;

		// The parent model.
		Model			*m_pParentModel;

		// The child model itself.  This will be NULL if the child model couldn't be loaded.
		Model			*m_pModel;

		// Set to FALSE if there is an error while loading the model.
		bool			m_bTreeValid;
	};



	// The Model::m_Anims list contains these.
	class AnimInfo
	{
	public:
					AnimInfo();
		
		// Returns the model this animation comes from.
		// Returns NULL if something's wrong.
		Model*		GetAnimOwner();

		// Save the anim info (dims and the animation itself).
		bool		Load(ILTStream &file);
		bool		Save(ILTStream &file, uint32& nAnimDataSize);

		// Returns TRUE if this animation comes from the parent model (ie: if it
		// can be edited in ModelEdit).
		bool		IsParentModel()	{return m_pChildInfo->m_pParentModel == m_pChildInfo->m_pModel;}

	public:
		
		ModelAnim		*m_pAnim;
		ChildInfo		*m_pChildInfo;


	// Note: the data in here is stored in the parent model for each child model animation.
	// (So if there's a base model and it has 2 parent models, each parent model has separate
	// user dims and translation).
	public:

		// USER dimensions.. settable in ModelEdit.
		LTVector			m_vDims;
		
		// Translation for this animation.
		LTVector			m_vTranslation;
	};


	class ModelSocket : public ModelTransform
	{
	public:
					ModelSocket();
		
		char*		GetName()	{return m_Name;}
		void		SetName(const char *pName);


	public:
		char		m_Name[MAX_SOCKETNAME_LEN];
		uint32		m_iNode;	// Which node it's attached to.
		
		// Only used by modeledit..
		Model		*m_pAttachment;
	};

	// ------------------------------------------------------------------------
	// ModelNode
	// hierarchical organization of model. coordinate system for model
	// ------------------------------------------------------------------------
	class ModelNode
	{
	friend class Model;
	
	public:
		enum EEVALSTATE	{ kEval, kDone, kIgnore };

						ModelNode(Model *pModel);
		virtual			~ModelNode();

		// Called when it's about to remove a vertex.. gives the exporter a chance to 
		// update its structures.
		virtual void	ChangeVertexReferences(PieceLOD *pPiece, uint32 iFrom, uint32 iTo) {}

		void			Term();
		void			Clear();

		uint32			NumChildren()			{return m_Children.GetSize();}
		ModelNode*		GetChild(uint32 index)	{return m_Children[index];}

		uint32			GetNodeIndex()			{return m_NodeIndex;}
		uint32			GetParentNodeIndex()	{return m_iParentNode;}
		
		char*			GetName()		{return m_pName;}
		void			SetName(char *pName);

		Model*			GetModel() {return m_pModel;}
		LAlloc*			GetAlloc();

		// Scratch data for some of the algorithms.
		uint32&			ScratchData() {return m_ScratchData;}

		// Calculates the number of nodes in this subtree (including this one).
		uint32			CalcNumNodes();


		// Fills in Model::m_FlatNodeList and sets its transform index.
		bool			FillNodeList(uint32 &curNodeIndex);

		// Sets m_iParentNode and recurses.
		void			SetParent_R(uint32 iNode);

		// Load/save.
		bool			Load(ILTStream &file);
		bool			Save(ILTStream &file);

		// Access the global transform.
		inline LTMatrix&		GetGlobalTransform()	{return m_mGlobalTransform;}
		inline LTMatrix&		GetInvGlobalTransform()	{return m_mInvGlobalTransform;}

		// gets transform local to parent. 
		LTMatrix				GetLocalTransform() ;
		// 
		
		 
		// this should be SetGlobalTransform( const LTMatrix & mMat ) ...
		inline void			SetGlobalTransform(LTMatrix mMat)
		{
			m_mGlobalTransform = mMat;
			m_mInvGlobalTransform = ~mMat;
		}
	
		// OBB related methods
		void SetOBB( const SOBB &new_obb );
		const SOBB& GetOBB() const { return m_OBB ; }
		bool IsOBBEnabled() { return m_bOBBEnabled ; }
		void EnableOBB()    { m_bOBBEnabled = true ; }
		void DisableOBB()   { m_bOBBEnabled = false ;}

	// Overridables.	
	public:

		// Allocate the same kind of node.
		virtual ModelNode*	AllocateSameKind(Model *pModel);

		// Change its model pointer.
		virtual void		SetModel(Model *pModel);

	
	public:

		// How far out along the parent's coordinate system is this node?
		// (~parent->m_GlobalTransform * m_GlobalTransform)
		LTVector			m_vOffsetFromParent;

		// The node index in all the lists.
		uint16				m_NodeIndex;
		uint8				m_Flags;
		

		uint32				m_ScratchData;
		
		CMoArray<ModelNode*, NoCache>	m_Children;

		// NODEPARENT_NONE if none.
		uint32			m_iParentNode;

		// eval state is used to cache transfroms, or ignore if appropriate.
		EEVALSTATE      m_EvalState;
	
	private:

		// For Model::m_DefaultRootNode.
						ModelNode();

		// The base global transform for this node.  This is setup from the first frame of
		// the first animation exported for this model, which is considered to be the
		// 'base' transform for the model (and it's what the skeletal->envelope relations
		// are setup from).  This transform is also used to relate child and parent model
		// skeletons so the parent model's skeleton can be moved around for larger characters.
		LTMatrix			m_mGlobalTransform;
		LTMatrix			m_mInvGlobalTransform;

		// obb for this model node.
		SOBB				m_OBB ; // oriented bounding box
		bool				m_bOBBEnabled ;// is it enabled
		

		Model			*m_pModel;
		char			*m_pName;
	};

	// ------------------------------------------------------------------------
	// PieceLOD
	// Level of Detail geometry and renderstyle. 
	// ------------------------------------------------------------------------
	class PieceLOD
	{
	public:
							PieceLOD(Model *pModel);
							PieceLOD();
							~PieceLOD();

		void				Init(Model *pModel);

		LAlloc*				GetAlloc();
		Model*				GetModel();

		uint32				NumVerts()		{return m_Verts.GetSize();}

	
		const char *		GetName() { return m_name.c_str() ; }
		void				SetName( const char *name ) { m_name = name ; }

	// T.F ModelPiece
		bool			ReplaceVertex(uint32 iVert, uint32 iReplaceWith);

		// remap node indices to match the node indices of destModel
		// returns false if matching nodes aren't found
		bool				RemapNodeIndices( Model* destModel );

		// copy the material info from another piece lod
		bool				SetMaterialInfo( const PieceLOD& sourceLOD );


	public:
		// number of textures used, and their actual indices
		uint32				m_nNumTextures;
		int32				m_iTextures[MAX_PIECE_TEXTURES];

		// RenderStyle index
		int32				m_iRenderStyle;

		//the render priority
		uint8				m_nRenderPriority;
	
		// Tris and vertices.
		CMoArray<ModelVert, NoCache>	m_Verts;
		CMoArray<ModelTri, NoCache>		m_Tris;

		// Point back to parent model.
		Model							*m_pModel;

		void				CalcUsedNodeList();
			// list of node index used by this lod.
		
		std::vector<uint32>			m_UsedNodeList ;

	private :
		std::string			m_name ;
	
	};


	// ------------------------------------------------------------------------
	// ModelPiece
	// model piece is a container for a collection of lod stages.
	// each lod stage is a render-object.
	// ------------------------------------------------------------------------
	class ModelPiece 
	{
	public:						
						ModelPiece(Model *pModel);
						~ModelPiece();

		void			Term();

		// If iLOD is PIECELOD_BASE, then it just returns the piece's LOD, otherwise
		// it returns the correct LOD (NULL if iLOD is an invalid index).
		inline uint32	NumLODs()	{return m_LODs.GetSize() ;}
		PieceLOD*		GetLOD(uint32 iLOD);

		// GetLOD( dist ) get the lod based on distance.
		// returns null if there are no lods associated with this modelpiece.
		PieceLOD*		GetLOD( float dist ) 
		{
			if( dist >= 0 )
			{
				uint32 num_lods = m_LODs.GetSize() -1 ;
				for(uint32 i = 0 ; i < num_lods  ; i++ )
				{
					if( dist < m_LODDists[i+1] )
					{
						return &m_LODs[i];
					}
				}
				// return the last one if we fell out of scale
				return &m_LODs[num_lods];
			}
			else
				if( m_LODs.GetSize() ) 
					return &m_LODs[0];
				else // theoretically this is an exception. 
					return NULL ; 
		}

		// AddLOD( PieceLOD, at_distance )
		//
		// returns 0 on fail (at_dist < 0) or
		// returns 1 on sucess inserted or appended new lod.
		uint32 AddLOD( PieceLOD &pPiece , float at_dist, bool append_only=false );

		uint32 AddNullLOD(float at_dist, bool append_only=false );

		// removes the specified LOD and slides higher LODs down
		bool RemoveLOD( uint32 iLOD );

		void SetMinMaxLODOffset( uint32 min, uint32 max )				{ m_MinLODOffset = min ;m_MaxLODOffset = max ; }
		void GetMinMaxLODOffset( uint32 &min, uint32 &max )			{ min=m_MinLODOffset  ;max = m_MaxLODOffset ; }

		// lodweights are used when (re)generating lods for the model.
		float			GetLODWeight() {return m_LODWeight;}
		void			SetLODWeight(float fWeight) {m_LODWeight = fWeight;}

		// ModelEdit related funcs 

		
		float GetMaxDist() 
		{  
			if( m_LODDists.GetSize() ) return m_LODDists[ m_LODDists.GetSize()-1] ; 
			else return 0.0f;
		}

		// get the distance value for this lod index
		float GetLODDist( uint32 lod ) { if( lod < m_LODs.GetSize() ) return m_LODDists[ lod ] ; else return 0.0f ;}

		// set the distance for this lod index (doesn't sort the piece lod list)
		// you must call SortLODs to get the piece lods in the correct order
		bool SetLODDist( uint32 lod, float dist );

		// sort the piece lod list
		bool SortLODs();


		// Accessors.
	public:

		char*			GetName()		{return m_Name;}
		void			SetName(const char *pName);

		// make sure every body knows who they belong to...
		Model*			GetModel() { return m_pModel ;}
		LAlloc*			GetAlloc();


	// Functions.
	public:
		// Copy the UV coordinates.
		// Returns FALSE if the geometries on the two models don't match.
		// T.F ModelPiece
		bool			CopyUVs(ModelPiece *pSrc);
		// T.F ModelPiece 
		bool			FindTriByPosition(LTVector *pVertPositions, 
			LTVector *pVert1, LTVector *pVert2, LTVector *pVert3, uint32 &triIndex);

		bool			ReplaceVertex(uint32 iVert, uint32 iReplaceWith);


	public:
	
		// Geometry for each LOD.  NEVER access these directly.. use NumLODs()..
		CMoArray<PieceLOD, NoCache>	m_LODs;
		CMoArray<float, NoCache>    m_LODDists ; 

		uint32			m_MinLODOffset, m_MaxLODOffset ;
		
		// used to determine collapse priority for automatically generated lods.
		float			m_LODWeight;


		// The base index of its vertices if all the nodes were laid out in a linear list
		// (in the order of m_FlatNodeList).
		uint32			m_VertOffset;
		
	
		// vertex animated pieces should know about what's affecting them.
		int								m_isVA ;    // am I a vertex animated node ?
		int								m_vaAnimNodeIdx ;// which anim do I point to


	private:

		Model*			m_pModel ;
		char			m_Name[MAX_PIECENAME_LEN];
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

		char*			GetName()	{return m_Name;}
		void			SetName(const char *pName);

		inline Model*	GetModel()	{return m_pModel;}
		LAlloc*			GetAlloc();

	public:
		
		char			m_Name[MAX_WEIGHTSETNAME_LEN];
		CMoArray<float>	m_Weights;
		Model			*m_pModel;
	};


	class LODInfo
	{
	public:

					LODInfo()
					{
						m_Dist = 0.0f;
					}

	public:
		// Desired distance to use this LOD.
		float		m_Dist;
	};


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
		char*			AddString(const char *pStr);

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
	// Model
	// Container that holds geometric, material and animation information. 
	// The data contained represents data for character models as well as props 
	// and things.
	// ------------------------------------------------------------------------
	class Model
	{
	public :
		
		uint32 m_RefCount ;
		bool IsFreeable() { return m_RefCount == 0 ; }
		uint32 AddRef()   { return m_RefCount++ ; }
		uint32 DecRef()   { return m_RefCount-- ; }

	public:
						Model(LAlloc *pAlloc=&g_DefAlloc, LAlloc *pDefAlloc=&g_DefAlloc);
		virtual			~Model();
		void			Term(bool bDeleteChildModels=FALSE);

		// Deletes all the animations.
		void			TermAnims();

		// Deletes and removes references to the child models.
		void			TermChildModels(bool bDeleteChildModels);

	
		// The save index is stored in the child model.
		uint32			GetSaveIndex() {return m_SelfChildModel.m_SaveIndex;}
		void			SetSaveIndex(uint32 index) {m_SelfChildModel.m_SaveIndex = index;}
		
		// Access the root node.
		ModelNode*		GetRootNode() {return m_pRootNode;}
		void			SetRootNode(ModelNode *pNode);

		// The way the lists are built, the root node is always node 0.
		bool			IsRootNode(uint32 iNode) {return iNode == 0;}

		// Get the allocators this model uses.
		inline LAlloc*	GetAlloc()		{return m_pAlloc;}
		inline LAlloc*	GetDefAlloc()	{return m_pDefAlloc;}

		uint32			NumAnims()	{return m_Anims.GetSize();}
		
		// Just gets the anim, skips past the AnimInfo.
		ModelAnim*		GetAnim(uint32 index) {return m_Anims[index].m_pAnim;}

		AnimInfo*		GetAnimInfo(uint32 index) {return &m_Anims[index];}

		// Asserts and returns NULL if the index is invalid.
		ModelAnim*		GetAnimCheck(uint32 index)
		{
			if(index >= m_Anims.GetSize())
			{
				ASSERT(FALSE);
				return NULL;
			}
			return m_Anims[index].m_pAnim;
		}
		
		// Gets the anim, but returns NULL if the animation's parent 
		ModelAnim*		GetAnimIfOwner(uint32 index)
		{
			if(m_Anims[index].GetAnimOwner() == this)
				return m_Anims[index].m_pAnim;
			else
				return NULL;
		}


		// Weight sets..
		uint32			NumWeightSets()			{return m_WeightSets.GetSize();}
		WeightSet*		GetWeightSet(uint32 i)	{return m_WeightSets[i];}
		bool			AddWeightSet(WeightSet *pSet);
		void			RemoveWeightSet(uint32 index);


		uint32			GetTotalNumVerts()	{return m_nTotalVerts;}		
		uint32			GetTotalNumTris()	{return m_nTotalTris;}		
		
		// ------------------------------------------------------------------------
		// NODES 

		// Precalculate the lists of nodes (and figures out how many there are).
		// This should be called whenever a node is added or removed.
		bool			PrecalcNodeLists(bool bRebuild=TRUE);

		ModelNode*		FindNode(const char *pName, uint32 *index=NULL);
		uint32			NumNodes() {return m_Transforms.GetSize();}
		ModelNode*		GetNode(uint32 i) {return m_FlatNodeList[i];}

		// Get the movement node, if any, for this model
		ModelNode*		GetMovementNode() {return m_pMovementNode;}

		// This call gets the transform for node "i" after anim has been applied. So this is the current Global position of that node.
		LTMatrix*		GetTransform(uint32 i) {return &m_Transforms[i];}
		
		// Find the parent of a given node.  Starts recursing at pRoot.
		// Fills in index if it's non-NULL.
		ModelNode*		FindParent(ModelNode *pNode, ModelNode *pRoot, uint32 *index=NULL);

		// Removes rotation from this node.  Adjusts its child nodes so they animate the same.
		// UnrotateNode just calls UnrotateAnimNode for all the animations on this node.
		void			UnrotateAnimNode(ModelAnim *pAnim, AnimNode *pAnimNode);
		void			UnrotateNode(ModelNode *pNode);


		// Removes the node from the tree.  Only works on null nodes currently.
		// NOTE: this function overwrites ModelNode::m_pSoftElem.
		bool			RemoveNode(ModelNode *pNode);

		// Calls RemoveNode on all the removable nodes.
		bool			RemoveRemovableNodes();

		void			ResetNodeEvalState();
		// ------------------------------------------------------------------------
		// PIECES 

		uint32			NumPieces() {return m_Pieces.GetSize();}
		ModelPiece*		GetPiece(uint32 i) {return m_Pieces[i];}
		ModelPiece*		FindPiece(const char *pName, uint32 *index=NULL); 
		WeightSet*		FindWeightSet(const char *pName, uint32 *index=NULL);

		// Swap the two pieces. (note[T.F] must update anims)
		bool			SwapPieces(uint32 iPiece1, uint32 iPiece2);
	
		void			SetupNodeEvalStateFromPieces(float lod_dist);
		void			SetupNodeEvalStateFromNode( uint32 iNode );


		// get vertex buffer offset from the piece specified
		// returns false if iPieceIndex is invalid
		bool GetVertBuffOffsetFor( uint32 iPieceIndex, int & iBuffOffset );

		// Add a string to the model's list.
		char*			AddString(const char* pStr);

		

		// Find an animation by name.
		ModelAnim*		FindAnim(char *pName, uint32 *index=NULL, AnimInfo **ppInfo=NULL);

		// Find an animation with the given model.
		AnimInfo*		FindAnimInfo(char *pAnimName, Model *pOwner, uint32 *index=NULL);

		// Returns the index of the given anim info.
		bool			GetAnimInfoIndex(AnimInfo *pInfo, uint32 &index);

		// Add an animation to the list.  It adds it in the appropriate position given
		// the ChildInfo.
		AnimInfo*		AddAnim(ModelAnim *pAnim, ChildInfo *pInfo);

		// Add an animation to this model (just calls AddAnim with m_SelfChildModel).
		AnimInfo*		AddAnimToSelf(ModelAnim *pAnim) {return AddAnim(pAnim, &m_SelfChildModel);}

		// Creates a single-frame animation with identity translations and rotations
		// using the vertex positions from the model's geometry.
		// This does NOT support deformation nodes currently (returns FALSE if there are any).
		bool			AddDummyAnimation(char *pName);

		// Load/save.
		bool			LoadString(ILTStream &file, char* &pStr);
		
		bool			LoadAnimBindings(ModelLoadRequest *pRequest, ILTStream &file, bool bAllowUpdates);
		bool			LoadSockets(ILTStream &file);
		bool			LoadWeightSets(ILTStream &file);


		// Used internally.. reads the header.
		bool			ValidateHeader( ILTStream &file, ModelAllocations &allocs);

		// Sets up m_BlockAlloc as the allocator and allocates a block as large
		// as the file requires.
		LTRESULT		SetupBlockAllocator(ILTStream &file, LAlloc *pAlloc, const char* pFilefname);
		
		bool			SaveAnimBindings(ILTStream &file);
		bool			SaveWeightSets(ILTStream &file);
		bool			SaveSockets(ILTStream &file);
		
		// Copy the UV coordinates.
		// Returns FALSE if the geometries on the two models don't match.
		bool			CopyUVs(Model *pSrc);

		// Calculates the total number of vertices in the model.
		uint32			CalcNumTris(uint32 iLOD = PIECELOD_BASE);
		uint32			CalcNumTris(float fDistLOD);
		uint32			CalcNumVerts();

		// Calculates the total number of vertex weights (ie: how expensive it is to 
		// weight all the vertices in the model).
		uint32			CalcNumVertexWeights();

		// Figures out how many animations are in the child models.
		uint32			CalcNumChildModelAnims(bool bIncludeSelf);

		// Returns how many animations in our m_Anims list come from this model.
		uint32			CalcNumParentAnims();

		// Parses the command string.
		void			ParseCommandString();

		// Access the filename.
		const char*		GetFilename();
		bool			SetFilename(const char *pFilename);
		void			FreeFilename(); // Free m_pFilename.
		

	// Functions to access child models.
	// NOTE: there is ALWAYS at least one child model and the first child model in the 
	// list is always m_SelfChildModel.
	public:

		ChildInfo*		GetSelfChildModel() {return m_ChildModels[0];}

		uint32			NumChildModels() {return m_nChildModels;}
		ChildInfo*		GetChildModel(uint32 index) {ASSERT(index < m_nChildModels); return m_ChildModels[index];}
		
		// Returns if the child model's tree is valid.
		bool			VerifyChildModelTree(Model *pChild, ModelNode* &pErrNode);

		// Setup the relation between the models.  If bScaleSkeleton is TRUE, then it 
		// scales the skeleton to match the other model's skeleton.
		bool			SetupChildModelRelation( ChildInfo *pInfo, 
												 Model *pChild, 
												 bool bScaleSkeleton=FALSE);
		
		// Used by ModelEdit.. shouldn't be used at runtime.  Adds a new ChildModel
		// with the specified info.
		ChildInfo*		AddChildModel( Model *pChild, char *pFilename, 
										char *pErrStr, 
										bool bScaleSkeleton);
		// Creates a reference to a child model, no actually child model is assigned. 
		// This message should only be used by tools that just need the child model filename and 
		// params.
		ChildInfo*		AddChildModelRef( char *pFilename, char *errStr );

		// Remove the specified child model.
		bool			RemoveChildModel(uint32 index);


		// Just initializes the model and sets our m_ChildModel[index] = pChildModel.
		bool			InitChildInfo(uint32 index, ChildInfo *pChildModel, Model *pModel, char *pFilename);
		
		// Find child models, first one by pointer reference.
		ChildInfo*		FindChildModel(Model *pModel);
		ChildInfo*		FindChildModelByFilename(char *pFilename, uint32 *index=NULL); // Case sensitive.

		// Make a unique animation name.
		bool			MakeUniqueAnimName(char *pBase, char *pName);

		// Sets ModelNode::m_vOffsetFromParent (and calls it on the children).
		void			SetNodeParentOffsets();


	public:

		// This is the 'reference' function for how to get the vertex positions on a given 
		// frame of animation.
		bool			GetVertexPositions(GVPStruct *pStruct, bool bSetupTransforms=TRUE, bool bWeight=TRUE);

		// Fix for a bug in ModelEdit where it sometimes saves animations in an order
		// different from animation bindings.
		bool			ForceAnimOrder();


	private:

		// These are used during precalculation.
		bool			BuildTransformList(bool bRebuild=TRUE);
		bool			BuildFlatNodeList(bool bRebuild=TRUE);

		// Not always set.. the engine uses this to determine if it has already
		// been loaded.  Use Get/SetFilename.
		char			*m_pFilename;


	public:

	// Model link for a linked list.

		LTLink			m_Link;

	
	// Misc. data.

		unsigned long	m_FileID;		// Used by server.
		unsigned long	m_Flags;		// MODELFLAG flags above.  Used by various things.


	// Geometry data.

		// The node list, generated by PREORDER traversal of the node tree.
		CMoArray<ModelNode*, NoCache>	m_FlatNodeList;
		
		// Geometry pieces.. these are where the verts and tris are stored.
		CMoArray<ModelPiece*, NoCache>	m_Pieces;

		// Weight sets..
		CMoArray<WeightSet*, NoCache>	m_WeightSets;

		CMoArray<NewVertexWeight, NoCache>	m_VertexWeights;


		// Setup with CalcNumVerts() in PrecalcNodeLists.
		// This is the total number of vertices in all the nodes.
		uint32			m_nTotalVerts;
		uint32			m_nTotalTris;

		// m_nNodes/4 and +1 if m_nNodes%4 != 0.
		unsigned long	m_nDWordNodes;

		// These are allocated when you load the model.  You can use these
		// this is the result of the animation update.
		CMoArray<LTMatrix, NoCache>	m_Transforms;

		// The command string for the model.  This is used for sequences of commands
		// that get stored with the model (like LOD ranges...)
		char			*m_CommandString;

		ModelStringList	m_StringList;

	// Stuff contained in the command string.
	public:

		float			m_GlobalRadius;		// Global radius.. MUST enclose the whole model.

		float			m_VisRadius;		// Defaults to DEFAULT_MODEL_VIS_RADIUS or can be 
											// overridden in the command string.
		// If the NoAnimation token is in the model file, then this is TRUE.  This means 
		// the renderer doesn't have to weight the verts so it's 15-25% faster.
		bool			m_bNoAnimation;

		// Ambient and directional light percentages.
		float			m_AmbientLight;
		float			m_DirLight;

		// Shadow stuff.
		bool			m_bShadowEnable;
		float			m_fShadowProjectLength;
		float			m_fShadowLightDist;
		float			m_fShadowSizeX;
		float			m_fShadowSizeY;
		LTVector		m_vShadowCenterOffset;

		// Reference node for the normals.  Since it doesn't animate the normals on the model,
		// the renderer can take the offset of this node from the first frame of this animation
		// and apply that to the normals.
		// If m_iNormalRefNode is INVALID_MODEL_NODE, then the reference node shouldn't be used.
		uint32			m_iNormalRefNode;
		uint32			m_iNormalRefAnim;
		LTMatrix		m_mNormalRef; // INVERSE of first frame of m_iNormalRefAnim.
		bool			m_bNormalRef; // Tells if the normal reference should be used at all.

		// Optional FOV offset used by the renderer.
		bool			m_bFOVOffset;
		float			m_xFOVOffset;
		float			m_yFOVOffset;

		// Does this model support specular highlighting?
		bool			m_bSpecularEnable;

        // Is this model rigid?
		bool			m_bRigid;
		uint32			m_iLTAFileType;


	// Sockets.
	public:

		uint32			NumSockets() {return m_Sockets.GetSize();}
		ModelSocket*	GetSocket(uint32 index) {return m_Sockets[index];}
		ModelSocket*	FindSocket(const char *pName, uint32 *index=NULL);

		bool			AddSocket(ModelSocket *pSocket);
		bool			RemoveSocket(uint32 index);
		
		// Gets the socket's transform, assumes the transforms for the model are already setup.
		bool			GetSocketTransform(ModelSocket *pSocket, LTMatrix *pSocketTransform);
		
		CMoArray<ModelSocket*,NoCache>	m_Sockets;
		

	// Other...
	public:

		bool			AnyVertsWeightedToNode(uint32 iNode);

	
	// Animation data.
	public:

		// Note: with skeletal models, some of these anims may not be from this model,
		// it should never assume that they are.
		CMoArray<AnimInfo, NoCache>		m_Anims;

		// add an index-texture relation.
		void AddTextureName( uint32 index, const char *filename );

		// returns false if we could not find the index.
		bool GetTextureName( uint32 index , std::string & name);
		
	// Misc.
	public:
		// *** TOOLS-INFO 
		// the base
		std::string m_sProjectDir ;
		std::map<uint32,std::string> m_TextureIndexMap ;
		typedef std::map<uint32,std::string>::iterator CTextureIndexMapIter;

		uint32 m_CompressionType ;
		bool   m_bExcludeGeom ;
		// *** TOOLS INFO


		// m_pAlloc = who's doing our BLOCK allocations.  Most allocations can go through 
		// here, but some must go thru m_pDefAlloc if we can't predict ahead of time how
		// many allocations there will be.
		LAlloc							*m_pAlloc;
		LAlloc							*m_pDefAlloc;

		// At runtime, models are loaded using this.
		LAllocSimpleBlock				m_BlockAlloc;

		// What version of the file did we come from?
		inline uint32	GetFileVersion()	{return m_FileVersion;}
		// Set the file version. This should only be used by an importer.
		inline void     SetFileVersion( uint32 vers ) { m_FileVersion = vers ; }

	// Child model list.
	private:

		// The list of child models.
		// The model itself (m_SelfChildModel) is ALWAYS first.
		ChildInfo*						m_ChildModels[MAX_CHILD_MODELS];
		uint32							m_nChildModels;
		
		// Saves an allocation.
		ChildInfo						m_SelfChildModel;

	
	private:

		// Set when it starts reading so it can track what file version it's reading in.
		uint32							m_FileVersion;

		
		// Default root node.. setup so it can be subclassed.
		ModelNode		m_DefaultRootNode;
		ModelNode		*m_pRootNode;

		// Movement node.  NULL if none exists.
		ModelNode		*m_pMovementNode;
	};



	//------------------------------------------------------------------ //
	// Functions.
	//------------------------------------------------------------------ //

	// This is used to find tokens in node and socket names.  Tokens start with _z.
	// Returns a pointer to the first character in the token.  So if the token you're
	// looking for is "Tex" and it finds _zTex, it'll return "Tex".
	// The comparison is case-insensitive.
	// Current tokens:
	// _zN  = don't remove this node
	// _zT0	= texture 0...
	char* model_FindExporterToken(char *pName, char *pToken);


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

	// ------------------------------------------------------------------------
	//	GetLocalTransform()
	// ------------------------------------------------------------------------
	inline LTMatrix	ModelNode::GetLocalTransform() 
	{ 
		if( m_iParentNode != NODEPARENT_NONE && m_pModel != NULL)
		{
			LTMatrix mat ;
			mat.Identity();
			ModelNode *parent = m_pModel->GetNode( m_iParentNode );
			if( parent!=NULL )
				mat = parent->GetInvGlobalTransform() * m_mGlobalTransform  ;
			else 
				mat = m_mGlobalTransform ;
			return mat;
		}
		return m_mGlobalTransform ;
	}
	
	inline LAlloc* AnimNode::GetAlloc()
	{
		return GetModel()->GetAlloc();
	}

	inline LAlloc* PieceLOD::GetAlloc()
	{
		return m_pModel->GetAlloc();
	}

	inline Model* PieceLOD::GetModel()
	{
		return m_pModel;
	}

	inline LAlloc* ModelPiece::GetAlloc()
	{
		return GetModel()->GetAlloc();
	}

	inline PieceLOD* ModelPiece::GetLOD(uint32 i)
	{
		if(i < m_LODs.GetSize())
			return &m_LODs[i];

		// Invalid LOD index.
		ASSERT(FALSE);
		return NULL;
	}

	inline LAlloc* WeightSet::GetAlloc()
	{
		return GetModel()->GetAlloc();
	}

	inline LAlloc* ChildInfo::GetAlloc()
	{
		return m_pParentModel->GetAlloc();
	}

	//inline LODInfo* Model::GetLOD(uint32 i)
	//{
		//if(i == 0)
			//return &m_BaseLODInfo;
//
		//--i;
		//if(i < m_LODs.GetSize())
			//return &m_LODs[i];
//
		//ASSERT(FALSE);
		//return NULL;
	//}


#endif  // __MODEL_H__

