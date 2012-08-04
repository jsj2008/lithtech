// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterHitBox.h
//
// PURPOSE : Character hit box object class definition
//
// CREATED : 01/05/00
//
// (c) 2000-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_HIT_BOX_H__
#define __CHARACTER_HIT_BOX_H__

#include "GameBase.h"
#include "TemplateList.h"
#include "IHitBoxUser.h"

LINKTO_MODULE( CharacterHitBox );

struct NodeRadiusStruct
{
	NodeRadiusStruct()
	{
        hModel = NULL;
		hNode = NULL;
	}

	~NodeRadiusStruct()
	{
		if (hModel)
		{
            g_pLTServer->RemoveObject(hModel);
		}
	}

	LTObjRef hModel;
	ModelsDB::HNODE hNode;
};

class CCharacterHitBox : public GameBase
{
	public :
		DEFINE_CAST( CCharacterHitBox );

		CCharacterHitBox();
		virtual ~CCharacterHitBox();

		bool	Init(HOBJECT hModel, IHitBoxUser* pUser);
		void	SetOffset(LTVector vOffset) { m_vOffset = vOffset; }
		const	LTVector& GetOffset() const { return m_vOffset; }

		void	Update();

		HOBJECT			GetModelObject() const { return m_hModel; }
		float			GetDamageModifier(ModelsDB::HNODE hModelNode) const { return m_pHitBoxUser->ComputeDamageModifier(hModelNode); }
		ModelsDB::HSKELETON	GetSkeleton() const { return m_pHitBoxUser->GetModelSkeleton(); }
		CAttachments*	GetAttachments() { return m_pHitBoxUser->GetAttachments(); }

		bool			CanActivate() const { return m_bCanActivate; }
		void			SetCanActivate(bool b) { m_bCanActivate = b; }
		void			SetModelNodeLastHit( ModelsDB::HNODE hModelNode ) { m_pHitBoxUser->SetModelNodeLastHit(hModelNode); }

		virtual uint32 EngineMessageFn(uint32 messageID, void *pData, float lData);
		virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		void	SetAnimControllingDims( bool bControls, HMODELANIM hAni ) { m_bAnimControlsDims = bControls; m_hControllingAnim = hAni; }
		bool	IsAnimControllingDims() { return m_bAnimControlsDims; }

		void	SetAnimControllingOffset( bool bControls, HMODELANIM hAni ) { m_bAnimControlsOffset = bControls; m_hControllingAnim = hAni; }	
		bool	IsAnimControllingOffset() { return m_bAnimControlsOffset; }

		void	SetDimsToModel( );
		void	GetDefaultModelDims( LTVector &vDims );
		void	EnlargeDims( LTVector &vDims );
		void	SetDims( LTVector const& vDims );

		void	FollowVisNode(bool bFollow=true) { m_bFollowVisNode = bFollow; }
		bool	IsFollowingVisNode() const { return m_bFollowVisNode; }
		
	protected :

		bool			GetNodeTransform(const char* const pszNodeName,LTTransform &NodeTransform) const ;
		float			GetNodeRadius( ModelsDB::HNODE hModelNode) const;

		IHitBoxUser*	m_pHitBoxUser;
		LTObjRef		m_hModel;
		LTVector		m_vOffset;
		
		bool			m_bAnimControlsDims;
		bool			m_bAnimControlsOffset;
		bool			m_bFollowVisNode;		// Follow our model's vis node?

		HMODELANIM		m_hControllingAnim;

	private :

		// Debugging helper...
		CTList<NodeRadiusStruct*>	m_NodeRadiusList;

		bool	m_bCanActivate;
		
		void CreateNodeRadiusModels();
		void RemoveNodeRadiusModels();
		void UpdateNodeRadiusModels();

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

	private:

		PREVENT_OBJECT_COPYING( CCharacterHitBox );
};


#endif  // __CHARACTER_HIT_BOX_H__
