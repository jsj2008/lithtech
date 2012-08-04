//------------------------------------------------------------------
//
// SpriteFile.h
// Utility class for handling the loading of sprite files
//

#ifndef __SPRITEFILE_H__
#define __SPRITEFILE_H__

class CSpriteFile
{
public:

	CSpriteFile();
	~CSpriteFile();

	//called to load in a sprite file and build up information about it
	bool	Load(const char* pszFilename);

	//called to save the current state out to a file
	bool	Save(const char* pszFilename) const;

	//called to determine the number of frames in the sprite file
	uint32	GetNumFrames() const;

	//called to get the name of a specified frame
	const char* GetFrame(uint32 nFrame) const;

	//sets the number of frames for the sprite file (this will clear all the frame
	//names)
	bool	SetNumFrames(uint32 nNumFrames);

	//sets a specific frame name
	bool	SetFrame(uint32 nFrame, const char* pszName);

	//the number of milliseconds per frame
	uint32	m_nFrameRate;
	bool	m_bTransparent;
	bool	m_bTranslucent;
	uint32	m_nKey;

	//utility function to determine if the specified filename is that of a sprite
	static bool IsSprite(const char* pszFilename);

private:

	//frees a given string
	void FreeFrame(uint32 nFrame);

	//frees all frames
	void FreeAllFrames();

	//the number of frames
	uint32	m_nNumFrames;

	//the list of frame names
	char**	m_ppszFrames;
};


#endif
