#include "cryptmgr.h"
#include "blowfish.h"
#include "string.h"





CCryptMgr::CCryptMgr()
{
}


CCryptMgr::CCryptMgr(char* key)
{
	InitializeBlowfish((UBYTE_08bits*)key, sizeof(key));
}


CCryptMgr::~CCryptMgr()
{
}



void CCryptMgr::SetKey(const char* key)
{
	InitializeBlowfish((UBYTE_08bits*)key, sizeof(key));
}




#if _MSC_VER >= 1300
void CCryptMgr::Encrypt(std::istream& is, std::ostream& os)
#else
void CCryptMgr::Encrypt(istream& is, ostream& os)
#endif // VC7
{
	int n = 0;
	char buf[8];

	while (!is.eof())
	{
		memset(buf, 0, 8);
		is.read(buf, 8);
		n = is.gcount();
		Blowfish_encipher((UWORD_32bits*)buf, (UWORD_32bits*)&buf[4]);
		os.write(buf, 8);
	}
	os.put((char)n);
}



#if _MSC_VER >= 1300
void CCryptMgr::Decrypt(std::istream& is, std::ostream& os)
#else
void CCryptMgr::Decrypt(istream& is, ostream& os)
#endif // VC7
{
	int n = 0;
	char buf[8];
	char tbuf[8];
	bool bFirstTime = true;

	// I know theres a better way - will fix later [Jpl]
	while (!is.eof())
	{
		is.read(buf, 8);
		n = is.gcount();
		if (n == 1)
			n = (int)buf[0];
		if (!bFirstTime)
			os.write(tbuf, n);
		else
			bFirstTime = false;
		Blowfish_decipher((UWORD_32bits*)buf, (UWORD_32bits*)&buf[4]);
		memcpy(tbuf, buf, 8);
	}
}
