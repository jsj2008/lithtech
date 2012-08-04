#ifndef __LZSSWINDOW_H__
#define __LZSSWINDOW_H__

#ifndef __LZSSLIMITS_H__
#	include "lzsslimits.h"
#endif

#ifndef __LTABITFILE_H__
#	include "ltabitfile.h"
#endif


class CLZSSWindow
{
public:

	CLZSSWindow();
	~CLZSSWindow();

	//initializes the window
	void Init();

	//adds an element into the look ahead list. If the look ahead buffer is
	//full, it will write out a token to make room and add the character in
	bool AddByte(uint8 nVal, CLTABitFile& BitFile);

	//writes out tokens until the entire look ahead buffer has been written.
	//should only be called when closing a file
	bool FlushLookAhead(CLTABitFile& BitFile);


private:

	//determines if the look ahead buffer is full
	inline bool IsLookAheadFull() const;

	//looks through the look ahead buffer, finds the largest match
	//in the window, and returns the offset and writes out the token
	//to the buffer
	bool WriteToken(CLTABitFile& BitFile);

	//--------------------
	//Tree manipulation

	//collapses a node. This takes a node with only one child, and replaces itself
	//in the tree with the children
	inline void CollapseNode(uint32 nNode, uint32 nChild);

	//replaces a node with the specified node
	void ReplaceNode(uint32 nNode, uint32 nReplaceWith);

	//deletes a node from the tree
	void DeleteNode(uint32 nNode);

	//adds a node into the tree, finding the largest match as it adds the node
	void AddNode(uint32 nWindowNode, uint32& nMatchPos, uint32& nMatchLen);

	//writes a span with the specified offset and length out to the given bitfile
	bool WriteSpan(CLTABitFile& BitFile, uint32 nOffset, uint32 nLength);

	
	//the tree element type
	struct CTreeNode
	{
		uint32 m_nParent;
		uint32 m_nSmaller;
		uint32 m_nLarger;
	};

	//the buffer for the window, the extra buffer is to enable a ghost
	//edge so that we can do straight memory comparisons
	uint8		m_Window[WINDOW_SIZE + LOOK_AHEAD_SIZE];

	//the tree for quick window matching
	CTreeNode	m_Tree[WINDOW_SIZE + 1];

	//the length of the look ahead buffer
	uint32		m_nLookAheadLen;

	//the window position for the start of the look ahead buffer
	uint32		m_nLookAheadPos;

	//the match amount of the previous token
	uint32		m_nMatchLen;

	//the position of the previous match
	uint32		m_nMatchPos;

	//a flag indicating whether or not the initial input buffer has been filled
	bool		m_bLookAheadInit;

};

#endif
