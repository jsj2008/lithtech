//----------------------------------------------------------
//
//	MODULE	: Clippers.h
//
//	PUROSE	: Clipping functions definition file
//
//	CREATED	: 10 / 27 / 1996
//
//----------------------------------------------------------

#ifndef __CLIPPERS_H_
	#define __CLIPPERS_H_

	// Includes....

	#include "Vector.h"

	BOOL ClipFront(CFXVector &v1, CFXVector &v2, float v);
	BOOL ClipBack(CFXVector &v1, CFXVector &v2, float v);

	BOOL ClipLeft(CFXVector &v1, CFXVector &v2, float v);
	BOOL ClipRight(CFXVector &v1, CFXVector &v2, float v);
	BOOL ClipTop(CFXVector &v1, CFXVector &v2, float v);
	BOOL ClipBottom(CFXVector &v1, CFXVector &v2, float v);

	BOOL ClipLeft3D(CFXVector &v1, CFXVector &v2, float v);
	BOOL ClipRight3D(CFXVector &v1, CFXVector &v2, float v);
	BOOL ClipTop3D(CFXVector &v1, CFXVector &v2, float v);
	BOOL ClipBottom3D(CFXVector &v1, CFXVector &v2, float v);

	//----------------------------------------------------------
	//
	// INLINE FUNCTIONS
	//
	//----------------------------------------------------------

	//----------------------------------------------------------
	//
	// FUNCTION : ClipLeft()
	//
	// PURPOSE	: Clips against left side
	//
	//----------------------------------------------------------

	inline BOOL ClipLeft(CFXVector &v1, CFXVector &v2, float v)
	{
		// Sanity checks....

		if ((v1.x < v) && (v2.x < v)) return FALSE;
		
		if ((v1.x >= v) && (v2.x >= v)) return TRUE;

		// Clip

		if (v1.x < v)
		{
			v1.y = v1.y + ((v - v1.x) * (v2.y - v1.y) / (v2.x - v1.x));
			v1.x = v;
		}
		else
		{
			v2.y = v1.y + ((v - v1.x) * (v2.y - v1.y) / (v2.x - v1.x));
			v2.x = v;
		}		
		
		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : ClipRight()
	//
	// PURPOSE	: Clips against right side
	//
	//----------------------------------------------------------

	inline BOOL ClipRight(CFXVector &v1, CFXVector &v2, float v)
	{
		// Sanity checks....

		if ((v1.x > v) && (v2.x > v)) return FALSE;
		
		if ((v1.x <= v) && (v2.x <= v)) return TRUE;

		// Clip

		if (v1.x > v)
		{
			v1.y = v1.y + ((v - v1.x) * (v2.y - v1.y) / (v2.x - v1.x));
			v1.x = v;
		}
		else
		{
			v2.y = v1.y + ((v - v1.x) * (v2.y - v1.y) / (v2.x - v1.x));
			v2.x = v;
		}

		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : ClipTop()
	//
	// PURPOSE	: Clips against top side
	//
	//----------------------------------------------------------

	inline BOOL ClipTop(CFXVector &v1, CFXVector &v2, float v)
	{
		// Sanity checks....

		if ((v1.y < v) && (v2.y < v)) return FALSE;
		
		if ((v1.y >= v) && (v2.y >= v)) return TRUE;

		// Clip

		if (v1.y < v)
		{
			v1.x = v1.x + ((v - v1.y) * (v2.x - v1.x) / (v2.y - v1.y));
			v1.y = v;
		}
		else
		{
			v2.x = v1.x + ((v - v1.y) * (v2.x - v1.x) / (v2.y - v1.y));
			v2.y = v;
		}

		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : ClipBottom()
	//
	// PURPOSE	: Clips against bottom side
	//
	//----------------------------------------------------------

	inline BOOL ClipBottom(CFXVector &v1, CFXVector &v2, float v)
	{
		// Sanity checks....

		if ((v1.y > v) && (v2.y > v)) return FALSE;
		
		if ((v1.y <= v) && (v2.y <= v)) return TRUE;

		// Clip

		if (v1.y > v)
		{
			v1.x = v1.x + ((v - v1.y) * (v2.x - v1.x) / (v2.y - v1.y));
			v1.y = v;
		}
		else
		{
			v2.x = v1.x + ((v - v1.y) * (v2.x - v1.x) / (v2.y - v1.y));
			v2.y = v;
		}

		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : ClipFront()
	//
	// PURPOSE	: Clips against front side
	//
	//----------------------------------------------------------

	inline BOOL ClipFront(CFXVector &v1, CFXVector &v2, float v)
	{
		float t;
		
		// Sanity checks....

		if ((v1.z < v) && (v2.z < v)) return FALSE;
		
		if ((v1.z >= v) && (v2.z >= v)) return TRUE;

		// Clip

		if (v1.z < v)
		{
			t = (v - v1.z) / (v2.z - v1.z);
			v1.x = v1.x + (t * (v2.x - v1.x));
			v1.y = v1.y + (t * (v2.y - v1.y));;
			v1.z = v;
		}
		else
		{
			t = (v - v1.z) / (v2.z - v1.z);
			v2.x = v1.x + (t * (v2.x - v1.x));
			v2.y = v1.y + (t * (v2.y - v1.y));;
			v2.z = v;
		}

		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : ClipBack()
	//
	// PURPOSE	: Clips against back side
	//
	//----------------------------------------------------------

	inline BOOL ClipBack(CFXVector &v1, CFXVector &v2, float v)
	{
		float t;
		
		// Sanity checks....

		if ((v1.z > v) && (v2.z > v)) return FALSE;
		
		if ((v1.z <= v) && (v2.z <= v)) return TRUE;

		// Clip

		if (v1.z > v)
		{
			t = (v - v1.z) / (v2.z - v1.z);
			v1.x = v1.x + (t * (v2.x - v1.x));
			v1.y = v1.y + (t * (v2.y - v1.y));;
			v1.z = v;
		}
		else
		{
			t = (v - v1.z) / (v2.z - v1.z);
			v2.x = v1.x + (t * (v2.x - v1.x));
			v2.y = v1.y + (t * (v2.y - v1.y));;
			v2.z = v;
		}

		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : ClipLeft3D()
	//
	// PURPOSE	: Clips against left side
	//
	//----------------------------------------------------------

	inline BOOL ClipLeft3D(CFXVector &v1, CFXVector &v2, float v)
	{
		// Sanity checks....

		if ((v1.x < v) && (v2.x < v)) return FALSE;
		
		if ((v1.x >= v) && (v2.x >= v)) return TRUE;

		// Clip

		if (v1.x < v)
		{
			v1.y = v1.y + ((v - v1.x) * (v2.y - v1.y) / (v2.x - v1.x));
			v1.z = v1.z + ((v - v1.x) * (v2.z - v1.z) / (v2.x - v1.x));
			v1.x = v;
		}
		else
		{
			v2.y = v1.y + ((v - v1.x) * (v2.y - v1.y) / (v2.x - v1.x));
			v2.z = v1.z + ((v - v1.x) * (v2.z - v1.z) / (v2.x - v1.x));
			v2.x = v;
		}		
		
		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : ClipRight3D()
	//
	// PURPOSE	: Clips against right side
	//
	//----------------------------------------------------------

	inline BOOL ClipRight3D(CFXVector &v1, CFXVector &v2, float v)
	{
		// Sanity checks....

		if ((v1.x > v) && (v2.x > v)) return FALSE;
		
		if ((v1.x <= v) && (v2.x <= v)) return TRUE;

		// Clip

		if (v1.x > v)
		{
			v1.y = v1.y + ((v - v1.x) * (v2.y - v1.y) / (v2.x - v1.x));
			v1.z = v1.z + ((v - v1.x) * (v2.z - v1.z) / (v2.x - v1.x));
			v1.x = v;
		}
		else
		{
			v2.y = v1.y + ((v - v1.x) * (v2.y - v1.y) / (v2.x - v1.x));
			v2.z = v1.z + ((v - v1.x) * (v2.z - v1.z) / (v2.x - v1.x));
			v2.x = v;
		}

		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : ClipTop3D()
	//
	// PURPOSE	: Clips against top side
	//
	//----------------------------------------------------------

	inline BOOL ClipTop3D(CFXVector &v1, CFXVector &v2, float v)
	{
		// Sanity checks....

		if ((v1.y < v) && (v2.y < v)) return FALSE;
		
		if ((v1.y >= v) && (v2.y >= v)) return TRUE;

		// Clip

		if (v1.y < v)
		{
			v1.x = v1.x + ((v - v1.y) * (v2.x - v1.x) / (v2.y - v1.y));
			v1.z = v1.z + ((v - v1.y) * (v2.z - v1.z) / (v2.y - v1.y));
			v1.y = v;
		}
		else
		{
			v2.x = v1.x + ((v - v1.y) * (v2.x - v1.x) / (v2.y - v1.y));
			v2.z = v1.z + ((v - v1.y) * (v2.z - v1.z) / (v2.y - v1.y));
			v2.y = v;
		}

		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : ClipBottom3D()
	//
	// PURPOSE	: Clips against bottom side
	//
	//----------------------------------------------------------

	inline BOOL ClipBottom3D(CFXVector &v1, CFXVector &v2, float v)
	{
		// Sanity checks....

		if ((v1.y > v) && (v2.y > v)) return FALSE;
		
		if ((v1.y <= v) && (v2.y <= v)) return TRUE;

		// Clip

		if (v1.y > v)
		{
			v1.x = v1.x + ((v - v1.y) * (v2.x - v1.x) / (v2.y - v1.y));
			v1.z = v1.z + ((v - v1.y) * (v2.z - v1.z) / (v2.y - v1.y));
			v1.y = v;
		}
		else
		{
			v2.x = v1.x + ((v - v1.y) * (v2.x - v1.x) / (v2.y - v1.y));
			v2.z = v1.z + ((v - v1.y) * (v2.z - v1.z) / (v2.y - v1.y));
			v2.y = v;
		}

		return TRUE;
	}

#endif