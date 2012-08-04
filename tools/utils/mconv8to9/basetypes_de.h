
#ifndef __BASETYPES_DE_H__
#define __BASETYPES_DE_H__


	#include <math.h>

	#include "de_codes.h"
	#include "dstream.h"


	#define INLINE_FN __inline
	
	#ifndef _LITHTECH_
		#define _LITHTECH_
	#endif

	// Engine version identifier.
	#ifndef _LITHTECH2
		#define _LITHTECH2
	#endif


	// Base types.
	#define DNULL 0
	#define DBOOL char
	#define DTRUE 1
	#define DFALSE 0
	#define DDWORD unsigned long
	#define D_WORD unsigned short
	#define DBYTE unsigned char
	typedef float DFLOAT;


	// Reference to a surface.
	typedef unsigned long HPOLY;
	#define INVALID_HPOLY 0xFFFFFFFF


	// Globally unique identifier.
	typedef struct DGUID_t
	{
		unsigned long	a;
		unsigned short	b;
		unsigned short	c;
		unsigned char   d[8];

	} DGUID;


	// Useful for objects who need to call Term and return a value
	// (like for loading functions).
	#define TERM_RET(__ret) \
		{ Term(); return __ret; }


	// Maximum number of sky objects.
	#define MAX_SKYOBJECTS	30


	// Parsing stuff.
	#define PARSE_MAXTOKENS		64
	#define PARSE_MAXTOKENSIZE	80

	// Helpful math definitions.
	#define MATH_PI				3.141592653589793f
	#define MATH_HALFPI			1.570796326795f
	#define MATH_CIRCLE			6.283185307178f
	#define MATH_ONE_OVER_PI	0.3183098861839f
	#define MATH_EPSILON		0.00000000001f
	#define MATH_DEGREES_TO_RADIANS(x) ((x) *  0.01745329251994f)
	#define MATH_RADIANS_TO_DEGREES(x) ((x) *  57.2957795130967f)

	#define MATH_ONE_OVER_255	0.003921568627451f


	#define DDIFF(a,b) (((a) < (b)) ? ((b) - (a)) : ((a) - (b)))
	#define DMIN(a,b) ((a) < (b) ? (a) : (b))
	#define DMAX(a,b) ((a) > (b) ? (a) : (b))
	#define DABS(a) ((a) > 0 ? (a) : -(a))
	#define DCLAMP(a, min, max) ((a) < (min) ? (min) : ((a) > (max) ? (max) : (a)))
	#define DLERP(min, max, t) ((min) + ((max) - (min)) * (t))



	#include "dvector.h"

	
	#define ROT_COPY(dest, src) \
		{\
		VEC_COPY((dest).m_Vec, (src).m_Vec);\
		(dest).m_Spin = (src).m_Spin;\
		}

	#define PLANE_COPY(dest, src) \
		{\
		VEC_COPY((dest).m_Normal, (src).m_Normal)\
		(dest).m_Dist = (src).m_Dist;\
		}

	#define PLANE_SET(plane, _x, _y, _z, _dist) \
	{\
		(plane).m_Normal.x = (_x);\
		(plane).m_Normal.y = (_y);\
		(plane).m_Normal.z = (_z);\
		(plane).m_Dist = (_dist);\
	}

	#define MAT_COPY(dest, src) \
		{\
		(dest).m[0][0] = (src).m[0][0];\
		(dest).m[0][1] = (src).m[0][1];\
		(dest).m[0][2] = (src).m[0][2];\
		(dest).m[0][3] = (src).m[0][3];\
													 \
		(dest).m[1][0] = (src).m[1][0];\
		(dest).m[1][1] = (src).m[1][1];\
		(dest).m[1][2] = (src).m[1][2];\
		(dest).m[1][3] = (src).m[1][3];\
													 \
		(dest).m[2][0] = (src).m[2][0];\
		(dest).m[2][1] = (src).m[2][1];\
		(dest).m[2][2] = (src).m[2][2];\
		(dest).m[2][3] = (src).m[2][3];\
													 \
		(dest).m[3][0] = (src).m[3][0];\
		(dest).m[3][1] = (src).m[3][1];\
		(dest).m[3][2] = (src).m[3][2];\
		(dest).m[3][3] = (src).m[3][3];\
		}

	#define MAT_SET(dest, \
		e00, e01, e02, e03,\
		e10, e11, e12, e13,\
		e20, e21, e22, e23,\
		e30, e31, e32, e33)\
		{\
		(dest).m[0][0] = e00; (dest).m[0][1] = e01; (dest).m[0][2] = e02; (dest).m[0][3] = e03;\
		(dest).m[1][0] = e10; (dest).m[1][1] = e11; (dest).m[1][2] = e12; (dest).m[1][3] = e13;\
		(dest).m[2][0] = e20; (dest).m[2][1] = e21; (dest).m[2][2] = e22; (dest).m[2][3] = e23;\
		(dest).m[3][0] = e30; (dest).m[3][1] = e31; (dest).m[3][2] = e32; (dest).m[3][3] = e33;\
		}


	// Get the distance from a point to a plane.
	#define DIST_TO_PLANE(vec, plane) ( VEC_DOT((plane).m_Normal, (vec)) - (plane).m_Dist )

	// Initializes the rotation to look down the positive Z axis.
	#define ROT_INIT(r) \
		{\
		(r).m_Vec.x = (r).m_Vec.y = (r).m_Vec.z = 0.0f;\
		(r).m_Spin = 1.0f;\
		}


	// Used for resizing arrays.. mostly for internal DirectEngine stuff.
	#define GENERIC_REMOVE(theArray, iIndex, nElements) \
		ASSERT(nElements > 0);\
		memmove(&theArray[iIndex], &theArray[iIndex+1], sizeof(theArray[0]) * (nElements-iIndex-1));\
		nElements--;

	#define GENERIC_INSERT(type, theArray, nElements, iIndex, value) \
		{\
			type *pNewArray = (type*)malloc(sizeof(type) * (nElements+1));\
			memcpy(pNewArray, theArray, nElements * sizeof(type));\
			memmove(&pNewArray[iIndex+1], &pNewArray[iIndex], sizeof(type)*(nElements-iIndex));\
			memcpy(&pNewArray[iIndex], &value, sizeof(type));\
			free(theArray);\
			theArray = pNewArray;\
			++nElements;\
		}
				
	#define GENERIC_INSERT_CPP(type, theArray, nElements, iIndex, value) \
		{\
			type *pNewArray = new type[nElements+1];\
			memcpy(pNewArray, theArray, nElements * sizeof(type));\
			memmove(&pNewArray[iIndex+1], &pNewArray[iIndex], sizeof(type)*(nElements-iIndex));\
			memcpy(&pNewArray[iIndex], &value, sizeof(type));\
			delete [] theArray;\
			theArray = pNewArray;\
			++nElements;\
		}
				
	#define GENERIC_APPEND(type, theArray, nElements, value) \
		GENERIC_INSERT(type, theArray, nElements, nElements, value);
				
	#define DTOCVEC(_vec) (*((CVector*)&(_vec)))
	#define CTODVEC(_vec) (*((DVector*)&(_vec)))



	typedef struct
	{
		float x, y, z; // RGB 0-255
		float a; // Alpha 0-255
	} PGColor;

	typedef struct DRotation_t
	{
		DRotation_t() {}

		DRotation_t(float x, float y, float z, float spin)
		{
			m_Vec.x = x;
			m_Vec.y = y;
			m_Vec.z = z;
			m_Spin = spin;
		}

		// <vector>, spin
		DVector	m_Vec;
		float	m_Spin;
	} DRotation;

	typedef struct DPlane_t
	{
		DPlane_t() {}
		DPlane_t(float x, float y, float z, float dist)
		{
			m_Normal.x = x;
			m_Normal.y = y;
			m_Normal.z = z;
			m_Dist = dist;
		}

		inline void	Init(DVector vec, float dist)
		{
			m_Normal = vec;
			m_Dist = dist;
		}

		inline float	DistTo(DVector vec)
		{
			return m_Normal.Dot(vec) - m_Dist;
		}

		DVector	m_Normal;
		float	m_Dist;
	} DPlane;

	typedef struct DFloatPt_t
	{
		float	x, y;
	} DFloatPt;

	class DIntPt
	{
	public:
		int	x, y;
	};

	typedef struct DWarpPt_t
	{
		float source_x, source_y;
		float dest_x, dest_y;
	} DWarpPt;

	typedef struct DRect_t
	{
		void	Init(int inLeft, int inTop, int inRight, int inBottom)
		{
			left = inLeft;
			top = inTop;
			right = inRight;
			bottom = inBottom;
		}

		int left, top, right, bottom;
	} DRect;

	typedef struct ArgList_t
	{
		char **argv;
		int argc;
	} ArgList;

	// Surface data.
	typedef struct SurfaceData_t
	{
		DVector		O, P, Q;				// Texture vectors.
		void		*m_pInternalWorld;		// Don't touch!  (MainWorld* on init).
		void		*m_pInternalWorldBsp;	// Don't touch!  (WorldBsp* on init).
		void		*m_pInternalSurface;	// Don't touch!  (Surface*)
	} SurfaceData;

	// Sky definition.
	typedef struct SkyDef_t
	{
		DVector	m_Min, m_Max;			// Box corners (in world coordinates).
		DVector	m_ViewMin, m_ViewMax;	// View min and max.  The viewer's position
										// in the main world is squished into this
										// box so you can get a tiny amount of parallax.
	} SkyDef;


	#include "matrix_ops.h"
	#include "dlink.h"



	typedef void (*ConsoleProgramFn)(int argc, char **argv);


	// This is used to distinguish between flag types.
	typedef enum
	{
		OFT_Flags = 0,	// FLAG_  #defines
		OFT_Flags2		// FLAG2_ #defines
	} ObjFlagType;


	// Object flags.
	#define FLAG_VISIBLE			(1<<0)

	#define FLAG_SHADOW				(1<<1)	// Does this model cast shadows?
	#define FLAG_UNSIGNED			(1<<1)	// Tells the polygrid to use unsigned bytes for its data.

	#define FLAG_MODELTINT			(1<<2)	// If this is set, it draws a model in 2 passes.  In the
											// second pass, it scales down the color with ColorR,
											// ColorG, and ColorB.  This is used to tint the skins
											// in multiplayer.  Note: it uses powers of 2 to determine
											// scale so the color scale maps like this:
											// > 253 = 1.0
											// > 126 = 0.5
											// > 62  = 0.25
											// > 29  = 0.12
											// otherwise 0
	#define FLAG_NOGLOBALLIGHTSCALE	(1<<2)	// Particle systems: disables use of global light scale.
	
	#define FLAG_CASTSHADOWS		(1<<3)	// Should this light cast shadows (slower)?
	#define FLAG_ROTATABLESPRITE	(1<<3)	// Sprites only.
	#define FLAG_ROTATEABLESPRITE	FLAG_ROTATABLESPRITE
	#define FLAG_DETAILTEXTURE		(1<<3)	// Models: draw with a detail texture.
	#define FLAG_UPDATEUNSEEN		(1<<3)	// Particle systems only.
											// If this is set, the engine will update
											// particles even when they're invisible.
											// You should check FLAG_WASDRAWN
											// on any particle systems you're iterating
											// over so you don't update invisible ones.
	
	#define FLAG_SOLIDLIGHT			(1<<4)	// Use the 'fastlight' method for this light.
	#define FLAG_MODELWIREFRAME		(1<<4)
	#define FLAG_WASDRAWN			(1<<4)	// The engine sets this if a particle system
											// or PolyGrid was drawn.  You can use this to 
											// determine whether or not to do some expensive
											// calculations on it.

	#define FLAG_GLOWSPRITE				(1<<5)	// Shrinks the sprite as the viewer gets nearer.
	#define FLAG_ONLYLIGHTWORLD			(1<<5)	// Lights only - tells it to only light the world.
	#define FLAG_ENVIRONMENTMAP			(1<<5)	// Environment map the model.
	#define FLAG_ENVIRONMENTMAPONLY		(1<<5)	// For PolyGrids - says to only use the environment map (ignore main texture).
	
	#define FLAG_SPRITEBIAS				(1<<6)	// Biases the Z towards the view so a sprite doesn't clip as much.
	#define FLAG_DONTLIGHTBACKFACING	(1<<6)	// Lights only - don't light backfacing polies.
	#define FLAG_REALLYCLOSE			(1<<6)	// Used for models really close to the view (like PV weapons).
												// Must use tricks so it doesn't clip off.

	#define FLAG_FOGDISABLE			(1<<7)	// Disables fog on WorldModels, Sprites, Particle Systems and Canvases only.
	#define FLAG_ANIMTRANSITION		(1<<7)	// Does a 200ms transition between model animations.
	#define FLAG_ONLYLIGHTOBJECTS	(1<<7)	// Lights only - tells it to only light objects (and not the world).

	#define FLAG_FULLPOSITIONRES	(1<<8)	// LT normally compresses the position and rotation info
											// to reduce packet size.  Some things must be exact 
											// (like some WorldModels) so this will 
											// enlarge the packets for better accuracy.

	#define FLAG_NOLIGHT			(1<<9)	// Just use the object's color and global light scale.
											// (Don't affect by area or by dynamic lights).

	#define FLAG_HARDWAREONLY		(1<<10)	// Don't draw this object if we're using software rendering.
	#define FLAG_PORTALVISIBLE		(1<<10)	// Draw in portals even if the object is invisible.

	#define FLAG_YROTATION			(1<<11)	// Uses minimal network traffic to represent rotation
											// (1 byte instead of 3, but only rotates around the Y axis).

	#define FLAG_SKYOBJECT			(1<<12)	// Don't render this object thru the normal stuff,
											// only render it when processing sky objects.

	#define FLAG_SOLID				(1<<13)	// Object can't go thru other solid objects.

	#define FLAG_BOXPHYSICS			(1<<14)	// Use simple box physics on this object (used for WorldModels and containers).
	#define FLAG_SPRITE_NOZ			(1<<14)	// Disable Z read/write on sprite (good for lens flares).

	#define FLAG_CLIENTNONSOLID		(1<<15)	// This object is solid on the server and nonsolid on the client.	

	// Which flags the client knows about.
	#define CLIENT_FLAGMASK			(FLAG_VISIBLE|FLAG_SHADOW|FLAG_MODELTINT|\
									FLAG_ROTATABLESPRITE|FLAG_SOLIDLIGHT|\
									FLAG_REALLYCLOSE|FLAG_SPRITE_NOZ|\
									FLAG_FULLPOSITIONRES|FLAG_NOLIGHT|FLAG_ENVIRONMENTMAP|\
									FLAG_HARDWAREONLY|FLAG_YROTATION|FLAG_SKYOBJECT|\
									FLAG_SOLID|FLAG_BOXPHYSICS|FLAG_CLIENTNONSOLID)

	// Server only flags.
	#define FLAG_TOUCH_NOTIFY		(1<<16)	// Gets touch notification.
	#define FLAG_GRAVITY			(1<<17)	// Gravity is applied.
	#define FLAG_STAIRSTEP			(1<<18)	// Steps up stairs.
	#define FLAG_MODELKEYS			(1<<19)	// The object won't get get MID_MODELSTRINGKEY messages unless
											// it sets this flag.
	#define FLAG_KEEPALIVE			(1<<20)	// Save and restore this object when switching worlds.
	#define FLAG_GOTHRUWORLD		(1<<21) // Object can pass through world
	#define FLAG_RAYHIT				(1<<22) // Object is hit by raycasts.
	#define FLAG_DONTFOLLOWSTANDING	(1<<23) // Dont follow the object this object is standing on.
	#define FLAG_FORCECLIENTUPDATE	(1<<24)	// Force client updates even if the object is OT_NORMAL or invisible.
											// Use this whenever possible.. it saves cycles.
	#define FLAG_NOSLIDING			(1<<25)	// Object won't slide agaist polygons

	#define FLAG_POINTCOLLIDE		(1<<26)	// Uses much (10x) faster physics for collision detection, but the
											// object is a point (dims 0,0,0).  Standing info is not set when
											// this flag is set.

	#define FLAG_REMOVEIFOUTSIDE	(1<<27)	// Remove this object automatically if it gets outside the world.

	#define FLAG_FORCEOPTIMIZEOBJECT	(1<<28)	// Force the engine to optimize this object
												// as if the FLAG_OPTIMIZEOBJECT flags were
												// cleared.  This can be used if you have a visible
												// object that's an attachment but it doesn't need
												// touch notifies or raycast hits (like a gun-in-hand).
	
	#define	FLAG_CONTAINER				(1<<29) // Will get an MID_AFFECTPHYSICS message for each object
												// that is inside its volume

	// Internal flags.  Descriptions are there to help the DE developers remember what
	// they're there for, NOT for general use!
	#define FLAG_INTERNAL1			(1<<30)	// (Did the renderer see the object).
	#define FLAG_INTERNAL2			(1<<31)	// (Used by ClientDE::FindObjectsInSphere).
	#define FLAG_LASTFLAG			FLAG_INTERNAL2 


	// If you clear these flags (flags &= ~FLAG_OPTIMIZEMASK) and the object doesn't have 
	// a special effect message, the engine never even iterates over the object for movement,
	// raycasting, visibility, etc.  Use this whenever you can.
	#define FLAG_OPTIMIZEMASK (FLAG_VISIBLE|FLAG_SOLID|FLAG_TOUCH_NOTIFY|\
		FLAG_RAYHIT|FLAG_FORCECLIENTUPDATE|FLAG_GOTHRUWORLD|FLAG_CONTAINER)



	// FLAG2_ defines.
	#define FLAG2_PORTALINVISIBLE	(1<<0)		// This object is invisible in portals.
	
	#define FLAG2_CHROMAKEY			(1<<1)		// For WorldModels and models - use chromakeying to draw.



	// Different object types.  Some can only be created on the client.
	#define OT_NORMAL			0	// Invisible object.  Note, client's aren't told about
									// these when they're created on the server! 
	#define OT_MODEL			1	// Model object.
	#define OT_WORLDMODEL		2	// WorldModel.
	#define OT_SPRITE			3	// Sprite.
	#define OT_LIGHT			4	// Dynamic light.
	
	#define OT_CAMERA			5	// Camera.
	#define OT_PARTICLESYSTEM	6	// Particle system.
	#define OT_POLYGRID			7	// Poly grid.
	#define OT_LINESYSTEM		8	// Line system.
	#define OT_CONTAINER		9	// Container.
	#define OT_CANVAS			10	// Canvas (game code renders it).
	#define NUM_OBJECTTYPES		11	// NOTE: the high bit of the object type is reserved
									// for the engine's networking.

	// Size defines used for Parse functions
	// Use these to size your argument buffer and argument pointer
	// buffer accordingly.
	#define PARSE_MAXARGS		30
	#define PARSE_MAXARGLEN		256


#endif 

