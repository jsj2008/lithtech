/*!

 MODULE  : ltengineobjects.h.

 PURPOSE : C++ LithTech engine object class(es) definition(s)

 CREATED : 9/17/97

*/

#ifndef __LTENGINEOBJECTS_H__
#define __LTENGINEOBJECTS_H__

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __LTSERVEROBJ_H__
#include "ltserverobj.h"
#endif

#ifndef __ILTBASECLASS_H__
#include "iltbaseclass.h"
#endif

class ILTServer;
extern ILTServer *g_pLTServer;


/*!
Defines basic object functionality for all objects in a game.

All server-side objects in your game must derive from \b BaseClass.

Each object must have an object type (defined by the \b OT_* defines).
Use the \b SetType function to set an object's type.

This class provides basic engine-to-object and object-to-object event
notification functionality (using the \b ILTBaseClass notification interface,
\b EngineMessageFn (obsolete) and \b ObjectMessageFn).

Generic behavior across different objects can be implemented by using aggregates.
An object will forward event notification messages to all its associated
aggregates. For example, a damage aggregate can be used to control damage
logic for all objects that can be damaged. Use the \b AddAggregate and \b RemoveAggregate functions to implement
aggregate functionality.

You can implement any custom behavior required for your derived object.

See the Object chapter of the LithTech Programming Guide for more information.
*/

class BaseClass : public ILTBaseClass {
public:

    BaseClass(uint8 nType = OT_NORMAL)
       : ILTBaseClass(nType) {}

    virtual ~BaseClass();

/*!
\param messageID One of the \b MID_* define values.
\param pData Optional. A void pointer to data appropriate to the message type.
\param lData Optional. A float value appropriate to the message type.
\return Returns 1.

Receives event notifications from the engine.  Note that the engine actually calls
one of the \b "On*" notification functions, where the default implementation is to call
\b EngineMessageFn() with the appropriate \b MID_*.

The meaning of \b pData and \b lData vary by message type. See the Object chapter of the
LithTech Programming Guide for more information.

Before returning, this function should call the \b EngineMessageFn() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

\b EngineMessageFn will be deprecated in a future release in favor of the virtual function
interface inherited from \b ILTBaseClass.

Used For: Obsolete.
*/
    virtual uint32 EngineMessageFn(uint32 messageID, void *pData, float lData);

/*!
\return A pointer to the engine's \b ILTServer instance.

Returns a pointer to the engine's \b ILTServer instance.

Used For: Object.
*/
    static ILTServer *GetServerDE() { return (ILTServer*)g_pLTServer; }

}; // Baseclass

/*!
The WorldSection class.
*/
class WorldSection : public BaseClass {
public:

};

/*!
MainWorld.
*/
class MainWorld : public BaseClass {
public:
    virtual uint32  EngineMessageFn(uint32 messageID, void *pData, float lData);

};

/*!
Container.
*/
class Container : public BaseClass {
public:
    virtual uint32  EngineMessageFn(uint32 messageID, void *pData, float lData);

    uint32  m_ContainerCode;
};

/*!
Sound.
*/
class Sound : public BaseClass {
public:
    virtual uint32  EngineMessageFn(uint32 messageID, void *pData, float lData);

    char    m_Filename[101];
    float   m_fOuterRadius;
    float   m_fInnerRadius;
    uint8   m_nVolume;
    bool  m_bAmbient;
    unsigned char   m_nPriority;
};

/*!
InsideDef.
These are used by the preprocessor to define 'inside' areas.
*/
class InsideDef : public BaseClass {
public:

};

/*!
OutsideDef class.
These are used if you want the preprocessor to make a leak file for a level.
*/
class OutsideDef : public BaseClass {
public:

};

/*!
FastApproxArea class.
This defines an area where only fast approximation vising will occur.  The
area is bounded by hullmakers and portals.
*/
class FastApproxArea : public BaseClass {
public:

};

/*!
Light class.
*/
class Light : public BaseClass {
public:

};

/*!
Engine_LightGroup class.
Defines the grouping for a set of world lights which can be turned on and off.
Note that this is engine-side and is intended to be inherited from on the game
side for controlling purposes.
*/
class Engine_LightGroup : public BaseClass {
public:

};

/*!
ObjectLight class (these lights only light objects, they don't
light the world).  These are used for landscape areas lit by
\b GlobalDirLight objects (which don't light objects).  
*/
class ObjectLight : public BaseClass {
public:

};

/*!
DirLight class.
*/
class DirLight : public BaseClass {
public:

};

/*!
StaticSunLight class.
*/
class StaticSunLight : public BaseClass {
public:
    virtual uint32  EngineMessageFn(uint32 messageID, void *pData, float lData);

    LTVector m_InnerColor;

    float m_BrightScale;
	float m_ObjectBrightScale;
	float m_fConvertToAmbient;
};

/*!
 Decal class.
*/
class Decal : public BaseClass {
 public:
};

/*!
 RenderGroup class.
*/
class RenderGroup : public BaseClass {
 public:
};

/*!
 EdgeGenerator class.
*/
class EdgeGenerator : public BaseClass {
 public:
};

/*!
 Ambient Override class.
*/
class AmbientOverride : public BaseClass {
 public:
};


/*!
Brush class.
*/
class Brush : public BaseClass {
public:

};

/*!
DemoSkyWorldModel class.

This is a \b WorldModel that adds itself to the sky object list and
has properties that level designers can edit to set the sky
dimensions.  If the sky box is set to zero, then it won't set it.
(This is so you can have multiple \b DemoSkyWorldModels in the world,
and only one sets the sky box).  The \b DemoSkyWorldModel uses \b
InnerPercentX, Y, and Z as a percentage of the \b SkyDims box for the
inner box.  Each sky world model must have a (unique) index from 0-30.
Ones with lower indices get drawn first.  
*/
class DemoSkyWorldModel : public BaseClass {
public:
    virtual uint32  EngineMessageFn(uint32 messageID, void *pData, float lData);

    LTVector m_SkyDims;
 
    float m_InnerPercentX, m_InnerPercentY, m_InnerPercentZ;
    int32 m_Index;
};


/*!
This works exactly the same as a \b DemoSkyWorldModel, except you
identify the object by name.
*/
class SkyPointer : public BaseClass {
public:
    virtual uint32  EngineMessageFn(
        uint32 messageID, void *pData, float lData);

    LTVector m_SkyDims;

    float m_InnerPercentX, m_InnerPercentY, m_InnerPercentZ;
    int32 m_Index;
    char m_ObjectName[100];
};

/*!
This has functionality to be almost any type of engine object.
*/
class GenericObject : public BaseClass {
public:
    virtual uint32  EngineMessageFn(
        uint32 messageID, void *pData, float lData);

    float m_LightRadius;
    float m_ColorR;
    float m_ColorG;
    float m_ColorB;
    float m_ColorA;
};


#endif  //! __LTENGINEOBJECTS_H__


