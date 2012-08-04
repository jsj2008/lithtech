#include "ltanodewriter.h"
#include "ltawriter.h"

//recursively saves the nodes out to the given writer
static bool SaveNodeRecurse(CLTAWriter& Writer, CLTANode* pNode)
{
	//sanity checks
	ASSERT(pNode);
	ASSERT(pNode->IsList());

	//push this node onto the stack
	if(Writer.BeginNode() == false)
	{
		return false;
	}

	//run through all the children
	for(uint32 nCurrElem = 0; nCurrElem < pNode->GetNumElements(); nCurrElem++)
	{
		//get this element
		CLTANode* pCurrNode = pNode->GetElement(nCurrElem);

		if(pCurrNode->IsAtom())
		{
			//if it is a value, write it out
			if(Writer.Write(pCurrNode->GetValue(), pCurrNode->IsString()) == false)
			{
				return false;
			}
		}
		else
		{
			//this isn't a value, so lets recurse
			if(SaveNodeRecurse(Writer, pCurrNode) == false)
			{
				return false;
			}
		}
		
	}

	//pop this node off
	return Writer.EndNode();
}

//given a node to start at, and a filename, it will create a LTA file from the
//node
bool CLTANodeWriter::SaveNode(CLTANode* pNode, const char* pszFilename, bool bCompress, bool bAppend)
{
	//sanity checks
	ASSERT(pNode);
	ASSERT(pszFilename);

	//attempt to open up the file
	CLTAWriter Writer;

	//attempt to open up the writer
	if(Writer.Open(pszFilename, bCompress, bAppend) == false)
	{
		//failed to open up the file
		return false;
	}

	if(SaveNode(pNode, &Writer) == false)
	{
		//failed to save the file
		return false;
	}

	//close out the file
	return Writer.Close();
}

//given a node to start at, and an LTA writer object, it will add the tree
//to the writer but will not close the writer
bool CLTANodeWriter::SaveNode(CLTANode* pNode, CLTAWriter* pWriter)
{
	ASSERT(pNode);
	ASSERT(pWriter);

	if(SaveNodeRecurse(*pWriter, pNode) == false)
	{
		//failed to save the file
		return false;
	}

	return true;
}