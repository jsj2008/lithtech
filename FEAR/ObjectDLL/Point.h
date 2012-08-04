// ----------------------------------------------------------------------- //
//
// MODULE  : Point.h
//
// PURPOSE : Point definition... a very simple class which just emulates a point in the world
//
// CREATED : 5/2/01
//
// ----------------------------------------------------------------------- //

#ifndef __POINT_H__
#define __POINT_H__

LINKTO_MODULE( Point );

class Point : public BaseClass
{
	public:

		Point() : BaseClass( OT_NORMAL )
		{
		}

	// Currently we dont need anything from this class
	// add methods as desired.
};


#endif // __POINT_H__
