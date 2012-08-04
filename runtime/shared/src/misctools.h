//////////////////////////////////////////////////////////////////////////////
// Misc tools

#ifndef __MISCTOOLS_H__
#define __MISCTOOLS_H__

// Compile-time determination of the size of an unsigned int
template <unsigned int NUMBER>
struct FNumBits
{
	enum {
		k_nValue = (FNumBits<NUMBER / 2>::k_nValue + 1)
	};
};

template<> struct FNumBits<0>
{
	enum {
		k_nValue = 0
	};
};

template<> struct FNumBits<1>
{
	enum {
		k_nValue = 1
	};
};

#endif //__MISCTOOLS_H__
