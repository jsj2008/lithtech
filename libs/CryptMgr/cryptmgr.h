#if !defined(_CRYPTMGR_H_)
#define _CRYPTMGR_H_

#if _MSC_VER >= 1300
#	include <iostream>
#else
#	include <iostream.h>
#endif // VC7


class CCryptMgr
{
public:

	CCryptMgr();
	CCryptMgr(char* key);
	~CCryptMgr();

	void SetKey(const char* key);  // Max of 56 characters

	// if using fstreams be sure to open them in binary mode
#if _MSC_VER >= 1300
	void Encrypt(std::istream& is, std::ostream& os);
#else
	void Encrypt(istream& is, ostream& os);
#endif // VC7

	// if using fstreams be sure to open them in binary mode
#if _MSC_VER >= 1300
	void Decrypt(std::istream& is, std::ostream& os);
#else
	void Decrypt(istream& is, ostream& os);
#endif // VC7

};






#endif
