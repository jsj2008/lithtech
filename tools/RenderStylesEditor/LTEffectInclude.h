#include <d3dx9shader.h>
#include <list>
#include <string>

typedef void (*OutputMsg)(char* fmt, ...);

class LTEffectInclude : public ID3DXInclude
{
public: 
	LTEffectInclude();

	STDMETHOD (Open)( D3DXINCLUDE_TYPE IncludeType,	LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
	STDMETHOD (Close)(LPCVOID pData);
	void	SetParentFilename(const char* szFilename);
	void	SetOutputLogFunction(OutputMsg fn){OutputMessage = fn;}

protected:

	typedef std::list<std::string> PathList;
	PathList	m_PathList;

	bool PushPath(const char* szPath);
	void PopPath();
	void BuildPath(char* szBuffer, const char* szPath, int nMaxLength);
	void (*OutputMessage)(char* fmt, ...);

};