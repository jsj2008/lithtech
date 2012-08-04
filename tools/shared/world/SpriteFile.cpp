#include "bdefs.h"
#include "SpriteFile.h"

#if _MSC_VER >= 1300
#include <fstream>
#else
#include <fstream.h>
#endif

CSpriteFile::CSpriteFile() :
	m_nNumFrames(0),
	m_ppszFrames(NULL),
	m_nFrameRate(0),
	m_bTransparent(true),
	m_bTranslucent(true),
	m_nKey(false)
{
}

CSpriteFile::~CSpriteFile()
{
	FreeAllFrames();
}

//called to load in a sprite file and build up information about it
bool CSpriteFile::Load(const char* pszFilename)
{
	//open up the file
#if _MSC_VER >= 1300
	std::ifstream InFile( pszFilename, std::ios::in | std::ios::binary );
#else
	ifstream InFile(pszFilename, ios::in | ios::nocreate | ios::binary);
#endif
	//check the open
	if(!InFile)
		return false;

	//now read in the header data
	uint32 nTempVal;

	//read in the number of frames
	InFile.read((char*)&nTempVal, sizeof(nTempVal));

	//resize accordingly
	if(!SetNumFrames(nTempVal))
		return false;

	//read in the rest of the header
	InFile.read((char*)&m_nFrameRate, sizeof(m_nFrameRate));

	InFile.read((char*)&nTempVal, sizeof(nTempVal));
	m_bTransparent = !!nTempVal;
	InFile.read((char*)&nTempVal, sizeof(nTempVal));
	m_bTranslucent = !!nTempVal;

	InFile.read((char*)&m_nKey, sizeof(m_nKey));


	//now read in all the names
	for(uint32 nCurrFrame = 0; nCurrFrame < m_nNumFrames; nCurrFrame++)
	{
		//the name of the frame
		char pszFrame[MAX_PATH + 1];

		//the length of the name
		uint16 nStrLen = 0; 

		//read in the name
		InFile.read((char*)&nStrLen, sizeof(nStrLen));

		//now read in each character
		for(uint32 nCurrChar = 0; nCurrChar < nStrLen; nCurrChar++)
		{
			char ch;
			InFile.read(&ch, sizeof(ch));

			if(nCurrChar < MAX_PATH)
			{
				pszFrame[nCurrChar] = ch;
				pszFrame[nCurrChar + 1] = '\0';
			}
		}

		//now set this name
		SetFrame(nCurrFrame, pszFrame);
	}

	//success
	return true;
}

//called to save the current state out to a file
bool CSpriteFile::Save(const char* pszFilename) const
{
	//open up the file
#if _MSC_VER >= 1300
	std::ofstream OutFile(pszFilename, std::ios::out | std::ios::binary);
#else
	ofstream OutFile(pszFilename, ios::out | ios::binary);
#endif

	//check the open
	if(!OutFile)
		return false;

	//write out the header
	uint32 nTempVal;

	OutFile.write((char*)&m_nNumFrames, sizeof(m_nNumFrames));
	OutFile.write((char*)&m_nFrameRate, sizeof(m_nFrameRate));
	
	nTempVal = m_bTransparent ? 1 : 0;
	OutFile.write((char*)&nTempVal, sizeof(nTempVal));
	nTempVal = m_bTranslucent ? 1 : 0;
	OutFile.write((char*)&nTempVal, sizeof(nTempVal));

	OutFile.write((char*)&m_nKey, sizeof(m_nKey));

	//now write out all the strings
	for(uint32 nCurrFrame = 0; nCurrFrame < GetNumFrames(); nCurrFrame++)
	{
		const char* pszFrame = GetFrame(nCurrFrame);

		uint16 nStrLen = pszFrame ? strlen(pszFrame) : 0;
		OutFile.write((char*)&nStrLen, sizeof(nStrLen));
		OutFile.write(pszFrame, nStrLen);
	}

	//success
	return true;
}


//called to determine the number of frames in the sprite file
uint32 CSpriteFile::GetNumFrames() const
{
	return m_nNumFrames;
}

//called to get the name of a specified frame
const char* CSpriteFile::GetFrame(uint32 nFrame) const
{
	assert(nFrame < GetNumFrames());
	return m_ppszFrames[nFrame];
}

//sets the number of frames for the sprite file (this will clear all the frame
//names)
bool CSpriteFile::SetNumFrames(uint32 nNumFrames)
{
	//out with the old...
	FreeAllFrames();

	//in with the new...
	m_ppszFrames = new char* [nNumFrames];

	if(m_ppszFrames == NULL)
		return false;

	//make sure the new is in a consistant state
	for(uint32 nCurrFrame = 0; nCurrFrame < nNumFrames; nCurrFrame++)
		m_ppszFrames[nCurrFrame] = NULL;

	m_nNumFrames = nNumFrames;

	//success
	return true;
}

//sets a specific frame name
bool CSpriteFile::SetFrame(uint32 nFrame, const char* pszName)
{
	assert(nFrame < GetNumFrames());

	FreeFrame(nFrame);

	m_ppszFrames[nFrame] = new char [strlen(pszName) + 1];

	if(!m_ppszFrames[nFrame])
		return false;

	strcpy(m_ppszFrames[nFrame], pszName);

	return true;
}

//frees a given string
void CSpriteFile::FreeFrame(uint32 nFrame)
{
	assert(nFrame < GetNumFrames());
	delete [] m_ppszFrames[nFrame];
	m_ppszFrames[nFrame] = NULL;
}

//frees all frames
void CSpriteFile::FreeAllFrames()
{
	for(uint32 nCurrFrame = 0; nCurrFrame < GetNumFrames(); nCurrFrame++)
	{
		FreeFrame(nCurrFrame);
	}
}


//utility function to determine if the specified filename is that of a sprite
bool CSpriteFile::IsSprite(const char* pszFilename)
{
	//just check the last 4 characters for .spr
	uint32 nLen = strlen(pszFilename);
	if((nLen > 4) && (stricmp(&pszFilename[nLen - 4], ".spr") == 0))
		return true;

	//not a sprite
	return false;
}

