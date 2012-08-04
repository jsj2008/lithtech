// ------------------------------------------------------------------------
// lithtech (c) 2000
// ltaScene.cpp
// contains operations for translating a parse node tree into a lta scene.
// which is an in-mem representation of the lta ascii file.
// ------------------------------------------------------------------------

#include <assert.h>
#pragma warning (disable:4786)

#include "ltaScene.h"
#include "ltamgr.h"

using namespace std;

ostream *CLTATranslStatus::s_CurErrorLog = NULL ;
void (*CLTATranslStatus::s_fnLoadLogCallBack)( const char *) = NULL ;


///////////////////////////////////////////////////////////////////////////
/// NODE 
///////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// Node Default constr
// ------------------------------------------------------------------------
Node::	Node() {
	shape=NULL ; // reference to shape, don't belong to node.
	index = 0 ; // ltmodel
	parent=NULL ;
	
		 mat.Identity();
	}

// ------------------------------------------------------------------------
// Node name constr
// ------------------------------------------------------------------------
Node::Node( string &name ){
	mat.Identity();
	shape=NULL ; // reference to shape, don't belong to node.
	index = 0 ; // ltmodel
	parent=NULL ;
	
	SetName(name);
}

// ------------------------------------------------------------------------
// deconstr
// ------------------------------------------------------------------------
Node::~Node()
{
	for( int i = 0 ; i < (int)children.size() ;i++ )
	{
		Node *child = children[i] ;
		delete child ;
		children[i] = NULL;
	}
	children.resize(0);
}

// finds the node with the name sname ...
Node *Node::FindName( const string & sname )
{
	if( sname == GetName() )
		return this ;
	else 
	{
		for( int i = 0 ;i < (int)children.size() ; i++ )
		{
			Node *node = children[i]->FindName(sname );
			if( node != NULL )
				return node ;
		}
	}
	return NULL ;
}

///////////////////////////////////////////////////////////////////////////
/// NODE 
///////////////////////////////////////////////////////////////////////////


CLTA_ABC_PC_MAT::CLTA_ABC_PC_MAT()
	:m_specular_power(0), m_specular_scale(0), 
	m_num_textures(0), CLTAMaterial( PC_MAT ) 
{
		m_diffuse[0] = m_diffuse[1] = m_diffuse[2] = 0.8f ;
		m_diffuse[3] = 1.0f ;
		m_ambient[0] = m_ambient[1] = m_ambient[2] = 0.0f ;
		m_emissive[0] = m_emissive[1] = m_emissive[2] = 0.0 ;
		m_specular[0] = m_specular[1] = m_specular[2] = 0.0 ;
		m_shininess = 0.0f ;
} 

CLTARenderStyle::CLTARenderStyle() :CLTAMaterial(RENDERSTYLE) 
	{
		m_nNumTextures = 1;	// Defaults...
		m_iTextures[0] = 0; m_iTextures[1] = 1; m_iTextures[2] = 2; m_iTextures[3] = 3;
		m_iRenderStyle = 0;
		m_nRenderPriority = 0;
	}


Shape::	Shape() :m_pAppearance(NULL)  {}
Shape::~Shape() { if(m_pAppearance != NULL ) delete m_pAppearance; }


CAnimSet::~CAnimSet() {
	// delete m_vAnims ...
	for( uint32 i = 0 ;i < m_vAnims.size() ;i++ )
	{
		delete m_vAnims[i] ;
	}
	m_vAnims.clear();
}

MetaModel::MetaModel() 		:m_LastAnim(-1),rootNode(NULL) 
	{}

MetaModel::~MetaModel()
	{
		for( uint32 i = 0 ; i < shapes.size() ; i++ )
		{
			Shape * shape = shapes[i] ;
			delete shape ;
			shapes[i] = NULL;
		}
		shapes.resize(0);

		for ( i = 0 ; i < m_vAnimSets.size() ; i++ )
		{
			CAnimSet *pAnimSet = m_vAnimSets[i];
			delete pAnimSet ;
			m_vAnimSets[i] = NULL ;
		}
		m_vAnimSets.resize(0);

		if( rootNode != NULL )
			delete rootNode ;
	}


	
///////////////////////////////////////////////////////////////////////////
/*************************************************************************/
///////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// KeyFrameTimeSlideToZero
// if the first key frame does not start at 0, slide down the rest of the 
// values.
// ------------------------------------------------------------------------
void KeyFrameTimeSlideToZero( CKeyFrame & kf )
{
	int num_times = kf.Size();
	
	if( num_times < 1 )
		return ;

	int base_time  = kf.GetTime( 0 ) ;

	if( base_time == 0 )
		return ;

	// shift the times values
	for( int i = 0 ;i < num_times ; i++ )
	{
		int new_time = kf.GetTime( i ) - base_time ;
		kf.SetTime( i , new_time );

	}
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// Parse tree to Meta Format 
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// ParseVector( ParseNode *vertex, float *array )
// reads an lta node in ( x y z ... ) format into a float array.
// array must be alloc'd & the same length as vrt->GetNumElements()
// ------------------------------------------------------------------------
void
LTA::ParseVector( CLTANode *vrt, float * vert )
{
	for( int i = 0 ; i < (int)vrt->GetNumElements() ; i++ )
	{
		vert[i] = (float)atof( vrt->GetElement(i)->GetValue());
	}
}

// ------------------------------------------------------------------------
// ParseShapeNode( shape, root )
// convert the parse node that's really a shape into a c++ -struct shape.
// pnroot = (shape name  
//             (parent name )
//             (geometry 
//				(mesh name ( vertex ( (v1) (v2) ... ) ) (tri-ifs < a b c>>)
//
// ------------------------------------------------------------------------
void
ParseShapeNode( Shape *shape, CLTANode *pnroot )
{
	CLTANode *tmp,*mesh ;
	uint32 i ;
	// tmp = (shape name ... 
	shape->SetName( pnroot->GetElement(1)->GetValue()  );
	CLTATranslStatus::AddToLog( " - " + shape->GetName() + "\r\n" );
	tmp = CLTAUtil::ShallowFindList(pnroot,"parent" );	
	// tmp = (parent name)
	if( tmp != NULL )
		shape->SetParentName(tmp->GetElement(1)->GetValue());


	// ------------------------------------------------
	tmp = CLTAUtil::ShallowFindList( pnroot , "geometry" );

	// mesh = ( mesh name (vertex ... ) (tri-fs ... ))
	mesh = CLTAUtil::ShallowFindList( tmp, "mesh");
	
	
	// if there is no mesh skip
	if( mesh != NULL )
	{
		// get the mesh name, if its there
		if( mesh->GetElement(1)->IsAtom() )
			shape->SetMeshName ( mesh->GetElement(1)->GetValue() );

		
		tmp = CLTAUtil::ShallowFindList( mesh , "vertex" );
		if(tmp )
		{
			// tmp = ( vertex ( (v1) (v2) .. )) 
			tmp = tmp->GetElement(1);// tmp = ( (v2) (v2) .. )

			float vert[3] ;
			// load up the vertex
			for( i = 0 ;i < (int)tmp->GetNumElements() ; i++ )
			{
				
				LTA::ParseVector( tmp->GetElement(i), vert );
				// shape->vertex[i] = new vert
				shape->GetMesh().vertex.push_back( 
						LTVector(vert[0],vert[1],vert[2]));
			}
		}

		tmp = CLTAUtil::ShallowFindList( mesh , "normals" );
		
		if(tmp )
		{
			// tmp = ( vertex ( (v1) (v2) .. )) 
			tmp = tmp->GetElement(1);// tmp = ( (v2) (v2) .. )
			float vert[3] ;
			// load up the vertex
			for( i = 0 ;i < tmp->GetNumElements() ; i++ )
			{	
				LTA::ParseVector( tmp->GetElement(i), vert );
				shape->GetMesh().normals.push_back(
						LTVector(vert[0],vert[1],vert[2]));	
			}

			tmp = CLTAUtil::ShallowFindList( mesh, "nrm-fs") ;
			if(tmp){
				tmp = tmp->GetElement(1) ; // get the list 
				for( i = 0 ;i < (int)tmp->GetNumElements() ; i++ )
				{
					int val = atoi(tmp->GetElement(i)->GetValue());
					//shape->uv_fs.push_back( val );
					shape->GetMesh().tri_NormSet.push_back( val );
				}
			}

		}

		tmp = CLTAUtil::ShallowFindList( mesh, "tri-fs");
		if(tmp)
		{
			// tmp = ( 1 2 3... )
			tmp = tmp->GetElement(1) ; // get the index list
			for( i =  0 ; i < (int)tmp->GetNumElements() ; i++ )
			{
				int val = atoi( tmp->GetElement(i)->GetValue() ) ;
				//shape->triangles.push_back( val );
				shape->GetMesh().tri_FaceSet.push_back( val ) ;
			}
		}

		// get the uv coords 
		tmp = CLTAUtil::ShallowFindList( mesh, "uvs" );
		if( tmp )
		{
			float vert[2];
			tmp = tmp->GetElement(1) ; // get the param
			for( i = 0 ; i < (int)tmp->GetNumElements() ; i++ )
			{
				LTA::ParseVector( tmp->GetElement(i), vert );
				shape->GetMesh().tcoords.push_back( LTVector( vert[0], vert[1], 0 ));
			}
		

			tmp = CLTAUtil::ShallowFindList( mesh, "tex-fs") ;
			if(tmp){
				tmp = tmp->GetElement(1) ; // get the list 
				for( i = 0 ;i < (int)tmp->GetNumElements() ; i++ )
				{
					int val = atoi(tmp->GetElement(i)->GetValue());
					//shape->uv_fs.push_back( val );
					shape->GetMesh().tri_UVSet.push_back( val );
				}
			}
			else 
			{
				CLTATranslStatus::AddToLog("WARNING : " );
				CLTATranslStatus::AddToLog(shape->GetName());
				CLTATranslStatus::AddToLog(" has no texture coords/index\r\n");
				// *** should report that there was no texture face set
			}
		}

		tmp = CLTAUtil::ShallowFindList( mesh , "colors" );
		if(tmp )
		{
			// tmp = ( vertex ( (v1) (v2) .. )) 
			tmp = tmp->GetElement(1);// tmp = ( (v2) (v2) .. )

			float vert[3] ;
			// load up the vertex
			for( i = 0 ;i < (int)tmp->GetNumElements() ; i++ )
			{
				
				LTA::ParseVector( tmp->GetElement(i), vert );
				// shape->vertex[i] = new vert
				shape->GetMesh().colors.push_back( 
						LTVector(vert[0],vert[1],vert[2]));
			}
		}


	}// if there's a mesh node 

	// Setup the appearance node.
	CLTARenderStyle *pRS = new CLTARenderStyle ;
	shape->SetAppearance( pRS );
			
  	tmp = CLTAUtil::ShallowFindList( pnroot , "appearance" );
	if (tmp) 
	{
  		tmp = tmp->GetElement(1) ; // cdr the appearance pair
		// we have a mat node
		if (tmp) 
		{ 
			// if old style material node short circuit it.
  			if( strcmp( tmp->GetElement(0)->GetValue(),  "material") == 0 || strcmp( tmp->GetElement(0)->GetValue() , "pc-mat") == 0) 
			{
				CLTANode* mat_attrib =  CLTAUtil::ShallowFindList( tmp, "texture-index");
				if (mat_attrib) 
				{
					pRS->m_nNumTextures = 1;
					pRS->m_iTextures[0] = atoi(mat_attrib->GetElement(1)->GetValue());
					pRS->m_iTextures[1] = 1;
					pRS->m_iTextures[2] = 2;
					pRS->m_iTextures[3] = 3; 
				} 
			}
			else if( strcmp( tmp->GetElement(0)->GetValue(), "render-style") == 0)
			{
				CLTANode* mat_attrib = CLTAUtil::ShallowFindList(pnroot, "texture-indices");
				if (mat_attrib) 
				{
					mat_attrib = mat_attrib->GetElement( 1 );
					pRS->m_nNumTextures = mat_attrib->GetNumElements();
					assert( pRS->m_nNumTextures > 0 && pRS->m_nNumTextures <= 4 );
					for( i = 0; i < pRS->m_nNumTextures; i++ ) 
					{
						pRS->m_iTextures[i] = atoi( mat_attrib->GetElement( i )->GetValue() ); 
					}
					for (;i<4;++i) 
						pRS->m_iTextures[i] = i; 
				}
			}	
		} 
	}else // there is no appearance node 
	{
		CLTANode* mat_attrib = CLTAUtil::ShallowFindList(pnroot, "texture-indices");
		if (mat_attrib) {
			mat_attrib = mat_attrib->GetElement( 1 );
			pRS->m_nNumTextures = mat_attrib->GetNumElements();
			for( i = 0; i < pRS->m_nNumTextures; i++ ) {
				pRS->m_iTextures[i] = atoi( mat_attrib->GetElement( i )->GetValue() ); }
			for (;i<4;++i) pRS->m_iTextures[i] = i; }

		// load the renderstyle index
		CLTANode* rs_attrib = CLTAUtil::ShallowFindList(pnroot, "renderstyle-index");
		if( rs_attrib )
			pRS->m_iRenderStyle = atoi( rs_attrib->GetElement( 1 )->GetValue() );

		// load the render priority
		CLTANode* renderpriority = CLTAUtil::ShallowFindList(pnroot, "render-priority");
		if( renderpriority )
			pRS->m_nRenderPriority = max(0, min(255, atoi( renderpriority->GetElement( 1 )->GetValue() )));

	}
}

// ------------------------------------------------------------------------
// 
// collect vertex animated orphans, create a node for them 
// if a va shape does not have parent then we need to create one for them
// b/c of the current setup in lt. this should not be inherent in lta. 
// lta should just say this animation modifies this thing without reference
// to how it is structured.
// this should be called after the metaModel has been fully loaded.
// ------------------------------------------------------------------------
static 
void FixVertexAnimatedOrphans( MetaModel & metaModel )
{
	std::vector< Shape *> orphans ;
	std::vector< Shape *> &model_shapes = metaModel.shapes ;
	Node *root_node = metaModel.rootNode ;

	// for each shape in meta check if it has the name d_ something
	// if there is no parent associated with the shape create one.
	// create a new node from the root, calling the new node the same as the shape.
	
	for( uint32 i = 0 ;i < model_shapes.size() ;i++ )
	// this is so that the animation has a place to go.
	for( uint32 i = 0 ;i < model_shapes.size() ;i++ )
	{
		// if we don't have a parent name 
		if( model_shapes[i]->GetParentName() == "" )
		{
			string & shape_name = model_shapes[i]->GetName() ;

			if( shape_name.size() < 2 )
				continue ;
			
			if( shape_name[0] == 'd' && shape_name[1] == '_' )
			{
				Node *new_node = new Node( shape_name );
				new_node = metaModel.FindNode( model_shapes[i]->GetName() );

				// if one does not exist make one.
				if( new_node == NULL ){
					new_node = new Node( shape_name );
					root_node->AddChild( new_node );
				}

				new_node->AddShape( model_shapes[i] );
			}
		}
	}

}

// ------------------------------------------------------------------------
// GetShapesFromPN( metaModel, CLTANode )
// get all the shapes from the ParseNode tree, stuff it into metaModel
// ------------------------------------------------------------------------
 void 
LTA::GetShapesFromPN( MetaModel & metaModel, CLTANode *pnroot )
{
	vector<CLTANode*> vshapes ;

	CLTAUtil::FindAll( vshapes, pnroot, "shape" );

	for( int i = 0 ;i < (int)vshapes.size() ; i++ )
	{
		ShapeFromPN( metaModel, vshapes[i] );
		//Shape *shape = new Shape ;
		//ParseShapeNode( shape, vshapes[i] );
		//metaModel.hookShapeToParent( shape );
		//metaModel.shapes.push_back(shape);
	}

}


// ------------------------------------------------------------------------
// ShapeFromPN
// ------------------------------------------------------------------------
void 
LTA::ShapeFromPN( MetaModel & metaModel, CLTANode *pnroot )
{
	Shape *shape = new Shape ;
	ParseShapeNode( shape, pnroot );
	metaModel.HookShapeToParent( shape );
	metaModel.shapes.push_back( shape );
}


// ------------------------------------------------------------------------
// GetLODGroupsFromPN
// must be called after all the shapes are in.
// ------------------------------------------------------------------------
void 
LTA::GetLODGroupsFromPN( MetaModel & metaModel, CLTANode *pnroot, CLTATranslStatus &status )
{
	vector<CLTANode*> create_lod_groups ;
	CLTANode * finder = CLTAUtil::FindList(pnroot,"on-load-cmds");
	if( finder )
	{
		finder = CLTAUtil::FindList(finder, "lod-groups");
		if( finder )
		{
			finder = finder->GetElement(1) ; // get data-list from lod-groups node
		
			for ( uint32 i = 0 ; i < finder->GetNumElements() ; i++ )
			{
				LTA::LODGroupFromPN( metaModel, finder->GetElement(i),  status );
			}
		}
	}
}

// ------------------------------------------------------------------------
// AddOBBFromPN()  
//	parses :
//		(add-node-obb "node-name" 
//			(pos ( x y z )) 
//			(orientation ( a b c d))
//			(size ( x y z )))
// ------------------------------------------------------------------------
static 
void GetOBBFromPN( MetaModel & metaModel, CLTANode *add_obb_cmd, CLTATranslStatus &status)
{
	CLTANode *name = add_obb_cmd->GetElement(1); // (aoc "name" ... )
	CLTANode *pos = CLTAUtil::ShallowFindList( add_obb_cmd,"position");
	CLTANode *quat= CLTAUtil::ShallowFindList( add_obb_cmd,"orientation");
	CLTANode *size = CLTAUtil::ShallowFindList( add_obb_cmd,"dimensions");
	CLTANode *enabled= CLTAUtil::ShallowFindList( add_obb_cmd, "enabled");
	CLTAOBB obb;
	
	obb.m_ParentNodeName = name->GetValue();
	if(pos)
	LTA::ParseVector( pos->GetElement(1), obb.m_Pos );
	if(quat)
	LTA::ParseVector( quat->GetElement(1),obb.m_Orientation);
	if(size)
	LTA::ParseVector( size->GetElement(1),obb.m_Size);

	if(enabled){
		if( strcmp(enabled->GetElement(1)->GetValue(),"false") == 0 )
			obb.m_Enabled = false ;
		else 
			obb.m_Enabled = true ;
	}


	obb.m_pParentNode = metaModel.FindNode(obb.m_ParentNodeName );
	metaModel.m_obbset.push_back( obb ); // add it to list
}

// ------------------------------------------------------------------------
// get obbs from on-load-cmd
// must be called after hierarchy is in.
// ------------------------------------------------------------------------
void
LTA::GetOBBFromOnLoadCmdsPN( MetaModel & metaModel, CLTANode *pnroot, CLTATranslStatus &status)
{
	CLTANode * finder = CLTAUtil::FindList(pnroot,"on-load-cmds");
	
	if( finder && (finder->GetNumElements() > 1))
	{
		finder = finder->GetElement(1); // get the list.
		finder = CLTAUtil::ShallowFindList(finder,"add-node-obb-list");
		if( finder && (finder->GetNumElements() > 1) )
		{
			finder = finder->GetElement(1); // get the list from node 
			for( uint32 add_obb = 0 ; add_obb < finder->GetNumElements(); add_obb++)
			{
				metaModel.m_obbset.reserve( finder->GetNumElements()) ;
				GetOBBFromPN( metaModel, finder->GetElement(add_obb), status );
			}
		}
	}
}


// ------------------------------------------------------------------------
// (create-lod-group name 
//		(min-max-offset 0 0 )
//		(dists ( ... ) )
//		(shapes ( ... ) ))
// ------------------------------------------------------------------------
void 
LTA::LODGroupFromPN(  MetaModel &metaModel, CLTANode *create_lod_group, CLTATranslStatus &status )
{
	if( create_lod_group )
	{
		string name = "UNNAMED";
		if( create_lod_group->GetElement(1)->IsAtom())
			name = create_lod_group->GetElement(1)->GetValue();
		else {
			status.AddToLog("WARNING: create-lod-group without name \r");
		}
		
		CLTANode *minmax= CLTAUtil::ShallowFindList( create_lod_group, "min-max-offset");
		CLTANode *dists=  CLTAUtil::ShallowFindList( create_lod_group, "lod-dists");
		CLTANode *shapes= CLTAUtil::ShallowFindList( create_lod_group, "shapes");

		// make a lod set
		metaModel.AddLODGroup( name );
		// get min-max offsets
		if( minmax && minmax->GetNumElements() >= 3)
		{
			int min,max ;
			min = atoi(minmax->GetElement(1)->GetValue());
			max = atoi(minmax->GetElement(2)->GetValue());

			metaModel.AddMinMaxToLODGroup( name, min, max );
		}

		if( shapes || dists )
		{
			dists = dists->GetElement(1) ; // pop
			shapes= shapes->GetElement(1); 

			// get dist-shape relations
			for( uint32 i = 0 ; i < shapes->GetNumElements() ; i++ )
			{
				string err_str ;
				float dist = (float)atof(dists->GetElement(i)->GetValue());

				string shape_name = shapes->GetElement(i)->GetValue() ;
				
				if(!metaModel.AddToLODGroup(name, shape_name, dist,err_str ))
				{
					status.AddToLog( string("ERROR : ") + err_str + "\r" );
				}
			}
		}
	}
}

// ------------------------------------------------------------------------
// parse hierarchy node's children
// types processed :
/*
children :
	(children ( (children ()) (transform ... ))) 
	would result in
	null -> null 
		 \> transform 

transform :
	(transform name 
		(matrix ( () ()()() ) )
		(children ( ... ) )
		)
shapes :
	(transform name 
		(matrix 
		(children ( (shape ... ) (shape ... ) ))
  */
// ------------------------------------------------------------------------
static int
GetTransformFromPN( Node *node, CLTANode *pnroot )
{
	CLTANode *children=NULL, *matrix ;

	// set up node
	node->mat.Identity();

	// check if the list is a transform 
	if( strcmp( pnroot->GetElement(0)->GetValue() , "transform" ) == 0)
	{
		// get the transform's name if there is one
		if( pnroot->GetNumElements() > 1 && pnroot->GetElement(1)->IsAtom() )
		{
			node->SetName( pnroot->GetElement(1)->GetValue() );	
		}

		// get the matrix from this transform
		matrix = CLTAUtil::ShallowFindList( pnroot, "matrix" );

		// plow through matrix list to get the basis
		if( matrix != NULL && matrix->IsList())
		{
			matrix = matrix->GetElement(1) ;
			// matrix = ( (...) (...) (...) (...) ) 

			LTMatrix mat;
		
			for( int i = 0 ; i < 4 ; i ++ )
			{
				LTA::ParseVector( matrix->GetElement(i),mat.m[i]);
			}
		
			node->mat  = mat ;
		}
		// get transform's children
		children = CLTAUtil::ShallowFindList( pnroot, "children");
	}
	// this node is a 'null' node 
	else  	if(strcmp( pnroot->GetElement(0)->GetValue() , "children" ) == 0 )
	{
		node->SetName("null");
		children = pnroot ;
	}
	// we have a child shape
	else if( strcmp( pnroot->GetElement(0)->GetValue() , "shape" ) == 0 )
	{
		CLTATranslStatus status;
		status.m_msg = "Shapes in hier not tested ... " ;
		status.OnError("Parsing Hierarchy",0);
	
		Shape *shape = new Shape ;
		ParseShapeNode( shape, pnroot );
		node->AddShape( shape );
	
	}

	// if children 
	if( children != NULL && children->GetNumElements() > 0)
	{
		// pop the children list to the list of transforms
		children = children->GetElement(1) ;// ( (t ...) (t ...) (t ...) )
		int index = node->index +1;
		for( int i = 0 ;i < (int)children->GetNumElements() ; i++ )
		{
			Node *chld = new Node ;
			chld->parent = node ;
			chld->index = index ;
			
			if(GetTransformFromPN(chld, children->GetElement(i) ))
			{
				node->children.push_back( chld );
				index++;
			}
			else {
				delete chld ;
			}	
		}
	}
	return 1;
}



// ------------------------------------------------------------------------
// KeyFrameFromPN
// KeyFrame from a parsenode 
// keyframe format : the name is optional.
//	(keyframe name 
//	         (times ( val val val )) 
//	         (value ( val val val )))
// ------------------------------------------------------------------------
static 
void KeyFrameFromPN( CLTANode *kf , CKeyFrame & KF, CLTATranslStatus & status  )
{
	// check that pn is a keyframe 
	if( kf->GetElement(0)->IsAtom() )
	{
		string name = kf->GetElement(0)->GetValue();
		if( name != "keyframe" )
		{
			status = status.FAIL;
			status.m_msg = "parse node is not a keyframe node " ;
			return ;
		}else {
			// get name if there's one ..
			if( kf->GetElement(1)->IsAtom()  )
			{
				KF.SetName( kf->GetElement(1)->GetValue() );
			}
		}

		CLTANode *times = CLTAUtil::FindList( kf , "times" );
		CLTANode *value = CLTAUtil::FindList( kf , "values");

		if( times  ) 
		{		
			// go through the times list
			// the second element the times node is the value list
			times = times->GetElement(1) ; 
			

			for( int i = 0 ; i < (int)times->GetNumElements() ; i++ )
			{
				int ival = atoi(times->GetElement(i)->GetValue()) ;
			 
				KF.Add( ival, i  );
			}
		}

		// fix the time range so it starts at zero.
		KeyFrameTimeSlideToZero( KF );

		// parse (value ( ... ) )
		if( value )
		{
			value = value->GetElement(1) ;
	
			// cut out of func here if value is null ... 
			if( value == NULL ) 
				return ;

			for( int i = 0 ; i < (int)value->GetNumElements() ; i++ )
			{
				
				string str_val ;
				
				// if there's an associated value...
				if( value->GetElement(i) != NULL && value->GetElement(i)->GetValue() != NULL )
				{
					const char *cval = value->GetElement(i)->GetValue();
					
					if( value && value->GetElement(i)->IsString() && strlen(cval) > 2 )
						str_val = value->GetElement(i)->GetValue() ;
				}
			 
				KF.AddValue( i , str_val );

			}// for the items in the value node 
		}// value node is there
	}// if parse nod eis a kf
}

// ------------------------------------------------------------------------
// AddAnimNodesToAnimSet
// returns 1 if function really changed/added data anim
// little thing to remember: data will not get filtered out here, if a file
// that has more animation channels than connections : (anim "foo" (parent "bif" )
// and there's no bif, well it gets read in anyway, let the consumer of the metaModel 
// deal with extra, unused data.
// ------------------------------------------------------------------------
static
int AddAnimNodesToAnimSet(  CLTANode *anim, 
							CAnimSet  *pCurAnimSet,
						    MetaModel & metaModel, 			   
						    CLTATranslStatus & status )
{
	CLTANode *keyF, *frames , *parent;
	CAnim *pAnim ;
	CAnimSet *pAnimSet = pCurAnimSet;
	string animName ;
	int    bHasFrameData=0 ; // t/f if there has been data read in

	// get the animation name 
	if( anim->GetElement(1)->IsAtom () )
	{
		string name ;
		animName = anim->GetElement(1)->GetValue();
	}

	// get the attribs
	keyF =  CLTAUtil::ShallowFindList( anim, "keyframe");
	frames= CLTAUtil::ShallowFindList( anim, "frames" );
	parent= CLTAUtil::ShallowFindList( anim, "parent" );

	if( frames == NULL )
	{
		status = status.FAIL ;
		status.m_msg = "no frames in this anim node " ;
		return bHasFrameData;
	}
	
	// get the frame attribs' data which is a list with its first element is the 
	// type 
	frames = frames->GetElement(1) ;// (frames (type ... )) => (type ... )
	if( frames != NULL )
	{
		assert( frames->GetElement(0) != NULL );
		string type = frames->GetElement(0)->GetValue();
	
		if( type == "vertex" )
		{
			CVtxAnim *pVtxAnim = new CVtxAnim ;
			int i ;
			Shape *shape = NULL ;
			
			// find the anim target 
			if( parent != NULL )
			{
				string parent_name = parent->GetElement(1)->GetValue();

				shape = metaModel.FindShape( parent_name );
				if ( shape == NULL )
				{
					// no shape error 
					//status = status.FAIL ;
					status.m_msg = " this vertex animation has no target .. " ;
				//	delete pVtxAnim ;
				//	return bHasFrameData;
				}
				pVtxAnim-> SetTarget( shape );

			}else {
				// error no target
			}
			
			
			// fish out the vertex animations
			CLTANode *tmp;
			// tmp = a list of list of vertex
			tmp = frames->GetElement(1);// tmp = ( (v2) (v2) .. )
			int tmpval = tmp->GetNumElements();
			for( int nLists = 0 ; nLists < (int)tmp->GetNumElements() ; nLists++ )
			{
				CLTANode *vecList = tmp->GetElement(nLists) ;
				
				vector<LTVector> vVecs ;
				float vert[3] ;
				int tmpval2 = vecList->GetNumElements();
				for( i = 0 ;i < (int)vecList->GetNumElements() ; i++ )
				{
					
					LTA::ParseVector( vecList->GetElement(i), vert );
					vVecs.push_back(LTVector(vert[0],vert[1],vert[2]));	
				}
				
				pVtxAnim->AppendVertexList( vVecs );
			}

			if (tmp->GetNumElements() > 0 ) 
				bHasFrameData = 1;

			pAnim = pVtxAnim ;
		}
		else if( type == "posquat" ) 
		{
			CPosQuatAnim *pPQAnim = new CPosQuatAnim ;
			Node *node = NULL ;

			if( parent != NULL )
			{
				string parent_name = parent->GetElement(1)->GetValue();
				Node *node = metaModel.FindNode( parent_name );
				if ( node == NULL )
				{
					// no shape error 
					//status = status.FAIL ;
					status.m_msg = parent_name ;
					status.m_msg += " " ;
					status.m_msg += " this POS/QUAT animation has no target .. " ;
			//		delete pPQAnim ;
			//		return bHasFrameData;
				}
				pPQAnim->SetTarget( node );
			}
		
			// read in pos-quats
			float pos[3];
			float quat[4];
			CLTANode *tmp;
			// tmp = a list of list of vertex
			tmp = frames->GetElement(1);// tmp = ( (v2) (v2) .. )
			int tmpval = tmp->GetNumElements();
			for( int nLists = 0 ; nLists < (int)tmp->GetNumElements() ; nLists++ )
			{
				CLTANode *posQuatLst = tmp->GetElement(nLists) ;
			
				int tmpval2 = posQuatLst->GetNumElements();
				
				LTA::ParseVector( posQuatLst->GetElement(0), pos );
				LTA::ParseVector( posQuatLst->GetElement(1), quat);
	
				pPQAnim->Append( pos, quat );
			}

			bHasFrameData = tmp->GetNumElements() > 0 ;
			pAnim = pPQAnim ;			
			
		} // pos quat anim
	} // has frame s

	// get keyframe from anim node
	// if there is no KF node in lta, get it from the anim set
	if( keyF != NULL )
	{
		CKeyFrame KeyFrame ;
		CLTATranslStatus kfstatus ;
		KeyFrameFromPN( keyF, KeyFrame , kfstatus );
		if( kfstatus != kfstatus.OK ){
			kfstatus.OnError("AddAnimNodeToAnimSet",true );
			status = kfstatus ;
			return 0;
		}

		pAnim->SetKeyFrameCopy( KeyFrame );
	}
	else 
	{
		pAnim->ShareKeyFrame( pAnimSet->GetKeyFrame()) ;
		// that's ok use anim sets' kf
	}

	// finalize this animatin & append it to set.
	pAnim->SetName( animName );

	// if there is no data, don't add the animation to the set.
	if( bHasFrameData )
		pAnimSet->AddAnim( pAnim );
	else 
		delete pAnim ;

	return bHasFrameData ;
}


// --------------------------------------------
// AnimSetFromAnimSetPN( meta-model, parse-tree = (animset ...), status 
// --------------------------------------------
void LTA::AnimSetFromPN( MetaModel & metaModel,
							CLTANode *animsetPN,
							CLTATranslStatus & status )
{
		CAnimSet *pAnimSet = new CAnimSet ;
		CLTANode *KFNode, *AnimList ;
		if( pAnimSet == NULL )
		{
			status = status.OUT_OF_MEM ;
			status.m_msg = " OUT OF MEMORY!!! ALLOC CAnimSet ";
			return ;
		}
		// get the anim set name 
		// if no name then anim + i 
		string name ;
		if( animsetPN->GetElement(1)->IsAtom() )
		{
			name = animsetPN->GetElement(1)->GetValue() ;
			CLTATranslStatus::AddToLog( name.c_str() );
			
		}else 
		{
			char buf[34];
			itoa( 0 , buf, 10 );
			name = "base" ;
			name += buf ;
		}

		pAnimSet->SetName( name );

		KFNode = CLTAUtil::ShallowFindList( animsetPN, "keyframe" );

		if( KFNode != NULL )
		{
			CKeyFrame KeyFrame ;
			CLTATranslStatus kfstatus ;
			// KFNode = (keyframe (keyframe ... )  we want elem 2
			KFNode = KFNode->GetElement(1) ;
			if( KFNode )
				KeyFrameFromPN( KFNode, KeyFrame , kfstatus );
			if( kfstatus != kfstatus.OK ){
				//MessageBox(NULL, kfstatus.m_msg.c_str(), "AnimSetFromPN", MB_OK);
				kfstatus.OnError("AnimSetFromPN");
				status = kfstatus ;
				return ;
			}
			pAnimSet->SetKeyFrame( KeyFrame );
		}

		// AnimList => (anims ( ... ) )
		AnimList = CLTAUtil::ShallowFindList( animsetPN, "anims" );
		int bAddAnim = 0 ; // return value from AddAnimNodesToAnimSet
		if( AnimList ){
			//AnimList => ( (anim ... ) (anim ... ) )
			AnimList = AnimList->GetElement(1) ;

 			for( int i = 0 ; i < (int)AnimList->GetNumElements() ; i++ )
			{
				bAddAnim = 1;
				AddAnimNodesToAnimSet( AnimList->GetElement(i), pAnimSet, metaModel, status );
			}
		}

		//if ( status != status.OK )
		//{
			//status.OnError("comming back from AddAnimNodeToAnimSet");
			//	return ;//
		//}

		if( bAddAnim ) 
			metaModel.AppendAnimSet( pAnimSet );
		else delete pAnimSet ;
	
}
// ------------------------------------------------------------------------
// ProcessAnimSetFromPN( meta-model, parse-tree, status )
// find all the anim sets from the parse tree, process their contents
// (animset name
//		(keyframe (keyframe ( ... ) ))
//		(anims ( ... ) ))
// ------------------------------------------------------------------------

void LTA::ProcessAnimSetFromLTA( MetaModel & metaModel,
									CLTANode *pnroot,
								CLTATranslStatus & status )
{
	vector< CLTANode *> animsets ;
	CLTAUtil::FindAll( animsets, pnroot, "animset" );

	for( int i = 0 ; i < (int)animsets.size() ; i++ )
	{
		CAnimSet *pAnimSet = new CAnimSet ;
		CLTANode *KFNode, *AnimList ;
		if( pAnimSet == NULL )
		{
			status = status.OUT_OF_MEM ;
			status.m_msg = " OUT OF MEMORY!!! ALLOC CAnimSet ";
			return ;
		}
		// get the anim set name 
		// if no name then anim + i 
		string name ;
		if( animsets[i]->GetElement(1)->IsAtom() )
		{
			name = animsets[i]->GetElement(1)->GetValue() ;
			
		}else 
		{
			char buf[34];
			itoa( i , buf, 10 );
			name = "base" ;
			name += buf ;
		}

		pAnimSet->SetName( name );

		KFNode = CLTAUtil::ShallowFindList( animsets[i], "keyframe" );

		if( KFNode != NULL )
		{
			CKeyFrame KeyFrame ;
			CLTATranslStatus kfstatus ;
			// KFNode = (keyframe (keyframe ... )  we want elem 2
			KFNode = KFNode->GetElement(1) ;
			if( KFNode )
				KeyFrameFromPN( KFNode, KeyFrame , kfstatus );
			if( kfstatus != kfstatus.OK ){
				//MessageBox(NULL, kfstatus.m_msg.c_str(), "AnimSetFromPN", MB_OK);
				kfstatus.OnError("AnimSetFromPN");
				status = kfstatus ;
				return ;
			}
			pAnimSet->SetKeyFrame( KeyFrame );
		}

		// AnimList => (anims ( ... ) )
		AnimList = CLTAUtil::ShallowFindList( animsets[i], "anims" );
		int bAddAnim = 0 ; // return value from AddAnimNodesToAnimSet
		if( AnimList ){
			//AnimList => ( (anim ... ) (anim ... ) )
			AnimList = AnimList->GetElement(1) ;

 			for( int i = 0 ; i < (int)AnimList->GetNumElements() ; i++ )
			{
				bAddAnim = 1;
				AddAnimNodesToAnimSet( AnimList->GetElement(i), pAnimSet, metaModel, status );
			}
		}

		//if ( status != status.OK )
		//{
			//status.OnError("comming back from AddAnimNodeToAnimSet");
			//	return ;//
		//}

		if( bAddAnim ) 
			metaModel.AppendAnimSet( pAnimSet );
		else delete pAnimSet ;
	}
}

// ------------------------------------------------------------------------
// HierarchyFromPN( 
// get the hierarchy from the pn.
// one caveat :
// if the hierarchy has more than one child, we create a child an call it
// the root node. If the hier has more than one child, that means that 
// the modeler has not collected all the nodes under one root. This should
// be ok.
/*
1. (h name (childs (  (a) (b) ))) 
2. (h name (childs ( (a (childs ( (c) (d) ))))))
*/
// ------------------------------------------------------------------------
 
void LTA::HierarchyFromPN(	MetaModel & metaModel,
							CLTANode *pnroot,
							CLTATranslStatus & status )

{
	// build hier
	// get heir.
	// get children list from there for every child call GetTransformFromPN
	CLTANode *lst ;
	lst = CLTAUtil::FindList( pnroot, "hierarchy");
	if( lst == NULL )
	{
		status.m_status = status.FAIL ; 
		status.m_msg = "this model has no hierarchy!! " ;
		status.OnError(" processing Hierarchy ", LT_FALSE );
		return ;
	}

	string hierName ;
	// lst = (hier name (children (...)))
	if( lst->GetElement(1)->IsAtom() )
	{
		hierName = lst->GetElement(1)->GetValue();
	}else {
		hierName = "node_root";
	}
	
	CLTATranslStatus::AddToLog( " - " + hierName + "\r\n");

	lst = CLTAUtil::FindList( lst, "children");
	if( lst == NULL )
	{
		status.m_status = status.FAIL ;
		status.m_msg    = "Hier has no children " ;
		status.OnError(" processing Hierarchy ", LT_FALSE );
		// error no children
		return ;
	}
	
	lst = lst->GetElement(1) ; // ( children ( ... ) )

	// The hierarchy node is not a node in itself, its the 
	// model.
	// if there is more than one child, make a rootnode put children as node's childs
	// else if one child then one child is root
	if( lst->GetNumElements() > 0 )
	{
		int index =0;
		Node * rootN = new Node() ;
		rootN->index = index;
		rootN->SetName("root");
		metaModel.rootNode = rootN ;
		//status.errLog() << " root has : " << lst->GetNumElements() << " num children " << endl;
		if( lst->GetNumElements() == 1 ){
			GetTransformFromPN( rootN, lst->GetElement(0) );
			
		}else
		{
			for( int i = 0 ;i < (int)lst->GetNumElements() ; i++ )
			{
				Node *newNode = new Node ;
				newNode->index = index ;
				rootN->children.push_back(newNode);
				GetTransformFromPN( newNode ,  lst->GetElement(i) );
				index++;
			}
		}
	}
}

// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// Arbitrary value to indicate in a pair that animation should NOT be merged.
// ------------------------------------------------------------------------
const int32 kShouldNotBeMerged = -1;

int LTA::ShouldMerge( CClassifiedAnimLst & CurLst, int index )
{
	return CurLst[index].second != kShouldNotBeMerged ;
}


// ------------------------------------------------------------------------
// CollectAndClassifyAnims( metaModel, result-list )
// check the animation sets in the MetaModel for name matches. 
// a name match is an indication that a vertex animset and a pos/quat animset
// should be merged. The criteria is an animation named nameVA and name both have
// the same root name. The VA postfix indicates that it is an vertex animation.
// ------------------------------------------------------------------------
int LTA::CollectAndClassifyAnims( MetaModel & metaModel, CClassifiedAnimLst & ClassifiedAnimLst )
{
	CAnimSet *cur_anim, *next_anim ;
	string search_name ;
	int32 i ;

	if( metaModel.NumAnimSets() >= 2 )
	{
		for( i = 0 ;i < metaModel.NumAnimSets() ; i++ )
		{
			cur_anim = metaModel.GetAnimSet(i);
			int type = cur_anim->GetAnimType();
			
			if( type == CAnim::VERTEX )
			{
				string name = cur_anim->GetName();
				search_name = name.substr(0,name.size()-2);
				next_anim = metaModel.GetAnimSet(i+1);
				if( next_anim != NULL )
				if( search_name == next_anim->GetName() ) 
				{
						ClassifiedAnimLst.push_back( pair<int,int>( i , i+1 )  );
						i++; // skip the next_anim next time around the loop
				}
			}else
			{
				ClassifiedAnimLst.push_back( pair<int,int>( i, kShouldNotBeMerged ) );
			}
		}
	}
	else // we have only one or no animations, if 1 classify single anim as a non merge.
	{
		if( metaModel.NumAnimSets() == 1 )
		{
			ClassifiedAnimLst.push_back( pair< int, int > ( 0, kShouldNotBeMerged ) ) ;
		}
	}

	return ClassifiedAnimLst.size();
}

// ------------------------------------------------------------------------
// read in the tools-info node.
// ------------------------------------------------------------------------
void LTA::ToolsInfoNodeFromPN( MetaModel & metaModel,
								CLTANode *pnroot)
{
	
	CLTANode *lst ,*tex_bindlst;
	lst = CLTAUtil::FindList( pnroot, "tools-info");
	if( lst == NULL )
	{
		// ok no tools-info node. 
		return ;
	}

	// get the list/arg to tools-info node.
	lst = lst->GetElement(1) ;
	
	tex_bindlst = CLTAUtil::ShallowFindList( lst, "texture-bindings");
	// ok, we got a texture-binding node
	if( tex_bindlst != NULL )
	{ 
		// get the arg to the node.
		tex_bindlst = tex_bindlst->GetElement(1);
		if(tex_bindlst != NULL )
		{
			for( uint32 i = 0 ; i < tex_bindlst->GetNumElements() ;i++ )
			{
				CLTANode *pair = tex_bindlst->GetElement(i) ;
				uint32 index = atoi(pair->GetElement(0)->GetValue());
				string path  = pair->GetElement(1)->GetValue() ;
				// 
				metaModel.m_ToolsInfo.m_TextureBindingMap[index] = path ;
			}
		}
	}

	CLTANode *compile_opts = CLTAUtil::ShallowFindList( lst, "compile-options");

	// defaults.
	metaModel.m_ToolsInfo.m_CompressionType = 2;
	metaModel.m_ToolsInfo.m_bExcludeGeom = false ;
	
	// get compile options.
	if( compile_opts )
	{
		CLTANode *tmp ;
		compile_opts = compile_opts->GetElement(1);

		if( (tmp = CLTAUtil::ShallowFindList( compile_opts, "compression-type")) != NULL )
		{
			CLTANode *val = tmp->GetElement(1);
			if( val )
			{
				metaModel.m_ToolsInfo.m_CompressionType = atoi(val->GetValue());
			}
		}
	

		if( (tmp = CLTAUtil::ShallowFindList( compile_opts, "exclude-geom")) != NULL )
		{
			CLTANode *val = tmp->GetElement(1);
			if( val )
			{
				metaModel.m_ToolsInfo.m_bExcludeGeom = (bool)(atoi(val->GetValue())== 1);
			}
		}
	}
}


// ------------------------------------------------------------------------
// Starting point for creating an in-memory representation of a model 
// lta file.
// ------------------------------------------------------------------------
	
void LTA::InitMetaModelFromPN( MetaModel & metaModel, 
								CLTANode *pnroot,
								CLTATranslStatus &status 
								)
{

	status.AddToLog("hierarchy\r\n ");
		// build meta model from lta parse nodes.
	LTA::HierarchyFromPN( metaModel, pnroot, status );
	

	// *** check status 
	status.AddToLog("shapes\r\n");
	LTA::GetShapesFromPN( metaModel, pnroot );
	LTA::GetLODGroupsFromPN( metaModel, pnroot, status );
	LTA::GetOBBFromOnLoadCmdsPN( metaModel, pnroot, status );

	// *** check status
	status.AddToLog( "animations\r\n " );

	// get animation data from the parse tree
	LTA::ProcessAnimSetFromLTA( metaModel, pnroot, status );

	status.AddToLog("tools-info\r\n");
	LTA::ToolsInfoNodeFromPN( metaModel, pnroot );

	// collect vertex animated orphans, create a node for them 
	// if a va shape does not have parent then we need to create one for them
	// b/c of the current setup in lt. this should not be inherent in lta. 
	// lta should just say this animation modifies this thing without reference
	// to how it is structured.
	FixVertexAnimatedOrphans( metaModel );

}
