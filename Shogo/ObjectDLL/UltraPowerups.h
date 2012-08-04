// ----------------------------------------------------------------------- //
//
// MODULE  : UltraPowerups.h
//
// PURPOSE : UltraPowerups definitions
//
// CREATED : 2/18/98
//
// ----------------------------------------------------------------------- //

#ifndef __ULTRAPOWERUPS_H__
#define __ULTRAPOWERUPS_H__

#include "UltraPowerupItem.h"

class UltraDamage : public UltraPowerupItem
{
	public:

		UltraDamage();
};

class UltraPowerSurge : public UltraPowerupItem
{
	public:

		UltraPowerSurge();

	protected:

		virtual void ObjectTouch (HOBJECT hObject);
};

class UltraHealth : public UltraPowerupItem
{
	public:

		UltraHealth();
	
	protected:

		virtual void ObjectTouch (HOBJECT hObject);
};

class UltraShield : public UltraPowerupItem
{
	public:

		UltraShield();
};

class UltraStealth : public UltraPowerupItem
{
	public:

		UltraStealth();

	protected:

		virtual void ObjectTouch (HOBJECT hObject);
};

class UltraReflect : public UltraPowerupItem
{
	public:

		UltraReflect();
};

class UltraNightVision : public UltraPowerupItem
{
	public:

		UltraNightVision();

	protected:

		virtual void ObjectTouch (HOBJECT hObject);
};

class UltraInfrared : public UltraPowerupItem
{
	public:

		UltraInfrared();

	protected:

		virtual void ObjectTouch (HOBJECT hObject);
};

class UltraSilencer : public UltraPowerupItem
{
	public:

		UltraSilencer();

	protected:

		virtual void ObjectTouch (HOBJECT hObject);
};

class UltraRestore : public UltraPowerupItem
{
	public:

		UltraRestore();

	protected:

		virtual void ObjectTouch (HOBJECT hObject);
};

#endif //  __ULTRAPOWERUPS_H__