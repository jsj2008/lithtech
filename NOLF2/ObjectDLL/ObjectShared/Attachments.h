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
	PROP_DEFINEGROUP(Attachments, PF_GROUP(6)) \

#define ADD_HUMANATTACHMENTS_AGGREGATE() \
	ADD_ATTACHMENTS_AGGREGATE() \
		ADD_STRINGPROP_FLAG(LeftHand,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightHand,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Head,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Eyes,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Nose,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Chin,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Back,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftFoot,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightFoot,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Snowmobile,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Light,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftLowerArm,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftUpperArm,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightLowerArm,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightUpperArm,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftLowerLeg,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftUpperLeg,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightLowerLeg,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightUpperLeg,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(UpperTorso,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Torso,			NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftShoulder,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightShoulder,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightHand2,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftHand2,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftLowerArm2,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftLowerLeg2,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftUpperArm2,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftUpperLeg2,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightUpperArm2,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightUpperLeg2,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightLowerArm2,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightLowerLeg2,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(UpperTorso2,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \

#define ADD_PLAYERATTACHMENTS_AGGREGATE() \
	ADD_HUMANATTACHMENTS_AGGREGATE() \

#define ADD_VEHICLEATTACHMENTS_AGGREGATE() \
	ADD_ATTACHMENTS_AGGREGATE() \
		ADD_STRINGPROP_FLAG(Driver,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \

#define ADD_HELICOPTERATTACHMENTS_AGGREGATE() \
	ADD_VEHICLEATTACHMENTS_AGGREGATE() \
		ADD_STRINGPROP_FLAG(Driver,			"Helicopter Window",	PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(FrontBelly,		NO_ATTACHMENT,			PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RearBelly,		NO_ATTACHMENT,			PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightSkid,		NO_ATTACHMENT,			PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(LeftSkid,		NO_ATTACHMENT,			PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(RightGunner,	NO_ATTACHMENT,			PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(TopRotor,		NO_ATTACHMENT,			PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(BackRotor,		NO_ATTACHMENT,			PF_GROUP(6)|PF_STATICLIST) \

#define ADD_ANIMALATTACHMENTS_AGGREGATE() \
	ADD_ATTACHMENTS_AGGREGATE() \

#define ADD_SHARKATTACHMENTS_AGGREGATE() \
	ADD_ANIMALATTACHMENTS_AGGREGATE() \
		ADD_STRINGPROP_FLAG(Mouth,			"Shark Bite",			PF_GROUP(6)|PF_STATICLIST) \

const int MAX_ATTACHMENT_POSITIONS		= 37;

const char* const KEY_ATTACH = "ATTACH";
const char* const KEY_DETACH = "DETACH";

enum AttachmentType
{
	ATTACHMENT_TYPE_PROP	= 0, 
	ATTACHMENT_TYPE_OBJECT	= 1,
	ATTACHMENT_TYPE_WEAPON	= 100, // not directly specifiable in butes file
};

#define ATTACHMENTS_TYPE_HUMAN			100
#define ATTACHMENTS_TYPE_PLAYER			200

class CHHWeaponModel;
class Prop;
class Body;
class CAttachmentPosition;

class CAttachment : public ILTObjRefReceiver
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
		virtual	void CreateAttachString(CAttachmentPosition* pAttachmentPosition, char* szAttach) {szAttach[0] = '\0';}


		// Handlers

		virtual LTBOOL HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo,
											  const LTVector& vDir, const LTVector& vFrom, ModelSkeleton eModelSkeleton, ModelNode eModelNode) = 0;

		// Engine stuff

		virtual uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg) { return 0; }

		virtual void Save(ILTMessage_Write *pMsg);
		virtual void Load(ILTMessage_Read *pMsg);

		// Simple accessors

		HOBJECT GetObject() { return m_hObject; }
		HOBJECT GetModel() { return m_hModel; }
		int GetID() { return m_nAttachmentID; }

		virtual AttachmentType GetType() = 0;

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );


	protected :

		int				m_nAttachmentID;	// The ID in the attachments butes file.
		LTObjRef		m_hObject;			// The object we're attaching to
		LTObjRefNotifier	m_hModel;			// The engine object handle to the attachment.
		uint8			m_nDebrisID;		// The ID of the debris to create when shot off.
};

class CAttachmentProp : public CAttachment
{
	public :

		// Methods

		void CreateSpawnString(char* szSpawn);

		// Handlers

		LTBOOL HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, 
									  const LTVector& vDir, const LTVector& vFrom, ModelSkeleton eModelSkeleton, ModelNode eModelNode);

		// Simple accessors

		AttachmentType GetType() { return ATTACHMENT_TYPE_PROP; }
};

class CAttachmentObject : public CAttachment
{
	public :

		// Methods

		void CreateSpawnString(char* szSpawn);

		// Handlers

		LTBOOL HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, 
									  const LTVector& vDir, const LTVector& vFrom, ModelSkeleton eModelSkeleton, ModelNode eModelNode);

		// Simple accessors

		AttachmentType GetType() { return ATTACHMENT_TYPE_OBJECT; }
};

class CAttachmentWeapon : public CAttachment
{
	public :

		// Ctors/dtors/etc

		void Init(HOBJECT hObject, HOBJECT hWeaponModel, int nAttachmentID, int nWeaponID, int nAmmoID);

		// Methods

		void CreateSpawnString(char* szSpawn);
		void CreateAttachString(CAttachmentPosition* pAttachmentPosition, char* szAttach);

		// Handlers

		LTBOOL HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, 
									  const LTVector& vDir, const LTVector& vFrom, ModelSkeleton eModelSkeleton, ModelNode eModelNode);

		// Engine stuff

		uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		// Simple accessors

		AttachmentType GetType() { return ATTACHMENT_TYPE_WEAPON; }
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

		uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		// Remove

		void RemoveWeapon();

		// Simple accessors

		void SetAttachment(CAttachment* pAttachment) { m_pAttachment = pAttachment; }
		CAttachment* GetAttachment() { return m_pAttachment; }
		LTBOOL HasAttachment() { return !!m_pAttachment; }

		const char* GetName() { return m_szName; }
		void SetName(const char* szName) { m_szName = szName; }

		void SetAttachmentName(HSTRING hstrAttachmentName) { FREE_HSTRING(m_hstrAttachmentName); m_hstrAttachmentName = hstrAttachmentName; }
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
        virtual LTBOOL HandleProjectileImpact(CProjectile* pProjectile, IntersectInfo& iInfo, const LTVector& vDir, const LTVector& vFrom, ModelSkeleton eModelSkeleton, ModelNode eModelNode);

		virtual void Init(HOBJECT hObject);
		virtual void ReInit(HOBJECT hObject);

		void ResetRequirements();
		void AddRequirements(ModelId eModelId);

		virtual void Attach(const char* szAttachmentPosition, const char* szAttachment);
		virtual void Detach(const char* szAttachmentPosition);
		CAttachment* GetAttachment(const char* szAttachmentPosition);

		void AddToObjectList( ObjectList *pObjList, eObjListControl eControl = eObjListNODuplicates );

		void HideAttachments(LTBOOL bHide);
		
		// This will remove all attachments associated with the model and reattach them.  Useful when switching filenames on a character.
		virtual void RemoveAndRecreateAttachments();
		
		// Detach all the attachments of specified type...
		virtual void DetachAttachmentsOfType( AttachmentType eType );

		// Simple accessors

		virtual uint32 GetType() = 0;

	protected :

		// Engine message functions

		uint32 EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData);
		uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);

		// Methods

		virtual void	Update();

		virtual void	CreateAttachment(CAttachmentPosition *pAttachmentPosition);
        void            CreateWeaponAttachment(CAttachmentPosition *pAttachmentPosition, const char* szAttachmentName, uint8 nWeaponID, uint8 nAmmoID);
		void			CreateObjectAttachment(CAttachmentPosition *pAttachmentPosition, int nAttachmentID);
		void			CreatePropAttachment(CAttachmentPosition *pAttachmentPosition, int nAttachmentID);

		// Engine stuff

		virtual LTBOOL	ReadProp(LPBASECLASS pObject, ObjectCreateStruct *pInfo);
        virtual void    Save(ILTMessage_Write *pMsg, uint8 nType);
        virtual void    Load(ILTMessage_Read *pMsg, uint8 nType);

		// Used to ensure the attachment follows the hidden state of the attachments
		void			SetAttachmentHiddenState( CAttachment& attachment )
		{
			g_pCommonLT->SetObjectFlags( attachment.GetModel( ), OFT_Flags, m_bHidden ? FLAG_FORCECLIENTUPDATE : FLAG_VISIBLE, FLAG_FORCECLIENTUPDATE | FLAG_VISIBLE );
		}

	protected :

		LTObjRef				m_hObject;	// The object I'm associated with
		CAttachmentPosition*	m_apAttachmentPositions[MAX_ATTACHMENT_POSITIONS];		// Array of pointers to attachment positions
		int						m_cAttachmentPositions;									// This should equal the size of the arrow below
		int						m_cWeapons;												// A count of our weapon attachments
		int						m_cObjects;												// A count of our Object attachments
		int						m_cProps;												// A count of our Prop attachments
		bool					m_bHidden;												// Indicates attachments are in a hidden state.
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
		CAttachmentPosition		m_Snowmobile;
		CAttachmentPosition		m_Light;
		CAttachmentPosition		m_LeftLowerArm;
		CAttachmentPosition		m_LeftUpperArm;
		CAttachmentPosition		m_RightLowerArm;
		CAttachmentPosition		m_RightUpperArm;
		CAttachmentPosition		m_LeftLowerLeg;
		CAttachmentPosition		m_LeftUpperLeg; 
		CAttachmentPosition		m_RightLowerLeg; 
		CAttachmentPosition		m_RightUpperLeg; 
		CAttachmentPosition		m_UpperTorso; 
		CAttachmentPosition		m_Torso;

		CAttachmentPosition		m_LeftShoulder;
		CAttachmentPosition		m_RightShoulder;

		CAttachmentPosition		m_RightHand2;
		CAttachmentPosition		m_LeftHand2;
		CAttachmentPosition		m_LeftLowerArm2;
		CAttachmentPosition		m_LeftLowerLeg2;
		CAttachmentPosition		m_LeftUpperArm2;
		CAttachmentPosition		m_LeftUpperLeg2;
		CAttachmentPosition		m_RightUpperArm2;
		CAttachmentPosition		m_RightUpperLeg2;
		CAttachmentPosition		m_RightLowerArm2;
		CAttachmentPosition		m_RightLowerLeg2;
		CAttachmentPosition		m_UpperTorso2;

	private:
		void AddPosition(const char* szName,CAttachmentPosition* pAttachPosition);

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
				SetWeaponAttachmentHiddenState( *pAttachmentWeapon );
			}
		}

		inline void ChangeWeapon(int nWeaponID)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->ChangeWeapon(nWeaponID);
				SetWeaponAttachmentHiddenState( *pAttachmentWeapon );
			}
		}

		inline void ObtainWeapon(int nWeaponID, int nAmmoID = AMMO_DEFAULT_ID,
            int nDefaultAmmo = -1, LTBOOL bNotifyClient=LTFALSE)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->ObtainWeapon(nWeaponID, nAmmoID, nDefaultAmmo, bNotifyClient);
				SetWeaponAttachmentHiddenState( *pAttachmentWeapon );
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

		inline void SetAmmo(int nAmmoId, int nAmount)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->SetAmmo(nAmmoId, nAmount);
			}
		}

		inline void ResetAllWeapons()
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->Reset();
				SetWeaponAttachmentHiddenState( *pAttachmentWeapon );
			}
		}

		// Handlers

		void HandleCheatFullAmmo();
		void HandleCheatFullWeapon();
		void HandleCheatFullMods();
		
		bool AcquireWeapon( uint8 nId );
		bool AcquireMod( uint8 nId , bool bDisplayMsg = true);
		bool AcquireAmmo( uint8 nId );

		// Simple accessors

		uint32 GetType() { return ATTACHMENTS_TYPE_PLAYER; }

	protected :

		inline CAttachmentWeapon* GetDefaultAttachmentWeapon()
		{
			return (CAttachmentWeapon*)m_RightHand.GetAttachment();
		}
		
		void SetWeaponAttachmentHiddenState( CAttachment& AttachmentWeapon );
};

// Attachments plugin definition

#ifndef __PSX2
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
#endif // __PSX2

#endif // __ATTACHMENTS_H__
