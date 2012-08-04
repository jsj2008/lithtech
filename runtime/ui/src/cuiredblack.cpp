//-------------------------------------------------------------------
//
//   MODULE    : CUIREDBLACK.CPP
//
//   PURPOSE   : implements the CUIRedBlack tree utility class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIREDBLACK_H__
#include "cuiredblack.h"
#endif


// --------------------------------------------------------------------------
CUIRedBlackTree::CUIRedBlackTree()
{
	// set up the sentinel node
	m_sentinel.m_pParent = 0;
	m_sentinel.m_key     = 0;
	m_sentinel.m_color   = CUI_RB_BLACK;
	m_sentinel.m_pLeft   = SENTINEL;
	m_sentinel.m_pRight  = SENTINEL;

	// assign the root of the tree
	m_pRoot = SENTINEL;
}


// --------------------------------------------------------------------------

void CUIRedBlackTree::rotateLeft(CUIRedBlackNode *pX) {

    //  rotate incoming node X to left

    CUIRedBlackNode *pY = pX->m_pRight;

    // set x->right
    pX->m_pRight = pY->m_pLeft;
    if (pY->m_pLeft != SENTINEL) pY->m_pLeft->m_pParent = pX;

    // set y->parent
    if (pY != SENTINEL) pY->m_pParent = pX->m_pParent;
    if (pX->m_pParent) {
        if (pX == pX->m_pParent->m_pLeft)
            pX->m_pParent->m_pLeft = pY;
        else
            pX->m_pParent->m_pRight = pY;
    } else {
        m_pRoot = pY;
    }

    // link x and y
    pY->m_pLeft = pX;
    if (pX != SENTINEL) pX->m_pParent = pY;
}


// --------------------------------------------------------------------------
void CUIRedBlackTree::rotateRight(CUIRedBlackNode *pX) {

   
    //  rotate node X to right  
   
    CUIRedBlackNode *pY = pX->m_pLeft;

    // set x->left
    pX->m_pLeft = pY->m_pRight;
    if (pY->m_pRight != SENTINEL) pY->m_pRight->m_pParent = pX;

    // set y->parent 
    if (pY != SENTINEL) pY->m_pParent = pX->m_pParent;
    if (pX->m_pParent) {
        if (pX == pX->m_pParent->m_pRight)
            pX->m_pParent->m_pRight = pY;
        else
            pX->m_pParent->m_pLeft = pY;
    } else {
        m_pRoot = pY;
    }

    // link x and y
    pY->m_pRight = pX;
    if (pX != SENTINEL) pX->m_pParent = pY;
}


// --------------------------------------------------------------------------
void CUIRedBlackTree::insertHelper(CUIRedBlackNode *pX) {

   
    //  make sure the Red-Black tree balances  
    //  after insertion of incoming node X           
   
    // check the Red-Black-ness 
    while (pX != m_pRoot && pX->m_pParent->m_color == CUI_RB_RED) {

        // if not right...
        if (pX->m_pParent == pX->m_pParent->m_pParent->m_pLeft) {
            
			CUIRedBlackNode *pY = pX->m_pParent->m_pParent->m_pRight;
            
			if (pY->m_color == CUI_RB_RED) {
                // uncle is CUI_RB_RED 
                pX->m_pParent->m_color = CUI_RB_BLACK;
                pY->m_color = CUI_RB_BLACK;
                pX->m_pParent->m_pParent->m_color = CUI_RB_RED;
                pX = pX->m_pParent->m_pParent;
            } else {
                // uncle is CUI_RB_BLACK
                if (pX == pX->m_pParent->m_pRight) {
                    // make X a left child
                    pX = pX->m_pParent;
                    rotateLeft(pX);
                }
                // set color and rotate 
                pX->m_pParent->m_color = CUI_RB_BLACK;
                pX->m_pParent->m_pParent->m_color = CUI_RB_RED;
                rotateRight(pX->m_pParent->m_pParent);
            }
        } else {

            // as above, but backwards
            CUIRedBlackNode *pY = pX->m_pParent->m_pParent->m_pLeft;

            if (pY->m_color == CUI_RB_RED) {
                // uncle is CUI_RB_RED
                pX->m_pParent->m_color = CUI_RB_BLACK;
                pY->m_color = CUI_RB_BLACK;
                pX->m_pParent->m_pParent->m_color = CUI_RB_RED;
                pX = pX->m_pParent->m_pParent;

            } else {
                // uncle is CUI_RB_BLACK
                if (pX == pX->m_pParent->m_pLeft) {
                    pX = pX->m_pParent;
                    rotateRight(pX);
                }

                pX->m_pParent->m_color = CUI_RB_BLACK;
                pX->m_pParent->m_pParent->m_color = CUI_RB_RED;
                rotateLeft(pX->m_pParent->m_pParent);

            }
        }
    }

    m_pRoot->m_color = CUI_RB_BLACK;
}


// --------------------------------------------------------------------------
STATUSTYPE CUIRedBlackTree::Insert(KEYTYPE key, void *data) {

    CUIRedBlackNode *pCurrent, *pParent, *pX;
    
    // find future parent
    pCurrent = m_pRoot;
    pParent  = NULL;

    while (pCurrent != SENTINEL) {

        if (EQUAL(key, pCurrent->m_key)) 
            return CUI_RB_KEY_ALREADY_EXISTS;

        pParent = pCurrent;
        pCurrent = LESSTHAN(key, pCurrent->m_key) ?
            pCurrent->m_pLeft : pCurrent->m_pRight;
    }

	//  allocate node for data and insert in tree
	LT_MEM_TRACK_ALLOC(pX = new CUIRedBlackNode(),LT_MEM_TYPE_UI);
	if (!pX) return CUI_RB_OUT_OF_MEMORY;

    // setup the new node
    pX->m_pParent = pParent;
    pX->m_pLeft   = SENTINEL;
    pX->m_pRight  = SENTINEL;
    pX->m_color   = CUI_RB_RED;
    pX->m_key     = key;
    pX->m_pData   = data;

    // insert the new node in the tree
    if(pParent) {
        if(LESSTHAN(key, pParent->m_key))
            pParent->m_pLeft = pX;
        else
            pParent->m_pRight = pX;
    } else {
        m_pRoot = pX;
    }

    insertHelper(pX);

    m_pLastFound = NULL;

    return CUI_RB_OK;
}


// --------------------------------------------------------------------------
void CUIRedBlackTree::removeHelper(CUIRedBlackNode *pX) {

	CUIRedBlackNode *pW;

    //  maintain Red-Black tree balance 
    //  after deleting node X           
   
    while (pX != m_pRoot && pX->m_color == CUI_RB_BLACK) {

        if (pX == pX->m_pParent->m_pLeft) {
            pW = pX->m_pParent->m_pRight;

            if (pW->m_color == CUI_RB_RED) {
                pW->m_color = CUI_RB_BLACK;
                pX->m_pParent->m_color = CUI_RB_RED;
                rotateLeft (pX->m_pParent);
                pW = pX->m_pParent->m_pRight;
            }

            if (pW->m_pLeft->m_color == CUI_RB_BLACK &&
				pW->m_pRight->m_color == CUI_RB_BLACK) 
			{
                pW->m_color = CUI_RB_RED;
                pX = pX->m_pParent;
            } else {
                if (pW->m_pRight->m_color == CUI_RB_BLACK) {
                    pW->m_pLeft->m_color = CUI_RB_BLACK;
                    pW->m_color = CUI_RB_RED;
                    rotateRight (pW);
                    pW = pX->m_pParent->m_pRight;
                }
                pW->m_color = pX->m_pParent->m_color;
                pX->m_pParent->m_color = CUI_RB_BLACK;
                pW->m_pRight->m_color = CUI_RB_BLACK;
                rotateLeft (pX->m_pParent);
                pX = m_pRoot;
            }

        } else {
            pW = pX->m_pParent->m_pLeft;
            if (pW->m_color == CUI_RB_RED) {
                pW->m_color = CUI_RB_BLACK;
                pX->m_pParent->m_color = CUI_RB_RED;
                rotateRight (pX->m_pParent);
                pW = pX->m_pParent->m_pLeft;
            }

            if (pW->m_pRight->m_color == CUI_RB_BLACK && 
				pW->m_pLeft->m_color == CUI_RB_BLACK) 
			{
                pW->m_color = CUI_RB_RED;
                pX = pX->m_pParent;
            } else {
                if (pW->m_pLeft->m_color == CUI_RB_BLACK) {
                    pW->m_pRight->m_color = CUI_RB_BLACK;
                    pW->m_color = CUI_RB_RED;
                    rotateLeft (pW);
                    pW = pX->m_pParent->m_pLeft;
                }

                pW->m_color = pX->m_pParent->m_color;
                pX->m_pParent->m_color = CUI_RB_BLACK;
                pW->m_pLeft->m_color = CUI_RB_BLACK;
                rotateRight (pX->m_pParent);
                pX = m_pRoot;
            }
        }
    }

    pX->m_color = CUI_RB_BLACK;
}


// --------------------------------------------------------------------------
STATUSTYPE CUIRedBlackTree::Remove(KEYTYPE key) {
    CUIRedBlackNode *pX, *pY, *pZ;

    //  remove node Z from tree  
    
    // find node in tree
    if (m_pLastFound && EQUAL(m_pLastFound->m_key, key)) {
        // if we just found node, use pointer
        pZ = m_pLastFound;
	}
    else {
        pZ = m_pRoot;

        while(pZ != SENTINEL) {
            if(EQUAL(key, pZ->m_key)) 
                break;
            else
                pZ = LESSTHAN(key, pZ->m_key) ? pZ->m_pLeft : pZ->m_pRight;
        }

        if (pZ == SENTINEL) return CUI_RB_KEY_NOT_FOUND;
    }

    if (pZ->m_pLeft == SENTINEL || pZ->m_pRight == SENTINEL) {
        // Y has a SENTINEL node as a child 
        pY = pZ;
    } else {
        // find tree successor with a SENTINEL node as a child 
        pY = pZ->m_pRight;
        while (pY->m_pLeft != SENTINEL) pY = pY->m_pLeft;
    }

    // X is Y's only child 
    if (pY->m_pLeft != SENTINEL)
        pX = pY->m_pLeft;
    else
        pX = pY->m_pRight;

    // remove Y from the parent chain 
    pX->m_pParent = pY->m_pParent;
    if (pY->m_pParent)
        if (pY == pY->m_pParent->m_pLeft)
            pY->m_pParent->m_pLeft = pX;
        else
            pY->m_pParent->m_pRight = pX;
    else
        m_pRoot = pX;


    if (pY != pZ) {
        pZ->m_key = pY->m_key;
        pZ->m_pData = pY->m_pData;
    }

    if (pY->m_color == CUI_RB_BLACK)
        removeHelper(pX);

    delete(pY);

    m_pLastFound = NULL; 

    return CUI_RB_OK;
}


// --------------------------------------------------------------------------
STATUSTYPE CUIRedBlackTree::Find(KEYTYPE key, void **data) {

    //  find node containing data
    
    CUIRedBlackNode *current = m_pRoot;

    while(current != SENTINEL) {

        if(EQUAL(key, current->m_key)) {
            *data = current->m_pData;
            m_pLastFound = current;
            return CUI_RB_OK;
        } else {
            current = LESSTHAN(key, current->m_key) ?
                current->m_pLeft : current->m_pRight;
        }
    }

    return CUI_RB_KEY_NOT_FOUND;
}


// --------------------------------------------------------------------------
void CUIRedBlackTree::Infix(CUI_RB_CALLBACK func, 
						    CUIRedBlackNode* pNode, 
							int32 depth)
{
	if (pNode == SENTINEL) return;

	// recursive in-order traversal
	Infix(func, pNode->m_pLeft, depth+1);
	func(pNode, depth);
	Infix(func, pNode->m_pRight, depth+1);	
}


// --------------------------------------------------------------------------
void CUIRedBlackTree::Prefix(CUI_RB_CALLBACK func, 
							 CUIRedBlackNode* pNode,
							 int32 depth)
{
	if (pNode == SENTINEL) return;

	// recursive pre-order traversal
	func(pNode, depth);
	Prefix(func, pNode->m_pLeft, depth+1);
	Prefix(func, pNode->m_pRight, depth+1);	
}


// --------------------------------------------------------------------------
void CUIRedBlackTree::Postfix(CUI_RB_CALLBACK func, 
							  CUIRedBlackNode* pNode,
							  int32 depth)
{
	if (pNode == SENTINEL) return;

	// recursive post-order traversal
	Postfix(func, pNode->m_pLeft, depth+1);
	Postfix(func, pNode->m_pRight, depth+1);
	func(pNode, depth);
}

