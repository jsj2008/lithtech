// ----------------------------------------------------------------------- //
//
// MODULE  : BaseCharacter.h
//
// PURPOSE : Base class for player and AI
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#ifndef __BASE_CHARACTER_H__
#define __BASE_CHARACTER_H__

#include "cpp_engineobjects_de.h"
#include "Destructable.h"
#include "InventoryMgr.h"
#include "Anim_Sound.h"
#include "ContainerCodes.h"
#include "ObjectUtilities.h"
#include "B2BaseClass.h"
#include "movement.h"

class VolumeBrush;

#define DEFAULT_LADDER_VEL			400.0f
#define DEFAULT_SWIM_VEL			175.0f

#define MAX_NUM_WEAPONS	5

class CBaseCharacter : public B2BaseClass
{
	public :

		CBaseCharacter(DBYTE nType=OT_MODEL); // : BaseClass(nType) {}
		virtual ~CBaseCharacter() {};

        DBOOL       IsDead() const { return m_damage.IsDead(); }
		CDestructable *GetDestructable() { return &m_damage; }
		DDWORD		GetSurfaceType() { return m_dwSurfType; }
        
		DBOOL		Trap(char nTrap, DBOOL bGravity);
		DBOOL		IsTrapped() { return m_nTrapped; }

        // For Treat assessments
        CWeapon*	CurrentWeapon()       { return m_InventoryMgr.GetCurrentWeapon(); }
		DFLOAT      HitPoints()		const { return (DFLOAT)m_damage.GetHitPoints(); }
		DFLOAT      ArmorPoints()   const { return (DFLOAT)m_damage.GetArmorPoints(); }
		CInventoryMgr*  GetInventoryMgr() { return &m_InventoryMgr; }
		DBOOL		IsItemActive(int nItem) { return m_InventoryMgr.IsItemActive(nItem); }

		void UpdateInLiquid(VolumeBrush* pBrush, ContainerPhysics* pCPStruct);
		void UpdateOnLadder(VolumeBrush* pBrush, ContainerPhysics* pCPStruct);

		CInventoryMgr   m_InventoryMgr;

	private:

		void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void		Load(HMESSAGEREAD hWrite, DDWORD dwSaveFlags);

	protected :

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		//for model key string parsing
		virtual void	OnStringKey(ArgList* pArgList);

		CDestructable	m_damage;
		CMovement		m_MoveObj;
        CAnim_Sound     m_Anim_Sound;

		ContainerCode	m_eContainerCode;
		ContainerCode	m_eLastContainerCode;
		DBOOL			m_bBodyInLiquid;
		DBOOL			m_bBodyOnLadder;
		DBOOL			m_bSwimmingOnSurface;
		DDWORD			m_dwSurfType;

		DFLOAT			m_fSwimVel;
		DFLOAT			m_fLadderVel;

		// Used to stop any movement from the character (flayer)
		int				m_nTrapped;
		DBOOL			m_bTrappedGrav;
};


#endif // __BASE_CHARACTER_H__
