#ifndef __RAGDOLLDISTANCECONSTRAINT_H__
#define __RAGDOLLDISTANCECONSTRAINT_H__

class CRagDollNode;

class CRagDollDistanceConstraint
{
public:

	CRagDollDistanceConstraint() :
		m_fDistance(0.0f)
	{
		m_pNode[0] = NULL;
		m_pNode[1] = NULL;
	}
	
	//returns true if the passed in node is involved in this constraint
	bool UsesNode(const CRagDollNode* pNode) const
	{
		return (m_pNode[0] == pNode) || (m_pNode[1] == pNode);
	}

	//returns the opposite node (assumes that the passed in node is used)
	CRagDollNode*	GetOpposite(const CRagDollNode* pNode)
	{
		return (m_pNode[0] == pNode) ? m_pNode[1] : m_pNode[0];
	}

	//the two nodes it links
	CRagDollNode*	m_pNode[2];

	//the distance that should be preserved between them
	float			m_fDistance;

private:
};

#endif
