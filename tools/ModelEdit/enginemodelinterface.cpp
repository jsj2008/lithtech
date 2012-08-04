
#include "dynarray.h"
#include "emodel/model.h"
#include "sysstreamsim.h"


Model *pCurModel = NULL ;


bool LoadEModel( const char *filename )
{
	pCurModel = new Model;
	ModelLoadRequest load_request ;
	
	load_request.m_pFile = streamsim_Open( filename, "rb");
	if( !load_request.m_pFile )
		return false ;

	load_request.m_bLoadChildModels = FALSE;

	if(pCurModel->Load( &load_request, filename ))
		return true;;

	return false ;
}


void RenderCurrentModel()
{
	
}
