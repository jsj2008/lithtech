// ----------------------------------------------------------------------- //
//
// MODULE  : Civilian1.h
//
// PURPOSE : Civilian1 - Definition
//
// CREATED : 1/28/98
//
// ----------------------------------------------------------------------- //

#ifndef __CIVILIAN1_H__
#define __CIVILIAN1_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Civilian1 : public BaseAI
{
	public :

 		Civilian1();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		DBOOL	ReadProp(ObjectCreateStruct *pData);
};


class BYSTANDER_Civilian1 : public Civilian1
{
	public :

 		BYSTANDER_Civilian1();
};

class ROGUE_Civilian1 : public Civilian1
{
	public :

 		ROGUE_Civilian1();
};

class CMC_Civilian1 : public Civilian1
{
	public :

 		CMC_Civilian1();
};

class SHOGO_Civilian1 : public Civilian1
{
	public :

 		SHOGO_Civilian1();
};

class UCA_Civilian1 : public Civilian1
{
	public :

 		UCA_Civilian1();
};

class FALLEN_Civilian1 : public Civilian1
{
	public :

 		FALLEN_Civilian1();
};

class CRONIAN_Civilian1 : public Civilian1
{
	public :

 		CRONIAN_Civilian1();
};

class STRAGGLER_Civilian1 : public Civilian1
{
	public :

 		STRAGGLER_Civilian1();
};

class UCA_BAD_Civilian1 : public Civilian1
{
	public :

 		UCA_BAD_Civilian1();
};

#endif // __CIVILIAN1_H__
