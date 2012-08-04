#ifndef __LTAUTIL_H__
#define __LTAUTIL_H__

#ifndef __LTANODE_H__
#	include "ltanode.h"
#endif

#include <vector>

class CLTAUtil
{
public:

	//given a filename, this determines if the file is compressed or not
	static bool		IsFileCompressed(const char* pszFilename);

	//given a node, it will determine the amount of memory being used
	static uint32	GetMemoryUsage(CLTANode* pNode);

	//searches through a node and looks for a list that has the first
	//element set to the specified value
	static CLTANode* ShallowFindList(CLTANode* pNode, const char* pszValue);

	//searches through a node and looks for all lists that have the first element
	//set to the specified value. It will not search any deeper than the
	//immediate children. It will return the number of nodes added to the list
	static uint32	ShallowFindAll(	std::vector<CLTANode*>& vNodeList,
									CLTANode* pStartNode,
									const char* pszValue);

	//searches through a node and looks for the first list to have an element
	//as the first item with a string that matches the specified value. This
	//will search into the tree
	static CLTANode* FindList(CLTANode* pNode, const char* pszValue);

	//builds up a list of all the lists that have the first element set to
	//the specified value. Returns the number of elements added below the
	//specified start node
	static uint32	FindAll(std::vector<CLTANode*>& vNodeList, 
							CLTANode* pStartNode,
							const char* pszValue);

private:

	//don't allow instantiation
	CLTAUtil()	{}
};

#endif
