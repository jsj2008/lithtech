// ----------------------------------------------------------------------- //
//
// MODULE  : ClientButeMgr.h
//
// PURPOSE : ClientButeMgr definition - Client-side attributes
//
// CREATED : 2/02/99
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_BUTE_MGR_H__
#define __CLIENT_BUTE_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"


#define CBMGR_DEFAULT_FILE "Attributes\\ClientButes.txt"

class CClientButeMgr;
extern CClientButeMgr* g_pClientButeMgr;

class CClientButeMgr : public CGameButeMgr
{
	public :

		CClientButeMgr();
		~CClientButeMgr();

        LTBOOL       Init(ILTCSBase *pInterface, const char* szAttributeFile=CBMGR_DEFAULT_FILE);
		void		Term();

        LTBOOL       WriteFile() { return m_buteMgr.Save(); }
		void		Reload()    { m_buteMgr.Parse(m_strAttributeFile); }

		int			GetNumCheatAttributes() const { return m_nNumCheatAttributes; }
        CString     GetCheat(uint8 nCheatNum);

		float		GetReverbAttributeFloat(char* pAttribute);

		int			GetCameraAttributeInt(char* pAttribute);
		float		GetCameraAttributeFloat(char* pAttribute);
		CString		GetCameraAttributeString(char* pAttribute);

		int			GetGameAttributeInt(char* pAttribute);
		float		GetGameAttributeFloat(char* pAttribute);
		CString		GetGameAttributeString(char* pAttribute);

		float		GetWeatherAttributeFloat(char* pAttribute);
		CString		GetWeatherAttributeString(char* pAttribute);

		float		GetSpecialFXAttributeFloat(char* pAttribute);
		CString		GetSpecialFXAttributeString(char* pAttribute);

		float		GetBreathFXAttributeFloat(char* pAttribute);
		int			GetBreathFXAttributeInt(char* pAttribute);
		CString		GetBreathFXAttributeString(char* pAttribute);
        LTVector     GetBreathFXAttributeVector(char* pAttribute);

		CString		GetInterfaceAttributeString(char* pAttribute);


		int			GetNumSingleWorldPaths()	const { return m_nNumSingleWorldPaths; }
		int			GetNumMultiWorldPaths()	const { return m_nNumMultiWorldPaths; }
        void        GetWorldPath(uint8 nPath, char* pBuf, int nBufLen, LTBOOL bSingle = LTTRUE);

	protected :

	private :

        uint8   m_nNumCheatAttributes;
		int		m_nNumSingleWorldPaths;
		int		m_nNumMultiWorldPaths;

};



#endif // __CLIENT_BUTE_MGR_H__