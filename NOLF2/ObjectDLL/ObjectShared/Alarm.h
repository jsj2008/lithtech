// ----------------------------------------------------------------------- //
//
// MODULE  : Alarm.h
//
// PURPOSE : An alarm object
//
// CREATED : 4/15/99
//
// ----------------------------------------------------------------------- //

#ifndef __ALARM_H__
#define __ALARM_H__

#include "ltengineobjects.h"
#include "CommandMgr.h"
#include "Prop.h"

LINKTO_MODULE( Alarm );

typedef std::vector<LTObjRef> AIREGION_LIST;

class Alarm : public Prop
{
	public :

		Alarm();
		~Alarm();

		// Simple accessors

		LTBOOL	IsLocked() const { return m_bLocked; }
		LTBOOL	IsRegionInRespondGroup(HOBJECT hRegion);
		LTBOOL	IsRegionInAlertGroup(HOBJECT hRegion);
		LTBOOL	IsRegionCovered(HOBJECT hRegion);
		uint32	GetNumSearchRegions() { return m_lstSearchRegions.size(); }
		HOBJECT	GetSearchRegion(uint32 iRegion);

	protected :

		enum State
		{
			eStateOff,
			eStateOn,
			eStateDestroyed,
			eStateDisabled,
		};

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32  ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	protected :

        LTBOOL  ReadProp(ObjectCreateStruct *pData);
        LTBOOL  Setup(ObjectCreateStruct *pData );
		void	PostPropRead(ObjectCreateStruct* pData);
        LTBOOL  InitialUpdate();
		void	CreateRegionLists();
		void	CreateRegionList(HSTRING& hstrRegions, AIREGION_LIST* pList);
		void	CalculateRegionsGroupRadius();
		void	CalculateGroupExtents(AIREGION_LIST* pList);

	private :

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

	protected :

		State	m_eState;
        LTBOOL  m_bPlayerUsable;
		LTBOOL	m_bLocked;
		LTFLOAT m_fAlarmSoundTime;
		HSTRING	m_hstrPlayerActivateCommand;
		HSTRING m_hstrAlertRegions;
		HSTRING m_hstrRespondRegions;
		HSTRING m_hstrSearchRegions;

		AIREGION_LIST	m_lstAlertRegions;
		AIREGION_LIST	m_lstRespondRegions;
		AIREGION_LIST	m_lstSearchRegions;

		LTVector		m_vRegionsGroupExtentsMin;
		LTVector		m_vRegionsGroupExtentsMax;
		LTVector		m_vRegionsGroupCenter;
		LTFLOAT			m_fRegionsGroupRadius;
};

class CAlarmPlugin : public CPropPlugin
{
	public:	

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

	protected:

		CCommandMgrPlugin	m_CommandMgrPlugin;
};

#endif // __ALARM_H__