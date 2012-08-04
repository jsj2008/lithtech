//
// compress.h - compression routines for common types 
//
// Copyright (C) 2001 LithTech All Rights Reserved.
//

// NOTE: this module also used by the Autoview library
//       Please do not start including engine headers willy-nilly

#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#ifndef __ILTCOMMON_H__
#include "iltcommon.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

// The closer to 1 this is, the more strict it is about when it compresses 
// rotations.. closer to 0, it compresses them more often but objects
// jitter around more too.
#define ROTATION_COMPRESS_LIMIT 0.999f

class ICompress : public IBase
{
public:
	interface_version(ICompress, 0);

	virtual void EncodeCompressVector( CompVector *pCVec, const LTVector *pVal ) = 0;
	virtual void DecodeCompressVector( LTVector *pVal, const CompVector *pCVec ) = 0;

	// optional bHiRes flag: if true, use an extra byte in the world position compression
	// but gives it a 256k resolution instead of 65k (lo res).

	virtual void EncodeCompressWorldPosition(CompWorldPos *pPos, const LTVector *pVal,
		const LTVector &world_pos_min, const LTVector &world_pos_inv_diff,
		bool bHiRes = true) = 0;
	virtual void DecodeCompressWorldPosition(LTVector *pVal, const CompWorldPos *pPos,
		const LTVector &world_pos_min, const LTVector &world_pos_inv_diff,
		bool bHiRes = true) = 0;

	virtual void EncodeCompressRotation(const LTRotation *pRot, CompRot *pCompRot) = 0;
	virtual void UncompressRotation(char *bytes, LTRotation *pRot) = 0;
};


#endif  //__COMPRESS_H__

