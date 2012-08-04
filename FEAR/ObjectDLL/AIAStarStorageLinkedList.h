// ----------------------------------------------------------------------- //
//
// MODULE  : AIAStarStorageLinkedList.h
//
// PURPOSE : AStar Storage class using a linked list.
//
// CREATED : 12/02/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_ASTAR_STORAGE_LINKED_LIST_H_
#define _AI_ASTAR_STORAGE_LINKED_LIST_H_

#include "AIAStarMachine.h"


class CAIAStarStorageLinkedList : public CAIAStarStorageAbstract
{
public:
	 CAIAStarStorageLinkedList();
	~CAIAStarStorageLinkedList();

	// CAIAStarStorageAbstract required functions.

	virtual CAIAStarNodeAbstract*	CreateAStarNode( ENUM_AStarNodeID eAStarNode );
	virtual void					DestroyAStarNode( CAIAStarNodeAbstract* pAStarNode );

	virtual void	ResetAStarStorage();

	virtual void	AddToOpenList( CAIAStarNodeAbstract* pAStarNode, CAIAStarMapAbstract* pAStarMap );
	virtual void	AddToClosedList( CAIAStarNodeAbstract* pAStarNode, CAIAStarMapAbstract* pAStarMap );

	virtual void	RemoveFromOpenList( CAIAStarNodeAbstract* pAStarNode );
	virtual void	RemoveFromClosedList( CAIAStarNodeAbstract* pAStarNode );

	virtual CAIAStarNodeAbstract*	RemoveCheapestOpenNode();

	virtual CAIAStarNodeAbstract*	FindInOpenList( ENUM_AStarNodeID eAStarNode );
	virtual CAIAStarNodeAbstract*	FindInClosedList( ENUM_AStarNodeID eAStarNode );

protected:

	// List management.

	void			ClearList( CAIAStarNodeAbstract*& pListHead );
	void			AddToList( CAIAStarNodeAbstract*& pListHead, CAIAStarNodeAbstract* pAStarNode );
	void			RemoveFromList( CAIAStarNodeAbstract*& pListHead, CAIAStarNodeAbstract* pAStarNode );

	CAIAStarNodeAbstract*	FindInList( CAIAStarNodeAbstract* pListHead, ENUM_AStarNodeID eAStarNode );

protected:

	CAIAStarNodeAbstract*	m_pOpenListHead;
	CAIAStarNodeAbstract*	m_pClosedListHead;
};


#endif // _AI_ASTAR_STORAGE_LINKED_LIST_H_
