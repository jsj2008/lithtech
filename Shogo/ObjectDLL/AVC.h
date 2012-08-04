// ----------------------------------------------------------------------- //
//
// MODULE  : AVC.h
//
// PURPOSE : AVC Mecha - Definition
//
// CREATED : 5/18/98
//
// ----------------------------------------------------------------------- //

#ifndef __AVC_H__
#define __AVC_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class AVC : public BaseAI
{
	public :

 		AVC();
};

class CMC_AVC : public AVC
{
	public :

 		CMC_AVC();
};

class CRONIAN_AVC : public AVC
{
	public :

 		CRONIAN_AVC();
};

class FALLEN_AVC : public AVC
{
	public :

 		FALLEN_AVC();
};

#endif // __AVC_H__
