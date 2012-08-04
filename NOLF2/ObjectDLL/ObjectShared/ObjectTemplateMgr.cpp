//////////////////////////////////////////////////////////////////////////////
// CObjectTemplateMgr implementation file

#include "stdafx.h"

#include "ObjectTemplateMgr.h"

CObjectTemplateMgr::CObjectTemplateMgr()
{
}

CObjectTemplateMgr::~CObjectTemplateMgr()
{
}

void CObjectTemplateMgr::AddTemplate(const ObjectCreateStruct *pOCS)
{
	m_cTemplates[pOCS->m_Name] = *pOCS;
	m_cTemplates[pOCS->m_Name].m_cProperties.AddProp("Template", GenericProp(false, LT_PT_BOOL));
}

const ObjectCreateStruct *CObjectTemplateMgr::FindTemplate(const char *pName) const
{
	TTemplateMap::const_iterator iTemplate = m_cTemplates.find(pName);
	if (iTemplate == m_cTemplates.end())
		return LTNULL;
	return &iTemplate->second;
}


void CObjectTemplateMgr::Clear()
{
	m_cTemplates.clear();
}

void CObjectTemplateMgr::Save( ILTMessage_Write *pMsg )
{
	SAVE_INT( m_cTemplates.size() );

	ObjectCreateStruct ocs;
	TTemplateMap::iterator iter; 

	for( iter = m_cTemplates.begin(); iter != m_cTemplates.end(); ++iter )
	{
		ocs = (*iter).second;
		
		SAVE_INT( ocs.m_ObjectType );
		SAVE_INT( ocs.m_ContainerCode );
		SAVE_INT( ocs.m_Flags );
		SAVE_INT( ocs.m_Flags2 );
		SAVE_VECTOR( ocs.m_GlobalForceOverride );
		SAVE_VECTOR( ocs.m_Pos );
		SAVE_VECTOR( ocs.m_Scale );
		SAVE_ROTATION( ocs.m_Rotation );
		SAVE_INT( ocs.m_UserData );
		char aClassName[MAX_OCS_CLASSNAME_LEN];
		g_pLTServer->GetClassName(ocs.m_hClass, aClassName, sizeof(aClassName));
		SAVE_CHARSTRING( aClassName );

//		union 
		{
			SAVE_CHARSTRING( ocs.m_Filename );

			for( int i = 0; i < MAX_CHILD_MODELS; ++i )
			{
				SAVE_CHARSTRING( ocs.m_Filenames[i] );
			}
		};

//		union
		{
			SAVE_CHARSTRING( ocs.m_SkinName );

			for( int i = 0; i < MAX_MODEL_TEXTURES; ++i )
			{
				SAVE_CHARSTRING( ocs.m_SkinNames[i] );
			}
		};

//		union
		{
			SAVE_CHARSTRING( ocs.m_RenderStyleName );

			for( int i = 0; i < MAX_MODEL_RENDERSTYLES; ++i )
			{
				SAVE_CHARSTRING( ocs.m_RenderStyleNames[i] );
			}

		};

		SAVE_CHARSTRING( ocs.m_Name );
		SAVE_FLOAT( ocs.m_NextUpdate );

		// Save all of the properties...
		SAVE_INT( ocs.m_cProperties.GetNumProps() );

		for( uint32 nCurrProp = 0; nCurrProp < ocs.m_cProperties.GetNumProps(); nCurrProp++)
		{
			// Get and save the name of this prop...
					
			std::string strName = (*iter).first;
			SAVE_CHARSTRING( ocs.m_cProperties.GetPropName(nCurrProp) );
						
			const GenericProp* pGenProp = ocs.m_cProperties.GetProp(nCurrProp);

			if(!pGenProp)
				continue;

			SAVE_INT( pGenProp->m_Type );
			SAVE_VECTOR( pGenProp->m_Vec );
			SAVE_VECTOR( pGenProp->m_Color );
			SAVE_CHARSTRING( pGenProp->m_String );
			SAVE_ROTATION( pGenProp->m_Rotation );
			SAVE_INT( pGenProp->m_Long );
			SAVE_FLOAT( pGenProp->m_Float );
			SAVE_bool( pGenProp->m_Bool );
		}

	}
}

void CObjectTemplateMgr::Load( ILTMessage_Read *pMsg )
{
	// Make sure we are empty...

	Clear();
	
	int nNumOCS = 0;
	ObjectCreateStruct ocs;

	// Load in the createstructs and add them to the template list...

	LOAD_INT( nNumOCS );

	for( int nOCS = 0; nOCS < nNumOCS; ++nOCS )
	{
		// Start fresh
		ocs.Clear();

		LOAD_INT( ocs.m_ObjectType );
		LOAD_INT( ocs.m_ContainerCode );
		LOAD_INT( ocs.m_Flags );
		LOAD_INT( ocs.m_Flags2 );
		LOAD_VECTOR( ocs.m_GlobalForceOverride );
		LOAD_VECTOR( ocs.m_Pos );
		LOAD_VECTOR( ocs.m_Scale );
		LOAD_ROTATION( ocs.m_Rotation );
		LOAD_INT( ocs.m_UserData );
		char aClassName[256];
		LOAD_CHARSTRING( aClassName, MAX_OCS_CLASSNAME_LEN );
		ocs.m_hClass = g_pLTServer->GetClass(aClassName);

//		union 
		{
			LOAD_CHARSTRING( ocs.m_Filename, MAX_CS_FILENAME_LEN );

			for( int i = 0; i < MAX_CHILD_MODELS; ++i )
			{
				LOAD_CHARSTRING( ocs.m_Filenames[i], MAX_CS_FILENAME_LEN );
			}
		};

//		union
		{
			LOAD_CHARSTRING( ocs.m_SkinName, MAX_CS_FILENAME_LEN );

			for( int i = 0; i < MAX_MODEL_TEXTURES; ++i )
			{
				LOAD_CHARSTRING( ocs.m_SkinNames[i], MAX_CS_FILENAME_LEN );
			}
		};

//		union
		{
			LOAD_CHARSTRING( ocs.m_RenderStyleName, MAX_CS_FILENAME_LEN );

			for( int i = 0; i < MAX_MODEL_RENDERSTYLES; ++i )
			{
				LOAD_CHARSTRING( ocs.m_RenderStyleNames[i], MAX_CS_FILENAME_LEN );
			}

		};

		LOAD_CHARSTRING( ocs.m_Name, MAX_CS_FILENAME_LEN );
		LOAD_FLOAT( ocs.m_NextUpdate );

		// Fill in the list of properties...
		
		GenericProp genProp;

		int nNumProps = 0;
		LOAD_INT( nNumProps );

		//make sure that the object create struct has enough room to holde our properties (and
		//the template property)
		if(ocs.m_cProperties.GetMaxProps() < (uint32)nNumProps + 1)
		{
			ocs.m_cProperties.ReserveProps(nNumProps + 1, false);
		}

		for( int nProp = 0; nProp < nNumProps; ++nProp )
		{
			// Get the name of this prop...
			
			char szName[MAX_CS_FILENAME_LEN+1] = {0};
			LOAD_CHARSTRING( szName, MAX_CS_FILENAME_LEN );
						
			LOAD_INT( genProp.m_Type );
			LOAD_VECTOR( genProp.m_Vec );
			LOAD_VECTOR( genProp.m_Color );
			LOAD_CHARSTRING( genProp.m_String, MAX_GP_STRING_LEN );
			LOAD_ROTATION( genProp.m_Rotation );
			LOAD_INT( genProp.m_Long );
			LOAD_FLOAT( genProp.m_Float );
			LOAD_bool( genProp.m_Bool );

			// Add the prop to the list..

			ocs.m_cProperties.AddProp( szName, genProp );
		}

		AddTemplate( &ocs );
	}
}