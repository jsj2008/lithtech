//----------------------------------------------------------
//
// MODULE  : CPP_ENGINEOBJECTS_DE.CPP
//
// PURPOSE : C++ DE engine objects
//
// CREATED : 9/17/97
//
//----------------------------------------------------------

#include "LMessage.h"
#include "cpp_engineobjects_de.h"

#define NO_C_BASECLASS
#include "engineobjects.c"


BEGIN_CLASS(BaseClass)
	ADD_STRINGPROP(Name, "noname")
	ADD_VECTORPROP(Pos)
	ADD_ROTATIONPROP(Rotation)
END_CLASS_DEFAULT_NOPARENT(BaseClass, BaseClass::_EngineMsgFn, BaseClass::_ObjectMsgFn)


BEGIN_CLASS(StartPoint)
END_CLASS_DEFAULT(StartPoint, BaseClass, NULL, NULL)


	
