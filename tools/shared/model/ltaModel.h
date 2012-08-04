// --------------------------------------------------------------------------
// ltaModel.h
// lithtech.(c) 2000
// t.f 4//00
// 
/*
	lta model reading/writing/creating
	*/
#ifndef LTA_MODEL_DEFINED
#define LTA_MODEL_DEFINED


#include "ltaScene.h"


// ------------------------------------------------------------------------
// ltaModelLoad( c++_in_stream, allocator )
// parses file into model file.
// bChildRefs is set to true, child models data will not be loaded, the model
// data base will have a reference to the child model localtion 
// ------------------------------------------------------------------------
Model*  ltaModelLoad( CLTAReader& InFile , LAllocCount & allocCount, bool bChildRefs = false );
Model*  ltaModelLoad( const char * filename, LAllocCount & allocCount, bool bChildRefs= false );

// ------------------------------------------------------------------------
// ltaLoadChildModel
// same as above except it does not process lods and does not get 
// child models bound to it.
// ------------------------------------------------------------------------
Model*  ltaLoadChildModel( const char * filename, LAlloc * allocCount  );



#endif