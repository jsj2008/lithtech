//-------------------------------------------------------------------
//
//   MODULE    : CUIREDBLACK.H
//
//   PURPOSE   : defines the CUIRedBlack tree utility class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIREDBLACK_H__
#define __CUIREDBLACK_H__


#ifndef __CUI_H__
#include "cui.h"
#endif


// forward decls.
class CUIRedBlackNode;
class CUIRedBlackTree;


// status messages for tree operations
typedef enum {
    CUI_RB_OK,
    CUI_RB_OUT_OF_MEMORY,
    CUI_RB_KEY_ALREADY_EXISTS,
    CUI_RB_KEY_NOT_FOUND
} STATUSTYPE;

// type of key
typedef int32 KEYTYPE;         

// Red-Black tree node colors
typedef enum { CUI_RB_BLACK, CUI_RB_RED } COLORTYPE;

// this callback type is used so you can call prefix(), infix(),
// and postfix() with your own functions
typedef void (*CUI_RB_CALLBACK) (CUIRedBlackNode* node, int32 depth);

// comparison functions
#define LESSTHAN(a,b) (a < b)
#define EQUAL(a,b) (a == b)

// all leaf nodes are sentinels 
#define SENTINEL &m_sentinel


//	-------------------------------------------------------------------------
class CUIRedBlackNode
{
	public:

		CUIRedBlackNode* m_pLeft;		// left child 
		CUIRedBlackNode* m_pRight;		// right child 
		CUIRedBlackNode* m_pParent;		// parent 
		COLORTYPE		 m_color;		// node color (CUI_RB_BLACK, CUI_RB_RED) 
		KEYTYPE			 m_key;			// key used for searching 
		void*			 m_pData;		// user data 

	private:

};


//	-------------------------------------------------------------------------
class CUIRedBlackTree
{
	public:

		CUIRedBlackTree();

		const char* GetClassName() { return "CUIRedBlackTree"; }

		// insert data into the tree
		STATUSTYPE Insert(KEYTYPE key, void *pData);

		// remove data from the tree
		STATUSTYPE Remove(KEYTYPE key);

		// find data associated with a given key
		STATUSTYPE Find(KEYTYPE key, void **ppData);

		// tree traversal operations
		void Infix(CUI_RB_CALLBACK pFunc, 
				   CUIRedBlackNode* pNode, 
				   int32 depth);

		void Prefix(CUI_RB_CALLBACK pFunc, 
				    CUIRedBlackNode* pNode, 
				    int32 depth);

		void Postfix(CUI_RB_CALLBACK pFunc, 
				     CUIRedBlackNode* pNode, 
				     int32 depth);

	public:

		CUIRedBlackNode* m_pRoot;
		CUIRedBlackNode* m_pLastFound;

		// note: this is a node, not a pointer!
		CUIRedBlackNode  m_sentinel;

	private:

		void rotateLeft  (CUIRedBlackNode *pNode);
		void rotateRight (CUIRedBlackNode *pNode);
		void insertHelper(CUIRedBlackNode *pNode);
		void removeHelper(CUIRedBlackNode *pNode);

};


#endif //__CUIREDBLACK_H__

