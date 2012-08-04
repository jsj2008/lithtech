#ifndef __LTMODULE_H__
#define __LTMODULE_H__

//****************************************************************************************
//
//
//  This header file contains classes and templates needed
//  to implement standard interfaces used in the LithTech
//  engine.  The main sections are the interface base class,
//  the interface holder class, and the API database class.
//
//  Important classes:
//      
//      IBase
//          Used as a base class for all Lithtech modules.
//
//  Important macros:
//
//      interface_version
//
//      interface_version_derived
//      
//      declare_interface
//
//      instantiate_interface
//
//      define_interface
//
//      implements_also
//      
//      define_holder
//
//      define_holder_to_instance
//
//      link_to_implementation
//
//
//
//****************************************************************************************

//need these headers.

//for int32, etc.
#ifndef __LTINTEGER_H__
#include "ltinteger.h"
#endif

//for NULL
#ifndef __STDLIB_H__
#include <stdlib.h>
#define __STDLIB_H__
#endif

#ifndef __LINUX
#define LTMODULE_EXPORT __declspec(dllexport)
#else
#define LTMODULE_EXPORT
#endif


//****************************************************************************************
//
//
//  Interface base class and implementation.
//
//
//****************************************************************************************

/*!
IBase is the root class of all interfaces managed by the engine.

The members of the class are for internal use only.  They are declared
and defined for IBase derived class using the other Interface Manager
macros.
*/
class IBase {
public:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    //returns the name of the implementation class.
    virtual const char *_InterfaceImplementation() = 0;
    #endif
};

/*!
\param interface_name The interface class name, without quotes.
\param version_num The version number of the interface.

This macro defines the version number of your interface.  It must be
placed in your interface class in a public section.  The version number
is an unsigned integer.  The suggested range of the version number
is 0 to 99.

Any time the interface class "footprint" is changed, the version number should
be incremented.  Changes to the footprint include adding, removing, or 
rearranging functions, changes in function parameter lists, or any other change
which will affect the compiled binary format of the class or it's virtual function
table.

 
The version number is checked any time a holder asks for an implementation
of an interface.  If the version number doesnt match, a debug assertion is triggered,
and the holder value will be \b NULL.
*/
#define interface_version(interface_name, version_num)                              \
    enum { _##interface_name##_VERSION_ = version_num}                              \


/*!
\param interface_name The derived interface class name, without quotes.
\param base_name The parent class name of the interface class.
\param version_num The version number of the derived interface.

This macro is a variation on the \b interface_version macro.  It is used when one
interface class is derived from another interface class that has its own 
version number.
*/
#define interface_version_derived(interface_name, base_name, version_num)           \
    enum { _##interface_name##_VERSION_ = version_num +                             \
        base_name::_##base_name##_VERSION_ * 100}                                   \

/*!
\param impl_class The name of this class.

This macro must be put in a public section of any class that is going to be 
instantiated with the \b instantiate_interface or \b define_interface macros.  
It defines the \b IBase virtual functions in your class so that it can 
be instantiated.
*/
#define declare_interface(impl_class)                                               \
    virtual const char *_InterfaceImplementation() {return #impl_class;}            \

/*!
\param impl_class The name of the class that is to be instantiated.
\param interface_name The name of the interface class that is exposed to others.
\param instance_name A token (no quotes) that identifies this instance of the interface.

This macro is used to instantiate an interface implementation class.  It 
instantiates your implementation class as a static global variable and
inserts it into the interface manager.  

The interface name joined with the instance name is used to identify the 
instantiated object.  Other implementations of the same interface can be instantiated
and inserted into the interface manager as long as instance name is different
from all others.  Instance names must be valid C++ tokens, and are specified
in the macro without any enclosing quotation marks.

Interface implementations are retrieved from the interface manager with the
\b define_holder and the \b define_holder_to_instance macros.
*/
#define instantiate_interface(impl_class, interface_class, instance_name)           \
    /* Declare a global instance of our implementation class. */                    \
    /* It is not static so that we will get link errors if two instances */         \
    /* are created with the same instance name. */                                  \
    static impl_class __impl_instance_##impl_class##_##instance_name##__;           \
    /* Declare a pointer to the instance, so that we can reference it and */        \
    /* get proper linkage if the instance is defined in a lib. */                   \
    impl_class *__impl_ptr_##impl_class##_##instance_name##__ =                     \
        &__impl_instance_##impl_class##_##instance_name##__;                        \
    /* Delcare a global instance of our CAPIInterfaceDefines class, to register */  \
    /* our implementation class for the given interaface class name. */             \
    static CAPIInstanceDefines<impl_class>                                          \
        __impl_##impl_class##_defines_##interface_class##_##instance_name##__(      \
            &__impl_instance_##impl_class##_##instance_name##__,                    \
            #interface_class "." #instance_name,                                    \
            interface_class::_##interface_class##_VERSION_);                        \
    /* Make a static search structure we can use to find this interface */          \
    /* definition in a compiled object file.                            */          \
    static SStaticSearchInterface                                                   \
        __var_search_##interface_class##_##impl_class##_##instance_name##_ = {      \
            SEARCH_MARKER_INTERFACE, SEARCH_MARKER_INT, #interface_class,           \
            #impl_class, #instance_name,                                            \
            interface_class::_##interface_class##_VERSION_};                        \

/*!
\param impl_class_name The class name of the implementation (not interface) class.
\param instance_name The instance name used when the implementation class was instantiated.

This macro is used to insure proper linkage when interface implementations are
defined in library files.  

When an interface contains all pure virtual functions (as most do), the C/C++ linker does
not count references to those pure virtual functions do not as references to the 
implementation class functions.  When a interface implementation class is 
properly insulated from other modules, there will be no direct reference to any
of the implementation class functions or other tokens.  Therefore, the C/C++ linker 
will "throw out" everything in the implementation class' \b OBJ file.  The result
will be that the interface goes unimplemented, and holders to the interface will
be \b NULL.

To prevent this, somewhere in your \b EXE or \b DLL project files, you should
use the \b link_to_implementation macro.  This macro will generate a reference
to a specific token defined in the \b instantiate_instance and 
\b define_interface macros, so that the implementation class will be correctly 
linked in.  

Using this macro does not require that any header file for the specified 
implementation class itself be included.
*/
#define link_to_implementation(impl_class_name, instance_name)                      \
    /* Forward declare the implementation class name. */                            \
    class impl_class_name;                                                          \
    /* declare an extern to the implementation instance pointer */                  \
    extern impl_class_name *__impl_ptr_##impl_class_name##_##instance_name##__;     \
    /* Reference the pointer to the instance defined in the lib. */                 \
    impl_class_name *__for_linker_ptr_##impl_class_name##_##instance_name##__ =     \
        __impl_ptr_##impl_class_name##_##instance_name##__;                         \

/*!
\param impl_class The name of the class that is to be instantiated.
\param interface_class The name of the interface class that is exposed to others.

This macro is the simple way to instantiate an interface implementation class.
Using this macro is equivalent to using the \b instantiate_interface macro with the
third parameter set to \b Default.

Interface implementations with the instance name \b Default can be retrieved with the
simpler \b define_holder macro.
*/
#define define_interface(impl_class, interface_class)                               \
    instantiate_interface(impl_class, interface_class, Default);                    \

/*!
\param impl_class The class name of the implementation class.
\param other_instance_name The instance name that was used when the implementation class was instantiated.
\param interface_class The alternate interface name that the specified instance implements.
\param instance_name The instance name that is associated with the new \b interface_class.

A single instance of an implementation class can sometimes implement more than
one exposed interface.  Use this macro to allow the same implementation instance 
to be inserted and retrieved from the interface manager with another interface 
class/instance name combination.
*/
#define implements_also(impl_class, other_instance_name, interface_class, instance_name)\
    /* Delcare a global instance of our CAPIInterfaceDefines class, to register */  \
    /* our implementation class for the given interaface class name. */             \
    static CAPIInstanceDefines<impl_class>                                          \
        __impl_##impl_class##_defines_##interface_class##_##instance_name##__(      \
            &__impl_instance_##impl_class##_##other_instance_name##__,              \
            #interface_class "." #instance_name,                                    \
            interface_class::_##interface_class##_VERSION_);                        \


#ifndef DOXYGEN_SHOULD_SKIP_THIS
//
//Internal implementation details.
//

//
//This structure is used so that we can parse a compiled object file
//and find information about the interface holders used.
//

typedef struct {
    //marks the beginning of the structure.
    char marker[32];
    int32 marker_int;
    char interface_name[64];
    char instance_name[64];
    char implementation[64];
    int32 version;
} SStaticSearchInterface;

//
//The marker string we use for static search.
//

#define SEARCH_MARKER_BASE "__search_marker"
#define SEARCH_MARKER_INTERFACE SEARCH_MARKER_BASE "_interface__"
#define SEARCH_MARKER_HOLDER SEARCH_MARKER_BASE "_holder__"
#define SEARCH_MARKER_INT 0xfaceface

//
//
//CAPIInstance is used to hold a global reference to the API implementation
//class and register it with the API database when the EXE or DLL is loaded.
//We define a variable of the interface in this class so that we can pass
//an initialized object of that type to the interface database.
//
//

template <class I>
class CAPIInstanceDefines {
  public:
    CAPIInstanceDefines(I *implementation, const char *implemented_class_name, int32 interface_version);
    ~CAPIInstanceDefines();

    //pointer to the implementation, so we can reference it in our destructor
    I *implementation;

    //the class name that this implementation implements.
    const char *implemented_class_name;
};

#endif //DOXYGEN_SHOULD_SKIP_THIS

//****************************************************************************************
//
//
//  Interface holder class.
//
//
//****************************************************************************************

/*!
\param interface_name The name of the interface that you want a pointer to.
\param ptr_var The name of your variable.
\param instance_name The instance name of the interface implementation that you want.

This macro is used to retrieve an implementation of the desired interface.  It should be
used at global scope, as such:
\code
#include "someinterfaceheader.h"
static ISomeInterface *ptr_to_someinterface;
define_holder_to_instance(ISomeInterface, ptr_to_someinterface, Default);
\endcode

If the instance name is \b Default, the more simple \b define_holder macro may be used.
*/
#define define_holder_to_instance(interface_name, ptr_var, instance_name)           \
    /* Make a static search struct for this holder so that we can search */         \
    /* for this holder definition in a compiled object file.             */         \
    static SStaticSearchHolder                                                      \
        __var_search_holder_##interface_name##_##instance_name##_= {                \
            SEARCH_MARKER_HOLDER, SEARCH_MARKER_INT,                                \
            #interface_name, #instance_name,                                        \
            interface_name::_##interface_name##_VERSION_};                          \
    /* Define the holder variable itself. */                                        \
    static CAPIHolder<interface_name>                                               \
        __api_holder_##interface_name##_##instance_name##__(                        \
            #interface_name "." #instance_name,                                     \
            ptr_var, interface_name::_##interface_name##_VERSION_);                 \

/*!
\param interface_name The name of the interface that you want a pointer to.
\param ptr_var The name of your variable.

This macro is used to retrieve an implementation of the desired interface.  It
is identical to the \b define_holder_to_instance and specifying \b Default as
the instance name.
*/
#define define_holder(interface_name, ptr_var)                                      \
    define_holder_to_instance(interface_name, ptr_var, Default);                    \


#ifndef DOXYGEN_SHOULD_SKIP_THIS
//
//Internal implementation details.
//

//
//This structure is used so that we can parse a compiled object file
//and find information about the interface holders used.
//

typedef struct {
    //marks the beginning of the structure.
    char marker[32];
    //non-string marker
    int32 marker_int;
    char interface_name[64];
    char instance_name[64];
    int32 version;
} SStaticSearchHolder;

//
//
//Base API holder class.  This class is used to keep track of 
//a pointer to an interface.  If the interface is ever unloaded
//the pointer will be cleared.  Also, when the API becomes available,
//the holder will be notified.  This class is not to be used directly, 
//instead use the derived template class, with the template parameter
//being the API you wish to use.
//
//

class CAPIHolderBase {
public:
    CAPIHolderBase(const char *api_string_name, int32 version);
    virtual ~CAPIHolderBase();

    //
    //functions used by the interface database.
    //

    //Called by the database to tell us the
    //interface we pointed to has been removed.
    virtual void APIRemoved() = 0;

    //Called when the interface we want is found.
    virtual void APIFound(IBase *api) = 0;

    //returns the interface we are currently connected to.
    virtual IBase *Interface() = 0;

    //returns the name of the api we want
    __inline const char *InterfaceName();

    //returns the version of the api we want.
    __inline int32 Version();

protected:
    //string name of the api.  Taken from the constructor, the string
    //pointed to by this var is not managed in any way, and is assumed to 
    //be allocated on the static heap.
    const char *api_name;

    //version of the api header file that was used when this var was compiled.
    int32 version;
};


//
//
//Template class that is actually used to hold an interface of a 
//particular type.
//
//

template <class I>
class CAPIHolder : public CAPIHolderBase {
public:
    CAPIHolder(const char *api_string_name, I *&holder_var, int32 version);
    ~CAPIHolder();

    //
    //functions used by the interface database.
    //

    //Called by the database to tell us the
    //interface we pointed to has been removed.
    virtual void APIRemoved();

    //Called when the interface we want is found.
    virtual void APIFound(IBase *api);

    //returns the interface we are currently connected to.
    virtual IBase *Interface();

public:
    //pointer to the interface that we keep track of.
    I *&p;
    
};


//
//
//  CAPIHolderBase functions.
//
//

const char *CAPIHolderBase::InterfaceName() {
    return api_name;    
}

int32 CAPIHolderBase::Version() {
    return version;
}


//****************************************************************************************
//
//
//  API Database class.
//
//
//****************************************************************************************

    //class forward references.
    template <class C, class id_type> class database_array;
    template <class C> class database_list;
    class CInterfaceDatabase;
    class CInterfaceNameMgr;
    class CInterfaceChooser;

    //
    //Global functions for DLL-enabled systems.
    //
    
    //
    //If the database is defined in a DLL, this function should be called so that
    //the DLL can merge it's database with the database of the loading executable.
    //
    extern "C" LTMODULE_EXPORT void SetMasterDatabase(CInterfaceDatabase *master_database);
    
    //typedef for pointer to a SetMasterDatabase function
    typedef void (*TSetMasterFn)(CInterfaceDatabase *master_database);

    //
    //Returns a pointer to the master database for this module.  Use this function
    //to get the database to pass to the SetMasterDatabase function of a DLL.
    //

    CInterfaceDatabase *GetMasterDatabase();


    //
    //Tracked pointer template.  Used during program shutdown/cleanup when we have 
    //multiple system modules (EXEs, DLLs, ect) that have been loaded where each has 
    //their own API Database functions linked in, but ultimately all point 
    //to a single master database.
    //
    //CPointerTracked implents a class that is used to keep track of the pointer.
    //The class C specified must implement the following functions:
    //  CPointerTracked<C> *&C::PointerTrackedList();
    //

    
    template <class C>
    class CPointerTracked {
      public:
        __inline CPointerTracked();
        __inline CPointerTracked(C *obj);

        ~CPointerTracked();

        //sets the object we point to.
        __inline CPointerTracked<C> &operator=(C *obj);
        __inline CPointerTracked<C> &operator=(CPointerTracked<C> &other);

        //dereference operator
        __inline C *operator->();

        //cast to our pointer type.
        __inline operator C *();

        //comparison operators.
        __inline bool operator==(C *obj);
        __inline bool operator!=(C *obj);

        //returns false if the pointer is NULL.
        __inline operator bool();

        //returns true if the pointer is NULL.
        __inline bool IsNull();
    
      protected:
        //The actual pointer to the object we point to.
        C *pointer;

        //The object we point to has access to us as part of a
        //doubly linked, circular list
        CPointerTracked<C> *prev, *next;

        //does the actual set work.
        void Set(C *obj);
        void Unset();
    };

    //
    //CPointerTrackedTarget is used by an object that is pointed to with
    //CPointerTracked.  When an instance of this class is embedded
    //within the object that is tracked, its destructor will ensure that
    //all tracked pointers to the object are set to NULL.
    //

    template <class C> 
    class CPointerTrackedTarget {
      public:
        __inline CPointerTrackedTarget();
        __inline ~CPointerTrackedTarget();

        //gets the list head.
	    operator CPointerTracked<C> *&() { return tracked_pointers; }

        //makes all tracked pointer that reference us point to NULL.
        void RedirectPointers(C *new_value);

      protected:
        //the pointer to the head element of the list of pointers that reverence us
        CPointerTracked<C> *tracked_pointers;
    };

    //
    //Use this macro in a public section of a class declaration
    //to quickly define the functions needed by CPointerTracked.
    //Pass in the class name itself as the parameter.
    //It defines a pointer to a CPointerTracked<C> structure, and
    //the following functions:
    //  CPointerTracked<C> *&PointerTrackedList();
    //      Returns the pointer to the first pointer_tracker.  The
    //      other pointer_trackers are part of a list connected to the first.
    //

    #define define_pointer_tracked_target(classname)                    \
      protected:                                                        \
        /*Pointer to the head of the tracked pointers list */           \
        CPointerTrackedTarget<classname> _##classname##our_tracked_pointers; \
      public:                                                           \
        __inline CPointerTracked<classname> *&PointerTrackedList() {    \
            return _##classname##our_tracked_pointers;}                 \


    //
    //
    //The API Database class.  The database is separated into 2 main parts.
    //The first part is a public interface, which is mostly static functions.
    //The functions are static and access an actual database object through
    //a set of global pointers set up so that every EXE or DLL that is
    //loaded can point to a single global database.  The static interface
    //is mostly responsible for keeping track of the lifetime of each
    //EXE/DLL's database pointers, and the database object itself.
    //The actual work of the database is done inside the private non-static
    //interface, which is called from the public static interface.
    //
    //

    //enum returned when chooser run functions are called.
    typedef enum {
        CR_BAD_PARAM,               //chooser passed in was bad.
        CR_BAD_STATE,               //database has invalid state.
        CR_BAD_INTERFACE,           //interface specified by chooser doesnt exist.
        CR_BAD_CHOOSER,             //the chooser given isnt in the database at all.
        CR_CHOOSER_WAS_OVERRIDDEN,  //the given chooser isnt allowed to control the interface, another chooser already is.
        CR_CANT_SET,                //tried to choose the implementation, but couldn't.
        CR_OK,                      //interface implementation selected successfully.
    } EChooserRun;

    class CInterfaceDatabase {
    public:
        CInterfaceDatabase();
        ~CInterfaceDatabase();

        //standard pointer_tracked interface.
        define_pointer_tracked_target(CInterfaceDatabase);

        //
        //Functions used to query or modify the database.
        //

        //adds an api to the database.
        static void AddAPI(IBase *api, const char *implemented_class_name, int32 interface_version);

        //removes an interface from the database.
        //Anyone using the interface will be disconnected.
        static void RemoveAPI(IBase *api, const char *implemented_class_name);

        //registers the holder with the database.  When the
        //database gets the interface that the holder needs,
        //the holder's APIFound function will be called.
        static void AddHolder(CAPIHolderBase *api_holder);

        //removes an API holder from the database.
        static void RemoveHolder(CAPIHolderBase *api_holder);

        //adds a chooser
        static void AddChooser(CInterfaceChooser *chooser);

        //removes a chooser from the database.
        static void RemoveChooser(CInterfaceChooser *chooser);

        //calls a chooser's Run function
        static EChooserRun RunChooser(CInterfaceChooser *chooser);

        //keeps a global count of all objects in the database that belong
        //to 'this' executable.
        static void DatabaseItemCountInc();
        static void DatabaseItemCountDec();

        //takes all the entries out of the given database, while
        //inserting them into us.  It is virtual so that it is called
        //in the same module that the 'this' was allocated from.
        virtual void TransferFrom(CInterfaceDatabase *other);
                      
        //destroys the database.  It is virtual so that the delete operator
        //will be called from the same module that allocated the object.
        virtual void Destroy();

    private:

        //
        //Our data
        //

        //Collection of CInterfaceNameMgr objects.  Each unique interface
        //goes with its own CInterfaceNameMgr object.
        database_array<CInterfaceNameMgr, const char *> *interfaces;

        //when we end up with version mismatch, we put info in these vars.
        bool version_check_failed;
        char *version_check_interface_name;

        //sets version check vars.
        void VersionError(const char *interface_name);

        //
        //Non-static worker functions called from the public interface.
        //

        //adds an interface implementation to the database.
        virtual void Add(IBase *api, const char *implementation_class_name, int32 interface_version);

        //removes an interface implementation from the database
        virtual void Remove(IBase *api, const char *implementation_class_name);

        //adds an interface holder to the database
        virtual void Add(CAPIHolderBase *holder);

        //removes an interface holder from the database
        virtual void Remove(CAPIHolderBase *holder);

        //adds a chooser
        virtual void Add(CInterfaceChooser *chooser);

        //removes a chooser
        virtual void Remove(CInterfaceChooser *chooser);

        //runs a chooser
        virtual EChooserRun Run(CInterfaceChooser *chooser);


        //
        //Helper functions used by the public interface.
        //

        //returns true if the database is empty.
        virtual bool IsEmpty();
    };

    //
    //CInterfaceNameMgr is an object that keeps track of a set of implementations
    //for a given interface, and the holders that use that interface.  It also keeps
    //a CInterfaceChooser object that determines which of the possible implementations
    //should be used at any given time.
    //

    class CInterfaceNameMgr {
    public:
        CInterfaceNameMgr(const char *name, int32 version);
        virtual ~CInterfaceNameMgr();

        //adds an interface implementation
        void Add(IBase *api);

        //adds a holder that uses our interface
        void Add(CAPIHolderBase *holder);

        //adds a chooser
        void Add(CInterfaceChooser *chooser);

        //removes an interface implementation.
        void Remove(IBase *api);

        //removes a holder
        void Remove(CAPIHolderBase *holder);

        //removes a chooser
        void Remove(CInterfaceChooser *chooser);

        //runs a chooser
        EChooserRun Run(CInterfaceChooser *chooser);

        //returns the name that this mgr mgrs
        inline const char *InterfaceName();

        //returns version number of the interface that is mgr'd.
        inline int32 InterfaceVersion();

        //returns true if there are no objects contained in this mgr.
        bool IsEmpty();

        //steals all the data from the given mgr and merges it with
        //our own existing data.
        void TransferFrom(CInterfaceNameMgr *other);

        //
        //Interface used by the choosers
        //

        //tells the name mgr to use the implementation with the given name.
        //Returns true if that implementation was found and is now being used.
        bool UseImplementation(const char *name);

        //gets the number of implementations we have.
        uint32 NumImplementations();

        //gets the name of an implementation.  returns NULL on error.
        const char *ImplemenationName(uint32 index);

    protected:
        //the name of the interface
        char *name;

        //the version of the interface.
        int32 version;

        //the array of interface implementations.
        database_array<IBase, const char *> *interfaces;

        //the array of holders that use this interface
        database_list<CAPIHolderBase> *holders;

        //the current interface we are using.
        IBase *current_interface;

        //stack of choosers, the last inserted is the one used.
        database_list<CInterfaceChooser> *choosers;

        //
        //Private helper functions.
        //

        //connects all holders to the given interface
        void ConnectHolders(IBase *api);

        //connect the given holder to the current interface
        void ConnectHolder(CAPIHolderBase *holder);

        //disconnects all the holders in the mgr
        void DisconnectHolders();
    };

    //
    //CInterfaceNameMgr inline functions
    //

    const char *CInterfaceNameMgr::InterfaceName() {
        return name;    
    }

    int32 CInterfaceNameMgr::InterfaceVersion() {
        return version;
    }


    //
    //
    //CInterfaceChooser is a base class for objects that can decide which of
    //a set of interface implementations should be used at any given time.
    //When the Add, Remove, and Run functions get called, the chooser has a chance
    //to call the CInterfaceNameMgr::UseImplementation().  If it fails to 
    //pick a valid interface implementation from the ones in the mgr, a default
    //choice will be made.
    //
    //

    class CInterfaceChooser {
    public:
        //Init function takes the name and version number of the 
        //interface we are set up to control.
        void Init(const char *name, int32 version);

        //called when an implementation is added to the name mgr.
        virtual void Add(IBase *api, CInterfaceNameMgr *mgr) = 0;

        //called when an implementation is removed from the name mgr
        virtual void Remove(IBase *api, CInterfaceNameMgr *mgr) = 0;

        //called to let the chooser pick an implementation.  To trigger
        //this function to get called, use CInterfaceDatabase::RunChooser().
        virtual EChooserRun Run(CInterfaceNameMgr *mgr) = 0;

        //returns the name and version number of the interface we control
        inline const char *InterfaceName();
        inline int32 InterfaceVersion();

    protected:
        //name of the interface we control.
        const char *name;

        //version of the interface we control.
        int32 version;
    };

    //
    //CInterfaceChooser inline functions.
    //

    const char *CInterfaceChooser::InterfaceName() {
        return name;    
    }

    int32 CInterfaceChooser::InterfaceVersion() {
        return version;
    }




    //
    //This interface chooser takes an array of strings that are interface implementation
    //names, and exposes an interface to select the current implementation via string.
    //Selecting an implementation causes that selection to become the new default.
    //

    class CInterfaceChooserList : private CInterfaceChooser {
    public:
        //Init takes the full list of implementation names, and the default name.
        void Init(const char *name, int32 version, uint32 num_choices, const char **choices, const char *def_name);

        //changes the current implementation.
        EChooserRun SetImplementation(uint32 index);
        EChooserRun SetImplementation(const char *name);

        //base class overrides.
        virtual void Add(IBase *api, CInterfaceNameMgr *mgr);
        virtual void Remove(IBase *api, CInterfaceNameMgr *mgr);
        virtual EChooserRun Run(CInterfaceNameMgr *mgr);

    protected:
        //our list of names.
        const char **choices;

        //how many there are
        uint32 num_choices;

        //the default imp name.
        char def_name[64];
    };

    //
    //
    //Macros to set up chooser vars.
    //
    //

    //
    //Use define_chooser_list like this:
    //
    //static const char *names[] = {"IFooImp1", "IFooImp2", "IFooImp3"};
    //static const char *def_imp = names[1];
    //static CInterfaceChooserList our_chooser;
    //define_chooser_list(IFoo, our_chooser, names, def_imp);
    //
    //void bar(uint32 num) {
    //    //pick that interface
    //    our_chooser.SetImplementation(num);
    //}
    //

    #define define_chooser_list(interface_name, var, imp_names, default_imp)                \
        /* make a function that will get called to initialize our chooser. */               \
        void __chooser_list_init_hook_##var##_() {                                          \
            /* initialize the object */                                                     \
            var->Init(#interface_name, interface_name::_##interface_name##_VERSION_,        \
                sizeof(imp_names)/sizeof(imp_names[0]), imp_names, default_imp);            \
        }                                                                                   \
        /* make the CChooserRegister variable */                                            \
        static CChooserRegister __chooser_register_##var##_(var,                            \
            __chooser_list_init_hook_##var##_);                                             \


    //
    //
    //Object used to register a CInterfaceChooser with the database.
    //
    //

    class CChooserRegister {
    public:
        //the constructor allocates the object, then calls the hook function.
        inline CChooserRegister(CInterfaceChooser &chooser_var, void (*hook)());
        inline ~CChooserRegister();

    protected:
        //chooser var we control
        CInterfaceChooser &chooser_var;
    };

    //
    //ChooserRegister inline functions
    //

    CChooserRegister::CChooserRegister(CInterfaceChooser &var, void (*hook)())
        : chooser_var(var)
    {
        //call the hook function
        hook();

        //increment database item count
        CInterfaceDatabase::DatabaseItemCountInc();

        //put the chooser in the database.
        CInterfaceDatabase::AddChooser(&chooser_var);
    }

    CChooserRegister::~CChooserRegister() {
        //remove the chooser from the database.
        CInterfaceDatabase::RemoveChooser(&chooser_var);

        //decrement database item count
        CInterfaceDatabase::DatabaseItemCountDec();
    }


//****************************************************************************************
//****************************************************************************************
//
//
//
//macros that will eventually be moved to some shared header file.
//
//
//
//
//****************************************************************************************
//****************************************************************************************

    //delete macros.
    #define delca(var) if (var != NULL) {delete [] var; var = NULL;}
    #define delc(var) if (var != NULL) {delete var; var = NULL;}

    //
    //ASSERT style macros, but opposite semanticly.
    //

    #ifdef _DEBUG
        #if defined(__LINUX)
            #define BREAK1()
        #else
            #define BREAK1() __asm {int 3}
        #endif
        #define BREAK(expression) if (expression) {BREAK1();}
    #else
        #define BREAK1()
        #define BREAK(expression)
    #endif

    #define IFBREAKRETURN(expression) if (expression) {BREAK1(); return;}
    #define IFBREAKNULL(expression) if (expression) {BREAK1(); return NULL;}
    #define IFBREAKRETURNVAL(expression, val) if (expression) {BREAK1(); return val;}
    #define IFBREAKFALSE(expression) if (expression) {BREAK1(); return false;}
    #define IFBREAKBREAK(expression) if (expression) {BREAK1(); break;}

    //
    //lets you define a block of code that looks like a function that will
    //get called on shutdown.
    //Use it like this.
    //  FILE_SHUTDOWN(us) {
    //      //our shutdown code...
    //  }
    //
    #define FILE_SHUTDOWN(name)                                                 \
        class _FILE_SHUTDOWN_class_##name##_ {                                  \
          public:                                                               \
            /* a destructor */                                                  \
            ~_FILE_SHUTDOWN_class_##name##_();                                  \
        };                                                                      \
        /*global var of that type.*/                                            \
        _FILE_SHUTDOWN_class_##name##_ _FILE_SHUTDOWN_var_##name##_;            \
        /* body of destructor*/                                                 \
        _FILE_SHUTDOWN_class_##name##_::~_FILE_SHUTDOWN_class_##name##_() 

    //
    //lets you define a block of code that looks like a function that will
    //get called on initialization, before main.
    //Use it like this.
    //  FILE_INIT(us) {
    //      //our shutdown code...
    //  }
    //
    #define FILE_INIT(name)                                                     \
        class _FILE_INIT_class_##name##_ {                                      \
          public:                                                               \
            /* a destructor */                                                  \
            _FILE_INIT_class_##name##_();                                       \
        };                                                                      \
        /*global var of that type.*/                                            \
        _FILE_INIT_class_##name##_ _FILE_INIT_var_##name##_;                    \
        /* body of destructor*/                                                 \
        _FILE_INIT_class_##name##_::_FILE_INIT_class_##name##_() 


	/////////////////////////////////////////
	

    template <class I>
    CAPIInstanceDefines<I>::CAPIInstanceDefines(I *implementation, const char *implemented_class_name, int32 interface_version) {
        //save a pointer to this implementaiton that we can use in our destructor
        this->implementation = implementation;

        //save the name of the implemented class.
        this->implemented_class_name = implemented_class_name;

        //increment the global database item count.
        CInterfaceDatabase::DatabaseItemCountInc();

        //register the api with the database.
        CInterfaceDatabase::AddAPI(implementation, implemented_class_name, interface_version);
    }

    template <class I>
    CAPIInstanceDefines<I>::~CAPIInstanceDefines() {
        //get our api out of the database.
        CInterfaceDatabase::RemoveAPI(implementation, implemented_class_name);

        //decrement the global database item count.
        CInterfaceDatabase::DatabaseItemCountDec();
    }

	//
    //
    //  CAPIHolder functions.
    //
    //

    template <class I>
    CAPIHolder<I>::CAPIHolder(const char *api_string_name, I *&var, int32 version) 
      : CAPIHolderBase(api_string_name, version), p(var)
    {
        //initialize our interface pointer.
        p = NULL;

        //increment database item count.
        CInterfaceDatabase::DatabaseItemCountInc();

        //tell the api database that we exist and need an interface.
        CInterfaceDatabase::AddHolder(this);
    }

    template <class I>
    CAPIHolder<I>::~CAPIHolder() {
        //we are being deleted, clear the pointer we keep track of.
        p = NULL;

        //tell the database that we no longer exist.
        CInterfaceDatabase::RemoveHolder(this);

        //decrement database item count
        CInterfaceDatabase::DatabaseItemCountDec();
    }

    template <class I>
    void CAPIHolder<I>::APIRemoved() {
        //clear our pointer
        p = NULL;
    }

    template <class I>
    void CAPIHolder<I>::APIFound(IBase *api) {
        //set the pointer.  we assume the database has given us the correct api type.
        p = (I *)api;
    }

    template <class I>
    IBase *CAPIHolder<I>::Interface() {
        //return our pointer
        return p;
    }


#endif //DOXYGEN_SHOULD_SKIP_THIS

#endif
