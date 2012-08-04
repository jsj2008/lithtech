// ----------------------------------------------------------------------- //
//
// MODULE  : Attachments.h
//
// PURPOSE : Attachments aggregate object - Definition
//
// CREATED : 
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __ATTACHMENTS_H__
#define __ATTACHMENTS_H__

#include "iaggregate.h"
#include "iobjectplugin.h"

#define NO_ATTACHMENT	"<none>"
#define SOCKET_ATTACHMENT	"<socket>"

#define ADD_ATTACHMENTS_AGGREGATE( group ) \
	PROP_DEFINEGROUP(Attachments, group, "This is a subset of properties that allows you to add attachments to the object.") \
		ADD_STRINGPROP_FLAG( Attachment0, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment1, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment2, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment3, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment4, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment5, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment6, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment7, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment8, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment9, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment10, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment11, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment12, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment13, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment14, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \
		ADD_STRINGPROP_FLAG( Attachment15, NO_ATTACHMENT, group | PF_STATICLIST, "The name of an attachment, specified in attachments.txt, that will be attached to this object." ) \

#define MAX_ATTACHMENT_NAMES 16


const char* const KEY_ATTACH = "ATTACH";
const char* const KEY_DETACH = "DETACH";

enum AttachmentType
{
	ATTACHMENT_TYPE_PROP	= 0, 
	ATTACHMENT_TYPE_OBJECT	= 1,
	ATTACHMENT_TYPE_INVALID = 2
};


class Prop;
class CAttachmentPosition;
class CProjectile;


class CAttachment : public ILTObjRefReceiver
{
	public :

		// Create/Destroy

		static CAttachment* Create(uint32 nAttachmentType);
		static void Destroy(CAttachment* pAttachment);

		// Ctors/dtors/etc

		CAttachment();
		virtual ~CAttachment();

		virtual void Init(HOBJECT hObject, HOBJECT hModel, HRECORD hAttachmentRecord);

		// Methods

		virtual void CreateSpawnString(char* szSpawn, uint32 nSpawnSize) = 0;
		virtual HCATEGORY GetAttachmentCategory() = 0;

		// Handlers

		virtual bool HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo,
			const LTVector& vDir, const LTVector& vFrom, ModelsDB::HSKELETON hModelSkeleton, ModelsDB::HNODE hModelNode) = 0;

		virtual void HandleSever(CAttachmentPosition* pAttachmentPosition, const LTVector& vSeverDir ) = 0;

		// Engine stuff

		virtual void Save(ILTMessage_Write *pMsg);
		virtual void Load(ILTMessage_Read *pMsg);

		// Simple accessors

		HOBJECT GetObject() { return m_hObject; }
		HOBJECT GetModel() { return m_hModel; }
		HRECORD GetID() { return m_hAttachmentRecord; }

		virtual AttachmentType GetType() = 0;

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );


	protected :

		HRECORD			m_hAttachmentRecord;	// The ID in the attachments butes file.
		LTObjRef		m_hObject;			// The object we're attaching to
		LTObjRefNotifier	m_hModel;			// The engine object handle to the attachment.
};

class CAttachmentProp : public CAttachment
{
	public :

		// Methods

		void CreateSpawnString(char* szSpawn, uint32 nSpawnSize);
		HCATEGORY GetAttachmentCategory();

		// Handlers

		bool HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, 
									  const LTVector& vDir, const LTVector& vFrom, ModelsDB::HSKELETON hModelSkeleton, ModelsDB::HNODE hModelNode);

		void HandleSever(CAttachmentPosition* pAttachmentPosition, const LTVector& vSeverDir );


		// Simple accessors

		AttachmentType GetType() { return ATTACHMENT_TYPE_PROP; }
};

class CAttachmentObject : public CAttachment
{
	public :

		// Methods

		void CreateSpawnString(char* szSpawn, uint32 nSpawnSize);
		HCATEGORY GetAttachmentCategory();

		// Handlers

		bool HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, 
									  const LTVector& vDir, const LTVector& vFrom, ModelsDB::HSKELETON hModelSkeleton, ModelsDB::HNODE hModelNode);

		void HandleSever(CAttachmentPosition* pAttachmentPosition, const LTVector& vSeverDir );


		// Simple accessors

		AttachmentType GetType() { return ATTACHMENT_TYPE_OBJECT; }
};

class CAttachmentPosition
{
	public :

		// Ctors/dtors/etc

		CAttachmentPosition();
		~CAttachmentPosition();

		// Engine stuff

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		// Simple accessors

		void SetAttachment(CAttachment* pAttachment) { m_pAttachment = pAttachment; }
		CAttachment*	GetAttachment() const { return m_pAttachment; }
		bool			HasAttachment() const { return !!m_pAttachment; }

		const char*		GetName() { return m_sName.c_str(); }
		void			SetName(const char* szName) { m_sName = szName; }

		void			SetAttachmentName(const char *pszAttachmentName) 
		{ 
			if (pszAttachmentName) m_sAttachmentName = pszAttachmentName;
			else { m_sAttachmentName.clear(); }
		}
		const char*		GetAttachmentName() { return m_sAttachmentName.c_str(); }
		bool			HasAttachmentName() { return !m_sAttachmentName.empty(); }

	protected :

		std::string		m_sName;				// The name of the position.
		std::string		m_sAttachmentName;		// The name of an attachment specified at this position
		CAttachment*	m_pAttachment;			// The attachment, if any, on this position
};

class CAttachments : public IAggregate
{
	public :

		// Create/Destroy

		static CAttachments* Create( );
		static void Destroy(CAttachments* pAttachments);

		// Ctors/dtors/etc

		CAttachments();
		virtual ~CAttachments();

		// Create and destroy using these functions.
		static CAttachmentPosition* CreateAttachmentPosition( );
		static void DestroyAttachmentPosition( CAttachmentPosition* pAttachmentPosition );

		bool			HasProps() { return m_cProps > 0; }
		bool			HasObjects() { return m_cObjects > 0; }

		virtual void HandleDeath();
		virtual bool HandleProjectileImpact(CProjectile* pProjectile, IntersectInfo& iInfo, const LTVector& vDir, const LTVector& vFrom, ModelsDB::HSKELETON hModelSkeleton, ModelsDB::HNODE hModelNode);
		virtual void HandleSever(const char* szAttachmentPosition, const LTVector& vDir);

		virtual void Init(HOBJECT hObject);
		virtual void ReInit(HOBJECT hObject);

		// Deletes all the CAttachmentPosition objects we own.
		void			DeleteAllAttachmentPositions( );

		virtual void	Attach( const char *szAttachment , const char *szPositionOverride = NULL );
		virtual void	Detach(const char *szAttachment, const char *szPositionOverride = NULL );
		CAttachment* GetAttachment(const char* szAttachmentPosition);

		void AddToObjectList( ObjectList *pObjList, eObjListControl eControl = eObjListNODuplicates );

		void			HideAttachments(bool bHide);
		
		// This will remove all attachments associated with the model and reattach them.  Useful when switching filenames on a character.
		virtual void RemoveAndRecreateAttachments();
		
		// Detach all the attachments of specified type...
		virtual void DetachAttachmentsOfType( AttachmentType eType );

	protected :

		// Engine message functions

		uint32			EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float lData);

		// Methods

		virtual void	Update();

		virtual void	CreateAttachment(CAttachmentPosition *pAttachmentPosition);
		void			CreateObjectAttachment(CAttachmentPosition *pAttachmentPosition, HRECORD hAttachmentRecord);
		void			CreatePropAttachment(CAttachmentPosition *pAttachmentPosition, HRECORD hAttachmentRecord);

		// Engine stuff

		virtual bool	ReadProp(LPBASECLASS pObject, const GenericPropList *pProps);
        virtual void    Save(ILTMessage_Write *pMsg, uint8 nType);
        virtual void    Load(ILTMessage_Read *pMsg, uint8 nType);

		// Used to ensure the attachment follows the hidden state of the attachments
		void			SetAttachmentHiddenState( CAttachment& attachment )
		{
			g_pCommonLT->SetObjectFlags( attachment.GetModel( ), OFT_Flags, m_bHidden ? FLAG_FORCECLIENTUPDATE : FLAG_VISIBLE, FLAG_FORCECLIENTUPDATE | FLAG_VISIBLE );
			
			// Keep the shadow LOD consistent with the object the attachments are associated with...
			EEngineLOD eShadowLOD = eEngineLOD_Never;
			g_pLTServer->GetObjectShadowLOD( m_hObject, eShadowLOD );
			g_pLTServer->SetObjectShadowLOD( attachment.GetModel( ), m_bHidden ? eEngineLOD_Never : eShadowLOD );
		}

	protected :

		LTObjRef				m_hObject;	// The object I'm associated with
		int						m_cObjects;												// A count of our Object attachments
		int						m_cProps;												// A count of our Prop attachments
		bool					m_bHidden;												// Indicates attachments are in a hidden state.

		StringArray				m_saAttachmentNames;						// Array of attachments that were specified in WorldEdit.

		//object banks to reduce the number of allocations performed when allocating CAttachmentPosition objects.
		static bool															m_bBankInitialized;
		static ObjectBank<CAttachmentPosition, LT_MEM_TYPE_OBJECTSHELL>		m_AttachmentPositionBank;

		typedef std::vector<CAttachmentPosition*, LTAllocator<CAttachmentPosition*, LT_MEM_TYPE_OBJECTSHELL> > AttachmentPositionArray;
		AttachmentPositionArray	m_vecAttachmentPositions;					// Array of attachment positions.

};

// Attachments plugin definition

class CAttachmentsPlugin : public IObjectPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

		void PopulateStringList(char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
};

#endif // __ATTACHMENTS_H__
