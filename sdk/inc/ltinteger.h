#ifndef __LTINTEGER_H__
#define __LTINTEGER_H__


/*!
Portable integer types. 
*/
typedef unsigned int        uint; // This is at least 16 bits

typedef char			int8;
typedef short int		int16;
typedef int				int32;

#ifdef _MSC_VER
typedef __int64         int64;
#else
typedef long long       int64;
#endif

typedef unsigned char		uint8;
typedef unsigned short int	uint16;
typedef unsigned int		uint32;

#ifdef _MSC_VER
typedef unsigned __int64    uint64;
#else
typedef unsigned long long  uint64;
#endif

#endif
