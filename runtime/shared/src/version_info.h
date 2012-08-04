#ifndef __VERSION_INFO_H__
#define __VERSION_INFO_H__


#define INVALID_LT_VERSION  0xFFFFFFFF


class LTVersionInfo {
public:

    LTVersionInfo() {
        m_MajorVersion = INVALID_LT_VERSION;
        m_MinorVersion = INVALID_LT_VERSION;
    }

    // Get the version string.
    void GetString(char *pStr, uint32 nStrBytes);

public:
    // If version is 109.1, then:
    // m_MajorVersion = 109
    // m_MinorVersion = 1
    uint32 m_MajorVersion;
    uint32 m_MinorVersion;
};


#endif


