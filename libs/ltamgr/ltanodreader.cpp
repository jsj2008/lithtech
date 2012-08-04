#include "ltanodereader.h"
#include "ltanodebuilder.h"
#include "ltareader.h"
#include "ltalimits.h"



//this uses the currently to parse through until it encounters a value with
//a string matching pszStartString. Upon finding it, it will create a root node
//and add add the value to it. It will then parse through the file until the
//list is properly closed. This can be used to pull certain sections of a file
//out. Note that this is sequential, and if you want to back up, the file
//should be closed and reopened
CLTANode* CLTANodeReader::LoadNode(CLTAReader* pReader, const char* pszStartValue, ILTAAllocator* pAllocator)
{
	ASSERT(pAllocator);
	ASSERT(pReader);
	ASSERT(pszStartValue);

	//make sure that the file is valid
	if(pReader->IsValid() == false)
	{
		return NULL;
	}

	//just skip over tokens until we can find a value that matches our
	//start string
	char pszValueBuffer[MAX_VALUE_LENGTH];
	CLTAReader::ETokenType eToken;

	//determine if the matching value we found was a string
	bool bIsString = false;

	//determine if the last token was a push token
	bool bWasPrevPush = false;

	do
	{
		eToken = pReader->NextToken(pszValueBuffer, MAX_VALUE_LENGTH);

		//see if we need hit a push (need to flag it as having the previous
		//node be a push)
		if(eToken == CLTAReader::TK_BEGINNODE)
		{
			bWasPrevPush = true;
			continue;
		}


		//now check the token
		if(bWasPrevPush && (eToken == CLTAReader::TK_VALUE))
		{
			//see if it matches
			if(strcmp(pszValueBuffer, pszStartValue) == 0)
			{
				//we found a hit!
				break;
			}
		}
		else if(bWasPrevPush && (eToken == CLTAReader::TK_STRING))
		{
			//see if it matches
			if(strcmp(pszValueBuffer, pszStartValue) == 0)
			{
				//we found a hit!
				bIsString = true;
				break;
			}
		}
		else if(eToken == CLTAReader::TK_ERROR)
		{
			//we hit the end of the file, but never found our start
			return NULL;
		}

		//the item wasn't a push, so clear the flag
		bWasPrevPush = false;

	}while(1);

	//the builder
	CLTANodeBuilder Builder(pAllocator);

	//set it up
	Builder.Init();

	//we need to make the first root node
	if(Builder.Push() == false)
	{
		Builder.AbortBuild();
		return NULL;
	}

	//now add the starting value
	if(Builder.AddValue(pszStartValue, bIsString) == false)
	{
		Builder.AbortBuild();
		return NULL;
	}

	//tokenize the file now that we know we need to add everything to the list
	do
	{
		eToken = pReader->NextToken(pszValueBuffer, MAX_VALUE_LENGTH);

		switch(eToken)
		{
		case CLTAReader::TK_BEGINNODE:
			if(Builder.Push() == false)
			{
				Builder.AbortBuild();
				return NULL;
			}
			break;
		case CLTAReader::TK_ENDNODE:
			if(Builder.Pop() == false)
			{
				//this isn't really a critical error. And is encountered
				//when the user has placed too many closing parenthesis
				//Builder.AbortBuild();
				//return false;
			}
			break;
		case CLTAReader::TK_VALUE:
			if(Builder.AddValue(pszValueBuffer, false) == false)
			{
				Builder.AbortBuild();
				return NULL;
			}
			break;
		case CLTAReader::TK_STRING:
			if(Builder.AddValue(pszValueBuffer, true) == false)
			{
				Builder.AbortBuild();
				return NULL;
			}
			break;
		default:
			break;
		}

	}while((eToken != CLTAReader::TK_ERROR) && (Builder.GetDepth() > 0));

	return Builder.DetachHead();

}


//this will open up the specified file and load the file entirely into a node
//tree
bool CLTANodeReader::LoadEntireFile(const char* pszFilename, bool bCompressed, CLTANode* pParent, ILTAAllocator* pAllocator)
{
	ASSERT(pAllocator);
	ASSERT(pszFilename);

	//first try and open up the file
	CLTAReader Reader;

	if(Reader.Open(pszFilename, bCompressed) == false)
	{
		//failed to open the file
		return false;
	}

	return LoadEntireFile(Reader, pParent, pAllocator);	
}

//same as above, but you can specify an already opened file
bool CLTANodeReader::LoadEntireFile(CLTAReader& InFile, CLTANode* pParent, ILTAAllocator* pAllocator)
{
	ASSERT(pAllocator);

	//make sure the file is valid
	if(InFile.IsValid() == false)
	{
		return false;
	}


	//the builder
	CLTANodeBuilder Builder(pAllocator);

	//set it up
	Builder.Init();

	//tokenize the file
	char pszValueBuffer[MAX_VALUE_LENGTH];
	CLTAReader::ETokenType eToken;

	do
	{
		eToken = InFile.NextToken(pszValueBuffer, MAX_VALUE_LENGTH);

		switch(eToken)
		{
		case CLTAReader::TK_BEGINNODE:
			if(Builder.Push() == false)
			{
				Builder.AbortBuild();
				return false;
			}
			break;
		case CLTAReader::TK_ENDNODE:
			if(Builder.Pop() == false)
			{
				//this isn't really a critical error. And is encountered
				//when the user has placed too many closing parenthesis
				//Builder.AbortBuild();
				//return false;
			}
			break;
		case CLTAReader::TK_VALUE:
			if(Builder.AddValue(pszValueBuffer, false) == false)
			{
				Builder.AbortBuild();
				return false;
			}
			break;
		case CLTAReader::TK_STRING:
			if(Builder.AddValue(pszValueBuffer, true) == false)
			{
				Builder.AbortBuild();
				return false;
			}
			break;
		default:
			break;
		}

	}while(eToken != CLTAReader::TK_ERROR);


	//close the file
	InFile.Close();

	Builder.DetachHeadsTo(pParent);
	return true;
}

