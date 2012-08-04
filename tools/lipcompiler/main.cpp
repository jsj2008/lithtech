#include "commandlineparser.h"

#if _MSC_VER >= 1300
#include <iostream>
#include <fstream>
#else
#include <iostream.h>
#include <fstream.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#include "ltvector.h"
#include "ltrotation.h"

#define LIPSYNC_FILE_ID				(('L' << 24) | ('I' << 16) | ('P' << 8) | 'S')
#define LIPSYNC_FILE_VERSION		1

#define DEG2RAD(X)		((X) * 0.0174532925199432957f)

struct SKeyFrame
{
	LTVector	m_vTranslation;
	LTVector	m_vEuler;
};

//Given an input and an output filename, this will compile the file and save it out
//if successful
static bool CompileFile(const char* pszFile, const char* pszOut)
{
	//open up the input file as a text file
#if _MSC_VER >= 1300
	std::ifstream InFile(pszFile);
#else
	ifstream InFile(pszFile);
#endif

	if(!InFile.good())
	{
#if _MSC_VER >= 1300
		std::cout << "Error opening " << pszFile << " for input" << std::endl;
#else
		cout << "Error opening " << pszFile << " for input" << endl;
#endif
		return false;
	}

	//also open up the output for writing so we don't need intermediate storage
#if _MSC_VER >= 1300
	std::ofstream OutFile(pszOut, std::ios::out | std::ios::binary);
#else
	ofstream OutFile(pszOut, ios::out | ios::binary);
#endif

	if(!OutFile.good())
	{
#if _MSC_VER >= 1300
		std::cout << "Error opening " << pszOut << " for output" << std::endl;
#else
		cout << "Error opening " << pszOut << " for output" << endl;
#endif
		return false;
	}

	//alright, both are successful, so now compile

	//write out the header information 
	uint32 nFileCode	= LIPSYNC_FILE_ID;
	uint32 nVersion		= LIPSYNC_FILE_VERSION;

	OutFile.write((const char*)&nFileCode, sizeof(nFileCode));
	OutFile.write((const char*)&nVersion, sizeof(nVersion));

	//alright, now write out the number of nodes we effect
	int nNumNodes;
	InFile >> nNumNodes;

	//sanity check
	if(nNumNodes <= 0)
	{
#if _MSC_VER >= 1300
		std::cout << "Invalid number of nodes found in file: " << nNumNodes << std::endl;
#else
		cout << "Invalid number of nodes found in file: " << nNumNodes << endl;
#endif
		return false;
	}
	
	OutFile.write((const char*)&nNumNodes, sizeof(nNumNodes));

	//now read in the total time of the lip sync
	int nAnimLen;
	InFile >> nAnimLen;

	//sanity check
	if(nAnimLen <= 0)
	{
#if _MSC_VER >= 1300
		std::cout << "Invalid animation length found in file: " << nAnimLen << std::endl;
#else
		cout << "Invalid animation length found in file: " << nAnimLen << endl;
#endif

		return false;
	}

	//alright, now it is time for the keyframes, we need to figure out how many there are, we use the
	//assumption that there are keyframes at 0, 33, and 66, therefore to get the number, first
	//divide it into hudndreds, then use the remainder. Integer truncation does the rest
	int nNumKeyFrames = (nAnimLen / 100) * 3 + (nAnimLen % 100) / 33;
	OutFile.write((const char*)&nNumKeyFrames, sizeof(nNumKeyFrames));

	//now for each node we need to write out the name in the form of Length - String
	int nCurrNode;
	for(nCurrNode = 0; nCurrNode < nNumNodes; nCurrNode++)
	{
		//read in the node name
		char pszNodeName[1024 + 1];
		InFile >> pszNodeName;

		//get the length
		int nNodeNameLen = strlen(pszNodeName);

		//ok, now output it to the file
		OutFile.write((const char*)&nNodeNameLen, sizeof(nNodeNameLen));
		OutFile.write(pszNodeName, nNodeNameLen);
	}

	//allocate memory to hold what we are about to read in
	SKeyFrame* pFrames = new SKeyFrame [nNumNodes * nNumKeyFrames];

	//check the allocation
	if(!pFrames)
	{
#if _MSC_VER >= 1300
		std::cout << "Error allocating memory" << std::endl;
#else
		cout << "Error allocating memory" << endl;
#endif
		return false;
	}

	//alright, now read in all the keyframes
	for(int nCurrFrame = 0; nCurrFrame < nNumKeyFrames; nCurrFrame++)
	{
		//read in the time number
		int nFrameTime;
		InFile >> nFrameTime;

		for(int nCurrNode = 0; nCurrNode < nNumNodes; nCurrNode++)
		{
			//alright, now we need to load in the translation/rotation, apply our custom conversions to them,
			//and write them out
			LTVector vEuler;
			LTVector vTranslation;

			InFile >> vTranslation.x >> vTranslation.y >> vTranslation.z;
			InFile >> vEuler.x >> vEuler.y >> vEuler.z;

			//now store them
			pFrames[nCurrNode * nNumKeyFrames + nCurrFrame].m_vTranslation = vTranslation;
			pFrames[nCurrNode * nNumKeyFrames + nCurrFrame].m_vEuler = vEuler;
		}
	}

	//now write them out so that each node can be read in in a single chunk
	for(int nCurrOutFrame = 0; nCurrOutFrame < nNumNodes * nNumKeyFrames; nCurrOutFrame++)
	{
		LTVector vTranslation = pFrames[nCurrOutFrame].m_vTranslation;
		LTVector vEuler = pFrames[nCurrOutFrame].m_vEuler;

		LTRotation rRotation;
		rRotation.Identity();

		//do some conversions from Maya to LT
		vTranslation.x = -vTranslation.x;
		vEuler.y = -vEuler.y;
		vEuler.z = -vEuler.z;

		//build up our rotation so we don't have to at runtime
		rRotation.Rotate(rRotation.Up(), DEG2RAD(vEuler.y));
		rRotation.Rotate(rRotation.Right(), DEG2RAD(vEuler.x));
		rRotation.Rotate(rRotation.Forward(), DEG2RAD(vEuler.z));

		//and finally write them out
		OutFile.write((const char*)&vTranslation, sizeof(vTranslation));
		OutFile.write((const char*)&rRotation, sizeof(rRotation));
	}

	delete [] pFrames;
	pFrames = NULL;

	OutFile.flush();
#if _MSC_VER >= 1300
	std::cout << pszOut << " successfully compiled." << std::endl;
#else
	cout << pszOut << " successfully compiled." << endl;
#endif
	return true;
}

//displays proper command line usage of this tools
static void DisplayUsage(const char* pszError)
{
#if _MSC_VER >= 1300
	std::cout << "LipCompiler /file <file to compile> [/out <name of output file>] [/help]" << std::endl;
	std::cout << std::endl;
	std::cout << pszError << std::endl;
#else
	cout << "LipCompiler /file <file to compile> [/out <name of output file>] [/help]" << endl;
	cout << endl;
	cout << pszError << endl;
#endif
}

//Main, handles parameter parsing and verification
int main(int nArgCount, char** ppArgs)
{
	//init our command line parser
	CCommandLineParser Parser;
	Parser.Init(nArgCount - 1, &(ppArgs[1]), '/');

	//setup our options
	// /file <filename>
	Parser.SetNumParameters("file", 1);

	// /out <filename>
	Parser.SetNumParameters("out", 1);

	// /help
	Parser.SetNumParameters("help", 0);

	//see if the user wants help
	if(Parser.IsOptionSet("help"))
	{
		//display command line usage and bail
		DisplayUsage("");
		return 1;
	}

	//--------------------------
	// Option verification

	//scan for unexpected options
	for(uint32 nCurrOption = 0; nCurrOption < Parser.GetNumOptions(); nCurrOption++)
	{
		const char* pszOptName = Parser.GetOptionName(nCurrOption);

		if(	stricmp(pszOptName, "file") &&
			stricmp(pszOptName, "out") &&
			stricmp(pszOptName, "help"))
		{
			DisplayUsage("");
#if _MSC_VER >= 1300
			std::cout << "Unknown option: " << pszOptName << std::endl;
#else
			cout << "Unknown option: " << pszOptName << endl;
#endif
			return 1;
		}
	}

	//make sure that we have a file to compile
	if(!Parser.IsOptionSet("file") || (Parser.GetNumParameters("file") < 1))
	{
		DisplayUsage("A file name must be specified for compiling");
		return 1;
	}


	//make sure that if they are outputting, that they have a file
	if(Parser.IsOptionSet("out") && (Parser.GetNumParameters("out") < 1))
	{
		DisplayUsage("A file name must be specified for output");
		return 1;
	}

	//determine the output file name
	char pszOutFile[_MAX_PATH + 1];

	if(Parser.IsOptionSet("out"))
	{
		//just read it in from what was specified
		strncpy(pszOutFile, Parser.GetParameter("out", 0), _MAX_PATH);
	}
	else
	{
		//not specified, just use the input name and switch the extension
		strncpy(pszOutFile, Parser.GetParameter("file", 0), _MAX_PATH);

		//start at the end, and work back to the period
		char* pszEnd = pszOutFile + strlen(pszOutFile);

		while(	(pszEnd >= pszOutFile) && 
				(*pszEnd != '.') && 
				(*pszEnd != '\\') &&
				(*pszEnd != '/'))
					pszEnd--;

		//see if we hit the beginning or a period
		if(*pszEnd == '.')
		{
			strcpy(pszEnd, ".lip");
		}
		else
		{
			strncat(pszOutFile, ".lip", _MAX_PATH);
		}
	}
			
	//Ok, we are good to go. Compile away.
	if(!CompileFile(Parser.GetParameter("file", 0), pszOutFile))
		return 1;

	return 0;
}