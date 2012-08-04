#ifndef __COLLISION_STREAM_H_
#define __COLLISION_STREAM_H_


#include "collision_data.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS


//---------------------------------------------------------------------------//
//Write the collision data buffer to a binary file using the C-standard file
//IO functions.
void WriteCollisionData
(
	const char*				file_name,
	const uint32			size,	//size of buffer, bytes
	const LTCollisionData*	pdata	//collision data buffer
);


//---------------------------------------------------------------------------//
//Build and write the collision data to a binary file using the C-standard
//file IO functions.  Triangles need to be sorted by physics surface type
//(and physics surfaces need to specify their corresponding triangle indices).
void BuildAndWriteCollisionData
(
	const char*			file_name,
	const LTPhysSurf	surf[],	//physics surface array
	const uint16		sc,		//physics surface count
	const LTTriangle	tri[],	//triangle array
	const uint16		tc,		//triangle count
	LTVector3f			V[],	//vertex array (discretized in place)
	const uint16		vc		//vertex count
);


//---------------------------------------------------------------------------//
//Allocate and read a collision data buffer from the given binary file using
//the C-standard file IO functions.
LTCollisionData* ReadCollisionData( const char* file_name );


#endif//doxygen


#endif
//EOF
