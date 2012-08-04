#ifndef __CLASSMGR_H__
#define __CLASSMGR_H__


class CServerMgr;
class CClass;
class CBindModuleType;
class HHashTable;

#ifndef __LTSERVEROBJ_H__
#include "ltserverobj.h"
#endif

#include "classbind.h"

// When an (BaseClass-derived) object's properties are initialized, this is 
// where the main variables go.
extern LTVector g_BaseObjectPosProp;
extern LTRotation g_BaseObjectRotationProp;
extern uint32 g_BaseObjectFlagsProp;
extern char g_BaseObjectFilenameProp[];




// ----------------------------------------------------------------------- //
// Information about each class in the runtime mugger is stored in here.
// ----------------------------------------------------------------------- //

struct CObjectTickData
{
	CObjectTickData()
	{
		m_ObjectName[0]		= '\0';
		m_nTotalTicks		= 0;
		m_nMaxTicks			= 0;
		m_nTickAveCnt		= 0;
		m_nTicksThisUpdate	= 0;
		m_bUpdatedData		= LTFALSE;
		m_bDisplayedTicks	= LTFALSE;	
	}

	inline uint32 GetAveTicks()			const { return (m_nTickAveCnt > 0 ? m_nTotalTicks / m_nTickAveCnt : 0); }

	char	m_ObjectName[64];		// Name of the object associated with this tick data
	uint32	m_nTotalTicks;			// Total ticks since counting started 
	uint32	m_nMaxTicks;			// Max ticks since counting started
	uint32	m_nTickAveCnt;			// Counter for determinining average ticks
	uint32	m_nTicksThisUpdate;		// Total ticks this update
	LTBOOL	m_bUpdatedData;			// Was the data updated this frame?
	LTBOOL	m_bDisplayedTicks;		// Did we display the ticks yet
};

class CClassData : public CGLLNode
{
	public:

		CClassData();
		~CClassData();

		void	ResetPerUpdateTickData();
		void	ResetAllTickData();

		void	UpdateTicks(LTObject* pObj, uint32 nTicksThisUpdate);
		void	CalculateTotalFrameTicks();

		// Display the formated tick counts, and return this classes totals for 
		// use in global tracking...
		void	DisplayTicks(uint32 & nTicks, uint32 & nAveTicks,
			uint32 & nMaxTicks, uint32 & nTotalObjs);

		// Display per-object ticks for this class...
		void	DisplayObjectTicks();

		inline void	IncrementTotalObjectCount() { m_nTotal++; }
		inline void	SetDisplayedTicks(LTBOOL b)	{ m_bDisplayedTicks = b; }

		inline uint32 GetTicksThisUpdate()	const { return m_nTicksThisUpdate; }
		inline LTBOOL HasDisplayedTicks()	const { return m_bDisplayedTicks; }
		inline uint32 GetAveTicks()			const { return (m_nTickAveCnt > 0 ? m_nTotalTicks / m_nTickAveCnt : 0); }
		inline uint32 GetTotalTicks()		const { return m_nTotalTicks; }
		inline uint32 GetTotalObjects()		const { return m_nTotal; }

		void GetClassName(char* pClassNameBuffer, uint32 nBufferLen, LTBOOL bPadWithSpace);

		StructBank	m_ObjectBank;
		ClassDef	*m_pClass;
		LTObject	*m_pStaticObject;	// Static object for this class, if any.
		uint16		m_ClassID;			// Unique ID for the class.

	protected :

		LTBOOL	IsDisplayingObjectTicks();
		void	ResetPerUpdateObjectTickData();
		void	ResetObjectAveMaxData();
		void	DestroyObjectTickData();
		void	CalculateTotalObjectFrameTicks();
		void	UpdateObjectDataTicks(LTObject* pObj, uint32 nTicksThisUpdate);
		void	GetObjectName(LTObject* pObj, char* pObjectNameBuffer, uint32 nBufferLen);
		CObjectTickData* FindObjectTickData(const char* szObjectName);

		// The following data members are only used for class tick information...

		uint32		m_nTicksThisUpdate;	// Total ticks this update (Used for ShowClassTicks)
		uint32		m_nUpdated;			// Total objects updated this frame
		uint32		m_nTotal;			// Total instances of this class.
		uint32		m_nTickAveCnt;		// Counter for determinining average ticks
		uint32		m_nTotalTicks;		// Total ticks since counting started
		uint32		m_nMaxTicks;		// Max ticks since counting started
		LTBOOL		m_bDisplayedTicks;	// Did we display the ticks yet

		// The following data is used for class instance (object) tick tracking...

		typedef std::vector<CObjectTickData*> TObjectTickDataList;
		TObjectTickDataList m_ObjectTickDataList;
};



// ----------------------------------------------------------------------- //
// The ClassInfo structure holds info on all the classes the server is
// interested in.
// ----------------------------------------------------------------------- //

class CClassMgr
{
public:

	CClassMgr();
	~CClassMgr();

	LTBOOL				Init();
	void				Term();

	CClassData*			FindClassData(const char *pName);

	// Clear all the tick count information for all the classes (this must be called
	// every update)...

	void				ClearTickCounts();
	
	// Display the tick count info to the console (if the applicible console variables
	// are set).  

	void				ShowTickCounts();

	inline CClassData*	GetClassData(ClassDef *pClass) {
		ASSERT(((CClassData*)pClass->m_pInternal[m_ClassIndex])->m_pClass == pClass);
		return (CClassData*)pClass->m_pInternal[m_ClassIndex];
	}


public:

	ClassBindModule		m_ClassModule;
	CBindModuleType     *m_hServerResourceModule;
	
	ClassDef			*m_pBaseClass;
	
	// Extra data associated with each class.
	CClassData			*m_ClassDatas;
	int					m_nClassDatas;

	HHashTable          *m_hClassNameHash;  // Name->CClassData.
	
	// ClassDefs have 2 slots for m_pInternal because the DLL
	// can potentially be loaded twice.
	int					m_ClassIndex;

protected :

	// The following methods are used for calculating class tick counts...

	void				CalculateVerboseTickInfo();
	void				CalculateSimpleTickInfo();

	// The following methods are used for displaying tick counts in the console...

	void				DisplayTickCounts();
	void				DisplayClassTickCounts(const char* pClassName);
};


// ----------------------------------------------------------------------- //
// Functions for operating on classes and stuff.
// ----------------------------------------------------------------------- //

// Loads scripts in the "SScripts" directory.
// Sets up all the CTransVars and CClass pointers.
// Instantiates a CServer object.
// Looks for object.dll in pBaseDir (if it's non-LTNULL), then in the registry.
LTRESULT LoadServerBinaries(CClassMgr *pInfo);


#endif  // __CLASSMGR_H__


