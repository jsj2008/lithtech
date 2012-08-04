// ----------------------------------------------------------------------- //
//
// MODULE  : BodyProp.h
//
// PURPOSE : Model BodyProp - Definition
//
// CREATED : 6/2/98
//
// ----------------------------------------------------------------------- //

#ifndef __BODY_PROP_H__
#define __BODY_PROP_H__

#include "Prop.h"
#include "BaseCharacter.h"
#include "ModelFuncs.h"

class BodyProp : public Prop
{
	public :

 		BodyProp();
		
		void Setup(CBaseCharacter* pChar);

		DDWORD		GetModelId()	const { return m_nModelId; }
		ModelSize	GetModelSize()	const { return m_eSize; }

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		void	SubClassSetup();
		void	Damage();

	private :

		void	ReadProp(ObjectCreateStruct *pData);
		void	Update();
		DBOOL	UpdateNormalDeath(DFLOAT fTime);
		DBOOL	UpdateFreezeDeath(DFLOAT fTime);
		DBOOL	UpdateVaporizeDeath(DFLOAT fTime);

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		void	CreateGibs();

		CharacterDeath	m_eDeathType;
		DBYTE			m_nModelId;
		ModelSize		m_eSize;
		CharacterClass	m_cc;
		DBOOL			m_bFirstUpdate;
		DFLOAT			m_fStartTime;
		DFLOAT			m_fAdjustFactor;
		DVector			m_vColor;
		DVector			m_vDeathDir;
		DamageType		m_eDamageType;
};

#endif // __BODY_PROP_H__
