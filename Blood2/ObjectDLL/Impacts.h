
#ifndef __IMPACTS_H__
#define __IMPACTS_H__



#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
//#include "InventoryMgr.h"


class CImpact : public BaseClass
{
	public :

		CImpact();
		virtual ~CImpact();

		void Setup(DVector vNormal, DVector vScaleMin, DVector vScaleMax, char* pSound,
					DFLOAT fSoundVol, DFLOAT fDuration, DBOOL bRotateable, DBOOL bFade);

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		void	FirstUpdate();

		virtual void UpdateScale();

	private :

		DBOOL InitialUpdate(DVector *pMovement);
		DBOOL Update(DVector *pMovement);

		DBOOL m_bFirstUpdate;
		DBOOL m_bPlaySound;
		HSTRING	m_hSoundName;
		DFLOAT	m_fSoundVol;

		DBOOL	m_bRotateable;
		DBOOL	m_bFade;

		float	red, green, blue, alpha;

		// @cmember should we update the scaling of the sprite?
		DBOOL m_bUpdateScale;

		// @cmember where scaling starts
		DVector m_vScaleMin;

		// @cmember where scaling stops
		DVector m_vScaleMax;

		// @cmember current scale value
		DVector m_vScale;

		// @cmember Normal of the surface we impacted on
		DVector m_vNormal;

		// @cmember how long should we stay alive?
		DFLOAT m_fDuration;

		// @cmember when were we created?
		DFLOAT m_fStartTime;

		// @cmember what sound should we play on an impact?
		char *m_pSoundFile;

		// @cmember how long do the sparks last
		DFLOAT m_fSparkDuration;
};


typedef struct CImpactRicochet_t
{
	BaseClass m_BaseObject;

	DFLOAT 	m_StartTime;
	DFLOAT	m_duration;
	DVector	m_normal;

	DBOOL	m_bFirstUpdate;
	DBOOL	m_bPlaySound;
	char	m_strSound[MAX_CS_FILENAME_LEN];

} CImpactRicochet;



#endif  // __IMPACTS_H__