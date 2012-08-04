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
#include "DamageTypes.h"

class VolumeBrush : public BaseClass
{
	public :

		VolumeBrush();
		~VolumeBrush();

		DFLOAT	GetGravity()	const { return m_fGravity; }
		DFLOAT	GetViscosity()	const { return m_fViscosity; }
		ContainerCode GetCode()	const { return m_eContainerCode; }
		DVector	GetCurrent() const {return m_vCurrent;}
		DBOOL	GetHidden() const	{return m_bHidden;}

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		virtual void UpdatePhysics(ContainerPhysics* pCPStruct);
		virtual void UpdateLiquidPhysics(ContainerPhysics* pCPStruct);
		virtual void UpdateZeroGravity(ContainerPhysics* pCPStruct);

		HCLASS			m_hPlayerClass;
		HSTRING			m_hstrSurfaceSprite;
		HOBJECT			m_hSurfaceObj;

		DVector			m_vCurrent;
		DVector			m_vSurfaceColor1;
		DVector			m_vSurfaceColor2;
		DVector			m_vLastPos;
		DVector			m_vFogColor;
		DFLOAT			m_fViscosity;
		DFLOAT			m_fDamage;
		DFLOAT			m_fSurfaceHeight;
		DFLOAT			m_fGravity;
		DFLOAT			m_fXScaleMin;
		DFLOAT			m_fXScaleMax;
		DFLOAT			m_fYScaleMin;
		DFLOAT			m_fYScaleMax;
		DFLOAT			m_fXScaleDuration;
		DFLOAT			m_fYScaleDuration;
		DFLOAT			m_fFogFarZ;
		DFLOAT			m_fFogNearZ;
		DFLOAT			m_fSurfAlpha;
		DDWORD			m_dwNumSurfPolies;
		DDWORD			m_dwFlags;
		DDWORD			m_dwSaveFlags;

		DamageType		m_eDamageType;
		ContainerCode	m_eContainerCode;
		DBOOL			m_bShowSurface;
		DBOOL			m_bFogEnable;
		DBOOL			m_bHidden;

	private :

		void CreateSurface();
		void HandleTrigger(HOBJECT hSender, HSTRING hMsg);

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();

		void ReadProp( ObjectCreateStruct *pStruct );
		void PostPropRead( ObjectCreateStruct *pStruct );
		void InitialUpdate();
		void Update();
};

#endif // __VOLUME_BRUSH_H__
