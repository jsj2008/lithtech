// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrush.h
//
// PURPOSE : VolumeBrush definition
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __VOLUME_BRUSH_H__
#define __VOLUME_BRUSH_H__

#include "cpp_engineobjects_de.h"
#include "ContainerCodes.h"
//#include "DamageTypes.h"
#include "B2BaseClass.h"

class VolumeBrush : public B2BaseClass
{
	public :

		VolumeBrush();
		~VolumeBrush();

		DFLOAT	GetGravity()	const { return m_fGravity; }
		DFLOAT	GetViscosity()	const { return m_fViscosity; }
		ContainerCode GetCode()	const { return m_eContainerCode; }
		void GetSurfaceColor(DVector *color1, DVector *color2)
			{ VEC_COPY(*color1, m_vSurfaceColor1); VEC_COPY(*color2, m_vSurfaceColor2); }

		void UndoViscosityCalculation(ContainerPhysics* pCPStruct);
		DVector&	GetCurrent() { return m_vCurrent; }
		DBOOL		GetHidden() { return m_bHidden; }

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		virtual void UpdatePhysics(ContainerPhysics* pCPStruct);
		virtual void UpdateLiquidPhysics(ContainerPhysics* pCPStruct, DBOOL bCharacter=DFALSE);
		virtual void UpdateZeroGravity(ContainerPhysics* pCPStruct);

	private:

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void	CacheFiles();

	protected:

		DFLOAT			m_fViscosity;
		ContainerCode	m_eContainerCode;
		DVector			m_vCurrent;
		DFLOAT			m_fDamage;
		DBYTE			m_nDamageType;
		DFLOAT			m_fSurfaceHeight;
		DBOOL			m_bShowSurface;
		DVector			m_vSurfaceColor1;
		DVector			m_vSurfaceColor2;
		DFLOAT			m_fSurfaceAlpha;
		DVector			m_vLastPos;
		DBOOL			m_bHidden;
		DFLOAT			m_fGravity;
		DFLOAT			m_fLastDamageTime;

		DFLOAT			m_fXScaleMin;
		DFLOAT			m_fXScaleMax;
		DFLOAT			m_fYScaleMin;
		DFLOAT			m_fYScaleMax;
		DFLOAT			m_fXScaleDuration;
		DFLOAT			m_fYScaleDuration;
		DDWORD			m_dwNumSurfacePolies;
		HSTRING			m_hstrSurfaceSprite;

		DDWORD			m_dwFlags;
		DDWORD			m_dwSaveFlags;
		HOBJECT			m_hSurfaceObj;
		DBOOL			m_bLocked;			// Needs a key to activate
		DBOOL			m_bUnlockKeyRemove;	// Key gets removed from inventory when used
		HSTRING			m_hstrKeyName;		// name of key needed for activation

		// Fog stuff
		DBOOL			m_bFogEnable;
		DFLOAT			m_fFogFarZ;
		DFLOAT			m_fFogNearZ;
		DVector			m_vFogColor;
		HCLASS			m_hPlayerClass;

	private :

		void CreateSurface();
		void HandleTrigger(HOBJECT hSender, HSTRING hMsg);

		void ReadProp(ObjectCreateStruct *pStruct);
		void InitialUpdate(int nData);
		void Update();
};

#endif // __VOLUME_BRUSH_H__
