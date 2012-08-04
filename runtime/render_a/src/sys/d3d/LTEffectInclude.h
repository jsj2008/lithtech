#include <d3dx9shader.h>
#include <iltclient.h>

class LTEffectInclude : public ID3DXInclude
{
public: 
	LTEffectInclude();

	STDMETHOD (Open)( D3DXINCLUDE_TYPE IncludeType,	LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
	STDMETHOD (Close)(LPCVOID pData);
	void	SetParentFilename(const char* szFilename);

protected:

	typedef std::list<std::string> PathList;
	PathList	m_PathList;

	bool PushPath(const char* szPath);
	void PopPath();
	void BuildPath(char* szBuffer, const char* szPath, int nMaxLength);

};