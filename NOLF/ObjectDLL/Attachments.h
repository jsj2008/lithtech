// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ATTACHMENTS_H__
#define __ATTACHMENTS_H__

#include "iaggregate.h"
#include "ltengineobjects.h"
#include "Weapons.h"
#include "iobjectplugin.h"
#include "AttachButeMgr.h"

#define NO_ATTACHMENT	"<none>"

#define ADD_ATTACHMENTS_AGGREGATE() \
	PROP_DEFINEGROUP(Attachments, PF_GROUP6) \

#define ADD_HUMANATTACHMENTS_AGGREGATE() \
	ADD_ATTACHMENTS_AGGREGATE() \
		ADD_STRINGPROP_FLAG(LeftHand,	NO_ATTACHMENT,		PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightHand,	NO_ATTACHMENT,		PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Head,		NO_ATTACHMENT,		PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Eyes,		NO_ATTACHMENT,		PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Nose,		NO_ATTACHMENT,		PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Chin,		NO_ATTACHMENT,		PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Back,		NO_ATTACHMENT,		PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftFoot,	NO_ATTACHMENT,		PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightFoot,	NO_ATTACHMENT,		PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Motorcycle,	NO_ATTACHMENT,		PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Snowmobile,	NO_ATTACHMENT,		PF_GROUP6|PF_STATICLIST) \

#define ADD_PLAYERATTACHMENTS_AGGREGATE() \
	ADD_HUMANATTACHMENTS_AGGREGATE() \

#define ADD_VEHICLEATTACHMENTS_AGGREGATE() \
	ADD_ATTACHMENTS_AGGREGATE() \
		ADD_STRINGPROP_FLAG(Driver,		NO_ATTACHMENT,		PF_GROUP6|PF_STATICLIST) \

#define ADD_HELICOPTERATTACHMENTS_AGGREGATE() \
	ADD_VEHICLEATTACHMENTS_AGGREGATE() \
		ADD_STRINGPROP_FLAG(Driver,			"Helicopter Window",	PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(FrontBelly,		NO_ATTACHMENT,			PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RearBelly,		NO_ATTACHMENT,			PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightSkid,		NO_ATTACHMENT,			PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftSkid,		NO_ATTACHMENT,			PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightGunner,	NO_ATTACHMENT,			PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(TopRotor,		NO_ATTACHMENT,			PF_GROUP6|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(BackRotor,		NO_ATTACHMENT,			PF_GROUP6|PF_STATICLIST) \

#define ADD_ANIMALATTACHMENTS_AGGREGATE() \
	ADD_ATTACHMENTS_AGGREGATE() \

#define ADD_SHARKATTACHMENTS_AGGREGATE() \
	ADD_ANIMALATTACHMENTS_AGGREGATE() \
		ADD_STRINGPROP_FLAG(Mouth,			"Shark Bite",			PF_GROUP6|PF_STATICLIST) \

#define MAX_ATTACHMENT_POSITIONS		16

#define ATTACHMENT_TYPE_PROP			0
#define ATTACHMENT_TYPE_OBJECT			1
#define ATTACHMENT_TYPE_WEAPON			100 // not directly specifiable in butes file

#define ATTACHMENTS_TYPE_HUMAN			100
#define ATTACHMENTS_TYPE_PLAYER			200
#define ATTACHMENTS_TYPE_ANIMAL			300
#define ATTACHMENTS_TYPE_SHARK			310
#define ATTACHMENTS_TYPE_VEHICLE		400
#define ATTACHMENTS_TYPE_HELICOPTER		410

class CHHWeaponModel;
class Prop;
class Body;
class CAttachmentPosition;

class CAttachment
{
	public :

		// Create/Destroy

		static CAttachment* Create(uint32 nAttachmentType);
		static void Destroy(CAttachment* pAttachment);

		// Ctors/dtors/etc

		CAttachment();
		virtual ~CAttachment();

		virtual void Init(HOBJECT hObject, HOBJECT hModel, int nAttachmentID);

		// Methods

		virtual void CreateSpawnString(char* szSpawn) = 0;

		// Handlers

		virtual LTBOOL HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom) = 0;

		// Engine stuff

		virtual uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead) { return 0; }

		virtual void Save(HMESSAGEWRITE hWrite);
		virtual void Load(HMESSAGEREAD hRead);

		// Simple accessors

		HOBJECT GetObject() { return m_hObject; }
		HOBJECT GetModel() { return m_hModel; }
		int GetID() { return m_nAttachmentID; }

		virtual uint32 GetType() = 0;

	protected :

		int				m_nAttachmentID;	// The ID in the attachments butes file.
		HOBJECT			m_hObject;			// The object we're attaching to
		HOBJECT			m_hModel;			// The engine object handle to the attachment.
};

class CAttachmentProp : public CAttachment
{
	public :

		// Methods

		void CreateSpawnString(char* szSpawn);

		// Handlers

		LTBOOL HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom);

		// Simple accessors

		uint32 GetType() { return ATTACHMENT_TYPE_PROP; }
};

class CAttachmentObject : public CAttachment
{
	public :

		// Methods

		void CreateSpawnString(char* szSpawn);

		// Handlers

		LTBOOL HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom);

		// Simple accessors

		uint32 GetType() { return ATTACHMENT_TYPE_OBJECT; }
};

class CAttachmentWeapon : public CAttachment
{
	public :

		// Ctors/dtors/etc

		void Init(HOBJECT hObject, HOBJECT hWeaponModel, int nAttachmentID, int nWeaponID, int nAmmoID);

		// Methods

		void CreateSpawnString(char* szSpawn);

		// Handlers

		LTBOOL HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom);

		// Engine stuff

		uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// Simple accessors

		uint32 GetType() { return ATTACHMENT_TYPE_WEAPON; }
		CWeapons* GetWeapons() { return &m_Weapons; }

	protected :

		CWeapons	m_Weapons;			// The weapons aggregate for this weapon attachment
};

class CAttachmentPosition
{
	public :

		// Ctors/dtors/etc

		CAttachmentPosition();
		~CAttachmentPosition();

		// Engine stuff

		uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// Remove

		void RemoveWeapon();

		// Simple accessors

		void SetAttachment(CAttachment* pAttachment) { m_pAttachment = pAttachment; }
		CAttachment* GetAttachment() { return m_pAttachment; }
		LTBOOL HasAttachment() { return !!m_pAttachment; }

		const char* GetName() { return m_szName; }
		void SetName(const char* szName) { m_szName = szName; }

		void SetAttachmentName(HSTRING hstrAttachmentName) { m_hstrAttachmentName = hstrAttachmentName; }
		HSTRING GetAttachmentName() { return m_hstrAttachmentName; }
		LTBOOL HasAttachmentName() { return !!m_hstrAttachmentName; }

	protected :

		const char*		m_szName;				// The name of the position. Assumes pointing to a const global string.
		HSTRING			m_hstrAttachmentName;	// The name of an attachment specified at this position
		CAttachment*	m_pAttachment;			// The attachment, if any, on this position
};

class CAttachments : public IAggregate
{
	public :

		// Create/Destroy

		static CAttachments* Create(uint32 nAttachmentsType);
		static void Destroy(CAttachments* pAttachments);

		// Ctors/dtors/etc

		CAttachments();
		virtual ~CAttachments();

		int EnumerateWeapons(CWeapon** apWeapons, CAttachmentPosition** apAttachmentPositions, int cMaxWeapons);
		int EnumerateProps(Prop** apProps, CAttachmentPosition** apAttachmentPositions, int cMaxProps);
		int EnumerateObjects(BaseClass** apObjects, CAttachmentPosition** apAttachmentPositions, int cMaxObjects);

		LTBOOL HasWeapons() { return m_cWeapons > 0; }
		LTBOOL HasProps() { return m_cProps > 0; }
		LTBOOL HasObjects() { return m_cObjects > 0; }

		void GetInfiniteAmmo();

		virtual void HandleDeath();
        virtual LTBOOL HandleProjectileImpact(CProjectile* pProjectile, IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom, ModelSkeleton eModelSkeleton, ModelNode eModelNode);

		virtual void Init(HOBJECT hObject);
		virtual void ReInit(HOBJECT hObject);

		void ResetRequirements();
		void AddRequirements(ModelId eModelId, ModelStyle eModelStyle);

		virtual void Attach(const char* szAttachmentPosition, const char* szAttachment);
		virtual void Detach(const char* szAttachmentPosition);

		// Simple accessors

		virtual uint32 GetType() = 0;

	protected :

		// Engine message functions

		uint32 EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData);
		uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		// Methods

		virtual void	Update();

		virtual void	CreateAttachment(CAttachmentPosition *pAttachmentPosition);
        void            CreateWeaponAttachment(CAttachmentPosition *pAttachmentPosition, const char* szAttachmentName, uint8 nWeaponID, uint8 nAmmoID);
		void			CreateObjectAttachment(CAttachmentPosition *pAttachmentPosition, int nAttachmentID);
		void			CreatePropAttachment(CAttachmentPosition *pAttachmentPosition, int nAttachmentID);

		// Handlers

		virtual void	HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);

		// Engine stuff

		virtual LTBOOL	ReadProp(LPBASECLASS pObject, ObjectCreateStruct *pInfo);
        virtual void    Save(HMESSAGEWRITE hWrite, uint8 nType);
        virtual void    Load(HMESSAGEREAD hRead, uint8 nType);

	protected :

		HOBJECT					m_hObject;	// The object I'm associated with
		CAttachmentPosition*	m_apAttachmentPositions[MAX_ATTACHMENT_POSITIONS];		// Array of pointers to attachment positions
		int						m_cAttachmentPositions;									// This should equal the size of the arrow below
		int						m_cWeapons;												// A count of our weapon attachments
		int						m_cObjects;												// A count of our Object attachments
		int						m_cProps;												// A count of our Prop attachments
};

class CHumanAttachments : public CAttachments
{
	public :

		// Ctor/dtor/etc

		CHumanAttachments();

		// Handlers

		void HandleCheatBigGuns();

		// Simple accessors

		CAttachmentPosition& GetRightHand() { return m_RightHand; }
		CAttachmentPosition& GetBack() { return m_Back; }

		uint32 GetType() { return ATTACHMENTS_TYPE_HUMAN; }

	protected :

		CAttachmentPosition		m_LeftHand;			// Our attachment positions...
		CAttachmentPosition		m_RightHand;
		CAttachmentPosition		m_Head;
		CAttachmentPosition		m_Eyes;
		CAttachmentPosition		m_Nose;
		CAttachmentPosition		m_Chin;
		CAttachmentPosition		m_Back;
		CAttachmentPosition		m_LeftFoot;
		CAttachmentPosition		m_RightFoot;
		CAttachmentPosition		m_Motorcycle;
		CAttachmentPosition		m_Snowmobile;
};

class CPlayerAttachments : public CHumanAttachments
{
	public :

		// Ctors/dtors/etc

		CPlayerAttachments();

		// Weapons aggregate methods

        inline CWeapon* GetWeapon(uint8 nWeaponId)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				return pAttachmentWeapon->GetWeapons()->GetWeapon(nWeaponId);
			}
			else
			{
				return NULL;
			}
		}

		inline CWeapon* GetWeapon()
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				return pAttachmentWeapon->GetWeapons()->GetCurWeapon();
			}
			else
			{
				return NULL;
			}
		}

		inline int GetAmmoCount(int iAmmoType)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				return pAttachmentWeapon->GetWeapons()->GetAmmoCount(iAmmoType);
			}
			else
			{
				return 0;
			}
		}

		inline void DeselectWeapon()
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->DeselectCurWeapon();
			}
		}

		inline void ChangeWeapon(int nWeaponID)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->ChangeWeapon(nWeaponID);
			}
		}

		inline void ObtainWeapon(int nWeaponID, int nAmmoID = AMMO_DEFAULT_ID,
            int nDefaultAmmo = -1, LTBOOL bNotifyClient=LTFALSE)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->ObtainWeapon(nWeaponID, nAmmoID, nDefaultAmmo, bNotifyClient);
			}
		}

		inline void AddAmmo(int nAmmoId, int nAmount)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->AddAmmo(nAmmoId, nAmount);
			}
		}

		inline void ResetAllWeapons()
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->Reset();
			}
		}

		// Handlers

		void HandleCheatFullAmmo();
		void HandleCheatFullWeapon();
		void HandleCheatFullMods();

		// Simple accessors

		uint32 GetType() { return ATTACHMENTS_TYPE_PLAYER; }

	protected :

		inline CAttachmentWeapon* GetDefaultAttachmentWeapon()
		{
			return (CAttachmentWeapon*)m_RightHand.GetAttachment();
		}

};

class CVehicleAttachments : public CAttachments
{
	public :

		// Ctor/dtor/etc

		CVehicleAttachments();

		// Simple accessors

		uint32 GetType() { return ATTACHMENTS_TYPE_VEHICLE; }

	protected :

		CAttachmentPosition		m_Driver;
};

class CHelicopterAttachments : public CVehicleAttachments
{
	public :

		// Ctor/dtor/etc

		CHelicopterAttachments();

		// Simple accessors

		uint32 GetType() { return ATTACHMENTS_TYPE_HELICOPTER; }

	protected :

		CAttachmentPosition		m_FrontBelly;		// Our attchment positions...
		CAttachmentPosition		m_RearBelly;
		CAttachmentPosition		m_LeftSkid;
		CAttachmentPosition		m_RightSkid;
		CAttachmentPosition		m_RightGunner;
		CAttachmentPosition		m_TopRotor;
		CAttachmentPosition		m_BackRotor;
};

class CAnimalAttachments : public CAttachments
{
	public :

		// Ctor/dtor/etc

		CAnimalAttachments() {}

		// Simple accessors

		uint32 GetType() { return ATTACHMENTS_TYPE_ANIMAL; }
};


class CSharkAttachments : public CAnimalAttachments
{
	public :

		// Ctor/dtor/etc

		CSharkAttachments();

		// Simple accessors

		uint32 GetType() { return ATTACHMENTS_TYPE_SHARK; }

	protected :

		CAttachmentPosition		m_Mouth;
};

// Attachments plugin definition

class CAttachmentsPlugin : public IObjectPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

		void PopulateStringList(char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

	private :

		static LTBOOL				sm_bInitted;
		static CAttachButeMgr		sm_AttachButeMgr;
		static CWeaponMgrPlugin		sm_WeaponMgrPlugin;
};

class CHumanAttachmentsPlugin : public CAttachmentsPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

};

class CVehicleAttachmentsPlugin : public CAttachmentsPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

};

class CHelicopterAttachmentsPlugin : public CVehicleAttachmentsPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

};

class CAnimalAttachmentsPlugin : public CAttachmentsPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

};

class CSharkAttachmentsPlugin : public CAnimalAttachmentsPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

};

#endif // __ATTACHMENTS_H__