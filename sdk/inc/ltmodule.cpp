//****************************************************************************************
//
//
//  This implementation file contains classes and templates needed
//  to implement standard interfaces used in the LithTech
//  engine.  The two sections are the interface holder class
//  and the API database class.
//
//
//****************************************************************************************

//for other functions and symbols
#include "ltbasedefs.h"

//the lithtech module headers.
#include "ltmodule.h"


//****************************************************************************************
//****************************************************************************************
//
//  Variations on CRT functions that may eventually end up in a shared module.
//
//****************************************************************************************
//****************************************************************************************

namespace
{
    //copys a string, the memory is allocated with new [].
    //if passed in string is NULL, it returns NULL.
    //Free the returned string with delete []
    inline char *LTStrCpyCreate(const char *str) {
        //check if str is NULL.
        if (str == NULL) return NULL;

        //get the length of the string
        int32 length = strlen(str);

        //allocate memory for our copy
        char *copy = new char[length + 1];

        //copy the string data over
        strcpy(copy, str);

        //return the copy.
        return copy;
    }
}

//****************************************************************************************
//****************************************************************************************
//
//  Template classes that might be moved to a widely accessible location.
//
//****************************************************************************************
//****************************************************************************************

    //
    //Sorted array of pointers to objects.
    //
    //The compare and find functions return RELATION_LESS if the relation of
    //the given objects is "less than", RELATION_EQUAL if they are "equal",
    //and RELATION_GREATER if the relation of the given objects is "greater than".
    //The find function operates in a similar way but instead takes only
    //one object and a templated-type ID.
    //

    enum RELATION {
        RELATION_LESS,
        RELATION_EQUAL,
        RELATION_GREATER,
    };

    template <class C, class id_type>
    class database_array {
      public:
        database_array();
        ~database_array();

        //returns the number of items in the array.
        uint32 Num();

        //adds an item to the array. takes the compare function to use.
        void add(C *item, RELATION (*compare_func)(C *other, C *obj));

        //gets an item, given its ID and find function.
        C *get(id_type identifier, RELATION (*find_func)(id_type identifier, C *obj));

        //removes an item, given its ID and a find function.
        C *remove(id_type identifier, RELATION (*find_func)(id_type identifier, C *obj));

        //removes the last item in the array.
        C *remove_last();

        //removes the given item.  returns true if the item was actually removed.
        bool remove(C *item);

        //removes the item at the given index.
        void remove(uint32 index);

        //gets the item at the given index.
        C *get(uint32 index);

        //counts the number of unique items in the array, using the compare func
        //to determine which are equal.
        uint32 NumUnique(RELATION (*compare_func)(C *other, C *obj));

      protected:
        //the number of items in the array.
        uint32 num_items;

        //the length of our array
        uint32 array_len;

        //the array of pointers.
        C **items;

        //lengthens the array.
        void lengthen();

        //searches with the find function for the index of an item with the given identifier.
        //returns -1 if it could not find an item.
        int32 index(id_type identifier, RELATION (*find_func)(id_type identifier, C *obj));

        //inserts the element into the array at the given index, which
        //can be any value >= 0 and <= num_items.  Items below are shifted
        //down.
        void insert(C *item, uint32 index);

        //removes the item from the array, items after it are all shifted up.
        void remove_shift(uint32 index);
    };


    //
    //Database list template.  Used to store unsorted collections of elements.
    //

    template <class C>
    class database_list {
      public:
        database_list();
        ~database_list();

        //returns the number of items in the array.
        uint32 Num();

        //adds an item to end of the array
        void add(C *item);

        //removes the item at the given index and returns it.
        C *remove(uint32 index);

        //removes the given item.  returns true if the item was actually removed.
        //the last item is moved up in the array to fill the space vacated by the
        //removed item.
        bool remove(C *item);

        //same as remove, but shifts elements below the removed one to fill the gap
        bool remove_shift(C *item);

        //gets the item at the given index.
        C *get(uint32 index);

      protected:
        //the number of items in the array.
        uint32 num_items;

        //the length of our array
        uint32 array_len;

        //the array of pointers.
        C **items;

        //lengthens the array.
        void lengthen();
    };


//****************************************************************************************
//****************************************************************************************
//
//
//  API Database implementation.
//
//
//****************************************************************************************
//****************************************************************************************

//struct to hold local pointers to our database.  We put them in a struct
//and allocate the struct when we need to use the pointers so that
//the pointer constructors will get called before operations try to use them
//within other global variable constructors.
//The pointers themselves have a different lifetime than the database.
//Our local pointers will be destroyed as soon as all the APIs and holders from this
//executable (EXE or DLL) have been removed from the database, or when the executable
//itself is unloaded.  The lifetime of the database itself is separate from these
//pointers; it will be deallocated when it is empty of all APIs and holders
//gathered from all executables.
class CDatabasePtrs 
{
public:
    //Our database.  When the first API is registered when this EXE or DLL is loaded,
    //the object will be allocated and the API inserted.
    CPointerTracked<CInterfaceDatabase> local_database;

    //The master database.  If we are informed of the existence of the master,
    //we will transfer our data to it and use it from then on.
    CPointerTracked<CInterfaceDatabase> master_database;

    //The database we actually will use.  It could be either our local database
    //or the master database.
    CPointerTracked<CInterfaceDatabase> use_database;
};

//allocated structure of pointers.
static CDatabasePtrs *ptrs = NULL;

//a count of APIs/holders/choosers that we have in the database from this EXE/DLL
static int32 database_item_count = 0;

//Allocates the pointers if they aren't already allocated.
static __inline void check_create_pointers() 
{
    if (ptrs == NULL)
	{
        LT_MEM_TRACK_ALLOC(ptrs = new CDatabasePtrs, LT_MEM_TYPE_INTERFACEDB);
    }
}

//Deletes the pointers if we have no apis/holders
//from this DLL/EXE in the database.
static __inline void check_delete_pointers() 
{
    if (ptrs != NULL && database_item_count < 1) 
	{
        delc(ptrs);
    }
}

void CInterfaceDatabase::DatabaseItemCountInc() {
    //increment the global count.
    database_item_count++;

    //create the pointers struct.
    check_create_pointers();
}

void CInterfaceDatabase::DatabaseItemCountDec() {
    //decrement the global count.
    database_item_count--;

    //check if we should delete the pointers structure
    check_delete_pointers();
}

//make sure we deallocate our pointers on shutdown.
//We either delete the pointers at the time we remove
//the last api or holder that we added to it, or we
//delete it here if we never added/removed any api's or
//holders.
FILE_SHUTDOWN(ltmod_database) {
    //check if we should delete them now.
    check_delete_pointers();
}

CInterfaceDatabase::CInterfaceDatabase() {
    //initialize version check debug vars.
    version_check_failed = false;
    version_check_interface_name = NULL;

    //allocate our array.
    interfaces = new database_array<CInterfaceNameMgr, const char *>;
}

CInterfaceDatabase::~CInterfaceDatabase() {
    //clean up version debug vars
    delca(version_check_interface_name);

    //delete our interfaces array
    delc(interfaces);
}

//compare and find functions for IBase and CInterfaceNameMgr classes
RELATION SortedArrayFindIBase(const char *name, IBase *api);
RELATION SortedArrayCompareIBase(IBase *other, IBase *api);
RELATION SortedArrayFindNameMgr(const char *name, CInterfaceNameMgr *mgr);
RELATION SortedArrayCompareNameMgr(CInterfaceNameMgr *other, CInterfaceNameMgr *mgr);

void CInterfaceDatabase::AddAPI(IBase *api, const char *implemented_class_name, int32 interface_version) {
    //check parameters.
    IFBREAKRETURN(api == NULL);

    //check state
    IFBREAKRETURN(ptrs == NULL);

    //check if we need to allocate the database now.
    if (ptrs->use_database.IsNull()) {
        //allocate the database.
        LT_MEM_TRACK_ALLOC(ptrs->use_database = ptrs->local_database = new CInterfaceDatabase(), LT_MEM_TYPE_INTERFACEDB);
    }

    //add the interface to the database.
    ptrs->use_database->Add(api, implemented_class_name, interface_version);
}

void CInterfaceDatabase::RemoveAPI(IBase *api, const char *implemented_class_name) {
    //check parameters.
    IFBREAKRETURN(api == NULL);

    //check state.
    IFBREAKRETURN(ptrs == NULL);

    //make sure we have a database.
    if (ptrs->use_database.IsNull()) return;

    //remove the interface from the database.
    ptrs->use_database->Remove(api, implemented_class_name);

    //check if the database is empty.
    if (ptrs->use_database->IsEmpty()) 
	{
        //the database is empty, destroy it.
        ptrs->use_database->Destroy();
		ptrs->use_database = NULL;
    }
}

void CInterfaceDatabase::AddHolder(CAPIHolderBase *holder) {
    //check parameters
    IFBREAKRETURN(holder == NULL);

    //check state
    IFBREAKRETURN(ptrs == NULL);

    //check if we need to allocate the database now.
    if (ptrs->use_database.IsNull()) {
        //allocate the database.
        LT_MEM_TRACK_ALLOC(ptrs->use_database = ptrs->local_database = new CInterfaceDatabase(), LT_MEM_TYPE_INTERFACEDB);
    }

    //add the holder to the database
    ptrs->use_database->Add(holder);
}

void CInterfaceDatabase::RemoveHolder(CAPIHolderBase *holder) {
    //check parameters
    IFBREAKRETURN(holder == NULL);

    //check state
    IFBREAKRETURN(ptrs == NULL);

    //make sure we have a database
    if (ptrs->use_database.IsNull()) return;

    //remove this holder from the database arrays
    ptrs->use_database->Remove(holder);

    //check if the database is empty.
    if (ptrs->use_database->IsEmpty()) 
	{
        //the database is empty, destroy it.
        ptrs->use_database->Destroy();
		ptrs->use_database = NULL;
        return;
    }
}

void CInterfaceDatabase::AddChooser(CInterfaceChooser *chooser) {
    //check parameters
    IFBREAKRETURN(chooser == NULL);

    //check state
    IFBREAKRETURN(ptrs == NULL);

    //check if we need to allocate the database now.
    if (ptrs->use_database.IsNull()) {
        //allocate the database.
        ptrs->use_database = ptrs->local_database = new CInterfaceDatabase();
    }

    //add the chooser to the database
    ptrs->use_database->Add(chooser);
}

void CInterfaceDatabase::RemoveChooser(CInterfaceChooser *chooser) {
    //check parameters
    IFBREAKRETURN(chooser == NULL);

    //check state
    IFBREAKRETURN(ptrs == NULL);

    //make sure we have a database
    if (ptrs->use_database.IsNull()) return;

    //remove this holder from the database arrays
    ptrs->use_database->Remove(chooser);

    //check if the database is empty.
    if (ptrs->use_database->IsEmpty()) 
	{
        //the database is empty, destroy it.
        ptrs->use_database->Destroy();
		ptrs->use_database = NULL;
        return;
    }
}

EChooserRun CInterfaceDatabase::RunChooser(CInterfaceChooser *chooser) {
    //check parameters
    IFBREAKRETURNVAL(chooser == NULL, CR_BAD_PARAM);

    //check state
    IFBREAKRETURNVAL(ptrs == NULL, CR_BAD_STATE);

    //make sure we have a database
    if (ptrs->use_database.IsNull()) return CR_BAD_STATE;

    //run the chooser
    return ptrs->use_database->Run(chooser);
}

void CInterfaceDatabase::Add(IBase *api, const char *implemented_class_name, int32 interface_version) {
    //check params
    IFBREAKRETURN(api == NULL);

    //look for an existing name mgr for this interface name.
    CInterfaceNameMgr *name_mgr = interfaces->get(implemented_class_name, SortedArrayFindNameMgr);

    //check if we got one.
    if (name_mgr == NULL) {
        //create a name mgr for this interface name.
        LT_MEM_TRACK_ALLOC(name_mgr = new CInterfaceNameMgr(implemented_class_name, interface_version), LT_MEM_TYPE_INTERFACEDB);

        //put the name mgr in our array.
        interfaces->add(name_mgr, SortedArrayCompareNameMgr);
    }

    //check the version number of this holder vs the version in the mgr
    if (name_mgr->InterfaceVersion() != interface_version) {
        //version error.  The passed in interface implementation has a different
        //version number than an holder/implementation/chooser of the same class that
        //was already inserted into the database.
        //Examine the variable below to determine the name of the interface
        //that had the version mismatch.

        //get the name of the interface in question.
        const char *conflicting_interface_name = implemented_class_name;

        //log the error.
        VersionError(conflicting_interface_name);
        return;
    }

    //add the api to the name mgr.
    name_mgr->Add(api);
}

void CInterfaceDatabase::Remove(IBase *api, const char *implemented_class_name) {
    //check params
    IFBREAKRETURN(api == NULL);

    //get the name mgr for this api.
    CInterfaceNameMgr *name_mgr = interfaces->get(implemented_class_name, SortedArrayFindNameMgr);
    if (name_mgr == NULL) return;

    //remove the api from the name mgr
    name_mgr->Remove(api);

    //check if the name mgr is empty.
    if (name_mgr->IsEmpty() == true) {
        //remove it from the array.
        interfaces->remove(name_mgr);

        //delete the object.
        delc(name_mgr);
    }
}

void CInterfaceDatabase::Add(CAPIHolderBase *holder) {
    //check params
    IFBREAKRETURN(holder == NULL);

    //look for an existing name mgr for this interface name.
    CInterfaceNameMgr *name_mgr = interfaces->get(holder->InterfaceName(), SortedArrayFindNameMgr);

    //check if we got one.
    if (name_mgr == NULL) {
        //create a name mgr for this interface name.
        LT_MEM_TRACK_ALLOC(name_mgr = new CInterfaceNameMgr(holder->InterfaceName(), holder->Version()), LT_MEM_TYPE_INTERFACEDB);

        //put the name mgr in our array.
        interfaces->add(name_mgr, SortedArrayCompareNameMgr);
    }

    //check the version number of this holder vs the version in the mgr
    if (name_mgr->InterfaceVersion() != holder->Version()) {
        //version error.  The passed in holder wants a version of it's interface
        //that is different than the version of another holder/implementation/chooser
        //that was already inserted into the database.
        //Examine the variable below to determine the name of the interface
        //that had the version mismatch.

        //get the name of the interface in question.
        const char *conflicting_interface_name = holder->InterfaceName();

        //log the error.
        VersionError(conflicting_interface_name);
        return;
    }

    //add the holder to the name mgr.
    name_mgr->Add(holder);
}

void CInterfaceDatabase::Remove(CAPIHolderBase *holder) {
    //check params
    IFBREAKRETURN(holder == NULL);

    //get the name mgr for this holder.
    CInterfaceNameMgr *name_mgr = interfaces->get(holder->InterfaceName(), SortedArrayFindNameMgr);
    if (name_mgr == NULL) return;

    //remove the holder from the name mgr
    name_mgr->Remove(holder);

    //check if the name mgr is empty.
    if (name_mgr->IsEmpty() == true) {
        //remove it from the array.
        interfaces->remove(name_mgr);

        //delete the object.
        delc(name_mgr);
    }
}

void CInterfaceDatabase::Add(CInterfaceChooser *chooser) {
    //check params
    IFBREAKRETURN(chooser == NULL);

    //look for an existing name mgr for this interface name.
    CInterfaceNameMgr *name_mgr = interfaces->get(chooser->InterfaceName(), SortedArrayFindNameMgr);

    //check if we got one.
    if (name_mgr == NULL) {
        //create a name mgr for this interface name.
        LT_MEM_TRACK_ALLOC(name_mgr = new CInterfaceNameMgr(chooser->InterfaceName(), chooser->InterfaceVersion()), LT_MEM_TYPE_INTERFACEDB);

        //put the name mgr in our array.
        interfaces->add(name_mgr, SortedArrayCompareNameMgr);
    }

    //check the version number of this holder vs the version in the mgr
    if (name_mgr->InterfaceVersion() != chooser->InterfaceVersion()) {
        //version error.  The passed in chooser wants a version of it's interface
        //that is different than the version of another holder/implementation/chooser
        //that was already inserted into the database.
        //Examine the variable below to determine the name of the interface
        //that had the version mismatch.

        //get the name of the interface in question.
        const char *conflicting_interface_name = chooser->InterfaceName();

        //log the error.
        VersionError(conflicting_interface_name);
        return;
    }

    //add the chooser to the name mgr.
    name_mgr->Add(chooser);
}

void CInterfaceDatabase::Remove(CInterfaceChooser *chooser) {
    //check params
    IFBREAKRETURN(chooser == NULL);

    //get the name mgr for this chooser.
    CInterfaceNameMgr *name_mgr = interfaces->get(chooser->InterfaceName(), SortedArrayFindNameMgr);
    if (name_mgr == NULL) return;

    //remove the chooser from the name mgr
    name_mgr->Remove(chooser);

    //check if the name mgr is empty.
    if (name_mgr->IsEmpty() == true) {
        //remove it from the array.
        interfaces->remove(name_mgr);

        //delete the object.
        delc(name_mgr);
    }
}

EChooserRun CInterfaceDatabase::Run(CInterfaceChooser *chooser) {
    //check params
    IFBREAKRETURNVAL(chooser == NULL, CR_BAD_PARAM);

    //get the name mgr for this chooser.
    CInterfaceNameMgr *name_mgr = interfaces->get(chooser->InterfaceName(), SortedArrayFindNameMgr);
    if (name_mgr == NULL) return CR_BAD_INTERFACE;

    //run the chooser
    return name_mgr->Run(chooser);
}

void CInterfaceDatabase::TransferFrom(CInterfaceDatabase *other) {
    //check parameters
    IFBREAKRETURN(other == NULL);

    //we have to merge the interface name mgrs from the other database into ours.
    //Go through all the name mgrs in the other database
    for (int32 i = other->interfaces->Num() - 1; i >= 0; i--) {
        //grab the name mgr from the other database
        CInterfaceNameMgr *other_mgr = other->interfaces->get(i);
        if (other_mgr == NULL) continue;

        //see if we have a mgr with the same interface name.
        CInterfaceNameMgr *our_mgr = interfaces->get(other_mgr->InterfaceName(), SortedArrayFindNameMgr);

        //if we have no mgr with this name, we need to make one now.
        if (our_mgr == NULL) {
            //make a mgr
            our_mgr = new CInterfaceNameMgr(other_mgr->InterfaceName(), other_mgr->InterfaceVersion());

            //add the mgr to our array
            interfaces->add(our_mgr, SortedArrayCompareNameMgr);
        }

        //check versions of the two mgrs.
        if (other_mgr->InterfaceVersion() != our_mgr->InterfaceVersion()) {
            //version error.  A holder or implementation of the interface in the loaded DLL
            //is different from the version compiled in this EXE/DLL.  Examine the variable below
            //to determine the name of the interface that had the version mismatch.

            //get the name of the interface in question.
            const char *conflicting_interface_name = other_mgr->InterfaceName();

            //log the error.
            VersionError(conflicting_interface_name);

            //do the next mgr.
            continue;
        }

        //merge the other mgr with our mgr.
        our_mgr->TransferFrom(other_mgr);
    }
}

bool CInterfaceDatabase::IsEmpty() {
    //check if we have any interfaces
    if (interfaces->Num() > 0) return false;

    //we are empty.
    return true;
}

void CInterfaceDatabase::Destroy() {
    delete this;
}

void CInterfaceDatabase::VersionError(const char *interface_name) {
    IFBREAKRETURN(interface_name == NULL);

    //trigger the debugger.  If you wind up in the debugger here, it probably
    //means that something needs to be recompiled.  Check the passed in
    //interface name, and look at the callstack to figure out what EXE/DLL needs to
    //be recompiled.
    BREAK1();

    //check if we already had an error
    if (version_check_failed == true) return;

    //we have an error
    version_check_failed = true;

    //save the name
	delca(version_check_interface_name);
	version_check_interface_name = LTStrCpyCreate(interface_name);
}

//
//
//compare and find functions
//
//

//sort IBase by implementation names.
RELATION SortedArrayFindIBase(const char *name, IBase *api) {
    //do strcmp.
    int32 cmp = strcmp(name, api->_InterfaceImplementation());

    //check the relation.
    if (cmp < 0) return RELATION_LESS;
    if (cmp == 0) return RELATION_EQUAL;
    return RELATION_GREATER;
}

RELATION SortedArrayCompareIBase(IBase *other, IBase *api) {
    //do strcmp.
    int32 cmp = strcmp(other->_InterfaceImplementation(), api->_InterfaceImplementation());

    //check the relation.
    if (cmp < 0) return RELATION_LESS;
    if (cmp == 0) return RELATION_EQUAL;
    return RELATION_GREATER;
}

//sort name mgrs by interface names.
RELATION SortedArrayFindNameMgr(const char *name, CInterfaceNameMgr *mgr) {
    //do strcmp.
    int32 cmp = strcmp(name, mgr->InterfaceName());

    //check the relation.
    if (cmp < 0) return RELATION_LESS;
    if (cmp == 0) return RELATION_EQUAL;
    return RELATION_GREATER;
}

RELATION SortedArrayCompareNameMgr(CInterfaceNameMgr *other, CInterfaceNameMgr *mgr) {
    //do strcmp.
    int32 cmp = strcmp(other->InterfaceName(), mgr->InterfaceName());

    //check the relation.
    if (cmp < 0) return RELATION_LESS;
    if (cmp == 0) return RELATION_EQUAL;
    return RELATION_GREATER;
}


//
//
//CInterfaceNameMgr implementation.
//
//

CInterfaceNameMgr::CInterfaceNameMgr(const char *name, int32 version) {
    //copy the name.
    this->name = LTStrCpyCreate(name);

    //save the version
    this->version = version;

    //we have no current interface implemenation
    current_interface = NULL;

    //allocate our arrays.
    interfaces = new database_array<IBase, const char *>;
    holders = new database_list<CAPIHolderBase>;
    choosers = new database_list<CInterfaceChooser>;
}

CInterfaceNameMgr::~CInterfaceNameMgr() {
    //delete our name
    delca(name);

    //delete our arrays
    delc(choosers);
    delc(holders);
    delc(interfaces);
}

bool CInterfaceNameMgr::IsEmpty() {
    //we are empty if we have no objects
    if (interfaces->Num() > 0) return false;
    if (holders->Num() > 0) return false;
    if (choosers->Num() > 0) return false;

    //we are empty
    return true;
}

void CInterfaceNameMgr::Add(IBase *api) {
    //check params.
    IFBREAKRETURN(api == NULL);

    //check if this is our first implementation.
    if (interfaces->Num() < 1) {
        //connect any holders we have to this interface.
        ConnectHolders(api);
    }

    //put the interface in our array of implementations
    interfaces->add(api, SortedArrayCompareIBase);

    //check if we have a chooser.
    if (choosers->Num() > 0) {
        //tell the last chooser that we have a new interface.
        choosers->get(choosers->Num() - 1)->Add(api, this);
    }
    else {
        //by default, we use the latest implementation added
        ConnectHolders(api);
    }
}

void CInterfaceNameMgr::Add(CAPIHolderBase *holder) {
    IFBREAKRETURN(holder == NULL);

    //connect the holder to the current interface
    ConnectHolder(holder);

    //add the holder to the list
    holders->add(holder);
}

void CInterfaceNameMgr::Add(CInterfaceChooser *chooser) {
    IFBREAKRETURN(chooser == NULL);

    //add the chooser to the end of our list
    choosers->add(chooser);

    //run the chooser, so it has a chance to pick the implementation it wants now
    chooser->Run(this);
}

void CInterfaceNameMgr::Remove(IBase *api) {
    IFBREAKRETURN(api == NULL);

    //remove it from our array
    if (interfaces->remove(api) == false) {
        //it wasnt in our array... that is probably a bad sign
        return;
    }

    //check if we have a chooser.
    if (choosers->Num() > 0) {
        //tell the chooser that we are removing an interface
        choosers->get(choosers->Num() - 1)->Remove(api, this);
    }

    //the chooser (if any) had a chance to switch the current implementation
    //Check if the implementation we removed is still marked as the current one.
    if (api != current_interface) {
        //everything is good.
        return;
    }

    //
    //we need to choose a different implementation.
    //

    //check if there are any to choose from.
    if (interfaces->Num() > 0) {
        //we have some, lets choose the first
        ConnectHolders(interfaces->get(0));

        //done.
        return;
    }

    //there are no implementations of this interface left.
    //Disconnect all of the holders we have.
    DisconnectHolders();
}

void CInterfaceNameMgr::Remove(CAPIHolderBase *holder) {
    IFBREAKRETURN(holder == NULL);

    //remove the holder from our array.  We dont care what it points to any more,
    //and we dont bother disconnecting it.
    holders->remove(holder);
}

void CInterfaceNameMgr::Remove(CInterfaceChooser *chooser) {
    IFBREAKRETURN(chooser == NULL);

    //remove the holder from the array, but keep the order correct
    choosers->remove_shift(chooser);
}

EChooserRun CInterfaceNameMgr::Run(CInterfaceChooser *chooser) {
    IFBREAKRETURNVAL(chooser == NULL, CR_BAD_PARAM);

    //we only run choosers if it was the last one added.  check if we have any choosers
    if (choosers->Num() < 1) return CR_BAD_CHOOSER;

    //check that the given chooser is last in the list
    if (choosers->get(choosers->Num() - 1) != chooser) return CR_CHOOSER_WAS_OVERRIDDEN;

    //run the chooser
    return chooser->Run(this);
}

void CInterfaceNameMgr::ConnectHolders(IBase *api) {
    IFBREAKRETURN(api == NULL);

    //check if this is the same as the current
    if (api == current_interface) return;

    //set the current interface pointer
    current_interface = api;

    //go through all the holders
    for (int32 i = holders->Num() - 1; i >= 0; i--) {
        //get the holder
        CAPIHolderBase *holder = holders->get(i);
        if (holder == NULL) continue;

        //tell the holder to use the current interface
        ConnectHolder(holder);
    }
}

void CInterfaceNameMgr::ConnectHolder(CAPIHolderBase *holder) {
    IFBREAKRETURN(holder == NULL);

    //check if we have a current holder
    if (current_interface == NULL) return;

    //connect the holder to the current interface
    holder->APIFound(current_interface);
}

void CInterfaceNameMgr::DisconnectHolders() {
    //go through our holder array
    for (int32 i = holders->Num() - 1; i >= 0; i--) {
        //get the holder
        CAPIHolderBase *holder = holders->get(i);
        if (holder == NULL) continue;

        //disconnect it
        holder->APIRemoved();
    }

    // We no longer have a "current" interface.  If this isn't
	// cleared and we reload a dll that gave us this interface,
	// then the current_interface checks in ConnectHolders
	// will not re-connect.
    current_interface = NULL;
}

bool CInterfaceNameMgr::UseImplementation(const char *name) {
    IFBREAKFALSE(name == NULL);

    //find the imp with the given name.
    IBase *api = interfaces->get(name, SortedArrayFindIBase);

    //check if we got it
    if (api == NULL) return false;

    //connect our holders to this interface
    ConnectHolders(api);

    //we used the one they asked us to
    return true;
}

uint32 CInterfaceNameMgr::NumImplementations() {
    return interfaces->Num();
}

const char *CInterfaceNameMgr::ImplemenationName(uint32 index) {
    //check the index
    IFBREAKNULL(index >= interfaces->Num());

    //get the api at that index
    IBase *api = interfaces->get(index);
    if (api == NULL) return NULL;

    //return the implementation name
    return api->_InterfaceImplementation();
}

void CInterfaceNameMgr::TransferFrom(CInterfaceNameMgr *other) {
    IFBREAKRETURN(other == NULL);

    //first take all the choosers from the other mgr.
    while (other->choosers->Num() > 0) {
        //pull out the first-in chooser.
        CInterfaceChooser *chooser = other->choosers->remove((uint32)0);

        //add it to the database.
        CInterfaceDatabase::AddChooser(chooser);
    }

    //transfer over all the holders in the other mgr.
    while (other->holders->Num() > 0) {
        //grab a holder
        CAPIHolderBase *holder = other->holders->remove(other->holders->Num() - 1);
        IFBREAKBREAK(holder == NULL);

        //put the holder in our array.
        CInterfaceDatabase::AddHolder(holder);
    }

    //take the implementations out of the other mgr one at a time.
    while (other->interfaces->Num() > 0) {
        //remove an implementation.
        IBase *api = other->interfaces->remove_last();
        IFBREAKBREAK(api == NULL);

        //add the implementation to ourself.
        CInterfaceDatabase::AddAPI(api, name, version);
    }
}

//****************************************************************************************
//****************************************************************************************
//
//
//CInterfaceChooser and derived class functions.
//
//
//****************************************************************************************
//****************************************************************************************


void CInterfaceChooser::Init(const char *name, int32 version) {
    //save the name.
    this->name = name;

    //save the version
    this->version = version;
}

void CInterfaceChooserList::Init(const char *name, int32 version,
    uint32 num_choices, const char **choices, const char *def_name)
{
    //call base class.
    CInterfaceChooser::Init(name, version);

    //save the parameters
    this->choices = choices;
    this->num_choices = num_choices;

    //copy the default name into our buffer.
    LTStrCpy(this->def_name, def_name, sizeof(this->def_name));
}

void CInterfaceChooserList::Add(IBase *api, CInterfaceNameMgr *mgr) {
    //call our run.
    Run(mgr);
}

void CInterfaceChooserList::Remove(IBase *api, CInterfaceNameMgr *mgr) {
    //call our run
    Run(mgr);
}

EChooserRun CInterfaceChooserList::Run(CInterfaceNameMgr *mgr) {
    //try to use the default interface.
    if (mgr->UseImplementation(def_name) == true) return CR_OK;

    //try to use the other choices we have
    for (uint32 i = 0; i < num_choices; i++) {
        //try to use this implementation.
        if (mgr->UseImplementation(choices[i]) == true) return CR_OK;
    }

    //couldnt set it correctly
    return CR_CANT_SET;
}

EChooserRun CInterfaceChooserList::SetImplementation(uint32 index) {
    //check the index
    IFBREAKRETURNVAL(index >= num_choices, CR_BAD_PARAM);

    //set this name as default
    LTStrCpy(this->def_name, choices[index], sizeof(this->def_name));

    //trigger our run function.
    return CInterfaceDatabase::RunChooser(this);
}

EChooserRun CInterfaceChooserList::SetImplementation(const char *name) {
    //check params.
    IFBREAKRETURNVAL(name == NULL, CR_BAD_PARAM);
    if (name[0] == '\0') return CR_BAD_PARAM;

    //set this as our default
    LTStrCpy(this->def_name, name, sizeof(this->def_name));

    //trigger our run function.
    return CInterfaceDatabase::RunChooser(this);
}







//****************************************************************************************
//****************************************************************************************
//
//
//  API Holder implementation.
//
//
//****************************************************************************************
//****************************************************************************************


CAPIHolderBase::CAPIHolderBase(const char *api_string_name, int32 version) {
    //increment the global interface/holder count.
    CInterfaceDatabase::DatabaseItemCountInc();

    //save the given api name.
    api_name = api_string_name;

    //save the version number
    this->version = version;
}

CAPIHolderBase::~CAPIHolderBase() {
    //decrement the global interface/holder count.
    CInterfaceDatabase::DatabaseItemCountDec();
}



//#if defined(_WIN32)

//****************************************************************************************
//****************************************************************************************
//
//
//The global DLL entry points.
//
//
//****************************************************************************************
//****************************************************************************************


void LTMODULE_EXPORT SetMasterDatabase(CInterfaceDatabase *new_database) {
    //check parameters
    IFBREAKRETURN(new_database == NULL);

    //make sure we have our pointers.  might not have them if we have no
    //interfaces or holders in this module
    check_create_pointers();

    //check if this is somehow our own database.
    if (new_database == ptrs->local_database) return;

    //check if we already have a master database.
    if (ptrs->master_database.IsNull() == false) return;

    //keep the pointer to the database we were given.
    ptrs->master_database = new_database;

    //check if we have a local database.
    if (ptrs->local_database.IsNull() == false) 
	{
        //transfer all of the entries in our database to the master database
        ptrs->master_database->TransferFrom(ptrs->local_database);

        //our database is empty now.
        ptrs->local_database->Destroy();
		ptrs->local_database = NULL;
    }

    //use the master database from now on.
    ptrs->use_database = ptrs->master_database;
}

CInterfaceDatabase *GetMasterDatabase() 
{
    //make sure we have our pointers.
    check_create_pointers();

    //check if we were given a master database.
    if (ptrs->master_database.IsNull() == false) 
	{
        //we were a loaded DLL, return the master pointer that was given to us.
        return ptrs->master_database;
    }

    //check if we need to allocate the database now.
    if (ptrs->use_database.IsNull()) 
	{
        //allocate the database.
        ptrs->use_database = ptrs->local_database = new CInterfaceDatabase();
    }

    //return our local database.
    return ptrs->local_database;
}

//#endif


//****************************************************************************************
//****************************************************************************************
//
//
//  Database template class function definitions.
//
//
//****************************************************************************************
//****************************************************************************************

    //
    //
    //database_array functions
    //
    //

    template <class C, class id_type>
    database_array<C, id_type>::database_array() {
        //we have no items.
        num_items = 0;

        //set the initial array len.
        array_len = 1;
        items = new C *[array_len];
    }

    template <class C, class id_type>
    database_array<C, id_type>::~database_array() {
        //delete the array.
        delca(items);
    }

    template <class C, class id_type>
    uint32 database_array<C, id_type>::Num() {
        return num_items;
    }

    template <class C, class id_type>
    void database_array<C, id_type>::lengthen() {
        //our array is longer now
        array_len *= 2;

        //make a new array.
        C **new_items;
		LT_MEM_TRACK_ALLOC(new_items = new C *[array_len * 2], LT_MEM_TYPE_INTERFACEDB);

        //copy the items.
        for (int32 i = 0; i < (int32)num_items; i++) {
            new_items[i] = items[i];
        }

        //delete the old array.
        delca(items);

        //swap in the new array.
        items = new_items;
    }

    template <class C, class id_type>
    void database_array<C, id_type>::add(C *item, RELATION (*compare_func)(C *other, C *obj)) {
        IFBREAKRETURN(item == NULL || compare_func == NULL);

        //we do a binary search through the array looking for where to insert
        //this new item.

        //the bounds of the search.
        int32 top = 0;
        int32 bottom = num_items - 1;

        //at the top of each iteration of this loop, we know the item
        //1) goes at or below top
        //2) goes at or above bottom + 1
        //When it starts, top <= bottom.
        //The search will stop when top = bottom + 1, at which time
        //we put the element at top's index.
        //Top will never be > bottom + 1.

        //do the binary search
        for (; bottom > top - 1;) {
            //check if the item goes below end
            if (compare_func(item, items[bottom]) == RELATION_GREATER) {
                //it goes below bottom
                insert(item, bottom + 1);
                return;
            }

            //it goes at or below (bottom - 1) + 1.  Adjust bottom per rule 2 above.
            bottom--;

            //check if the item goes above the top.
            if (compare_func(item, items[top]) == RELATION_LESS) {
                //it goes where top is
                insert(item, top);
                return;
            }

            //we now know it goes at or below top + 1.  Adjust top per rule 1 above.
            top++;

            //get the middle index.
            int32 middle = (top + bottom) / 2;

            //get the relation of the middle item with the item we are inserting.
            RELATION relation = compare_func(item, items[middle]);

            //check if it goes below middle.
            if (relation == RELATION_LESS) {
                //it goes at or above middle (or (middle - 1) + 1).
                bottom = middle - 1;
            }
            else if (relation == RELATION_GREATER) {
                //it goes at or below middle + 1
                top = middle + 1;
            }
        }

        //put it at top,
        insert(item, top);
    }

    template <class C, class id_type>
    int32 database_array<C, id_type>::index(id_type identifier, RELATION (*find_func)(id_type identifier, C *obj)) {
        //the bounds of the search.
        int32 top = 0;
        int32 bottom = num_items - 1;

        //do binary search until we only have a few items left.
        for (; bottom - top > 5;) {
            //get the middle index.
            int32 middle = (top + bottom) / 2;

            //check the relation of the middle item with the item we are looking for.
            RELATION relation = find_func(identifier, items[middle]);

            if (relation == RELATION_LESS) {
                //the item we are looking for is above the middle.
                bottom = middle - 1;
            }
            else if (relation == RELATION_GREATER) {
                //the item we are looking for is below the middle.
                top = middle + 1;
            }
            if (relation == RELATION_EQUAL) {
                //the middle item is the one we are looking for.
                return middle;
            }
        }

        //now that we narrowed it down to a small area, look at each item
        for (int32 item = top; item <= bottom; item++) {
            //check if this is the item we are looking for.
            if (find_func(identifier, items[item]) == RELATION_EQUAL) {
                //we found the item
                return item;
            }
        }

        //cant find the item
        return -1;

    }

    template <class C, class id_type>
    C *database_array<C, id_type>::get(id_type identifier, RELATION (*find_func)(id_type identifier, C *obj)) {
        IFBREAKNULL(find_func == NULL);

        //get the index of the item.
        int32 index = this->index(identifier, find_func);

        //check if we found one.
        if (index < 0) return NULL;

        //return the item.
        return items[index];
    }

    template <class C, class id_type>
    C *database_array<C, id_type>::remove(id_type identifier, RELATION (*find_func)(id_type identifier, C *obj)) {
        IFBREAKNULL(find_func == NULL);

        //get the index of the item.
        int32 index = this->index(identifier, find_func);

        //check if we found one.
        if (index < 0) return NULL;

        //get the item.
        C *item = items[index];

        //remove the item from the array.
        remove_shift(index);

        //return the item.
        return item;
    }

    template <class C, class id_type>
    C *database_array<C, id_type>::remove_last() {
        //make sure we have items.
        IFBREAKNULL(num_items < 1);

        //get the item.
        C *item = items[num_items - 1];

        //we have 1 less item now
        num_items--;

        //return the item.
        return item;
    }

    template <class C, class id_type>
    bool database_array<C, id_type>::remove(C *item) {
        //search for the item.
        for (uint32 i = 0; i < num_items; i++) {
            //check if the item is here.
            if (item == items[i]) {
                //it is here.  Move all items after this one up in the array.
                for (i++; i < num_items; i++) {
                    //move it up.
                    items[i - 1] = items[i];
                }

                //we have 1 less item
                num_items--;

                //done, we removed it.
                return true;
            }
        }

        //couldn't find the item.
        return false;
    }

    template <class C, class id_type>
    void database_array<C, id_type>::remove(uint32 index) {
        //move all items after it up.
        for (index++; index < num_items; index++) {
            //move it up.
            items[index - 1] = items[index];
        }

        //we have 1 less item.
        num_items--;
    }

    template <class C, class id_type>
    void database_array<C, id_type>::insert(C *item, uint32 index) {
        //check the index
        IFBREAKRETURN(index < 0 || index > num_items);
        IFBREAKRETURN(item == NULL);

        //check if we need more space for this item.
        if (num_items >= array_len) {
            //lengthen the array.
            lengthen();
        }

        //move all elements after the given index down.
        for (int32 i = num_items - 1; i >= (int32)index; i--) {
            //move it down one slot
            items[i + 1] = items[i];
        }

        //insert it
        items[index] = item;

        //we now have one more item.
        num_items++;
    }


    template <class C, class id_type>
    void database_array<C, id_type>::remove_shift(uint32 index) {
        //shift all elements after this up.
        for (int32 i = index + 1; i < (int32)num_items; i++) {
            items[i - 1] = items[i];
        }

        //we have one less item.
        num_items--;
    }

    template <class C, class id_type>
    C *database_array<C, id_type>::get(uint32 index) {
        //check if index is valid.
        if (index >= num_items) return NULL;

        //return the item
        return items[index];
    }

    template <class C, class id_type>
    uint32 database_array<C, id_type>::NumUnique(RELATION (*compare_func)(C *other, C *obj)) {
        //check if we have no items
        if (num_items < 1) return 0;

        //
        //Note: we rely on the array being sorted.
        //

        //we have at least 1 unique item
        uint32 num_unique = 1;

        //go through the items
        for (uint32 i = 1; i < num_items; i++) {
            //check if this item is the same as the one above
            if (compare_func(items[i - 1], items[i]) != RELATION_EQUAL) {
                //it is diferent.  another unique item.
                num_unique++;
            }
        }

        //return the number of items we found
        return num_unique;
    }



    //
    //
    //database_list functions.
    //
    //

    template <class C>
    database_list<C>::database_list() {
        //we have no items.
        num_items = 0;

        //set the initial array len.
        array_len = 1;
        items = new C *[array_len];
    }

    template <class C>
    database_list<C>::~database_list() {
        //delete the array.
        delca(items);
    }

    template <class C>
    uint32 database_list<C>::Num() {
        return num_items;
    }

    template <class C>
    void database_list<C>::lengthen() {
        //our array is longer now
        array_len *= 2;

        //make a new array.
        C **new_items;

		LT_MEM_TRACK_ALLOC(new_items = new C *[array_len * 2], LT_MEM_TYPE_INTERFACEDB);

        //copy the items.
        for (int32 i = 0; i < (int32)num_items; i++) {
            new_items[i] = items[i];
        }

        //delete the old array.
        delca(items);

        //swap in the new array.
        items = new_items;
    }

    template <class C>
    void database_list<C>::add(C *item) {
        //check params
        IFBREAKRETURN(item == NULL);

        //check if we need more space for this item.
        if (num_items >= array_len) {
            //lengthen the array.
            lengthen();
        }

        //insert the item at the end
        items[num_items] = item;

        //we have 1 more item
        num_items++;
    }

    template <class C>
    C *database_list<C>::remove(uint32 index) {
        //check the index
        IFBREAKNULL(index >= num_items);

        //get the item
        C *item = items[index];

        //move all items after this item up in the array
        for (index++; index < num_items; index++) {
            //move this item up
            items[index - 1] = items[index];
        }

        //we have 1 less item
        num_items--;

        //return the item.
        return item;
    }

    template <class C>
    bool database_list<C>::remove(C *item) {
        //search for the item
        for (uint32 i = 0; i < num_items; i++) {
            //check if it is this item.
            if (item != items[i]) continue;

            //we found it.

            //move the last item up to this spot.
            items[i] = items[num_items - 1];

            //we have 1 less item
            num_items--;

            //we removed it
            return true;
        }

        //could not find it.
        return false;
    }

    template <class C>
    bool database_list<C>::remove_shift(C *item) {
        //search for the item
        for (uint32 i = 0; i < num_items; i++) {
            //check if it is this item.
            if (item != items[i]) continue;

            //we found it.

            //move items below this one up
            for (i++; i < num_items; i++) {
                items[i - 1] = items[i];
            }

            //we have 1 less item
            num_items--;

            //we removed it
            return true;
        }

        //could not find it.
        return false;
    }

    template <class C>
    C *database_list<C>::get(uint32 index) {
        //check the param
        IFBREAKNULL(index >= num_items);

        //return the item
        return items[index];
    }



    //
    //
    //CPointerTracked function body definitions.
    //
    //

    template <class C>
    CPointerTracked<C>::CPointerTracked() {
        //start with no object.
        pointer = NULL;

        //we can leave the list pointers uninitialized, they are never used
        //when the pointer is NULL.
    }

    template <class C>
    CPointerTracked<C>::CPointerTracked(C *obj) {
        Set(obj);
    }

    template <class C>
    CPointerTracked<C>::~CPointerTracked() {
        Unset();
    }

    template <class C>
    CPointerTracked<C> &CPointerTracked<C>::operator=(C *obj) {
        //unset our old value.
        Unset();

        //set our new value.
        Set(obj);

        //return ourself
        return *this;
    }

    template <class C>
    CPointerTracked<C> &CPointerTracked<C>::operator=(CPointerTracked<C> &other) {
        //unset our old value.
        Unset();

        //set our new value.
        Set(other.pointer);

        //return ourself
        return *this;
    }

    template <class C>
    C *CPointerTracked<C>::operator->() {
        return pointer;
    }

    template <class C>
    CPointerTracked<C>::operator C *() {
        return pointer;
    }

    template <class C>
    bool CPointerTracked<C>::operator==(C *obj) {
        return pointer == obj;
    }

    template <class C>
    bool CPointerTracked<C>::operator!=(C *obj) {
        return pointer != obj;
    }

    template <class C>
    CPointerTracked<C>::operator bool() {
        return pointer != NULL;
    }

    template <class C>
    bool CPointerTracked<C>::IsNull() {
        return pointer == NULL;
    }

    template <class C>
    void CPointerTracked<C>::Set(C *obj) {
        //we assume when we enter this function that any old values or data
        //stored are taken care of.

        //set our pointer.
        pointer = obj;

        //check if it is NULL
        if (obj == NULL) return;

        //get the list from the object.
        CPointerTracked<C> *&list = obj->PointerTrackedList();

        //check if this is the first pointer to that object.
        if (list == NULL) {
            //we are the first pointer to the object.

            //we now assume the role of list head.
            list = this;

            //fix our list pointers.
            prev = next = this;

            //done
            return;
        }

        //insert ourselves before the existing list head (back of the list)
        next = list;
        prev = list->prev;
        prev->next = this;
        list->prev = this;
    }

    template <class C>
    void CPointerTracked<C>::Unset() {
        //check if we point to an object.
        if (pointer == NULL) return;

        //get the list of pointers from the object.
        CPointerTracked<C> *&list = pointer->PointerTrackedList();

        //set the pointer to NULL now, we are done with it.
        pointer = NULL;

        //check if we are the only element in the list.
        if (next == this) {
            //we are the last pointer to this object, it has no more list.
            list = NULL;

            //done
            return;
        }

        //there are other elements in the list, see if we are the list head.
        if (list == this) {
            //make a different element the list head.
            list = next;
        }

        //remove ourself from the list.
        next->prev = prev;
        prev->next = next;
    }


    //
    //
    //CPointerTrackedTarget function body definitions.
    //
    //

    template <class C>
    CPointerTrackedTarget<C>::CPointerTrackedTarget() {
        tracked_pointers = NULL;
    }

    template <class C>
    CPointerTrackedTarget<C>::~CPointerTrackedTarget() {
        //reset all our references to NULL
        RedirectPointers(NULL);
    }


    template <class C>
    void CPointerTrackedTarget<C>::RedirectPointers(C *new_value) {
        //check if we have a list
        if (tracked_pointers == NULL) return;

        //make sure this is a different value than what the pointers have now
        if (new_value == (C *)tracked_pointers) return;

        //Reset the value of all of our pointers.
        //Setting the value of each pointer will do the acutal removal
        //from the list, so we just go until we have no more list.
        while (tracked_pointers != NULL) {
            //set the value of the first pointer.
            *tracked_pointers = new_value;
        }
    }

