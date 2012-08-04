
#ifndef __NODE_H__
#define __NODE_H__


	#include "preplane.h"
	class CPrePoly;

	// Special node indices.
	#define NODE_IN		((CNode*)-1)
	#define NODE_OUT	((CNode*)-2)

	class CNode : public CGLLNode
	{
		public:

						CNode()
						{
							m_pPlane				= NULL;
							m_pPoly					= NULL;
							m_pParentNode			= NULL;
							m_Sides[0] = m_Sides[1] = NULL;
						}

			const CPrePlane*	GetPlane() const	{ return m_pPlane; }
			const PVector&		Normal() const		{ ASSERT(GetPlane()); return GetPlane()->m_Normal; }
			PReal				Dist() const		{ ASSERT(GetPlane()); return GetPlane()->m_Dist; }

			void			ClearPolyPostRemove();
			void			ClearNodePostRemove();

			// Used while saving.
			uint32			m_Index;

			uint8			m_bPostRemove;

			const CPrePlane	*m_pPlane;
			CPrePoly		*m_pPoly;

			CNode			*m_pParentNode;
			CNode			*m_Sides[2];

	};

	typedef CGLinkedList<CNode*>	CNodeList;
	typedef CNode*					NODEREF;

	inline BOOL IsValidNode(NODEREF node) {return !!node && node!=NODE_IN && node!=NODE_OUT;}


#endif  // __NODE_H__

