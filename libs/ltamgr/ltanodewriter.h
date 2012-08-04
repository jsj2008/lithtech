//-------------------------------------------------------------------
// LTANodeWriter.h
//
// Provides the definition for CLTANodeWriter which given a CLTANode
// will write the node out to the specified file
//
// Created: 1/17/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------

#ifndef __LTANODEWRITER_H__
#define __LTANODEWRITER_H__

#ifndef __LTANODE_H__
#	include "ltanode.h"
#endif

#ifndef __LTAWRITER_H__
#	include "ltawriter.h"
#endif

class CLTANodeWriter
{
public:

	//given a node to start at, and a filename, it will create a LTA file from the
	//node
	static bool SaveNode(CLTANode* pNode, const char* pszFilename, bool bCompress, bool bAppend = false);

	//given a node to start at, and an LTA writer object, it will add the tree
	//to the writer but will not close the writer
	static bool SaveNode(CLTANode* pNode, CLTAWriter* pWriter);


private:

	//prevent instantiation
	CLTANodeWriter()		{}

};

#endif
