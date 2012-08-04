
// This module defines the lightmap compression and decompression routines.
// The compression is a simple lossless RLE scheme.

// These routines are used by the preprocessor, dedit, and the engine while
// dealing with lightmap data.

#ifndef __LIGHTMAP_COMPRESS_H__
#define __LIGHTMAP_COMPRESS_H__



	//compresses the 24 bit lightmap data found in pData into the output buffer pOut. 
	//Returns the sucess code
	//
	//the compression algorithm is of the form of TGA files, where a tag byte
	//indicates if it is a run or not with the high bit, and then uses the remaining
	//seven bits to encode the length. If it is not a run, raw data follows for the
	//specified span.
	//
	// pOut MUST be (LIGHTMAP_MAX_DATA_SIZE) in length.
	// outLen is filled in with how much data is actually needed in pOut.
	// outLen is guaranteed to be less than LIGHTMAP_MAX_DATASIZE (and usually less than
	// or equal to width * height * 3).
	LTBOOL CompressLMData(const uint8 *pData, 
		uint32 width, uint32 height, 
		uint8 *pOut, uint32 &outLen);

	// Decompresses the data in pCompressed, storing it in pOut.
	// pOut MUST be (LIGHTMAP_MAX_TOTAL_PIXELS * 3) in length.
	// returns the success code.
	LTBOOL DecompressLMData(const uint8 *pCompressed, uint32 dataLen, uint8 *pOut);

#endif



