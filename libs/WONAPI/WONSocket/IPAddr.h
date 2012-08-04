#ifndef __WON_IPADDR_H__
#define __WON_IPADDR_H__
#include "WONShared.h"
#include "WONCommon/Platform.h"

#include <string>
#include <set>
#include <vector>
#include <list>

namespace WONAPI
{

class IPAddr
{
private:
	bool mRememberHostString;
	std::string mHostString;
	SOCKADDR_IN mAddr;
	bool mIsValid;
	
public:
	IPAddr();
	IPAddr(const std::string &theHostAndPort);
	IPAddr(const char* theHostAndPort);
	IPAddr(const std::string &theHost, unsigned short thePort);
	IPAddr(long theHost, unsigned short thePort); // both in host byte order
	IPAddr(const SOCKADDR_IN &theAddr);

	static IPAddr GetLocalAddr();

	std::string GetHostString(bool useOriginal = true) const;
	std::string GetHostAndPortString(bool useOriginal = true) const;
	long GetHost() const; // in host byte order
	unsigned short GetPort() const; // in host byte order

	const char* GetSixByte() const;
	void SetSixByte(const void* theBuf);
	
	bool IsValid() const;

	const SOCKADDR_IN& GetSockAddrIn() const;

	bool Set(const std::string &theHostAndPort);
	bool Set(const std::string &theHost, unsigned short thePort);
	void Set(long theHost, unsigned short thePort); // both in host byte order
	void Set(const SOCKADDR_IN &theAddr);
	bool SetWithDefaultPort(const std::string &theHostAndPort, unsigned short theDefaultPort);
	void SetThePort(unsigned short thePort); // Windows defines a SetPort macro

	void SetRememberHostString(bool remember) { mRememberHostString = remember; }

	bool operator <(const IPAddr& theAddr) const;
	bool operator ==(const IPAddr& theAddr) const;
	bool operator !=(const IPAddr& theAddr) const;
};

typedef std::set<IPAddr> AddrSet;
typedef std::vector<IPAddr> AddrVec;
typedef std::list<IPAddr> AddrList;

}; // namespace WONAPI

#endif
