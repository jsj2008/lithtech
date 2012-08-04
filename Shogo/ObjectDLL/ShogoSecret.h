// ----------------------------------------------------------------------- //
//
// MODULE  : ShogoSecret.h
//
// PURPOSE : ShogoSecret definition
//
// CREATED : 8/6/98
//
// ----------------------------------------------------------------------- //

#ifndef __SHOGO_SECRET_H__
#define __SHOGO_SECRET_H__

#include "cpp_engineobjects_de.h"
#include "PickupItem.h"


class ShogoSecret : public PickupItem
{
	public :

		ShogoSecret();

	protected :

		virtual void ObjectTouch(HOBJECT hObject);
};

class Shogo_S : public ShogoSecret
{
	public :

		Shogo_S();
};

class Shogo_H : public ShogoSecret
{
	public :

		Shogo_H();
};

class Shogo_O : public ShogoSecret
{
	public :

		Shogo_O();
};

class Shogo_G : public ShogoSecret
{
	public :

		Shogo_G();
};

#endif // __SHOGO_SECRET_H__
