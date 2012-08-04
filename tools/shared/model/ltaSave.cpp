// ------------------------------------------------------------------------
// lithtech (c) 2000
// all rights reserved.
//
// lta saver 
// takes current memory image and saves it out as an lta file.
// keep this file as independent as possible from LT apps and tools
// we want to be able to use this file any where in the LT program space.
// ------------------------------------------------------------------------
#pragma warning (disable:4786)

#include "ltaSave.h"
#include "model.h"
#include "ltamgr.h"
#include <fstream>

using namespace std ;


void PrettyPrint( ostream & os,  CLTANode * node, int depth );

// (set-node-flags ( ( "name" flag ) ( "name" flag ) ) )

// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
const int kIsString = 1;



void list_build_add( CLTANodeBuilder & lb, unsigned long val )
{
	static char buf[2048];
	sprintf(buf,"%u", val);
	lb.AddValue(buf);
}

static CLTADefaultAlloc m_DefaultAlloc ;
// ------------------------------------------------------------------------
// CLTA_ModelExport
// 
// ------------------------------------------------------------------------
class CLTA_ModelExport {

    int foo;


	Model *m_pModelToExport ;
	CLTANode root ;


	// build sections
	CLTANodeBuilder m_ModelListBuild ;
	CLTANodeBuilder m_OnLoadCmds ;
	CLTANodeBuilder m_AnimBindings ;
	CLTANodeBuilder m_DeformerSpace ;
	CLTANodeBuilder m_FinalBuilder ;
	

public :
	CLTA_ModelExport()
		:m_ModelListBuild(&m_DefaultAlloc),
		m_OnLoadCmds(&m_DefaultAlloc),
		m_AnimBindings(&m_DefaultAlloc),
		m_DeformerSpace(&m_DefaultAlloc),
		m_FinalBuilder(&m_DefaultAlloc){}

	~CLTA_ModelExport() {
	}

	//CLTA_ModelExport() :
		//m_ModelListBuild(&m_Allocator),
		//m_OnLoadCmds(&m_Allocator),
		//m_AnimBindings(&m_Allocator),
		//m_DeformerSpace(&m_Allocator),
		//m_FinalBuilder(&m_Allocator)
	//{
	//}

	// accessor 
	CLTANodeBuilder &GetBuilder() { return m_FinalBuilder ; }

	// export process calls 


	// begin export process 
	void Begin(Model *pTargetModel)
	{
		m_pModelToExport = pTargetModel ;

		if( m_pModelToExport->NumAnims() )
		{
			m_AnimBindings.Push("anim-bindings");
			m_AnimBindings.Push();
		}
			
		
		// if err == 1 ; we screwed up.
	}


	// end export process
	void End()
	{
		if( m_pModelToExport->NumAnims() )
		{
			m_AnimBindings.Pop();
			m_AnimBindings.Pop();
		}

		CLTANodeBuilder CmdsBuilder(&m_DefaultAlloc);

		CmdsBuilder.Push("on-load-cmds");
		CmdsBuilder.Push();
		CmdsBuilder.MoveElements(m_AnimBindings);
		CmdsBuilder.MoveElements(m_OnLoadCmds);
		CmdsBuilder.MoveElements(m_DeformerSpace);

		m_FinalBuilder.MoveElements(CmdsBuilder);
		m_FinalBuilder.MoveElements(m_ModelListBuild);

	
	}


	void FreeParseNodes()
	{
		m_ModelListBuild.AbortBuild();
	}
	
	// ------------------------------------------------------------------------
	// Top LEVEL 
	// pieces to shapes
	// ------------------------------------------------------------------------
	void Build_LTA_ShapesPN(  )
	{
		//CLTANodeBuilder m_DeformerCmds;
		//m_ModelListBuilds.createListBuilderFromCurrentPN( m_DeformerCmds ) ;

		for( int i = 0; i < (int)m_pModelToExport->m_Pieces.GetSize() ; i++ )
		{
			Piece_To_ShapePN( m_ModelListBuild, m_pModelToExport->m_Pieces[i] );
		}
	}

	// ------------------------------------------------------------------------
	// Top Level 
	// ------------------------------------------------------------------------
	void Build_LTA_HierarchyPN( )
	{
        m_OnLoadCmds.Push("set-node-flags");
        m_OnLoadCmds.Push();
        
		m_ModelListBuild.Push("hierarchy");
			m_ModelListBuild.Push("children");
			m_ModelListBuild.Push();

			ModelNode *RootNode = m_pModelToExport->GetRootNode();
			if( RootNode  )
			{
					ModelNode_To_TransformPN( m_ModelListBuild, RootNode );
			
			}

			m_ModelListBuild.Pop(); // children list
		m_ModelListBuild.Pop(); // children
		m_ModelListBuild.Pop(); // hierarchy

        m_OnLoadCmds.Pop();
        m_OnLoadCmds.Pop();
        
	}

	// make path relative to base.
	const char* make_relative( const string &base, const string &path )
	{
		// find where the intersection of the two end.
		int pos = 0 ;
		for( int i = 0 ; i < (int)base.size() ; i++ )
		{
			pos = i ;
			if( base[i] != path[i] )
				break ;
		}
		

		if( pos < (int)path.size() )
			return path.c_str() + pos  ;
		else 
			return path.c_str() ;
	}

	// make the tool info node.
	void Build_Tool_Info()
	{
		m_ModelListBuild.Push("tools-info");
		m_ModelListBuild.Push();
		
		// export the texture-bindings list
		if( !m_pModelToExport->m_TextureIndexMap.empty() )
		{
			m_ModelListBuild.Push("texture-bindings");
			m_ModelListBuild.Push();

			Model::CTextureIndexMapIter it = m_pModelToExport->m_TextureIndexMap.begin();
			Model::CTextureIndexMapIter end= m_pModelToExport->m_TextureIndexMap.end();
			// export the index to texture name list
			for(  ; it != end ;	it++ )
			{
				m_ModelListBuild.Push();
				long tmp_val = (*it).first;
				m_ModelListBuild.AddValue( tmp_val );
				const char * tmp = make_relative(m_pModelToExport->m_sProjectDir, (*it).second);
				m_ModelListBuild.AddValue(  tmp + 1, kIsString);
			
				m_ModelListBuild.Pop();
			}

			// texture-bindings
			m_ModelListBuild.Pop();
			m_ModelListBuild.Pop();
		}
		
		m_ModelListBuild.Push("compile-options");
		m_ModelListBuild.Push();
		if( m_pModelToExport->m_CompressionType != 0 )
		{
			m_ModelListBuild.Push("compression-type");
			m_ModelListBuild.AddValue( (int32)m_pModelToExport->m_CompressionType );
			m_ModelListBuild.Pop();
		}

		if( m_pModelToExport->m_bExcludeGeom != false )
		{
			m_ModelListBuild.Push("exclude-geom");
			m_ModelListBuild.AddValue( (int32)m_pModelToExport->m_bExcludeGeom );
			m_ModelListBuild.Pop();
		}
		m_ModelListBuild.Pop();
		m_ModelListBuild.Pop();

		// tools-info
		m_ModelListBuild.Pop();
		m_ModelListBuild.Pop();
	}

	// ------------------------------------------------------------------------
	// AnimSets
	// ------------------------------------------------------------------------
	void Build_LTA_AnimSetsPN()
	{
		// for each model anim export it as an anim set
		for(int i = 0 ;i < (int)m_pModelToExport->m_Anims.GetSize() ; i++ )
		{
			AnimInfo_To_AnimSetPN( m_pModelToExport->m_Anims[i] );
		}

	}

	// ------------------------------------------------------------------------
	// AnimSets
	// ------------------------------------------------------------------------
	void DirectDumpAnimSets( CLTAWriter& OutFile )
	{
		// for each model anim export it as an anim set
		for(int i = 0 ;i < (int)m_pModelToExport->m_Anims.GetSize() ; i++ )
		{
			DirectAnimDump( OutFile ,  m_pModelToExport->m_Anims[i] );
		}

	}

	// ------------------------------------------------------------------------
	//  build the on_load_cmd node
	// ------------------------------------------------------------------------
	void Build_Additional_LTA_OnLoadCmds() 
	{
		Add_CmdStrings();
	//	Add_LOD_PN(); // old style that forces loader to generate lods.
		Add_Create_lod_group_PN();

		m_OnLoadCmds.Push( "set-global-radius" );
		m_OnLoadCmds.AddValue( m_pModelToExport->m_GlobalRadius );
		m_OnLoadCmds.Pop();

		Add_OBBList();

		Add_SocketsCmds();

		Add_AddChildCmds();

		Add_WeightSetCmd();
	}

	// -----------------------------------------------------------------------
	// (add-node-obb-list (
	//		(add-node-obb "node-name" 
	//			(position ( x y z )) 
	//			(orientation ( a b c d))
	//			(dimensions ( x y z )))
	//  ))
	// -----------------------------------------------------------------------
	void Add_OBBList()
	{
		m_OnLoadCmds.Push("add-node-obb-list");
		m_OnLoadCmds.Push();
		// for every node in m-db check if the obb is there
		// if so dump it out for the name... 
		for( uint32 iNode = 0 ; iNode < m_pModelToExport->m_FlatNodeList.GetSize() ; iNode++ )
		{
			ModelNode *pNode = m_pModelToExport->m_FlatNodeList[iNode];

			//if(pNode->IsOBBEnabled() == true )
			
			SOBB obb = pNode->GetOBB();
			m_OnLoadCmds.Push("add-node-obb");
			m_OnLoadCmds.AddValue( pNode->GetName(), true);
			m_OnLoadCmds.Push("position");
			m_OnLoadCmds.Push();
			m_OnLoadCmds.AddArray( &obb.m_Pos.x , 3);
			m_OnLoadCmds.Pop();	m_OnLoadCmds.Pop();
			
			m_OnLoadCmds.Push("orientation");
			m_OnLoadCmds.Push();
			m_OnLoadCmds.AddArray( obb.m_Orientation.m_Quat , 4);
			m_OnLoadCmds.Pop();	m_OnLoadCmds.Pop();
			
			m_OnLoadCmds.Push("dimensions");
			m_OnLoadCmds.Push();
			m_OnLoadCmds.AddArray( &obb.m_Size.x , 3);
			m_OnLoadCmds.Pop();	m_OnLoadCmds.Pop();

			if(pNode->IsOBBEnabled() != true )
			{
				m_OnLoadCmds.Push("enabled");
				m_OnLoadCmds.AddValue(pNode->IsOBBEnabled());
				m_OnLoadCmds.Pop();
			}

			m_OnLoadCmds.Pop();

		}

		m_OnLoadCmds.Pop(); // container
		m_OnLoadCmds.Pop(); // command
	}
	

private :

// ------------------------------------------------------------------------
// 	(transform name 
//		(matrix ( () ()()() ) )
//		(children ( ... ) )
//		)
// ------------------------------------------------------------------------

void LTMatrix_To_MatrixPN( CLTANodeBuilder & list_build, LTMatrix & mat )
{
	list_build.Push();
	for( int i = 0 ; i < 4 ; i ++ )
	{
		list_build.Push();
		list_build.AddArray(mat.m[i], 4 );
		list_build.Pop();
	}
	list_build.Pop();
}

// ------------------------------------------------------------------------
// 	(transform name 

//		(matrix ( () ()()() ) )
//		(children ( ... ) )
//		)
// ------------------------------------------------------------------------
void ModelNode_To_TransformPN( CLTANodeBuilder & list_build, ModelNode *curNode )
{
	list_build.Push("transform");
	list_build.AddValue( curNode->GetName(), kIsString  );

	list_build.Push("matrix");
	LTMatrix_To_MatrixPN( list_build, curNode->GetGlobalTransform() );
	list_build.Pop();// matrix ;

    m_OnLoadCmds.Push();
    m_OnLoadCmds.AddValue( curNode->GetName() , kIsString );
    m_OnLoadCmds.AddValue( (int32)curNode->m_Flags );
    m_OnLoadCmds.Pop();
    
	
	if( curNode->m_Children )
	{
		list_build.Push("children");
		list_build.Push();
		for( int i = 0 ;i < (int)curNode->m_Children.GetSize() ; i++ )
			ModelNode_To_TransformPN( list_build, curNode->m_Children[i] );
		list_build.Pop();// list
		list_build.Pop();//children
	}

	list_build.Pop();
}




// ------------------------------------------------------------------------
// lt-vector -> ( x y z )
// ------------------------------------------------------------------------
inline
void VecList_From_LTVector( CLTANodeBuilder & list_build, LTVector & vec )
{
	list_build.Push();
		list_build.AddArray( &(vec.x) , 3 );
	list_build.Pop();
}

// ------------------------------------------------------------------------
// float_arryay -> ( x y z )
// ------------------------------------------------------------------------
inline
void VecList_From_FloatArr( CLTANodeBuilder & list_build, float *float_array, int size )
{
	list_build.Push();
		list_build.AddArray( float_array, size );
	list_build.Pop();
}



// ------------------------------------------------------------------------
// MeshGeom_From_Piece
// (mesh "name"
		//(vertex  ( ( x y z ) ( x y z )) )
		//(normals ( ( x y z ) ( x y z )) )
		//(uvs     ( ( u v )   ( u v   )) )
		//(tri-fs  ( a b c d e f ))
		//(tex-fs  ( a b c d e f )))
// ------------------------------------------------------------------------
void MeshGeom_From_Piece( CLTANodeBuilder & list_build, 
						  ModelPiece *pPiece,
						  uint32 lod_level,
						  const char *name )
{
	uint32 i;
	char buf[1024];
	PieceLOD *pLOD = pPiece->GetLOD(lod_level);

	list_build.Push("mesh");
		list_build.AddValue( name , kIsString );

		// vertex
		list_build.Push("vertex");
			list_build.Push();
			for( i = 0 ; i < pLOD->m_Verts.GetSize() ; i++ )
			{
				VecList_From_LTVector( list_build, pLOD->m_Verts[i].m_Vec );
			}
			list_build.Pop();

		list_build.Pop();// vertex 

		// Normals
		list_build.Push("normals");
			list_build.Push();
			for( i = 0 ; i < pLOD->m_Verts.GetSize() ; i++ )
			{
				VecList_From_LTVector( list_build, pLOD->m_Verts[i].m_Normal );
			}
			list_build.Pop();
		list_build.Pop();

		// UVs Texture Coords 		
		list_build.Push("uvs");
			list_build.Push();
		
			for( i = 0 ; i < pLOD->m_Tris.GetSize() ; i++ )
			{
				for( int cnt = 0 ; cnt < 3 ; cnt++ )
				{
					list_build.Push();
					sprintf(buf,"%f" , pLOD->m_Tris[i].m_UVs[cnt].tu);
					list_build.AddValue( buf ); 
					sprintf(buf,"%f" , pLOD->m_Tris[i].m_UVs[cnt].tv);
					list_build.AddValue( buf );
					list_build.Pop();
				}
			}
			list_build.Pop();
		list_build.Pop();

		// Texture triangle face set
		list_build.Push("tex-fs");
			list_build.Push();
			int tcnt = 0;
			for( i = 0 ; i < pLOD->m_Tris.GetSize() ; i++ )
			{
				for( int cnt = 0 ; cnt < 3 ; cnt++ )
				{
				//	sprintf(buf,"%d" , pPiece->m_Tris[i].m_Indices[cnt]);
					sprintf(buf,"%d", tcnt);
					list_build.AddValue( buf );
					tcnt++;
				}
			}
			list_build.Pop();
		list_build.Pop();


		// Triangle Face set
		list_build.Push("tri-fs");
			list_build.Push();
			
			for( i = 0 ; i < pLOD->m_Tris.GetSize() ; i++ )
			{
				for( int cnt = 0 ; cnt < 3 ; cnt++ )
				{
					sprintf(buf,"%d" , pLOD->m_Tris[i].m_Indices[cnt]);
					list_build.AddValue( buf );
					
				}
			}
			list_build.Pop();
		list_build.Pop();
	list_build.Pop();// mesh
	
}

//  ----------------------------------------------------------------
//  Simplified version of MeshGeom_From_Piece
// wherein we skip everything except for the vertex node
// since every thing else is gotten from the master lod level.
//  ----------------------------------------------------------------
void LOD_MeshGeom_From_Piece( CLTANodeBuilder & list_build, PieceLOD *pPiece , char *name )
{
	int i;

	list_build.Push("mesh");
//		list_build.AddValue( pPiece->GetName() , kIsString );
    		list_build.AddValue( name  , kIsString );

		// vertex
		list_build.Push("vertex");
			list_build.Push();
			for( i = 0 ; i < (int)pPiece->m_Verts.GetSize() ; i++ )
			{
				VecList_From_LTVector( list_build, pPiece->m_Verts[i].m_Vec );
			}
			list_build.Pop();

		list_build.Pop();// vertex 		
	list_build.Pop();// mesh
	
}


	// returns 1 if the name starts with d_
	int isVertexAnimPiece( char *name  )
	{
		if( name != NULL && strlen( name ) > 2 )
		{
			if( name[0] == 'd' && name[1] == '_' )
				return 1;
			else return 0;
		}
		return 0 ;
	}


	// ------------	------------------------------------------------------------
	// (add-deformer (skel-deformer "name"
	//						(target "shape_name")
	//						(influences ( "node_name" "node_name" ... ))
	//                      (weightsets (  ( i w i w )  ( i w i w) )
	//
    // note this should be fixed a little. influences is the complete set of nodes
    // in the model. It should just be a list of the name of the nodes that 
    // influence the vertex only. This means changing the index of the weights
     // while outputting.
	// ------------	------------------------------------------------------------
	void AddDeformerNode( PieceLOD *pPiece, const char *name, Model *pModel  )
	{
		uint32 i;

		// if this pPiece skip the deformer.
		//if( isVertexAnimPiece( pPiece->GetName()) )
			//return ;

		m_DeformerSpace.Push( "add-deformer" );
			m_DeformerSpace.Push( "skel-deformer");
				m_DeformerSpace.Push( "target");
					m_DeformerSpace.AddValue( name, kIsString ) ;
				m_DeformerSpace.Pop();

			
			m_DeformerSpace.Push("influences" );
				m_DeformerSpace.Push();
				for(  i = 0 ; i < pModel->m_Transforms.GetSize() ; i++ )
				{
					m_DeformerSpace.AddValue( pModel->GetNode(i)->GetName(), kIsString );
				}
				m_DeformerSpace.Pop();
			m_DeformerSpace.Pop();
			

			m_DeformerSpace.Push("weightsets" );
				m_DeformerSpace.Push();
				for(  i = 0 ;i < (int)pPiece->m_Verts.GetSize() ; i++ )
				{
					m_DeformerSpace.Push();
					NewVertexWeight *nvw = pPiece->m_Verts[i].m_Weights;
					for( int j = 0 ; j < pPiece->m_Verts[i].m_nWeights ; j++ )
					{
						m_DeformerSpace.AddValue((int32)nvw[j].m_iNode );
						m_DeformerSpace.AddValue( nvw[j].m_Vec[3] );	
					}
					m_DeformerSpace.Pop();
				
				}
				m_DeformerSpace.Pop();
			m_DeformerSpace.Pop(); // weightset
			

			m_DeformerSpace.Pop(); // deformer
		m_DeformerSpace.Pop(); // add-defomrer

		
	}

	// note this trusts that this is a deforming mesh
	void AddDeformerNode( PieceLOD *pPiece , char *name )
	{
		uint32 i;

		m_DeformerSpace.Push( "add-deformer" );
			m_DeformerSpace.Push( "skel-deformer");
				m_DeformerSpace.Push( "target");
					m_DeformerSpace.AddValue( name , kIsString ) ;
				m_DeformerSpace.Pop();

			
			m_DeformerSpace.Push("influences" );
				m_DeformerSpace.Push();
				Model *pModel = pPiece->m_pModel;
				for(  i = 0 ; i < pModel->m_Transforms.GetSize() ; i++ )
				{
					m_DeformerSpace.AddValue( pModel->GetNode(i)->GetName(), kIsString );
				}
				m_DeformerSpace.Pop();
			m_DeformerSpace.Pop();
			

			m_DeformerSpace.Push("weightsets" );
				m_DeformerSpace.Push();
				for(  i = 0 ;i < pPiece->m_Verts.GetSize() ; i++ )
				{
					m_DeformerSpace.Push();
					NewVertexWeight *nvw = pPiece->m_Verts[i].m_Weights;
					for( int j = 0 ; j < pPiece->m_Verts[i].m_nWeights ; j++ )
					{
						m_DeformerSpace.AddValue( (int32)nvw[j].m_iNode );
						m_DeformerSpace.AddValue( nvw[j].m_Vec[3] );	
					}
					m_DeformerSpace.Pop();
				
				}
				m_DeformerSpace.Pop();
			m_DeformerSpace.Pop(); // weightset
		
			m_DeformerSpace.Pop(); // deformer
		m_DeformerSpace.Pop(); // add-defomrer

		
	}

	// (set-command-string " foo x ; bar 0 ; ... ")
	void Add_CmdStrings()
	{
		char *cmd_string = m_pModelToExport->m_CommandString;

		m_OnLoadCmds.Push( "set-command-string" );
		m_OnLoadCmds.AddValue( cmd_string , kIsString );
		m_OnLoadCmds.Pop();
	}


	// ------------------------------------------------------------------------

	/* Add_Create_lod_group_PN
	;; dump out explicit lod levels for every model piece.
	(create-lod-group "group-name"
		(lod-dists ( 0 100 200 300 ))
		(shapes ("shape1" "shape2" "shape3" )))

	*/
	// ------------------------------------------------------------------------
	void Add_Create_lod_group_PN()
	{

		// go through every piece in the model create a lod group for each
		// indicating the dists at every lod and make up a name for every 
		// shape in the lods group some thing like model_piece_lod#

		m_OnLoadCmds.Push("lod-groups");
		m_OnLoadCmds.Push();
		for( uint32 iPiece = 0 ; iPiece < m_pModelToExport->NumPieces() ; ++iPiece)
		{
			CLTANodeBuilder ShapeNames(&m_DefaultAlloc) ;
			CLTANodeBuilder Distances(&m_DefaultAlloc) ;
			const char *piece_name = m_pModelToExport->GetPiece(iPiece)->GetName();
			char name_buf[256];
			uint32 min, max ;

			m_OnLoadCmds.Push("create-lod-group");

			m_OnLoadCmds.AddValue( piece_name , kIsString);

			// if min max are default don't bother exporting them.
			m_pModelToExport->GetPiece(iPiece)->GetMinMaxLODOffset(min,max);
			
			// if the values for min/max are not default values export them.
			if( min != 0 && max < ( m_pModelToExport->GetPiece(iPiece)->NumLODs()-1))
			{
				m_OnLoadCmds.Push("min-max-offset");
				m_OnLoadCmds.AddValue( (int32)min );
				m_OnLoadCmds.AddValue( (int32)max );
				m_OnLoadCmds.Pop();
			}
			
			// export the shapes and distances
			ShapeNames.Push("shapes");
			ShapeNames.Push();
			Distances.Push("lod-dists");
			Distances.Push();
			
			for(uint32 iLods = 0;
				iLods < m_pModelToExport->GetPiece(iPiece)->NumLODs();
				++iLods)
			{
				if(m_pModelToExport->GetPiece(iPiece)->GetLOD(iLods)->NumVerts() == 0)
				{
					//we have a NULL LOD piece
					ShapeNames.AddValue("NULL", kIsString);
				}
				else
				{
					if(iLods == 0 )
						sprintf(name_buf,"%s",piece_name);
					else 
						sprintf(name_buf,"%s_%d",piece_name,iLods-1);
	
					ShapeNames.AddValue(name_buf, kIsString );
				}

				Distances.AddValue( m_pModelToExport->GetPiece(iPiece)->GetLODDist(iLods));
			}

			Distances.Pop(); Distances.Pop(); 
			ShapeNames.Pop();ShapeNames.Pop();

			m_OnLoadCmds.AddElement( Distances.DetachHead() );
			m_OnLoadCmds.AddElement( ShapeNames.DetachHead() );
			m_OnLoadCmds.Pop();
		}


		m_OnLoadCmds.Pop(); m_OnLoadCmds.Pop(); // lod-groups

	}

	// ------------------------------------------------------------------------
	// add socket commands 
	// (add-socket (socket "sock_name" 
	//    (parent "node-name")
	//       (pos ( x y z )) 
	//       (quat ( a b c d ))))
	//)
	// ------------------------------------------------------------------------
	void Add_SocketsCmds()
	{
		ModelSocket *pSocket ;

		if( m_pModelToExport->NumSockets() == 0 ) 
			return ;


		m_OnLoadCmds.Push("add-sockets");
		m_OnLoadCmds.Push();				

		for( int i=0; i < (int)m_pModelToExport->NumSockets(); i++)
		{
			pSocket = m_pModelToExport->GetSocket(i );
			m_OnLoadCmds.Push("socket");
					m_OnLoadCmds.AddValue( pSocket->m_Name, kIsString );

					m_OnLoadCmds.Push("parent");
						m_OnLoadCmds.AddValue( m_pModelToExport->GetNode( pSocket->m_iNode )->GetName() , kIsString );
					m_OnLoadCmds.Pop();

					m_OnLoadCmds.Push("pos");
						m_OnLoadCmds.Push();
							m_OnLoadCmds.AddArray( &pSocket->m_Pos.x , 3 );
						m_OnLoadCmds.Pop();
					m_OnLoadCmds.Pop();

					m_OnLoadCmds.Push("quat");
						m_OnLoadCmds.Push();
							m_OnLoadCmds.AddArray( pSocket->m_Rot.m_Quat , 4 );
						m_OnLoadCmds.Pop();
					m_OnLoadCmds.Pop();

					m_OnLoadCmds.Push("scale");
						m_OnLoadCmds.Push();
							m_OnLoadCmds.AddArray( &pSocket->m_Scale.x , 3 );
						m_OnLoadCmds.Pop();
					m_OnLoadCmds.Pop();
	
				m_OnLoadCmds.Pop(); // socket 
			
		}
		m_OnLoadCmds.Pop(); // list
		m_OnLoadCmds.Pop(); // add-slciet
	}

	// strips .abc from filename 
	void StripTrailingABC( string & filename )
	{
		if( filename.size() > 4 )
		{
			size_t where = filename.find_last_of(".abc");
			size_t WHERE = filename.find_last_of(".ABC");

			if( WHERE != filename.npos )
				where = WHERE ;

			if( where != filename.npos )
				filename.resize( where );
		}
	}
	

	// ------------------------------------------------------------------------
	// 
	// (add-childmodels (
	//			(child-model (filename "name") (node-relations ( (... )) ))))
	// ------------------------------------------------------------------------
	void Add_AddChildCmds() 
	{
		int i;
		ChildInfo *pChildInfo ;

		if( m_pModelToExport->NumChildModels() > 1 )
		{
			m_OnLoadCmds.Push("add-childmodels");
			m_OnLoadCmds.Push();

			// the first child model is self
			for( uint32 iChildModels = 1 ; iChildModels < m_pModelToExport->NumChildModels() ; iChildModels++ )
			{
					pChildInfo=		m_pModelToExport->GetChildModel(iChildModels);

					// skip if the child info is self
					if( strcmp( pChildInfo->m_pFilename, "SELF" ) == 0 ) 
						continue ;

					m_OnLoadCmds.Push("child-model");
						m_OnLoadCmds.Push("filename" );
						string filename = pChildInfo->m_pFilename ;
						StripTrailingABC( filename );

						m_OnLoadCmds.AddValue( filename.c_str() , kIsString );
						m_OnLoadCmds.Pop();

						m_OnLoadCmds.Push("save-index");
						m_OnLoadCmds.AddValue( (int32)pChildInfo->m_SaveIndex );
						m_OnLoadCmds.Pop();
			

	
					// check before outputing the relations if there are any
					// that'll save us trouble later.
					float pos_sum = 0;
					int quat_sum = 0;
					
					for( i = 0 ;i < (int)pChildInfo->m_Relation.GetSize() ; i++ )
					{
						LTVector &vec = pChildInfo->m_Relation[i].m_Pos ;
						LTRotation &rot = pChildInfo->m_Relation[i].m_Rot ;
						pos_sum += ( vec.x + vec.y + vec.z );
						quat_sum += !(rot.IsIdentity());
					}
					// if we have any data to export, lets doit.
					if( pos_sum != 0 || quat_sum != 0 )
					{
						m_OnLoadCmds.Push("node-relations");
						m_OnLoadCmds.Push(); // +list

						for( i = 0 ;i < (int)pChildInfo->m_Relation.GetSize() ; i++ )
						{
							m_OnLoadCmds.Push(); // +list 
							m_OnLoadCmds.Push("pos");
							VecList_From_LTVector( m_OnLoadCmds, pChildInfo->m_Relation[i].m_Pos );
							m_OnLoadCmds.Pop(); // -pos

							m_OnLoadCmds.Push("quat");
							VecList_From_FloatArr( m_OnLoadCmds, pChildInfo->m_Relation[i].m_Rot.m_Quat,4 );
							m_OnLoadCmds.Pop(); // -quat 
							m_OnLoadCmds.Pop(); // -list 
						}
					
						m_OnLoadCmds.Pop(); // node-relations
						m_OnLoadCmds.Pop(); // -list
					}
					
					m_OnLoadCmds.Pop(); // child models
			}

			m_OnLoadCmds.Pop(); // -list 
			m_OnLoadCmds.Pop(); // add childmodels

		}
	}


	// ------------------------------------------------------------------------
	// (add-animweightsets (
	//   (anim-weightset (name "name" ) (weights ( a b c ... ))
	//   .. 
	// ))
	// ------------------------------------------------------------------------
	void Add_WeightSetCmd() 
	{
		if( m_pModelToExport->NumWeightSets() < 1 ) 
			return;


		m_OnLoadCmds.Push("anim-weightsets");
		m_OnLoadCmds.Push();
	
		for( uint32 iWtSet = 0 ; iWtSet < m_pModelToExport->NumWeightSets() ; iWtSet++)
		{
			WeightSet *ws = m_pModelToExport->GetWeightSet( iWtSet );
			//m_OnLoadCmds.Push("weightset");
			m_OnLoadCmds.Push( "anim-weightset");
			m_OnLoadCmds.Push("name" );
			m_OnLoadCmds.AddValue( ws->GetName() , kIsString );
			m_OnLoadCmds.Pop();
			
			m_OnLoadCmds.Push("weights");
			m_OnLoadCmds.Push();
			for( int wtcnt = 0 ; wtcnt < (int)ws->m_Weights.GetSize() ; wtcnt ++ )
			{
				m_OnLoadCmds.AddValue( ws->m_Weights[ wtcnt ] );
			}
			m_OnLoadCmds.Pop();
			m_OnLoadCmds.Pop();

			m_OnLoadCmds.Pop();
		}


		m_OnLoadCmds.Pop();
		m_OnLoadCmds.Pop();



	}


	// ------------------------------------------------------------------------
	// (shape "name"
	//		(geometry ... )
	//	    (appearance ... ))
	// ------------------------------------------------------------------------
	void Piece_To_ShapePN( CLTANodeBuilder & list_build, ModelPiece *pPiece )
	{

		// for each lod level in the piece dump out a shape. 
		// the shape name should be the model piece and _lod number appended
		uint32 iLOD;
		char *piece_name = pPiece->GetName();
		char name_buffer[256];

		for( iLOD = 0 ; iLOD < pPiece->NumLODs() ; ++iLOD )
		{
			PieceLOD *pLOD = pPiece->GetLOD(iLOD);

			list_build.Push( "shape" );

			// make it so that the first shape-name isn't mangled.
			if(iLOD == 0 )
				sprintf(name_buffer,"%s",piece_name);
			else 
				sprintf(name_buffer,"%s_%d",piece_name,iLOD-1);
			
				
			list_build.AddValue( name_buffer, kIsString );


			list_build.Push("geometry");
				MeshGeom_From_Piece( list_build, pPiece, iLOD, name_buffer );
			list_build.Pop();

			list_build.Push( "texture-indices" );
				list_build.Push();
					list_build.AddArray( (int32*)pLOD->m_iTextures, pLOD->m_nNumTextures );
				list_build.Pop();
			list_build.Pop();
			list_build.Push( "renderstyle-index" );
				list_build.AddValue( pLOD->m_iRenderStyle );
			list_build.Pop();
			list_build.Push( "render-priority" );
				list_build.AddValue( (int32)pLOD->m_nRenderPriority );
			list_build.Pop();


			list_build.Pop(); // close shape

			// create deformer node
			AddDeformerNode( pLOD, name_buffer, pPiece->GetModel() );
		}
		
	}



	void KeyFrame_From_ModelAnim( ModelAnim & model_anim )
	{

		//we just want to add these to the default list
		KeyFrame_From_ModelAnim(m_ModelListBuild, model_anim);

	}

	void KeyFrame_From_ModelAnim(  CLTANodeBuilder &list_build, ModelAnim & model_anim )
	{
		CLTANodeBuilder Times(&m_DefaultAlloc),Value(&m_DefaultAlloc);

		Times.Push("times");
		Times.Push();
		Value.Push("values");
		Value.Push();

		for( int i = 0; i < (int)model_anim.m_KeyFrames.GetSize() ; i++ )
		{
			AnimKeyFrame &pKF = model_anim.m_KeyFrames[i];
			//Times.AddValue( pKF.m_Time );
			list_build_add( Times, pKF.m_Time );
			Value.AddValue( pKF.m_pString, kIsString );
			//Value.AddValue( (int)pKF.m_KeyType );
		}
		Value.Pop();
		Value.Pop();

		Times.Pop();
		Times.Pop();

		list_build.Push( "keyframe");
		list_build.AddElement(Times.DetachHead());
		list_build.AddElement(Value.DetachHead());
		list_build.Pop();
	}

	// pos quat anim data
	// ( ( (x y z ) ( a b c d ) ) ( ( X y Z ) ( A b c D ) ) ... )
	void GeneratePosQuatPN( AnimNode *pAnimNode )
	{
		m_ModelListBuild.Push(); // list
	
		for( int i = 0 ;i < (int)pAnimNode->m_KeyFrames.GetSize() ; i++ )
		{
			NodeKeyFrame &pKF = pAnimNode->m_KeyFrames[i] ;
			m_ModelListBuild.Push(); // list
			VecList_From_LTVector( m_ModelListBuild, pKF.m_Translation );
			VecList_From_FloatArr( m_ModelListBuild, pKF.m_Quaternion.m_Quat, 4);
			m_ModelListBuild.Pop(); // list

		}

		m_ModelListBuild.Pop(); // list
	}

		// pos quat anim data
	// ( ( (x y z ) ( a b c d ) ) ( ( X y Z ) ( A b c D ) ) ... )
	void GeneratePosQuatPN( CLTANodeBuilder &list_build, AnimNode *pAnimNode )
	{
		list_build.Push(); // list
	
		for( int i = 0 ;i < (int)pAnimNode->m_KeyFrames.GetSize() ; i++ )
		{
			NodeKeyFrame &pKF = pAnimNode->m_KeyFrames[i] ;
			list_build.Push(); // list
			VecList_From_LTVector( list_build, pKF.m_Translation );
			VecList_From_FloatArr( list_build, pKF.m_Quaternion.m_Quat, 4);
			list_build.Pop(); // list

		}

		list_build.Pop(); // list
	}

	// ------------------------------------------------------------------------
	// create vertex anim data.
	// ------------------------------------------------------------------------
	void GenerateVertexPN( AnimNode *pAnimNode )
	{
		m_ModelListBuild.Push(); // list
	
		for( int i = 0 ;i < (int)pAnimNode->m_KeyFrames.GetSize() ; i++ )
		{
			NodeKeyFrame &pKF = pAnimNode->m_KeyFrames[i] ;
			CDefVertexLst *pDVL = pKF.m_pDefVertexLst;

			m_ModelListBuild.Push(); // list
			
			for( int i = 0 ;i <  pDVL->size() ; i++ )
			{
				float *pos = pDVL->getValue(i);
				VecList_From_FloatArr( m_ModelListBuild, pos, 3 );
			}
//			
			//pKF->m_pDefVertexLst
			m_ModelListBuild.Pop(); // list

		}

		m_ModelListBuild.Pop(); // list
	}

	// ------------------------------------------------------------------------
	// create vertex anim data.
	// ------------------------------------------------------------------------
	void GenerateVertexPN( CLTANodeBuilder & list_build, AnimNode *pAnimNode )
	{
		list_build.Push(); // list
	
		for( int i = 0 ;i < (int)pAnimNode->m_KeyFrames.GetSize() ; i++ )
		{
			NodeKeyFrame &pKF = pAnimNode->m_KeyFrames[i] ;
			CDefVertexLst *pDVL = pKF.m_pDefVertexLst;

			list_build.Push(); // list
			
			for( int i = 0 ;i <  pDVL->size() ; i++ )
			{
				float *pos = pDVL->getValue(i);
				VecList_From_FloatArr( list_build, pos, 3 );
			}
//			
			//pKF->m_pDefVertexLst
			list_build.Pop(); // list

		}

		list_build.Pop(); // list
	}

	// create an anim container from the lt animnode
	void Anim_From_AnimNode( AnimNode *pAnimNode )
	{
		m_ModelListBuild.Push("anim");
		
		m_ModelListBuild.Push("parent");
		m_ModelListBuild.AddValue( pAnimNode->m_pNode->GetName(), kIsString );
		m_ModelListBuild.Pop();

		m_ModelListBuild.Push("frames");

		if( pAnimNode->isVertexAnim() )
		{
			m_ModelListBuild.Push("vertex");
			GenerateVertexPN( pAnimNode );
			m_ModelListBuild.Pop();
		}
		else
		{
			m_ModelListBuild.Push("posquat");
		
			GeneratePosQuatPN( pAnimNode );

			m_ModelListBuild.Pop();//posquat
		}
		
		m_ModelListBuild.Pop();// frames
		m_ModelListBuild.Pop();// anim

		// do this for all the children
		for( int i = 0 ;i < (int)pAnimNode->m_Children.GetSize() ; i++ )
			Anim_From_AnimNode( pAnimNode->m_Children[i] );

	}

		// create an anim container from the lt animnode
	void Anim_From_AnimNode( CLTANodeBuilder &list_build, AnimNode *pAnimNode )
	{
		list_build.Push("anim");
		
		list_build.Push("parent");
		list_build.AddValue( pAnimNode->m_pNode->GetName(), kIsString );
		list_build.Pop();

		list_build.Push("frames");

		if( pAnimNode->isVertexAnim() )
		{
			list_build.Push("vertex");
			GenerateVertexPN(list_build, pAnimNode );
			list_build.Pop();
		}
		else
		{
			list_build.Push("posquat");
		
			GeneratePosQuatPN( list_build, pAnimNode );

			list_build.Pop();//posquat
		}
		
		list_build.Pop();// frames
		list_build.Pop();// anim

		// do this for all the children
		for( int i = 0 ;i < (int)pAnimNode->m_Children.GetSize() ; i++ )
			Anim_From_AnimNode( list_build, pAnimNode->m_Children[i] );

	}

	// create an anim node from a ModelAnim
	void Anims_From_ModelAnim( ModelAnim & model_anim )
	{
		m_ModelListBuild.Push("anims");
		m_ModelListBuild.Push();// list

		Anim_From_AnimNode( model_anim.GetRootNode() );
		

		m_ModelListBuild.Pop();// list
		m_ModelListBuild.Pop();// anims
	}

	// ------------------------------------------------------------------------
	// 
	// ------------------------------------------------------------------------
	void AnimInfo_To_AnimSetPN( AnimInfo & anim_info )
	{
		// if this anim info node is does not belong to model
		// just move on. The model belongs to a child model.
		if(  (!anim_info.IsParentModel()) )
			return ;

		ModelAnim &model_anim = *(anim_info.m_pAnim) ; // gahrg, ref or deref ptr
				
		m_AnimBindings.Push("anim-binding");
	
		m_AnimBindings.Push("name" );
		m_AnimBindings.AddValue(model_anim.GetName(), kIsString );
		m_AnimBindings.Pop();

		m_AnimBindings.Push("dims");
		m_AnimBindings.Push();
		m_AnimBindings.AddArray(  &anim_info.m_vDims.x , 3 );
		m_AnimBindings.Pop();
		m_AnimBindings.Pop();

		m_AnimBindings.Push("translation");
		m_AnimBindings.Push();
		m_AnimBindings.AddArray(  &anim_info.m_vTranslation.x , 3 );
		m_AnimBindings.Pop();
		m_AnimBindings.Pop();

		m_AnimBindings.Push("interp-time");
		m_AnimBindings.AddValue( (int32)model_anim.m_InterpolationMS );
		m_AnimBindings.Pop();

		if( (model_anim.m_ModelEditWeightSet != INVALID_MODEL_WEIGHTSET) &&  
			( model_anim.m_ModelEditWeightSet < m_pModelToExport->NumWeightSets())   )
		{
			m_AnimBindings.Push("weight-set");
			char *name = m_pModelToExport->GetWeightSet( model_anim.m_ModelEditWeightSet )->GetName();
			m_AnimBindings.AddValue( name , kIsString);
			m_AnimBindings.Pop();
		}

		// some error reporting here if the weightset number is 
		if(	(model_anim.m_ModelEditWeightSet != INVALID_MODEL_WEIGHTSET) &&  
			m_pModelToExport->NumWeightSets() <= model_anim.m_ModelEditWeightSet )
		{
			char buf[256];
			sprintf(buf,"ERROR: Weight set index %d for anim ", model_anim.m_ModelEditWeightSet);
			CLTATranslStatus::AddToLog(buf);
			CLTATranslStatus::AddToLog(model_anim.GetName());
			CLTATranslStatus::AddToLog("\n\r");
		}

		m_AnimBindings.Pop();
	}


	// ------------------------------------------------------------------------
	// This method dumps out the animation data to out stream without buffering
	// into memory.
	// ------------------------------------------------------------------------
	void DirectAnimDump( CLTAWriter& OutFile, AnimInfo & anim_info )
	{
		// if this anim info node is does not belong to model
		// just move on. The model belongs to a child model.
		if(  (!anim_info.IsParentModel()) )
			return ;
	
		ModelAnim &model_anim = *(anim_info.m_pAnim) ; // gahrg, ref or deref ptr
		
		CLTANodeBuilder list_build(&m_DefaultAlloc) ;
	
		list_build.Push("animset");
		list_build.AddValue( model_anim.GetName() , kIsString );
		

		list_build.Push("keyframe" );
		KeyFrame_From_ModelAnim( list_build, model_anim );
		list_build.Pop( );
	
		list_build.Push("anims");
		list_build.Push();// list

		Anim_From_AnimNode( list_build, model_anim.GetRootNode() );
		

		list_build.Pop();// list
		list_build.Pop();// anims
		
		list_build.Pop(); // close out animset
		
		CLTANode* pRoot = list_build.DetachHead();

		// dump to output buffer the animation
		CLTANodeWriter::SaveNode( pRoot, &OutFile );

        m_DefaultAlloc.FreeNode(pRoot);
//		list_build.GetAllocator()->FreeNode( pRoot );
	}

}; // class end


// ------------------------------------------------------------------------
// export the animations
// ------------------------------------------------------------------------


// ------------------------------------------------------------------------
// ltaSave( filename , model )
// ------------------------------------------------------------------------
bool ltaSave( const char *filename , Model *pExportModel, CLTATranslStatus& status )
{

	CLTA_ModelExport lta_modelExporter ;

	CLTAWriter OutFile(filename, CLTAUtil::IsFileCompressed(filename));

	if( !OutFile.IsValid() )
	{
		status = status.FAIL ;
		status << "could not open " << filename ;
		
		return false;
	}

	// process the export model
	lta_modelExporter.Begin( pExportModel );
	
	lta_modelExporter.Build_LTA_HierarchyPN();
	lta_modelExporter.Build_LTA_ShapesPN();
	lta_modelExporter.Build_LTA_AnimSetsPN();
	lta_modelExporter.Build_Additional_LTA_OnLoadCmds();
	lta_modelExporter.Build_Tool_Info() ;
	lta_modelExporter.End();

	CLTANodeBuilder&  Builder = lta_modelExporter.GetBuilder();

	//create a node to hold all the nodes in the builder
	CLTANode list;
	Builder.DetachHeadsTo( &list );

	OutFile.BeginNode();

	const char* name="lt-model-0";
	OutFile.Write(name, strlen(name), false);

	//write out all the heads
	for(uint32 nCurrElement = 0; nCurrElement < list.GetNumElements(); nCurrElement++)
	{
		CLTANodeWriter::SaveNode( list.GetElement( nCurrElement ), &OutFile);
	}

 	//list.Free( Builder.GetAllocator() );
	list.Free(&m_DefaultAlloc);
	// export the animations seperately because of the memory required.	
	lta_modelExporter.DirectDumpAnimSets( OutFile );

	OutFile.EndNode();

	return OutFile.Close();

    
}

