#ifndef __FILEMARKER_H__
#define __FILEMARKER_H__

#include "abstractio.h"

// If you instatiate one of these inside a block ({ and }), it will 
// write a marker in the file where it leaves the scope if bAuto
// is TRUE.  Otherwise, call Mark() to leave a marker.
class CFileMarker
{
public:
	CFileMarker(CAbstractIO &theFile, bool bAuto)
	{
		m_bAuto			= bAuto;
		m_pFile			= &theFile;
		m_MarkerPos		= theFile.GetCurPos();

		theFile << (uint32)0;
	}

	~CFileMarker()
	{
		if(m_bAuto)
		{
			Mark();
		}
	}

	void		Mark()
	{
				uint32 curPos;
				
				curPos = m_pFile->GetCurPos();
				m_pFile->SeekTo(m_MarkerPos);
				*m_pFile << curPos;
				m_pFile->SeekTo(curPos);
	}

private:

	bool		m_bAuto;
	CAbstractIO	*m_pFile;
	uint32		m_MarkerPos;
};

#endif
