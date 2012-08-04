/****************************************************************************
;
;   MODULE:     ILTInfo (.H)
;
;   PURPOSE:    General purpose functions for version and performance
;               information
;
;   HISTORY:    Mar-1-2000
;
***************************************************************************/

#ifndef __ILTINFO_H__
#define __ILTINFO_H__



/*!  Enhanced version info.  */

enum
{
    LT_VI_LANG_ENGLISH =     0,
    LT_VI_LANG_FRENCH =      1,
    LT_VI_LANG_GERMAN =      2,
    LT_VI_LANG_SPANISH =     3
};


enum
{
    LT_VI_PLAT_PCWIN =		0,    
    LT_VI_PLAT_PS2 =		1,
	LT_VI_PLAT_XBOX =		2,
    LT_VI_PLAT_LINUX =		3,
    LT_VI_PLAT_DREAMCAST =	4,
    LT_VI_PLAT_MAC =		5
};




/*!  Current build version info.  */

#define LT_VI_VER_MAJOR         4
#define LT_VI_VER_MINOR         69
#define LT_VI_BUILD_NUM         1
#define LT_VI_BUILD_MONTH       12
#define LT_VI_BUILD_DAY         14
#define LT_VI_BUILD_YEAR        2005
#define LT_VI_BUILD_NAME        "Jupiter 69"
#define LT_VI_LANG              LT_VI_LANG_ENGLISH
#ifdef __LINUX
#define LT_VI_PLAT              LT_VI_PLAT_LINUX
#elif __XBOX
#define LT_VI_PLAT				LT_VI_PLAT_XBOX
#else
#define LT_VI_PLAT              LT_VI_PLAT_PCWIN
#endif


/*!  Enhanced version info.  */

typedef struct LTVersionInfoExt_t
{
    uint32  m_dwSize;
    uint32  m_dwFlags;
    uint32  m_dwMajorVersion;
    uint32  m_dwMinorVersion;
    uint32  m_dwBuildNumber;
    uint32  m_dwBuildMonth;
    uint32  m_dwBuildDay;
    uint32  m_dwBuildYear;
    char    m_sBuildName[128];
    uint32  m_dwLanguage;
    uint32  m_dwPlatform;
    uint32  m_dwReserved1;
    uint32  m_dwReserved2;

} LTVERSIONINFOEXT;


/*!  Enhanced run-time performance info.  */

typedef struct LTPerformanceInfo_t
{
    uint32  m_dwSize;
    uint32  m_dwFlags;
    uint32  m_dwScreenWidth;
    uint32  m_dwScreenHeight;
    uint32  m_dwFps;
    uint32  m_dwNumWorldPolys;
    uint32  m_dwNumWorldPolysProcessed;
    uint32  m_dwNumModelPolys;
    uint32  m_dwReserved1;

} LTPERFORMANCEINFO;

#endif //! __ILTINFO_H__

