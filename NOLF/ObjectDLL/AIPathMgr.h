// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_PATH_MGR_H__
#define __AI_PATH_MGR_H__

#include "AIPath.h"

class CAI;
class CAIPath;

// Externs

extern class CAIPathMgr* g_pAIPathMgr;

// Classes

class CAIPathMgr
{
	public :

		// Ctors/Dtors/etc

		CAIPathMgr();
		~CAIPathMgr();

		void Init();
		void Term();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Path finding methods

        LTBOOL FindPath(CAI* pAI, const LTVector& vPosDest, CAIPath* pPath);
        LTBOOL FindPath(CAI* pAI, CAINode* pNodeDest, CAIPath* pPath);
        LTBOOL FindPath(CAI* pAI, CAIVolume* pVolumeDest, CAIPath* pPath);

		// Simple accessors

        inline LTBOOL IsInitialized() { return m_bInitialized; }

	protected :

        LTBOOL EstimatePath(CAI* pAI, const LTVector& vPosSrc, CAIVolume* pVolumeSrc, const LTVector& vPosDest, CAIVolume* pVolumeDest, LTFLOAT* pfDistanceEstimate);
        LTBOOL FindPath(CAI* pAI, const LTVector& vPosSrc, CAIVolume* pVolumeSrc, const LTVector& vPosDest, CAIVolume* pVolumeDest, CAIPath* pPath);
        void ReversePath(CAIVolume* pVolume, CAIVolume* pVolumeNext = LTNULL);
        void BuildPath(CAI* pAI, CAIPath* pPath, CAIVolume* pVolume, const LTVector& vPosDest);
		void BuildEstimate(CAI* pAI, CAIVolume* pVolume, const LTVector& vPosCurrent, const LTVector& vPosDest, LTFLOAT* pfDistanceEstimate);

	private :

        LTBOOL       m_bInitialized;
};

#endif // __AI_PATH_MGR_H__