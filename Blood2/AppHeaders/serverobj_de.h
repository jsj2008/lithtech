
// serverobj_de defines all the necessary things for writing an object.

#ifndef __SERVEROBJ_DE_H__
#define __SERVEROBJ_DE_H__


	#include "basedefs_de.h"
	#include "servershell_de.h"



	// Used for containers to apply physics to an object.
	// You can change velocity, acceleration, and flags without changing the object's
	// actual values.
	typedef struct ContainerPhysics_t
	{
		DVector		m_Acceleration;
		DVector		m_Velocity;
		DDWORD		m_Flags;
		HOBJECT		m_hObject;
	} ContainerPhysics;


 	// Different property types.
	#define PT_STRING	0
	#define PT_VECTOR	1
	#define PT_COLOR	2
	#define PT_REAL		3
	#define PT_FLAGS	4
	#define PT_BOOL		5
	#define PT_LONGINT	6
	#define PT_ROTATION	7
	#define NUM_PROPERTYTYPES 8

	// Property flags.
	#define PF_HIDDEN		(1<<0)	// Property doesn't show up in DEdit.
	#define PF_RADIUS		(1<<1)	// Property is a number to use as radius for drawing circle.  There can be more than one.
	#define PF_DIMS			(1<<2)	// Property is a vector to use as dimensions for drawing box. There can be only one.
	#define PF_FIELDOFVIEW	(1<<3)	// Property is a field of view.
	#define PF_LOCALDIMS	(1<<4)	// Used with PF_DIMS.  Causes DEdit to show dimensions rotated with the object.
	#define PF_GROUPOWNER	(1<<5)	// This property owns the group it's in.
	#define PF_GROUP1		(1<<6)	// This property is in group 1.
	#define PF_GROUP2		(1<<7)	// This property is in group 2.
	#define PF_GROUP3		(1<<8)	// This property is in group 3.
	#define PF_GROUP4		(1<<9)	// This property is in group 4.
	#define PF_GROUP5		(1<<10)	// This property is in group 5.
	#define PF_GROUP6		(1<<11)	// This property is in group 6.

	// Used internally..
	#define PF_GROUPMASK	(PF_GROUP1|PF_GROUP2|PF_GROUP3|PF_GROUP4|PF_GROUP5|PF_GROUP6)

	// Class flags.
	#define CF_HIDDEN		(1<<0)	// Instances of the class can't be created in DEdit.
	#define CF_NORUNTIME	(1<<1)	// This class doesn't get used at runtime (the engine 
									// won't instantiate these objects out of the world file).
	#define CF_STATIC		(1<<2)	// This is a special class that the server creates an
									// instance of at the same time that it creates the
									// server shell.  The object is always around.  This
									// should be used as much as possible instead of adding
									// code to the server shell.
	#define CF_ALWAYSLOAD	(1<<3)	// Objects of this class and sub-classes are always loaded from the level
									// file and can't be saved to a save game.



	#define PRECREATE_NORMAL		0.0f	// Object is being created at runtime.
	#define PRECREATE_WORLDFILE		1.0f	// Object is being loaded from a world file.  Read props in.
	#define PRECREATE_STRINGPROP	2.0f	// Object is created from CreateObjectProps.  Use GetPropGeneric to read props.
	#define PRECREATE_SAVEGAME		3.0f	// Object comes from a savegame.
									
	#define INITIALUPDATE_NORMAL		0.0f	// Normal creation.
	#define INITIALUPDATE_WORLDFILE		1.0f	// Being created from a world file.
	#define INITIALUPDATE_STRINGPROP	2.0f	// Object is created from CreateObjectProps.  Use GetPropGeneric to read props.
	#define INITIALUPDATE_SAVEGAME		3.0f	// Created from a savegame.


	// Here are all the message IDs and structures that LithTech uses.

	// This is called right before the server uses the ObjectCreateStruct
	// to create its internal structure for the object.  
	// pData = ObjectCreateStruct*
	// fData = a PRECREATE_ define above.
	#define MID_PRECREATE		0

	// This is called right after your object is created (kind of like the opposite
	// of MID_POSTPROPREAD).
	// fData is an INITIALUPDATE_ define above.
	#define MID_INITIALUPDATE	1
	
	// This is called when NextUpdate goes to zero.
	#define MID_UPDATE			2

	// This is called when you touch another object.
	// pData is an HOBJECT for the other object.
	// fData is the collision (stopping) force (based on masses and velocities).
	#define MID_TOUCHNOTIFY		3

	// This is notification when a link to an object is about to be broken.
	// pData is an HOBJECT to the link's object.
	#define MID_LINKBROKEN		4

	// This is notification when a model string key is crossed.
	// (You only get it if your FLAG_MODELKEYS flag is set).
	// pData is an ArgList*.
	#define MID_MODELSTRINGKEY	5

	// Called when an object pushes you into a wall.  It won't
	// move any further unless you make yourself nonsolid (ie: a player would
	// take damage from each crush notification, then die).
	// pData is the HOBJECT of the object crushing you.
	#define MID_CRUSH			6

	// Load and save yourself for a serialization.
	// pData is an HMESSAGEREAD or HMESSAGEWRITE.
	// fData is the dwParam passed to ServerDE::SaveObjects or ServerDE::RestoreObjects.
	#define MID_LOADOBJECT		7
	#define MID_SAVEOBJECT		8

	// Called for a container for objects inside it each frame.  This gives you a chance
	// to modify the physics applied to an object WITHOUT actually modifying its
	// velocity or acceleration (great way to dampen velocity..)
	// pData is a ContainerPhysics*.
	#define MID_AFFECTPHYSICS	9

	// The parent of an attachment between you and it is being removed.
	#define MID_PARENTATTACHMENTREMOVED	10

	// Called every frame on client objects.  This gives you a chance to force
	// updates on certain objects so they never get removed for the client.
	// pData is a ForceUpdate*.
	// (LT automatically adds the client object and the sky objects to this list to start with).
	#define MID_GETFORCEUPDATEOBJECTS	11


	


	// These are your (optional) construct/destruct functions.
	// In C++, you should ALWAYS use the default ones, because they
	// will call your constructor/destructor.  In C, you can use
	// them to init and term your object.  In most cases, you should
	// add any aggregates you have in your construct function, since
	// ReadProp/PostPropRead get called right after and your aggregates
	// might want to look at them.
	typedef void (*ConstructObjectFn)(void *pObject);
	typedef void (*DestructObjectFn)(void *pObject);

	
	// Your object should implement this and call its base class's
	// message function at the end.
	typedef DDWORD (*EngineMessageFn)(LPBASECLASS pObject, DDWORD messageID, void *pData, float fData);

	// This is called when an object sends a message to you.
	// You should call your base class's object message function after you handle this.
	typedef DDWORD (*ObjectMessageFn)(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);


	typedef struct PropDef_t
	{

				PropDef_t(char *pName, short type, DVector valVec,
					float valFloat, char *valString, short propFlags)
				{
					m_PropName = pName;
					m_PropType = type;
					m_DefaultValueVector = valVec;
					m_DefaultValueFloat = valFloat;
					m_DefaultValueString = valString;
					m_PropFlags = propFlags;
					m_pInternal = 0;
				}
	
		char	*m_PropName;
		
		// One of the PT_ defines above.
		short	m_PropType;
		
		// Default vector/color value.
		DVector	m_DefaultValueVector;

		float	m_DefaultValueFloat;
		char	*m_DefaultValueString;

		short	m_PropFlags;

		// Don't touch!
		void	*m_pInternal;
	
	} PropDef;


	typedef struct ClassDef_t
	{
		
		char			*m_ClassName;

		struct ClassDef_t	*m_ParentClass;

		// A combination of the CF_ flags above.
		DDWORD				m_ClassFlags;

		ConstructObjectFn	m_ConstructFn;
		DestructObjectFn	m_DestructFn;
		EngineMessageFn		m_EngineMessageFn;
		ObjectMessageFn		m_ObjectMessageFn;

		short			m_nProps;
		PropDef			*m_Props;

		// How big an object of this class is (set automatically).
		long			m_ClassObjectSize;

		// Don't touch!
		void	*m_pInternal[2];
	
	} ClassDef;


	#include "server_de.h"
	#include "aggregate_de.h"


	// This is always available, once you create your server shell.
	class ServerDE;
	extern ServerDE *g_pServerDE;


	// Used to avoid crashes from version mismatches.
	#define SERVEROBJ_VERSION 1




	// You MUST have one source file that lists out all the classes you have defined
	// using these macros.
	#ifdef COMPILE_WITH_C
		#define BEGIN_CLASSDEFS1() 	
	#else
		#define BEGIN_CLASSDEFS1() 	__ClassDefiner *__g_ClassDefinerHead=0;
	#endif

	#define DEFINECLASS1(name) extern ClassDef _##name##_Class__;
	#define END_CLASSDEFS1()

	#define BEGIN_CLASSDEFS2() \
		static ClassDef *__GlobalClassDefList__[] = { \

	#define DEFINECLASS2(name) \
		&_##name##_Class__,

	#define END_CLASSDEFS2() \
		}; \
		static int __GlobalClassDefListSize__ = sizeof(__GlobalClassDefList__) / sizeof(__GlobalClassDefList__[0]); \
		ServerDE *g_pServerDE=(ServerDE*)0;\
		BEGIN_EXTERNC() \
			__declspec(dllexport) ClassDef** ObjectDLLSetup(int *nDefs, ServerDE *pServer, int *version); \
		END_EXTERNC() \
		ClassDef** ObjectDLLSetup(int *nDefs, ServerDE *pServer, int *version) \
		{\
			*version = SERVEROBJ_VERSION;\
			g_pServerDE = pServer;\
			*nDefs = __GlobalClassDefListSize__; \
			return __GlobalClassDefList__; \
		}


	// In C++, you can use this macro instead of the BEGIN_CLASSDEFS macros
	// so you don't have to add each object to the list.
	#ifndef COMPILE_WITH_C
		class __ClassDefiner;
		extern __ClassDefiner *__g_ClassDefinerHead;
		
		class __ClassDefiner
		{
			public:

				__ClassDefiner(ClassDef *pDef)
				{
					m_pClass = pDef;
					m_pNext = __g_ClassDefinerHead;
					__g_ClassDefinerHead = this;
				}

				ClassDef		*m_pClass;
				__ClassDefiner	*m_pNext;

		};
		
		extern ClassDef **__g_cpp_classlist;
		class __cpp_classlist_auto_free	{
			public:
				~__cpp_classlist_auto_free() {
					if(__g_cpp_classlist) {
						free(__g_cpp_classlist);
						__g_cpp_classlist = 0;
					}
				}
		};

		#define DEFINE_CLASSES() \
			ClassDef **__g_cpp_classlist=0;\
			__cpp_classlist_auto_free __free_the_g_cpp_classlist;\
			ServerDE *g_pServerDE=(ServerDE*)0;\
			BEGIN_EXTERNC() \
				__declspec(dllexport) ClassDef** ObjectDLLSetup(int *nDefs, ServerDE *pServer, int *version); \
			END_EXTERNC() \
			__ClassDefiner *__g_ClassDefinerHead=0;\
			ClassDef** ObjectDLLSetup(int *nDefs, ServerDE *pServer, int *version) \
			{\
				int nClasses;\
				__ClassDefiner *pCurDefiner;\
				*version = SERVEROBJ_VERSION;\
				g_pServerDE = pServer;\
				nClasses=0;\
				pCurDefiner = __g_ClassDefinerHead;\
				while(pCurDefiner)\
				{\
					pCurDefiner = pCurDefiner->m_pNext;\
					++nClasses;\
				}\
				__g_cpp_classlist = (ClassDef**)malloc(sizeof(ClassDef*) * nClasses);\
				nClasses=0;\
				pCurDefiner = __g_ClassDefinerHead;\
				while(pCurDefiner)\
				{\
					__g_cpp_classlist[nClasses] = pCurDefiner->m_pClass;\
					pCurDefiner = pCurDefiner->m_pNext;\
					++nClasses;\
				}\
				*nDefs = nClasses; \
				return __g_cpp_classlist; \
			}

	#endif




	// -------------------------------------------------------- //
	// These are the macros that you use to define classes.
	// -------------------------------------------------------- //

	#ifdef __cplusplus
		#define ALLOCATEONE(type) (void*)(new type)
		#define DELETEONE(type, ptr) delete ((type*)ptr)
	#else
		#define ALLOCATEONE(type) malloc(sizeof(type))
		#define DELETEONE(type, ptr) free(ptr)
	#endif


	#ifndef COMPILE_WITH_C	
		inline void* operator new(size_t size, void *ptr, int asdf, char a)
		{
			return ptr;
		}
		
		#if _MSC_VER != 1100
			inline void operator delete(void *pDataPtr, void *ptr, int asdf, char a)
			{
			}
		#endif
	#endif


	// It adds a dummy property here so C syntax doesn't break when you have no properties..
	#define NOCOLOR DVector(0.0f, 0.0f, 0.0f)


	// If you want level designers to be able to set object flags, you should
	// use these macros so the property names are standardized.
	
	// Use the ADD_X_FLAG macros in your property list.

	#define ADD_VISIBLE_FLAG(defVal, flags) \
		ADD_PROP_FLAG(Visible, PT_BOOL, defVal, 0, flags)

	#define ADD_SHADOW_FLAG(defVal, flags) \
		ADD_PROP_FLAG(Shadow, PT_BOOL, defVal, 0, flags)

	#define ADD_ROTATEABLESPRITE_FLAG(defVal, flags) \
		ADD_PROP_FLAG(RotateableSprite, PT_BOOL, defVal, 0, flags)

	#define ADD_CHROMAKEY_FLAG(defVal, flags) \
		ADD_PROP_FLAG(Chromakey, PT_BOOL, defVal, 0, flags)

	#define ADD_SOLID_FLAG(defVal, flags) \
		ADD_PROP_FLAG(Solid, PT_BOOL, defVal, 0, flags)

	#define ADD_GRAVITY_FLAG(defVal, flags) \
		ADD_PROP_FLAG(Gravity, PT_BOOL, defVal, 0, flags)

	#define ADD_TOUCHNOTIFY_FLAG(defVal, flags) \
		ADD_PROP_FLAG(Touch Notify, PT_BOOL, defVal, 0, flags)

	#define ADD_RAYHIT_FLAG(defVal, flags) \
		ADD_PROP_FLAG(Rayhit, PT_BOOL, defVal, 0, flags)


	// Full property definitions.
	#define ADD_PROP_FLAG(name, type, valFloat, valString, flags) \
		PropDef_t(#name, type, NOCOLOR, valFloat, valString, flags),

	#define ADD_REALPROP_FLAG(name, val, flags) \
		PropDef_t(#name, PT_REAL, NOCOLOR, val, "", flags),

	#define ADD_STRINGPROP_FLAG(name, val, flags) \
		PropDef_t(#name, PT_STRING, NOCOLOR, 0.0f, val, flags),

	#define ADD_VECTORPROP_FLAG(name, flags) \
		PropDef_t(#name, PT_VECTOR, NOCOLOR, 0.0f, (char*)0, flags),

	#define ADD_VECTORPROP_VAL_FLAG(name, defX, defY, defZ, flags) \
		PropDef_t(#name, PT_VECTOR, DVector(defX, defY, defZ), 0.0f, (char*)0, flags),

	#define ADD_LONGINTPROP_FLAG(name, val, flags) \
		PropDef_t(#name, PT_LONGINT, NOCOLOR, (float)val, (char*)0, flags),

	#define ADD_ROTATIONPROP_FLAG(name, flags) \
		PropDef_t(#name, PT_ROTATION, NOCOLOR, 0.0f, (char*)0, flags),

	#define ADD_BOOLPROP_FLAG(name, val, flags) \
		PropDef_t(#name, PT_BOOL, NOCOLOR, (float)val, (char*)0, flags),

	#define ADD_COLORPROP_FLAG(name, valR, valG, valB, flags) \
		PropDef_t(#name, PT_COLOR, DVector(valR, valG, valB), 0.0f, (char*)0, flags),



	// Add properties without flags (only here for backward compatibility).
	#define ADD_PROP(name, type, valFloat, valString) \
		ADD_PROP_FLAG(name, type, valFloat, valString, 0)

	#define ADD_REALPROP(name, val) \
		ADD_REALPROP_FLAG(name, val, 0)

	#define ADD_STRINGPROP(name, val) \
		ADD_STRINGPROP_FLAG(name, val, 0)

	#define ADD_VECTORPROP(name) \
		ADD_VECTORPROP_FLAG(name, 0)

	#define ADD_VECTORPROP_VAL(name, defX, defY, defZ) \
		ADD_VECTORPROP_VAL_FLAG(name, defX, defY, defZ, 0)

	#define ADD_LONGINTPROP(name, val) \
		ADD_LONGINTPROP_FLAG(name, val, 0)

	#define ADD_ROTATIONPROP(name) \
		ADD_ROTATIONPROP_FLAG(name, 0)

	#define ADD_BOOLPROP(name, val) \
		ADD_BOOLPROP_FLAG(name, val, 0)

	#define ADD_COLORPROP(name, valR, valG, valB) \
		ADD_COLORPROP_FLAG(name, valR, valG, valB, 0)

	
	// Define a group with this.
	#define PROP_DEFINEGROUP(groupName, groupFlag) \
		PropDef_t(#groupName, PT_LONGINT, NOCOLOR, (float)0.0f, (char*)0, groupFlag|PF_GROUPOWNER), 
		

	#define BEGIN_CLASS(name) \
		static PropDef  _##name##_Props__[] = {	\
			ADD_STRINGPROP("__NOPROP__!!", "")

	
	// Define the default constructor/destructor functions.
	#ifdef COMPILE_WITH_C
		#define DO_AUTO_CLASSLIST(name)
		
		#define DO_DEFAULT_FUNCTIONS(_className) \
			void Default##_className##Constructor(void *ptr) {}\
			void Default##_className##Destructor(void *ptr) {}
	#else
		#define DO_AUTO_CLASSLIST(name) \
			static __ClassDefiner __##name##_definer(&_##name##_Class__);
		
		#define DO_DEFAULT_FUNCTIONS(_className)\
			void Default##_className##Constructor(void *ptr)\
			{\
				::new(ptr, (int)0, (char)0) _className;\
			}\
			void Default##_className##Destructor(void *ptr)\
			{\
				_className *thePtr = (_className*)ptr;\
				thePtr->~_className();\
			}
	#endif

	#define END_CLASS_SYMBOL(name, parentSymbol, flags, construct_fn, destruct_fn, enginemessage_fn, objectmessage_fn, classFlags) \
		ClassDef _##name##_Class__ = { \
			#name, parentSymbol, \
			flags,\
			construct_fn, destruct_fn, \
			(EngineMessageFn)enginemessage_fn,\
			(ObjectMessageFn)objectmessage_fn,\
			(sizeof(_##name##_Props__) / sizeof(PropDef)) - 1,\
			&_##name##_Props__[1], \
			sizeof(name),\
			(void*)0, (void*)0 }; \
			DO_AUTO_CLASSLIST(name)

	// End class macros with flags.
	#define END_CLASS_FLAGS(name, parent, construct_fn, destruct_fn, enginemessage_fn, objectmessage_fn, flags) \
		}; \
		extern ClassDef _##parent##_Class__; \
		END_CLASS_SYMBOL(name, &_##parent##_Class__, flags, construct_fn, destruct_fn, enginemessage_fn, objectmessage_fn, 0)

	#define END_CLASS_DEFAULT_FLAGS(name, parent, enginemessage_fn, objectmessage_fn, flags)\
		}; \
		extern ClassDef _##parent##_Class__; \
		DO_DEFAULT_FUNCTIONS(name)\
		END_CLASS_SYMBOL(name, &_##parent##_Class__, flags, Default##name##Constructor, Default##name##Destructor, enginemessage_fn, objectmessage_fn, 0)

	// Normal end class macros.
	#define END_CLASS(name, parent, construct_fn, destruct_fn, enginemessage_fn, objectmessage_fn) \
		END_CLASS_FLAGS(name, parent, construct_fn, destruct_fn, enginemessage_fn, objectmessage_fn, 0)

	#define END_CLASS_DEFAULT(name, parent, enginemessage_fn, objectmessage_fn)\
		END_CLASS_DEFAULT_FLAGS(name, parent, enginemessage_fn, objectmessage_fn, 0)		


	// Only used internally.
	#define END_CLASS_NOPARENT(name, construct_fn, destruct_fn, enginemessage_fn, objectmessage_fn) \
		}; \
		END_CLASS_SYMBOL(name, (ClassDef*)0, 0, construct_fn, destruct_fn, enginemessage_fn, objectmessage_fn, 0)

	#define END_CLASS_DEFAULT_NOPARENT(name, enginemessage_fn, objectmessage_fn) \
		}; \
		DO_DEFAULT_FUNCTIONS(name)\
		END_CLASS_SYMBOL(name, (ClassDef*)0, 0, Default##name##Constructor, Default##name##Destructor, enginemessage_fn, objectmessage_fn, 0)


#endif  // __SERVEROBJ_DE_H__


