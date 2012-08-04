// ----------------------------------------------------------------------- //
//
// MODULE  : TractorBeam.h
//
// PURPOSE : Ordog Grappling Hook (Ordog Special Move)
//
// CREATED : 2/24/98
//
// ----------------------------------------------------------------------- //

#ifndef __TRACTORBEAM_H__
#define __TRACTORBEAM_H__

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"

struct BeamInfo
{
	BeamInfo()	{ VEC_INIT(vFrom); VEC_INIT(vTo); PLANE_SET(plane,0.0f,0.0f,0.0f,0.0f); hObjectSrc = DNULL; hObjectDst = DNULL; nSurfaceFlags = 0; }

	DVector		vFrom;				// originating point
	DVector		vTo;				// termination point
	DPlane		plane;				// plane of termination point
	HOBJECT		hObjectSrc;			// object at originating point
	HOBJECT		hObjectDst;			// object at termination point
	DDWORD		nSurfaceFlags;		// surface flags of object at termination

	void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
	void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
};

inline void BeamInfo::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, hObjectSrc);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, hObjectDst);
	pServerDE->WriteToMessageVector(hWrite, &vFrom);	
	pServerDE->WriteToMessageVector(hWrite, &vTo);	
	pServerDE->WriteToMessageVector(hWrite, &plane.m_Normal);	
	pServerDE->WriteToMessageFloat(hWrite, plane.m_Dist);	
	pServerDE->WriteToMessageDWord(hWrite, nSurfaceFlags);	
}

inline void BeamInfo::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &hObjectSrc);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &hObjectDst);
	pServerDE->ReadFromMessageVector(hRead, &vFrom);	
	pServerDE->ReadFromMessageVector(hRead, &vTo);	
	pServerDE->ReadFromMessageVector(hRead, &plane.m_Normal);	
	plane.m_Dist	= pServerDE->ReadFromMessageFloat(hRead);	
	nSurfaceFlags	= pServerDE->ReadFromMessageDWord(hRead);	
}

class TractorBeam : public BaseClass
{
	public :

		TractorBeam();
		~TractorBeam();

		void			Remove();
		BeamInfo&		Info() {return m_info;}

	protected :

		DDWORD			EngineMessageFn (DDWORD messageID, void *pData, DFLOAT lData);

		DBOOL			InitialUpdate ();
		DBOOL			Update ();
		void			PostPropRead (ObjectCreateStruct* pStruct);
	
	protected:

		void			Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void			Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		BeamInfo		m_info;
		DBOOL			m_bTargetIsPlayer;

};

#endif // __TRACTORBEAM_H__
