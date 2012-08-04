#include "collision_stream.h"
#include <stdio.h>


//---------------------------------------------------------------------------//
void WriteCollisionData
(
	const char*				file_name,
	const uint32			size,	//size of buffer, bytes
	const LTCollisionData*	pdata	//collision data buffer
)
{
	//write to a binary file
	if( FILE* file = fopen( file_name, "wb" ) )
	{
		//number of bytes to read in
		fwrite( &size, sizeof(size), 1, file );
		//buffer
		fwrite( pdata, size, 1, file );

		fclose(file);
	}
}


//---------------------------------------------------------------------------//
void BuildAndWriteCollisionData
(
	const char*			file_name,
	const LTPhysSurf	surf[],	//material array
	const uint16		sc,		//material count
	const LTTriangle	tri[],	//triangle array
	const uint16		tc,		//triangle count
	LTVector3f			V[],	//vertex array (discretized in place)
	const uint16		vc		//vertex count
)
{
	//build and allocate buffer
	uint32 size;
	LTCollisionData* pdata = BuildCollisionData( size, surf, sc, tri, tc, V, vc );

	if( pdata )
	{
		WriteCollisionData( file_name, size, pdata );

		//delete buffer
		delete pdata;
	}
}


//---------------------------------------------------------------------------//
LTCollisionData* ReadCollisionData( const char* file_name )
{
	//open file
	FILE* file = fopen( file_name, "rb" );
	{
		//read the number of bytes
		uint32 byte_count;

		fread( &byte_count, sizeof(byte_count), 1, file );

		//allocate a 4-byte aligned buffer and stream in data
		uint32* buffer;
		LT_MEM_TRACK_ALLOC(buffer = new uint32[byte_count/4 + 1],LT_MEM_TYPE_PHYSICS);

		fread( buffer, byte_count, 1, file );

		//NOTE:  whoever gets this must delete it
		return (LTCollisionData*)buffer;
	}
	fclose( file );
}

//EOF
