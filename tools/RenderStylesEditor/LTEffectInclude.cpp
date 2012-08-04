#include "stdafx.h"

#include "LTEffectInclude.h"
#include "RenderStylesEditor.h"
#include <io.h>

LTEffectInclude::LTEffectInclude():
OutputMessage(NULL)
{

}

HRESULT LTEffectInclude::Open(      
							  D3DXINCLUDE_TYPE IncludeType,
							  LPCSTR pFileName,
							  LPCVOID pParentData,
							  LPCVOID *ppData,
							  UINT *pBytes
							  )
{

	if(pFileName[0] == '\0')
	{
		return E_FAIL;
	}

	if(PushPath(pFileName) == false)
	{
		*ppData = NULL;
		*pBytes = 0;
		return E_FAIL;
	}

	char szFilename[_MAX_PATH];
	BuildPath(szFilename, pFileName, _MAX_PATH);

	intptr_t hFile;
	struct _finddata_t file;
	int nFileSize = 0;
	if( (hFile = _findfirst( szFilename, &file )) != -1L )
	{
		nFileSize = (int)file.size;
	}
	else
	{
		return E_FAIL;
	}

	uint8* pData = new uint8[nFileSize];
	memset(pData, 0, nFileSize);
	if(pData == NULL)
	{
		return E_OUTOFMEMORY;
	}


#ifdef _DEBUG
	// Output our include solver tree...
	char szDepth[_MAX_PATH];
	memset(szDepth, 0, _MAX_PATH);
	for(int k = 0; k < (int)m_PathList.size(); ++k){ szDepth[k] = '\t'; }
	char szDebugBuf[_MAX_PATH];
	sprintf(szDebugBuf, "%s: %s%s size: (%d)\n", ((int)m_PathList.size() <= 1) ? "Solving" : "Including", szDepth, szFilename, nFileSize);
	OutputDebugString(szDebugBuf);
#endif

	FILE *fp = fopen(szFilename, "rb");
	if(!fp)
	{
		delete [] pData;
		*ppData = NULL;
		*pBytes = 0;
		return E_FAIL;
	}

	fread(pData, nFileSize, 1, fp);
	fclose(fp);

	//pData[nFileSize] = '\0';

	*ppData = pData;
	*pBytes = nFileSize;//+1;

	return S_OK;
}

HRESULT LTEffectInclude::Close(LPCVOID pData)
{
	if(pData)
	{
		delete [] pData;
	}

	PopPath();

	return S_OK;
}

void	LTEffectInclude::SetParentFilename(const char* szFilename)
{
	PushPath(szFilename);
}



bool LTEffectInclude::PushPath(const char* szPath)
{
	if(szPath[0] == '\0')
	{
		return false;
	}

	char szBuf[_MAX_PATH];
	szBuf[0] = '\0';

	// Chop off the file name. We just want the path.
	strncpy(szBuf, szPath, _MAX_PATH - 1);
	int nLen = strlen(szBuf);
	int nFilenameStart = 0;
	if(nLen > 0)
	{
		for(int i = (nLen-1); i >= 0; --i)
		{
			if( (szBuf[i] == '\\') || (szBuf[i] == '/') )
			{
				nFilenameStart = i + 1;
				break;
			}
			else
			{
				szBuf[i] = '\0';
			}
		}
	}

	// Check to see if we're jumping to a parent path.
	if(szBuf[0] == '.')
	{
		int nNumBackSteps = 0;
		int i = 0;
		for(i = 0; i < nLen; ++i)
		{
			if(szBuf[i] == '.')
			{
				szBuf[i] = '\0';
			}
			else
			if( (szBuf[i] == '\\') || (szBuf[i] == '/') )
			{
				szBuf[i] = '\0';
				++nNumBackSteps;
			}
			else
			{
				//we're done!
				//We are no longer back tracing...
				break;
			}
		}

		char szMidPath[512];
		szMidPath[0] = '\0';
		szMidPath[511] = '\0';
		if(szBuf[i] != '\0')
		{
			strncpy(szMidPath, &szBuf[i], 511);
		}

		//
		//Ok, now build a path out of this...
		//
		
		// Backstep from the parent path if needed
		char szTempBuf[_MAX_PATH];
		strncpy( szTempBuf, (*(m_PathList.begin())).c_str(), _MAX_PATH);

		int nTempLen = strlen(szTempBuf);
		szTempBuf[nTempLen - 1] = '\0';
		nTempLen = strlen(szTempBuf);
		for(int i = (nTempLen - 1); i >= 0; --i)
		{
			if( (szTempBuf[i] == '\\') || (szTempBuf[i] == '/') )
			{				
				--nNumBackSteps;
				if(nNumBackSteps <= 0)
				{
					break;
				}
				szTempBuf[i] = '\0';
			}
			else
			{
				szTempBuf[i] = '\0';
			}
		}

		if(nNumBackSteps >= 2)
		{
			//dsi_ConsolePrint("Error: (%s) is not a valid directory depth!!!", szPath);

			return false;
		}

		strncpy(szBuf, szTempBuf, _MAX_PATH);

		if(szMidPath[0] != '\0')
		{
			strncat(szBuf, szMidPath, _MAX_PATH-1);
		}

	}
	else
	{
		//
		// If the list is not empty, then we may need to combine
		// the paths to make a full path
		//		
		if(m_PathList.begin() != m_PathList.end())
		{
			char szTempBuf[_MAX_PATH];

			strncpy(szTempBuf, (*(m_PathList.begin())).c_str(), _MAX_PATH - 1);
			strcat(szTempBuf, szBuf);
			strncpy(szBuf, szTempBuf, _MAX_PATH);
		}		
	}

	m_PathList.push_front(szBuf);

	return true;
}

void LTEffectInclude::PopPath()
{
	if(m_PathList.size() > 0)
	{
		m_PathList.pop_front();
	}
}

void LTEffectInclude::BuildPath(char* szBuffer, const char* szPath, int nMaxLength)
{
	//build filename
	int nLen = strlen(szPath);

	//rewind until we find a slash or end of string
	int i = 0;
	for(i = (nLen-1); i > 0; --i )
	{
		if( (szPath[i] == '\\') || (szPath[i] == '/') )
		{
			break;
		}
	}

	char* pszFilename = (char*)&szPath[ ((i > 0) ? i+1 : 0) ];
	strncpy(szBuffer, (*(m_PathList.begin())).c_str(), nMaxLength);
	strncat(szBuffer, pszFilename, nMaxLength);
}