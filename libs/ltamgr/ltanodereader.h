//-------------------------------------------------------------------
// LTANodeReader.h
//
// Provides the definition for CLTANodeReader, which allows for 
// parsing of LTA files by either loading them up in their entirety
// with LoadEntireFile, or opening a file, and sequentially pulling
// parts out of the file with continual calls to LoadNode.
//
// Created: 1/17/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------

#ifndef __LTANODEREADER_H__
#define __LTANODEREADER_H__

#ifndef __LTANODE_H__
#	include "ltanode.h"
#endif

#ifndef __LTAREADER_H__
#	include "ltareader.h"
#endif

//forward declaration for the allocator class that is needed to handle LTA memory
//management
class ILTAAllocator;

class CLTANodeReader
{
public:

	//this uses the currently open file to parse through until it encounters a value with
	//a string matching pszStartString. Upon finding it, it will create a root node
	//and add add the value to it. It will then parse through the file until the
	//list is properly closed. This can be used to pull certain sections of a file
	//out. Note that this is sequential, and if you want to back up, the file
	//should be closed and reopened. Another thing to note is that this value
	//must come after a (. so, (a, b) with LoadNode("b") would return NULL, since
	//the b does not come after the push
	static CLTANode* LoadNode(	CLTAReader* pReader, const char* pszStartValue, 
								ILTAAllocator* pAllocator);


	//this will open up the specified file and load the file entirely into a node
	//tree. This tree will be added as children to the specified node
	static bool LoadEntireFile(	const char* pszFilename, bool bCompressed, CLTANode* pParent, 
								ILTAAllocator* pAllocator);

	//same as above, but you can specify an already opened file
	static bool LoadEntireFile(	CLTAReader& InFile, CLTANode* pParent, 
								ILTAAllocator* pAllocator);
	
private:

	//don't allow instantiation
	CLTANodeReader()		{}
};

#endif

