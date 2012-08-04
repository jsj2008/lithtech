// ------------------------------------------------------------------------
// ltaScene.h
// lithtech (c) 2000
// 
// scene graph associated with lta files
// each node in the graph corresponds to a node in the lta file format.
// ------------------------------------------------------------------------

#ifndef LTA_SCENEGRAPH_DEFINED
#define LTA_SCENEGRAPH_DEFINED

#ifndef LT_TRUE 
#define LT_TRUE 1
#endif
#ifndef LT_FALSE
#define LT_FALSE 0
#endif  

#ifdef _WINDOWS
// windows ...
#include <windows.h>
// handle debug errors from windoze
#pragma warning(disable:4786 4018)
#endif


#include "ltbasetypes.h"
#include "ltamgr.h"

// std c++
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
//using namespace std;

// ------------------------------------------------------------------------
// ILTAObject 
// base object for lta import stuff
// every lta node has name.
// ------------------------------------------------------------------------
class ILTAObject {
	std::string m_Name ;
public :
	std::string			& GetName ();
	const std::string	& GetName() const ;
	void				SetName( std::string & name ) ;
	void				SetName( const std::string & name ) ;
};


// ------------------------------------------------------------------------
// CLTAMesh 
// mesh reprsentation. All members public, treat as struct, no funcs/methods etc.
//
// ------------------------------------------------------------------------
class CLTAMesh : public ILTAObject 
{
public :
	std::vector<LTVector> vertex ;
	std::vector<LTVector> normals ;
	std::vector<LTVector> tcoords ;
	std::vector<LTVector> colors ;

	// list of triangles
	std::vector< int >    tri_FaceSet ;
	std::vector< int >    tri_UVSet ;
	std::vector< int >	 tri_ColorSet ;
	std::vector< int >	 tri_NormSet ;
	std::vector< int >	 tri_FaceStripSet ;

	// list of polygons 
};


// ------------------------------------------------------------------------
// Container for geometry
// ------------------------------------------------------------------------
class CLTAGeometry : public ILTAObject {
public :
	enum { MESH, BSURF };

	CLTAMesh m_Mesh ;
};



// ------------------------------------------------------------------------
// Base class for all materials
// this hierarchy should be flat. meaning that all decendents from this
// class should be final. 
// ------------------------------------------------------------------------
class CLTAMaterial : public ILTAObject {

protected :
	int m_Type ;
public :
	enum { PC_MAT, PS2_MAT, RENDERSTYLE  }; 

	CLTAMaterial( int type ) :m_Type(type) {}

	
	int			GetType() const ;
};

// ------------------------------------------------------------------------
// Material definition as it is in the  abc (13) format
// ------------------------------------------------------------------------
class CLTA_ABC_PC_MAT : public CLTAMaterial {

public :
	// defaults
	CLTA_ABC_PC_MAT();

	// params for the specular map
	float m_specular_power ;
	float m_specular_scale ;
	
	int   m_num_textures ;
	int   m_textures[4];

	// colors 
	float m_diffuse[4];
	float m_ambient[4] ;
	float m_specular[4] ;
	float m_emissive[4] ;
	float m_shininess ;

};


// ------------------------------------------------------------------------
// render style appearance
// ------------------------------------------------------------------------
class CLTARenderStyle : public CLTAMaterial {
public :
	CLTARenderStyle();


	uint32			m_nNumTextures;	// Number of textures used, and their actual indices
	int32			m_iTextures[4]; //MAX_PIECE_TEXTURES];
	int32			m_iRenderStyle;	// RenderStyle index
	uint8			m_nRenderPriority; //render priority, the order in which pieces are drawn

};


// ------------------------------------------------------------------------
// CLTAShape
// container for a mesh and its appearance
// LTA Node :
// (shape name
//			(geometry ( ... ))
//          (appearance (...)))
// ------------------------------------------------------------------------
class Shape  : public ILTAObject {
	
	CLTAGeometry	m_Geometry;
	CLTAMaterial   *m_pAppearance ;
	std::string		m_ParentName;
	
public:
	Shape() ;
	~Shape();

	// accessors
	void			SetParentName( const char *parent_name ) ;
	void			SetMeshName  ( const char *mesh_name   ) ;
	
	const std::string& GetParentName() ;
	const std::string& GetMeshName()   ;
	
	CLTAMesh&		GetMesh()			;
	CLTAMaterial*	GetAppearance()		;
	void			SetAppearance(CLTAMaterial *val);
};


// ------------------------------------------------------------------------
// CLTANode 
// A node is a hierarchy element. usually a transform
// ------------------------------------------------------------------------
class Node : public ILTAObject {

public :

	// transform for the node 
	LTMatrix mat ;	// s/b called transform 

	// Reference back to shape that points to this node. 
	Shape    *shape ; // shape that references this node.

	// Shapes associated with this transform.
	std::vector<Shape*> shapeList ;
	
	// flat list index
	int      index ;
	
	Node    *parent ;
	std::vector<Node*> children;	
	
public:
	Node() ;
	Node( std::string &name );
	~Node();
	

	// finds the node with the name sname ...
	Node *FindName( const std::string & sname );
	void AddShape( Shape *shape )          ;


	void AddChild( Node *new_node ) ;

};

// ------------------------------------------------------------------------
// CLTAKeyFrames
// time-value pairs, the value is an index
// (keyframe name (times ( ... ) ) )
// ------------------------------------------------------------------------
class CKeyFrame : public ILTAObject {

	struct key_value {
		key_value( int t, int idx ):time(t),index(idx) {}
		key_value( int t, std::string &val ):time(t),value(val) {}
		key_value():time(-1),index(-1) {}

		int time ;
		int index ; //  value ;
		std::string value ; // conform to lt where we have named times.
	};

	int m_LastTime ;

	std::vector<key_value> m_times ;
	
	void Append( const key_value & kv ) ;

	// ------------------------------------------------------------------------
	// 
	// ------------------------------------------------------------------------
	public :
	
		CKeyFrame() :m_LastTime(0) {}

		// add 
		void Add( int time, int value  ) ;

		void Add( int time , std::string & value ) ;

		// add the value at INDEX, NOT TIME
		void AddValue( int index, std::string & val  );


	// assignment  
	void operator=( const CKeyFrame & kf ) ;
	

	// accessors 
	int Size()           const;

	int  GetTime( int i ) const ;
	void SetTime( int index, int new_time ) ;

	const std::string& GetVal( int i ) const ;

	
	// ------------------------------------------------------------------------
	// ret getIndex( time, start-time, end-time, percent-in-between)
	//
	// get the keyframe index, given time (brute force seach)
	// fills ret-params with the the closest frames and the exact parametric point in between
	// returns 1 on success, 0 on not finding any appropriate values
	// ------------------------------------------------------------------------
	int GetIndex( int time, int &start, int &end, float &percent_in_between) const ;
};

// ------------------------------------------------------------------------
// CLTAAnim 
// super class of all animations.
// class holds the global key-times 
// 
//(anim [name]
//		(keyframe (keyframe ... ))
//		(frames ("type" ... )))
//
// ------------------------------------------------------------------------
class CAnim : public ILTAObject {

	int			m_Type ;
	int			m_bKF_Shared ; 	// LT_TRUE if shared, LT_FALSE if not
	CKeyFrame * m_pKeyFrame ;

protected :
	CAnim( int type ):m_Type(type),m_pKeyFrame(NULL),m_bKF_Shared(LT_FALSE) {}
	
public :

	// possible types of anims 
	enum { UNDEFINED, VERTEX, POSQUAT, COLOR };

	CAnim():m_Type(UNDEFINED),m_pKeyFrame(NULL),m_bKF_Shared(LT_FALSE) {}

	virtual ~CAnim() {
		if( m_pKeyFrame != NULL && !m_bKF_Shared )
			delete m_pKeyFrame ;
	}


	int			GetType() ; // get animation type

	// Make Own Copy of KF
	// set from a ref
	void		SetKeyFrameCopy( const CKeyFrame & kf );
	
	
	// Assign a shared version of the keyframe object
	void		ShareKeyFrame( CKeyFrame *kf );
	

	// getKeyFrame 
	CKeyFrame *	GetKeyFrame() ;

	// returns 0 on fail
	int			GetKeyFrameIndexFromTime( int tm, int &start, int &end, float &percent );
	
	// get target name 
	virtual std::string GetTargetName() = 0;
};

// ------------------------------------------------------------------------
// CLTAVtxAnim
// version of the animation node that holds vertex animation.
/* 
(anim [name]
	(keyframe (keyframe ... ))
	(frames ("type" ... )))

this is the anim node, this class will be instanced if the "type" is vertex.
*/
// ------------------------------------------------------------------------

class CVtxAnim : public CAnim  {

	Shape *  m_target ;

	struct DefVertexLst {
		std::vector<LTVector> m_vVertex ;
	};

	std::vector<DefVertexLst> m_vFrames ;

public :

	CVtxAnim(): CAnim(VERTEX),m_target(NULL) {}
	

	// set what we are acting on,
	// name the animation after target
	void SetTarget( Shape * bs)   ;
	Shape *GetTarget();
	
	// add an animation
	void AppendVertexList ( std::vector<LTVector> & v_list );

	// frame acces 
	int NumVertexPerFrame();
	// get value at frame/mesh index
	LTVector Get( int frame, int index ) ;
	// get target name
	std::string GetTargetName() ;
	
};

// ------------------------------------------------------------------------
// CLTAPosQuatAnim
// version of anim that holds position/quaternion pairs.
// ------------------------------------------------------------------------
class CPosQuatAnim : public CAnim {

private :
	// private structure that holds pos/quat pairs
	struct PosQuat {	
		float pos[3];
		float quat[4];
	};

	std::vector<PosQuat> m_vPosQuatDat ;
	Node *m_target ;

	
	void SetPosQuat( PosQuat & pq, float *p, float *q );
	void GetPosQuat( PosQuat &pq, float *p , float *q );


public :
	
	CPosQuatAnim(): CAnim(POSQUAT),m_target(NULL) {}

	// gets frame X
	void Get( int frame, LTVector & pos, float *quat );


	// append a new position/quaternion on to list
	void Append( float *p, float *q );


	// set what we are acting on,
	// name the animation after target
	void SetTarget( Node * bs);
	
	Node *GetTarget();

	std::string GetTargetName() ;
};

// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
class CChannelAnim : public CAnim {
	
	
};

// ------------------------------------------------------------------------
// CLTAAnimSet
// collection of anims :
// (animset name 
//			(keyframe (keyframe ... ))
//			(anims    ( ... )))
// 
// ------------------------------------------------------------------------
class CAnimSet : public ILTAObject {
public :	
	CKeyFrame m_KeyFrame ;
	
	// list of animation frames
	std::vector<CAnim*> m_vAnims ;
	
	~CAnimSet();
	
	// ------------------------------------------------------------------------
	// share the animset's keyframe with the sub anims
	// ------------------------------------------------------------------------
	void KFFix() ;

	// append
	void AddAnim( CAnim * nan ) ;

	
	// find an animation by what its target is 
	CAnim *FindAnimByTargetName( const std::string & name );

	// find an animation by name. Returns null if not found.
	CAnim *FindAnim( const std::string & name );
	int NumAnims() { return m_vAnims.size(); }
	CAnim *GetAnim( uint32 index ) { return m_vAnims[index]; }

	void SetKeyFrame( const CKeyFrame & new_kf );
	CKeyFrame *GetKeyFrame()  ;
	
	// empty returns if there are no anims... 
	int Empty() ;
	// returns the type of all the anims. if the set is heterogenous or empty it returns UNDEFINED
	int GetAnimType() ;
	
};


// ------------------------------------------------------------------------
// lod-sets
// lod group :
// (distance - shape )
// ------------------------------------------------------------------------
class CLTALODGroup : public ILTAObject {

	typedef std::map<float, Shape *> CLODMap ;
	typedef std::map<float, Shape *>::iterator CLODMapIter ;

	int32 m_MinMax[2] ;
	CLODMap m_LODs ;
public :
	

	CLTALODGroup() {}
	CLTALODGroup( const std::string &name ) {
		SetName(name);
	}

	void SetMinMax( int min, int max );
	void GetMinMax( uint32 &min, uint32 &max )  ;

	void AddLOD( Shape *shape, float dist );

	// get lod based on index
	Shape *GetLOD( uint32 index );
	// get lod based on distance value
	Shape *GetLOD( float dist );

};

class CLTAOBB {
public :
	CLTAOBB():m_pParentNode(NULL),m_Enabled(true) {
	}

	std::string m_ParentNodeName ;
	Node	   *m_pParentNode;
	float		m_Pos[3];
	float		m_Orientation[4];
	float		m_Size[3];
	bool		m_Enabled ;
};

// ------------------------------------------------------------------------
// Tools info node in the model db
// ------------------------------------------------------------------------
class CToolsInfo {
public :
	// class types 
	typedef std::map<uint32,std::string> TextureBindingMap ;
	typedef std::map<uint32,std::string>::iterator TextureBindingMapIter;


	TextureBindingMap m_TextureBindingMap ;

	// export parameters ;
	int32 m_CompressionType ;
	bool m_bExcludeGeom ;
	
};


// ------------------------------------------------------------------------
// CLTAMetaModel
// Intermediary model, stores stuff read from lta.
// ------------------------------------------------------------------------
class MetaModel {

	typedef std::map<std::string, CLTALODGroup>::iterator CLTALODGroupIterator ;
	std::map<std::string, CLTALODGroup> m_LODGroups ;

public :
	std::vector<CAnimSet*> m_vAnimSets ;
	int m_LastAnim ;

	std::vector< CLTAOBB > m_obbset;


	Node *rootNode ;		// hierarchy
	std::vector<Shape*> shapes ;

	CToolsInfo     m_ToolsInfo ; // extra information for the tools

// METHODS 

	// constructors/
	MetaModel() ;
	~MetaModel();

	
	// ------------------------------------------------------------------------
	// animation set access
	// CreateAnimSet()    create a new set, optional name
	// getLastAnimSet()   returns the last animset created.
	// numAnimSets()      returns the number of animation sets
	// getAnimSet(index)  gets an animation set
	// ------------------------------------------------------------------------
	
	// Create a new animations set. 
	// metaModel is responsible for creating sets
	CAnimSet * CreateAnimSet();
	

	// create named animation set.
	CAnimSet * CreateAnimSet(std::string & name );
	

	// add an anim set.
	void AppendAnimSet( CAnimSet *newAnimSet );
	
	// returns null if empty.
	CAnimSet * GetLastAnimSet();
	
	int  NumAnimSets();

	// get the anim set // note, no range check ... 
	CAnimSet *GetAnimSet( int i ) ;
	
	// find a shape by name .
	Shape * FindShape( const std::string & name ) ;
	
	// find a node by name 
	Node * FindNode( const std::string & name );
	
	// hook shape to parent node; associates a shape to a node.
	int HookShapeToParent( Shape *shape );

	// removes animsets that have no data.
	void RemoveEmptyAnimSets();
	
	// sum up the number of verts in all the shapes .
	int CalcNumVertex() ;
	
	// add a lod set with name to meta model
	void AddLODGroup( const std::string &name );

	// add information to a lod set.
	bool AddToLODGroup( const std::string &lod_name, 
					  const std::string &shape_name, 
					  float dist, 
					  std::string &err_str );
	
	// add min max lod offset to a lod-group
	void AddMinMaxToLODGroup( const std::string &lod_name, int min, int max );
	
	// get a lod 
	Shape *GetLOD( uint32 piece_index, float dist );
	
	// get numpieces
	uint32 GetNumLODGroups();

};


// ------------------------------------------------------------------------
// CLTATranslStatus
// lta-translator-status
// a light weight error class for static funcs in this file

// ------------------------------------------------------------------------
class CLTATranslStatus {
	//
	static std::ostream *s_CurErrorLog ;
	// 
	static void (*s_fnLoadLogCallBack) (const char *);
public:
	
	enum { OK, FAIL, SUCCEED, OUT_OF_MEM, BAD_PARAM};
	int m_status ;
	std::string m_msg ;

	// default constructor
	CLTATranslStatus() :m_status(OK) {}
	// functor constr 
	CLTATranslStatus( char *errormsg, char *ErrorTitle=NULL ) { m_msg = errormsg ; OnError( ErrorTitle ) ; }

	
	// evaluation operator:
	// set the status value
	CLTATranslStatus & operator=( int val  ) { m_status = val ; return *this;}
	// compare status values.
	bool operator!=( int val ) { return val != m_status ; }
	bool operator==(int val  ) { return val == m_status ; }

	// call this to announce an error has occured.
	void OnError( char *ErrorTitle=NULL, int bShowErrWin = 1) ;
	

	// ------------------------------------------------------------------------
	// seterror log ( out-stream )
	// static function that sets the error log stream to which 
	// all messages are going to be written to.
	// ------------------------------------------------------------------------
	static void SetErrorLog( std::ostream * ostr );
	// sets the function that gets called each time AddToLog gets called.
	static void SetLoadLogCB( void (*fnPtr)(const char *) );

	static void AddToLog( const std::string & str );

	// returns the current outstream that all msgs are sent to.
	std::ostream& errLog() ;

	// the << operator. treat instance as a out-stream
	CLTATranslStatus & operator<<( const char *msg );
	
	// resets the status for more use. 
	void reset() ;
};


// ------------------------------------------------------------------------
// LTA->ParseNode Function set
// ------------------------------------------------------------------------
class LTA {
public :

	// Interpret Parse nodes' hierarchy node, put result into meta-model.
	static 
	void HierarchyFromPN(	MetaModel & metaModel,		// data container
							CLTANode *pnroot,			// parse node tree
							CLTATranslStatus & status   // error reporting
							); 

	
	// Evaluate the parse tree for shapes.
	// fill the metamodel container with new data.
	static
	void GetShapesFromPN( MetaModel & metaModel, CLTANode *pnroot );

	static 
	void GetLODGroupsFromPN( MetaModel & metaModel, 
							 CLTANode *pnroot,
							 CLTATranslStatus & status );

	static 
	void LODGroupFromPN( MetaModel &metaModel,
				         CLTANode *create_lod_group, 
						 CLTATranslStatus & status );

	static 
	void
	GetOBBFromOnLoadCmdsPN( MetaModel & metaModel, 
							CLTANode *pnroot, 
							CLTATranslStatus &status);

	// process & add parsenode that reps a shape node into metamodel
	static 
	void ShapeFromPN( MetaModel & metaModel, CLTANode *pnroot );

	// ------------------------------------------------------------------------
	// Get the tools info node from the lta and associate it with the meta model.
	// ------------------------------------------------------------------------
	static 
	void ToolsInfoNodeFromPN( MetaModel &metaModel, CLTANode *pnroot );

	// ------------------------------------------------------------------------
	// Starting point for creating an in-memory representation of a model 
	// lta file.
	// ------------------------------------------------------------------------
	static 
	void InitMetaModelFromPN( MetaModel & new_MetaModel, 
								CLTANode *pnroot,
								CLTATranslStatus &status 
								);



	///////////////////////////////////////////////////////////////////////
	// Utils 

	typedef std::vector< std::pair<int,int> > CAnimPairLst  ;

	typedef std::vector< std::pair<int,int> > CClassifiedAnimLst ;
	


	// given a classified list list and an index into that list
	// return true if the classification for that index indicates that
	// the the animation should be merged.
	static
	int ShouldMerge( CClassifiedAnimLst & CurLst, int index );
	
	
	// ------------------------------------------------------------------------
	// CollectAndClassifyAnims( metaModel, result-list )
	// check the animation sets in the MetaModel for name matches. 
	// a name match is an indication that a vertex animset and a pos/quat animset
	// should be merged. The criteria is an animation named nameVA and name both have
	// the same root name. The VA postfix indicates that it is an vertex animation.
	// ------------------------------------------------------------------------
	static 
	int CollectAndClassifyAnims( MetaModel & metaModel, CClassifiedAnimLst & );

	// ------------------------------------------------------------------------
	// ParseVector( pn-list, destination-vector)
	// The pn-list should be a list that contains an array of floats.
	// The destination vector should have enough space to fill the result.
	// pn-list => ( a b c ) = destination-vector[] = { a, b, c };
	// ------------------------------------------------------------------------
	static
	void ParseVector( CLTANode *vrt, float * vert );

	// ------------------------------------------------------------------------
	// finds all the animsets in the parsenode tree and processes them.
	// ------------------------------------------------------------------------
	static
	void ProcessAnimSetFromLTA( MetaModel & metaModel,
								CLTANode *pnroot,
								CLTATranslStatus & status );

	// AnimSetFromPN ( MetaModel, start-parse-node, return-status )
	// Evaluate parsenode for anim-set information,
	// put result into metaModel .
	static
	void AnimSetFromPN( MetaModel & metaModel,          // data container
						CLTANode *pnroot,				// parse node tree = (animset
						CLTATranslStatus & status		// error reporting
					);


};


/////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------//
// Inlines																   //
// ------------------------------------------------------------------------//
////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// ILTAObject 
// base object for lta import stuff
// every lta node has name.
// ------------------------------------------------------------------------
inline std::string & ILTAObject::GetName () { return m_Name ; }
inline const  std::string & ILTAObject::GetName() const { return m_Name ; }
inline void     ILTAObject::SetName( std::string & name ) { m_Name = name ; } 
inline void     ILTAObject::SetName( const std::string & name ) { m_Name = name ; }

// ------------------------------------------------------------------------
// Base class for all materials
// this hierarchy should be flat. meaning that all decendents from this
// class should be final. 
// ------------------------------------------------------------------------
	
inline int CLTAMaterial::GetType() const  { return m_Type ; } 

// ------------------------------------------------------------------------
// CLTAShape
// ------------------------------------------------------------------------

inline	void Shape::SetParentName( const char *parent_name ) { m_ParentName = parent_name ; }
inline	void Shape::SetMeshName  ( const char *mesh_name   ) { m_Geometry.m_Mesh.SetName( mesh_name ) ; } 	
inline	const std::string& Shape::GetParentName() { return m_ParentName ; } 
inline	const std::string& Shape::GetMeshName()   { return m_Geometry.m_Mesh.GetName() ; } 
inline	CLTAMesh&		Shape::GetMesh()			{ return m_Geometry.m_Mesh ; }
inline	CLTAMaterial*	Shape::GetAppearance()		{ return m_pAppearance  ; }

inline void Shape::SetAppearance(CLTAMaterial *val) { if (m_pAppearance != NULL ) delete m_pAppearance; m_pAppearance = val ;}



// ------------------------------------------------------------------------
// CLTANode 
// A node is a hierarchy element. usually a transform
// ------------------------------------------------------------------------	
	
inline void Node::AddShape( Shape *shape )           { shapeList.push_back(shape); }
inline void Node::AddChild( Node *new_node ) { children.push_back( new_node ); }



// ------------------------------------------------------------------------
// CLTAKeyFrames
// ------------------------------------------------------------------------
inline	int  CKeyFrame::Size()           const { return m_times.size() ; }
inline	void CKeyFrame::Append( const key_value & kv ) { m_times.push_back( kv ) ; }
inline	void CKeyFrame::Add( int time, int value  ) { m_times.push_back( key_value( time, value ) ) ; } 
inline	void CKeyFrame::Add( int time , std::string & value ) { m_times.push_back( key_value( time, value ) ) ; }
inline	void CKeyFrame::AddValue( int index, std::string & val  )   { m_times[ index ] .value = val ; }
inline	void CKeyFrame::operator=( const CKeyFrame & kf ) 
{
	m_times.clear();
	for( uint32 i = 0 ; i < kf.m_times.size() ; i++ )
		Append( kf.m_times[i] );
}

	// accessors 


inline	int  CKeyFrame::GetTime( int i ) const { return m_times[i].time ; } 
inline	void CKeyFrame::SetTime( int index, int new_time ) { m_times[index].time = new_time ;}
inline	const std::string& 
             CKeyFrame::GetVal( int i ) const { return m_times[i].value ; } 

	
// ------------------------------------------------------------------------
// ret CKeyFrame::GetIndex( time, start-time, end-time, percent-in-between)
//
// get the keyframe index, given time.
// fills ret-params with the the closest frames and the exact parametric point in between
// returns 1 on success, 0 on not finding any appropriate values
// ------------------------------------------------------------------------
inline	int CKeyFrame::GetIndex( int time, int &start, int &end, float &percent_in_between) const 
	{
		int timesSize = m_times.size();

		if( timesSize >= 2 )
		for( int i = 1 ;i < timesSize ; i++ )
		{
			if( time <= m_times[i].time )
			{
				start = i -1;
				end   = i   ;

				int startT = m_times[start].time ;
				int endT   = m_times[end].time ;

				percent_in_between = (( time - startT ) / (float)( endT - startT ));
				return 1;
			}
		}
		else {
			if( m_times.size() == 1 )
			{
				if( m_times[0].time == time )
				{
					start = 0;
					end   = 0;
					percent_in_between = 1.0f ;
					return 1;
				}
			}
		}
		return 0;
	}


// ------------------------------------------------------------------------
// CLTAAnim 
// ------------------------------------------------------------------------
inline int CAnim::GetType() { return m_Type ; }

// Make Own Copy of KF set from a ref
inline void CAnim::SetKeyFrameCopy( const CKeyFrame & kf )
{
	if( m_pKeyFrame == NULL )
	{
		m_pKeyFrame = new CKeyFrame ;	
	}

	*m_pKeyFrame = kf ;
	m_bKF_Shared = LT_FALSE ;
}

// Assign a shared version of the keyframe object
inline void CAnim::ShareKeyFrame( CKeyFrame *kf )
{
	// if not null, if already shared then overwrite the pointer, else delete old
	// data and assign the reference
	if( m_pKeyFrame != NULL )
	{
		if( m_bKF_Shared == LT_TRUE )
		{
			m_pKeyFrame = kf ;
		}else // the kf is not shared delete it to share it
		{
			delete m_pKeyFrame ;
		}
	}

	m_pKeyFrame = kf ;
	m_bKF_Shared = LT_TRUE ;	
}

inline CKeyFrame *CAnim::GetKeyFrame() { return m_pKeyFrame ; }

// returns 0 on fail
inline int  CAnim::GetKeyFrameIndexFromTime( int tm, int &start, int &end, float &percent )
{
	if( m_pKeyFrame ){
		return m_pKeyFrame->GetIndex( tm, start, end, percent );
	}else
		return 0;
}


// ------------------------------------------------------------------------
// CLTAVtxAnim
// version of the animation node that holds vertex animation.
/* 
(anim [name]
	(keyframe (keyframe ... ))
	(frames ("type" ... )))

this is the anim node, this class will be instanced if the "type" is vertex.
*/
// ------------------------------------------------------------------------

	// set what we are acting on,
	// name the animation after target
inline	void CVtxAnim::SetTarget( Shape * bs)               { m_target = bs ; if(bs!=NULL) SetName( bs->GetName() ) ;}
	inline Shape *CVtxAnim::GetTarget()                         { return m_target ; }
	
	// add an animation
inline	void CVtxAnim::AppendVertexList ( std::vector<LTVector> & v_list )
	{
		DefVertexLst newList ;
		newList.m_vVertex = v_list ;
		m_vFrames.push_back( newList  );
	}

	
	// frame acces 
inline	int CVtxAnim::NumVertexPerFrame()              { return m_vFrames[0].m_vVertex.size() ; }

inline	LTVector CVtxAnim::Get( int frame, int index ) 
	{
		// if debugging 
		
		//ASSERT( frame < m_vFrames.size() );
		//ASSERT( index < m_vFrames[frame].m_vVertex.size() );

		return m_vFrames[frame].m_vVertex[index];
	}

inline	std::string CVtxAnim::GetTargetName() {
		if( m_target )
			return m_target->GetName() ;
		return "";
	}


// ------------------------------------------------------------------------
// CLTAPosQuatAnim
// ------------------------------------------------------------------------
inline	void CPosQuatAnim::SetPosQuat( PosQuat & pq, float *p, float *q )
{
	memcpy( pq.pos, p, sizeof(float)*3);
	memcpy( pq.quat, q, sizeof(float)*4);
}

inline	void CPosQuatAnim:: GetPosQuat( PosQuat &pq, float *p , float *q )
{
	memcpy( p, pq.pos, sizeof(float)*3);
	memcpy( q, pq.quat,sizeof(float)*4);
}


// gets frame X
inline	void CPosQuatAnim:: Get( int frame, LTVector & pos, float *quat )
{
	GetPosQuat( m_vPosQuatDat[frame], &pos.x, quat); 
}

// append a new position/quaternion on to list
inline	void CPosQuatAnim:: Append( float *p, float *q )
{
	PosQuat pq ;
	SetPosQuat( pq, p, q );
	m_vPosQuatDat.push_back( pq );
}

// set what we are acting on,
// name the animation after target
inline	void CPosQuatAnim:: SetTarget( Node * bs)       { m_target = bs ; if(bs!=NULL) SetName( bs->GetName() ) ;}

inline	Node *CPosQuatAnim::GetTarget()                         { return m_target ; }

inline	std::string CPosQuatAnim::GetTargetName() { if( m_target) return m_target->GetName() ; else return std::string("");}


// ------------------------------------------------------------------------
// CLTAAnimSet
// ------------------------------------------------------------------------
	
inline	void CAnimSet::KFFix() 
{
	if( m_vAnims.size() > 0 ) 
	{
		CKeyFrame *pKF ;
		for( uint32 i = 0  ; i < m_vAnims.size() ; i++ )
		{
			pKF = m_vAnims[0]->GetKeyFrame() ;
			if( pKF != NULL )
				break ;
		}
		SetKeyFrame( *pKF );
	}
}

	// append
inline	void CAnimSet::AddAnim( CAnim * new_anim ) {		m_vAnims.push_back( new_anim ) ; }
	
	// find an animation by what its target is 
inline	CAnim *CAnimSet::FindAnimByTargetName( const std::string & name )
{
	for( uint32 i = 0 ;i < m_vAnims.size() ; i++ )
	{
		if( m_vAnims[i]->GetTargetName() == name ) 
		{
			return m_vAnims[i];
		}
	}
	return NULL ;
}

	// findAnim( name )
	// find an animation by name. Returns null if not found.
inline	CAnim *CAnimSet::FindAnim( const std::string & name )
{
	for( uint32 i = 0 ;i < m_vAnims.size() ; i++ )
	{
		if( m_vAnims[i]->GetName() == name ) 
		{
			return m_vAnims[i];
		}
	}
	return NULL ;
}

inline	void CAnimSet::SetKeyFrame( const CKeyFrame & new_kf )	{m_KeyFrame = new_kf ;	}

inline	CKeyFrame *CAnimSet::GetKeyFrame()  { return & m_KeyFrame ; }
	
	// empty returns if there are no anims... 
inline	int CAnimSet::Empty() 
	{
		return m_vAnims.size() != 0 ;
	}

	// returns the type of all the anims. if the set is heterogenous or empty it returns UNDEFINED
inline	int CAnimSet::GetAnimType() {
		int type=CAnim::UNDEFINED ;
		
		if( m_vAnims.size() > 0 )
		{
			type = m_vAnims[0]->GetType();

			for( uint32 i = 1 ;i < m_vAnims.size() ; i++ )
			{
				
				int t = m_vAnims[i]->GetType() ;
				if( t != type )
				{
					return CAnim::UNDEFINED  ;
				}
			}
		}
		return type ;
	}


// ------------------------------------------------------------------------
// lod-sets
// lod group :
// (distance - shape )
// ------------------------------------------------------------------------

	
inline void CLTALODGroup::SetMinMax( int min, int max ) { m_MinMax[0] = min ; m_MinMax[1] = max ; }
inline void CLTALODGroup::GetMinMax( uint32 &min, uint32 &max ) { min =m_MinMax[0] ;max = m_MinMax[1];  }

inline void CLTALODGroup::AddLOD( Shape *shape, float dist )
{
	m_LODs[dist] = shape ;
}

inline	Shape *CLTALODGroup::GetLOD( uint32 index )
{
	CLODMapIter it = m_LODs.begin();
	// check index's range?
	while( index-- ) it++ ;
	return (*(it)).second;
}

	
inline	Shape *CLTALODGroup::GetLOD( float dist )
{ 
	CLODMapIter it = m_LODs.lower_bound( dist );
	return (*it).second ;
}



// ------------------------------------------------------------------------
// CMetaModel
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// animation set access
// CreateAnimSet()    create a new set, optional name
// getLastAnimSet()   returns the last animset created.
// numAnimSets()      returns the number of animation sets
// getAnimSet(index)  gets an animation set
// ------------------------------------------------------------------------

// Create a new animations set. 
// metaModel is responsible for creating sets
inline CAnimSet * MetaModel::CreateAnimSet()
{
	
	CAnimSet * as = new CAnimSet ;
	if( as == NULL )
		return NULL ;
	m_LastAnim++;
	m_vAnimSets.push_back( as );
	return as ;
}

// create named animation set.
inline CAnimSet * MetaModel::CreateAnimSet(std::string & name )
{
	CAnimSet *as = CreateAnimSet();
	as->SetName( name );
	return as ;
}

// add an anim set.
inline void MetaModel::AppendAnimSet( CAnimSet *newAnimSet )
{
	m_vAnimSets.push_back( newAnimSet );
	m_LastAnim++;
}

// returns null if empty.
inline CAnimSet * MetaModel::GetLastAnimSet()
{
	if( m_LastAnim >= 0 )
		return m_vAnimSets[m_LastAnim];
	else return NULL ;
}

inline int  MetaModel::NumAnimSets(){ return m_vAnimSets.size() ; }

// get the anim set // note, no range check ... 
inline CAnimSet *MetaModel::GetAnimSet( int i ) 
{
	return m_vAnimSets[i]; 
}


// ------------------------------------------------------------------------
// find a shape by name .
// ------------------------------------------------------------------------
inline Shape * MetaModel::FindShape( const std::string & name ) 
{
	for( uint32 i = 0 ;i < shapes.size() ; i++ )
	{
		if( shapes[i]->GetName() == name ) 
		{
			return shapes[i] ;
		}
	}
	return NULL ;
}

inline Node * MetaModel::FindNode( const std::string & name )
{
	if( rootNode )
		return rootNode->FindName( name );
	return NULL ; 
}

// hook shape to parent node
// associates a shape to a node.
inline int MetaModel::HookShapeToParent( Shape *shape )
{
	Node *pNode = FindNode( shape->GetParentName() );
	if( pNode != NULL )
	{
		pNode->shape = shape ;
		return 1;
	}
	
	return 0 ;
}

// removes animsets that have no data.
inline void MetaModel::RemoveEmptyAnimSets()
{
	CAnimSet *pCurAnimSet ;
	std::vector<CAnimSet*>::iterator it;
	

	for(it = m_vAnimSets.begin(); it != m_vAnimSets.end() ; it++ )
	{
		pCurAnimSet = *it ;
		if( pCurAnimSet->Empty() ) {
			m_vAnimSets.erase( it );
			// delete pCurAnim
		}
	}
}

// sum up the number of verts in all the shapes .
inline int MetaModel::CalcNumVertex() 
{
	int totalVs = 0;
	for( uint32 i = 0 ;i < shapes.size(); i++ )
		totalVs+= shapes[i]->GetMesh().vertex.size() ;
	return totalVs ;
}

// add a lod set with name to meta model
inline void MetaModel::AddLODGroup( const std::string &name ) {
	m_LODGroups[ name ] =  CLTALODGroup(name) ;
}

// add information to a lod set.
inline bool MetaModel::AddToLODGroup( const std::string &lod_name, 
				  const std::string &shape_name, 
				  float dist, 
				  std::string &err_str )
{
	Shape * shape = FindShape( shape_name );
	if( shape != NULL)
	{
		m_LODGroups[ lod_name ].AddLOD( shape, dist );
		//m_LODGroups[ lod_name ].m_LODs[ dist ] = shape ;
	}
	else if(strcmp(shape_name.c_str(), "NULL") != 0)
	{
		err_str += " could not find shape " + shape_name ;
		err_str += " for lod group named : " + lod_name ; 

		return false ;
	}
	return true ;
}

// add min max lod offset to a lod-group
inline void MetaModel::AddMinMaxToLODGroup( const std::string &lod_name, int min, int max )
{
	m_LODGroups[ lod_name ].SetMinMax( min, max );
}

// get a lod 
inline Shape *MetaModel::GetLOD( uint32 piece_index, float dist )
{
	CLTALODGroupIterator &iter = m_LODGroups.begin();

	if( m_LODGroups.end() == iter )
	{
		return (*iter).second.GetLOD( dist );
	}
}

// get numpieces
inline uint32 MetaModel::GetNumLODGroups() {
	m_LODGroups.size();
}




// ------------------------------------------------------------------------
// CLTATranslStatus
// ------------------------------------------------------------------------


// call this to announce an error has occured.
inline void CLTATranslStatus::OnError( char *ErrorTitle, int bShowErrWin ) 
{	
	std::string msg ;
	if( ErrorTitle )
	{
		msg = ErrorTitle ;
	}else 
		msg = "error " ;
#ifdef _WINDOWS
	if( bShowErrWin )
		MessageBox(NULL, m_msg.c_str(), msg.c_str() , MB_OK );
#endif
	if( s_fnLoadLogCallBack )
	{
		s_fnLoadLogCallBack( msg.c_str() );
		s_fnLoadLogCallBack( m_msg.c_str());
		s_fnLoadLogCallBack( "\r\n");
	}

	errLog() << msg.c_str() << " " << m_msg.c_str() << std::endl;
}

// ------------------------------------------------------------------------
// seterror log ( out-stream )
// static function that sets the error log stream to which 
// all messages are going to be written to.
// ------------------------------------------------------------------------
inline void CLTATranslStatus::SetErrorLog( std::ostream * ostr )
{
	s_CurErrorLog = ostr ;
}

inline void CLTATranslStatus::SetLoadLogCB( void (*fnPtr)(const char *) )
{
	s_fnLoadLogCallBack = fnPtr ;
}

inline void CLTATranslStatus::AddToLog( const std::string & str )
{
	if( s_fnLoadLogCallBack )
		s_fnLoadLogCallBack( str.c_str() );
}

// returns the current buffer that all msgs are sent to.
inline std::ostream& CLTATranslStatus::errLog() { if(s_CurErrorLog) return *s_CurErrorLog ; else return std::cout ; }

// the << operator. treat instance as a out-stream
inline CLTATranslStatus & CLTATranslStatus::operator<<( const char *msg ){
	m_msg += std::string(msg);

	//if(s_fnLoadLogCallBack != NULL ){
		//s_fnLoadLogCallBack( m_msg.c_str() );
	//}

	return *this ;
}


// resets the status for more use. 
inline void CLTATranslStatus::reset() {
	m_status = OK;
	m_msg = "";
}


#endif
