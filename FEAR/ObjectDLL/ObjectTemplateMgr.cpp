//////////////////////////////////////////////////////////////////////////////
// CObjectTemplateMgr implementation file

#include "Stdafx.h"

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
		return NULL;
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
		SAVE_VECTOR( ocs.m_Pos );
		SAVE_FLOAT( ocs.m_Scale );
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
				SAVE_CHARSTRING( ocs.m_ChildModels[i] );
			}
		};

		{
			for( int i = 0; i < MAX_MATERIALS_PER_MODEL; ++i )
			{
				SAVE_CHARSTRING( ocs.m_Materials[i] );
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

			//save out our type
			SAVE_INT( pGenProp->GetType() );

			//now save out the data associated with that type
			switch(pGenProp->GetType())
			{
			case LT_PT_STRING:
				SAVE_CHARSTRING( pGenProp->GetString() );
				break;
			case LT_PT_VECTOR:
				SAVE_VECTOR( pGenProp->GetVector() );
				break;
			case LT_PT_COLOR:
				SAVE_VECTOR( pGenProp->GetColor() );
				break;
			case LT_PT_REAL:
				SAVE_FLOAT( pGenProp->GetReal() );
				break;
			case LT_PT_BOOL:
				SAVE_bool( pGenProp->GetBool() );
				break;
			case LT_PT_LONGINT:
				SAVE_INT( pGenProp->GetLongInt() );
				break;
			case LT_PT_ROTATION:
				SAVE_ROTATION( pGenProp->GetRotation() );
				break;
			case LT_PT_COMMAND:
				SAVE_CHARSTRING( pGenProp->GetCommand() );
				break;
			case LT_PT_STRINGID:
				SAVE_CHARSTRING( pGenProp->GetStringID() );
				break;
			default:
				LTERROR("Error: Invalid property type found");
				break;
			}
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
		LOAD_VECTOR( ocs.m_Pos );
		LOAD_FLOAT( ocs.m_Scale );
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
				LOAD_CHARSTRING( ocs.m_ChildModels[i], MAX_CS_FILENAME_LEN );
			}
		};

		{
			for( int i = 0; i < MAX_MATERIALS_PER_MODEL; ++i )
			{
				LOAD_CHARSTRING( ocs.m_Materials[i], MAX_CS_FILENAME_LEN );
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
			char szName[MAX_PROP_NAME_LEN] = {0};
			LOAD_CHARSTRING( szName, LTARRAYSIZE(szName) );

			//get this property
			GenericProp* pProp = ocs.m_cProperties.AddProp(szName);
			
			uint32 nPropType;
			LOAD_INT( nPropType );

			//TODO: Currently this limits the length of the strings. A solution should be added that supports
			//unlimited length strings
			char szString[512+1] = {0};

			switch(nPropType)
			{
			case LT_PT_STRING:
				{
					LOAD_CHARSTRING(szString, LTARRAYSIZE(szString));
					if(pProp)
						pProp->SetString(szString);
				}
				break;
			case LT_PT_VECTOR:
				{
					LTVector vVec;
					LOAD_VECTOR(vVec);
					if(pProp)
						pProp->SetVector(vVec);
				}					
				break;
			case LT_PT_COLOR:
				{
					LTVector vVec;
					LOAD_VECTOR(vVec);
					if(pProp)
						pProp->SetColor(vVec);
				}			
				break;
			case LT_PT_REAL:
				{
					float fVal;
					LOAD_FLOAT(fVal);
					if(pProp)
						pProp->SetReal(fVal);
				}			
				break;
			case LT_PT_BOOL:
				{
					bool bVal;
					LOAD_bool(bVal);
					if(pProp)
						pProp->SetBool(bVal);
				}		
				break;
			case LT_PT_LONGINT:
				{
					int32 nVal;
					LOAD_INT(nVal);
					if(pProp)
						pProp->SetLongInt(nVal);
				}		
				break;
			case LT_PT_ROTATION:
				{
					LTRotation rVal;
					LOAD_ROTATION(rVal);
					if(pProp)
						pProp->SetRotation(rVal);
				}		
				break;
			case LT_PT_COMMAND:
				{
					LOAD_CHARSTRING(szString, LTARRAYSIZE(szString));
					if(pProp)
						pProp->SetCommand(szString);
				}
				break;
			case LT_PT_STRINGID:
				{
					LOAD_CHARSTRING(szString, LTARRAYSIZE(szString));
					if(pProp)
						pProp->SetStringID(szString);
				}
				break;
			default:
				LTERROR("Error: Invalid property type found");
				break;
			}
		}

		AddTemplate( &ocs );
	}
}
