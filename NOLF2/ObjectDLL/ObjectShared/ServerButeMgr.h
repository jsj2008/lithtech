// ----------------------------------------------------------------------- //
//
// MODULE  : ServerButeMgr.h
//
// PURPOSE : ServerButeMgr definition - Server-side attributes
//
// CREATED : 2/02/99
//
// ----------------------------------------------------------------------- //

#ifndef __SERVER_BUTE_MGR_H__
#define __SERVER_BUTE_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"


class CServerButeMgr;
extern CServerButeMgr* g_pServerButeMgr;

class CServerButeMgr : public CGameButeMgr
{
	public :

		CServerButeMgr();
		~CServerButeMgr();

        LTBOOL       Init(const char* szAttributeFile="Attributes\\ServerButes.txt");
		void		Term();

        LTBOOL       WriteFile() { return m_buteMgr.Save(); }
		void		Reload()    { m_buteMgr.Parse(m_strAttributeFile); }

		int			GetPlayerAttributeInt(char* pAttribute, int nDefVal);
		float		GetPlayerAttributeFloat(char* pAttribute);
		void		GetPlayerAttributeString(char* pAttribute, char* pBuf, int nBufLen);

		int			GetSecurityCameraInt(char* pAttribute);
		float		GetSecurityCameraFloat(char* pAttribute);
		void		GetSecurityCameraString(char* pAttribute, char* pBuf, int nBufLen);

		int			GetNumWONServers()			{return m_nNumWONServers;}
		void		GetWONAddress(int nServer, char* pBuf, int nBufLen);
        uint32      GetWONPort(int nServer);

		LTFLOAT		GetBodyStairsFallSpeed();
		LTFLOAT		GetBodyStairsFallStopSpeed();

		LTFLOAT		GetSummaryDelay();

		int			GetAmmoBoxAttributeInt( const char* pAttribute, int nDefVal );
		float		GetAmmoBoxAttributeFloat( const char* pAttribute);
		void		GetAmmoBoxAttributeString( const char* pAttribute, char* pBuf, int nBufLen );
		void		GetAmmoBoxAttributeListReader( CButeListReader *pList, const char* pszAttName, const uint32 nStrLen );

		// Doomsday values.
		bool		GetDoomsDayDeviceFxName( char* pszFxName, int nLen );
		float		GetDoomsDayDeviceDamageDelay( );
		float		GetDoomsDayDeviceDamageInterval( );
		float		GetDoomsDayDeviceEndTime( );
		bool		GetDoomsDayDeviceDamageType( char* pszDamageType, int nLen );
		float		GetDoomsDayDeviceDamageAmount( );
		float		GetDoomsDayDeviceRespawnTime( );

	private:
		int			m_nNumWONServers;
};



#endif // __SERVER_BUTE_MGR_H__