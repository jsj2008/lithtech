////////////////////////////////////////////////////////////////
//
// ltwintreekey.h
//
// The key class used by the tree manager to uniquely identify
// an item
//
// Author: John O'Rorke
// Created: 7/21/00
// Modification History:
//
////////////////////////////////////////////////////////////////
#ifndef __LTWINTREEKEY_H__
#define __LTWINTREEKEY_H__

class CLTWinTreeItem;

//the keys used to identify unique items
typedef CLTWinTreeItem* CLTWinTreeKey;	

//a null item
#define NULLITEM		((CLTWinTreeKey)0)

#endif
