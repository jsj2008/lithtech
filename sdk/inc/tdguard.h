//------------------------------------------------------------------
//
//  FILE      : TdGuard.h
//
//  PURPOSE   : Aegis class
//
//  COPYRIGHT : LithTech Inc., 1996-2003
//
//------------------------------------------------------------------

#ifndef __TDGUARD_H__
#define __TDGUARD_H__



namespace TdGuard
{

class Aegis
{
public:

	~Aegis();

	// singleton access
	static Aegis&			GetSingleton();

	// initialize
	virtual bool			Init();

	// do something
	virtual bool 			DoWork();

private:

	Aegis();
};


} // namespace TdGuard



#endif // __TDGUARD_H__
