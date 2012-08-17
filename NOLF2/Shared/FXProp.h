//------------------------------------------------------------------
//
//   MODULE  : FXPROP.H
//
//   PURPOSE : Defines the structure FX_PROP which is a generic
//				data holder for all possible parameter types
//				to an effect.
//
//   CREATED : 10/25/01
//
//------------------------------------------------------------------
#ifndef __FXPROP_H__
#define __FXPROP_H__

	struct FX_PROP
	{
		struct FX_CLRKEY
		{
			float							m_tmKey;
			uint32							m_dwCol;
		};

		// Enumerations
		enum eDataType
		{
			STRING,
			INTEGER,
			FLOAT,
			COMBO,
			VECTOR,
			VECTOR4,
			CLRKEY,
			PATH,
			ENUM
		};

		// Constructor

											FX_PROP()
											{
												memset(this, 0, sizeof(FX_PROP));
											}

		// Member Functions

		void								Enum(char *sName, int value)
											{
												strcpy(m_sName, sName);
												m_nType = eDataType::ENUM;
												m_data.m_nVal = value;
											}
		
		void								Int(char *sName, int value) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::INTEGER;
												m_data.m_nVal = value;
											}

		void								Float(char *sName, float value) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::FLOAT;
												m_data.m_fVal = value;
											}

		void								String(char *sName, char *sString) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::STRING;												
												strcpy(m_data.m_sVal, sString);
											}

		void								Combo(char *sName, char *sString) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::COMBO;
												strcpy(m_data.m_sVal, sString);
											}

		void								Vector(char *sName, float *pfVec) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::VECTOR;
												m_data.m_fVec[0] = pfVec[0];
												m_data.m_fVec[1] = pfVec[1];
												m_data.m_fVec[2] = pfVec[2];
											}

		void								Vector4(char *sName, float *pfVec4) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::VECTOR;
												m_data.m_fVec[0] = pfVec4[0];
												m_data.m_fVec[1] = pfVec4[1];
												m_data.m_fVec[2] = pfVec4[2];
												m_data.m_fVec[3] = pfVec4[3];
											}

		void								ClrKey(char *sName, float tmKey, uint32 dwCol)
											{
												strcpy(m_sName, sName);
												m_nType = eDataType::CLRKEY;
												m_data.m_clrKey.m_tmKey = tmKey;
												m_data.m_clrKey.m_dwCol = dwCol;
											}

		void								Path(char *sName, char *sString) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::PATH;
												strcpy(m_data.m_sVal, sString);
											}

		inline int							GetIntegerVal( ) const
											{
												assert(m_nType == eDataType::INTEGER);
												return m_data.m_nVal;
											}

		inline float						GetFloatVal( ) const
											{
												assert(m_nType == eDataType::FLOAT);
												return m_data.m_fVal;
											}

		inline int							GetComboVal(  )
											{
												assert(m_nType == eDataType::COMBO);
												return atoi( strtok( m_data.m_sVal, "," ));
											}

		inline LTVector						GetVector( ) const
											{
												assert(m_nType == eDataType::VECTOR);
												return LTVector( m_data.m_fVec[0], m_data.m_fVec[1], m_data.m_fVec[2] );
											}


		inline void							GetPath( char * const szPathOut )
											{
												// Get the path name
												assert(m_nType == eDataType::PATH);

												char *sExt  = strtok( m_data.m_sVal, "|" );
												char *sPath = strtok( LTNULL, "|" );
												
												if( sPath && _stricmp( sPath, "..." ))
													SAFE_STRCPY( szPathOut, sPath );
											}

		inline void							GetStringVal( char * const szStrOut )
											{
												assert(m_nType == eDataType::STRING);
												SAFE_STRCPY( szStrOut, m_data.m_sVal );
											}

													
		// Member Variables


		char								m_sName[128];
		eDataType							m_nType;
		union								FX_DATATYPE
		{
			char							m_sVal[128];
			int								m_nVal;
			float							m_fVal;
			float							m_fVec[3];
			float							m_fVec4[4];
			FX_CLRKEY						m_clrKey;
		}									m_data;
	};

#endif // __FXPROP_H__