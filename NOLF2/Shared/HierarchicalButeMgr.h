// ----------------------------------------------------------------------- //
//
// MODULE  : HierarchicalButeMgr.h
//
// PURPOSE : HierarchicalButeMgr definition - Base class for all hierarchical bute mgrs (have Parent property)
//
// CREATED : 11/2/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HIERARCHICAL_BUTE_MGR_H__
#define __HIERARCHICAL_BUTE_MGR_H__

//
// Includes...
//

	#include "GameButeMgr.h"


class CHierarchicalButeMgr : public CGameButeMgr
{
		enum MicrosoftHackForInlineConstant
		{
			g_nMaxResursionLevel = 100
		};


	public:

		virtual ~CHierarchicalButeMgr() {}

		virtual LTBOOL Init( const char* szAttributeFile="" )
		{
			if( !szAttributeFile ) return LTFALSE;
			return Parse( szAttributeFile );
		}

		// Indicates previous call found real value rather than just default.
		bool Success( ) { return m_bSuccess; }

		LTBOOL Exist(const char* tag_name, const char* att_name = NULL)
		{
			static int nRecursionLevel = 0;

			if( !tag_name || !tag_name[0] ) return LTFALSE;

			if( ++nRecursionLevel > g_nMaxResursionLevel )
			{
				CString cstrErrorMessage;
				cstrErrorMessage.Format("Max recursion level reached for [%s]:%s.  Circular reference in Parent?",
					tag_name, att_name);
				GBM_DisplayError(cstrErrorMessage);

				--nRecursionLevel;
				return LTFALSE;
			}

			if( m_buteMgr.Exist(tag_name,att_name) )
			{
				--nRecursionLevel;
				return LTTRUE;
			}

			const CString parent_name = m_buteMgr.GetString(tag_name,"Parent",CString(""));
			if( stricmp(parent_name,tag_name)  == 0) 
			{
				--nRecursionLevel;
				return false;
			}

			const LTBOOL bResult = Exist( parent_name, att_name );

			--nRecursionLevel;
			return bResult;
		}

		int GetInt(const char * tag_name, const char * att_name, int def_value = 0)
		{
			static int nRecursionLevel = 0;

			m_bSuccess = false;

			if( !tag_name|| !att_name || !tag_name[0] ) 
				return def_value;

			if( ++nRecursionLevel > g_nMaxResursionLevel )
			{
				CString cstrErrorMessage;
				cstrErrorMessage.Format("Max recursion level reached for [%s]:%s.  Circular reference in Parent?",
					tag_name, att_name);
				GBM_DisplayError(cstrErrorMessage);

				--nRecursionLevel;
				return def_value;
			}

			int nResult = m_buteMgr.GetInt(tag_name,att_name, def_value);
			if( m_buteMgr.Success( ))
			{
				--nRecursionLevel;
				m_bSuccess = true;
				return nResult;
			}

			const CString parent_name = m_buteMgr.GetString(tag_name,"Parent",CString(""));
			if( stricmp(parent_name,tag_name)  == 0) 
			{
				--nRecursionLevel;
				return def_value;
			}

			nResult = GetInt( parent_name, att_name, def_value );

			--nRecursionLevel;
			return nResult;
		}

		double GetDouble(const char * tag_name, const char * att_name, double def_value = 0.0 )
		{
			static int nRecursionLevel = 0;

			m_bSuccess = false;

			if( !tag_name|| !att_name || !tag_name[0] ) 
				return def_value;

			if( ++nRecursionLevel > g_nMaxResursionLevel )
			{
				CString cstrErrorMessage;
				cstrErrorMessage.Format("Max recursion level reached for [%s]:%s.  Circular reference in Parent?",
					tag_name, att_name);
				GBM_DisplayError(cstrErrorMessage);

				--nRecursionLevel;
				return def_value;
			}

			double fResult = m_buteMgr.GetDouble(tag_name,att_name, def_value );
			if( m_buteMgr.Success( ))
			{
				--nRecursionLevel;
				m_bSuccess = true;
				return fResult;
			}

			const CString parent_name = m_buteMgr.GetString(tag_name,"Parent",CString(""));
			if( stricmp(parent_name,tag_name)  == 0) 
			{
				--nRecursionLevel;
				return def_value;
			}
			
			fResult = GetDouble( parent_name, att_name, def_value );
		
			--nRecursionLevel;
			return fResult;
		}


		CString GetString(const char * tag_name, const char * att_name, const CString & def_value = CString("") )
		{
		
			static int nRecursionLevel = 0;

			m_bSuccess = false;

			if( !tag_name|| !att_name || !tag_name[0] ) 
				return def_value;

			if( ++nRecursionLevel > g_nMaxResursionLevel )
			{
				CString cstrErrorMessage;
				cstrErrorMessage.Format("Max recursion level reached for [%s]:%s.  Circular reference in Parent?",
					tag_name, att_name);
				GBM_DisplayError(cstrErrorMessage);

				--nRecursionLevel;
				return def_value;
			}

			CString cstrResult = m_buteMgr.GetString(tag_name,att_name, def_value );
			if( m_buteMgr.Success( ))
			{
				--nRecursionLevel;
				m_bSuccess = true;
				return cstrResult;
			}

			const CString parent_name = m_buteMgr.GetString(tag_name,"Parent",CString(""));
			if( stricmp(parent_name,tag_name)  == 0) 
			{
				--nRecursionLevel;
				return def_value;
			}
			
			cstrResult = GetString( parent_name, att_name, def_value );

			--nRecursionLevel;
			return cstrResult;
		}


		CPoint GetPoint(const char * tag_name, const char * att_name, CPoint & def_value )
		{
			static int nRecursionLevel = 0;

			m_bSuccess = false;

			if( !tag_name|| !att_name || !tag_name[0] ) 
				return def_value;

			if( ++nRecursionLevel > g_nMaxResursionLevel )
			{
				CString cstrErrorMessage;
				cstrErrorMessage.Format("Max recursion level reached for [%s]:%s.  Circular reference in Parent?",
					tag_name, att_name);
				GBM_DisplayError(cstrErrorMessage);

				--nRecursionLevel;
				return def_value;
			}

			const CPoint ptResult = m_buteMgr.GetPoint(tag_name,att_name, def_value);
			if( m_buteMgr.Success( ))
			{
				--nRecursionLevel;
				m_bSuccess = true;
				return ptResult;
			}

			const CString parent_name = m_buteMgr.GetString(tag_name,"Parent",CString(""));
			if( stricmp(parent_name,tag_name)  == 0) 
			{
				--nRecursionLevel;
				return def_value;
			}

			const CPoint ptRecurseResult = GetPoint( parent_name, att_name, def_value );

			--nRecursionLevel;
			return ptRecurseResult;
		}

		CPoint GetPoint(const char * tag_name, const char * att_name)
		{
			static CPoint def_value(0,0);
			return GetPoint(tag_name,att_name,def_value);
		}

		CAVector GetVector(const char * tag_name, const char * att_name, CAVector & def_value )
		{
			static int nRecursionLevel = 0;

			m_bSuccess = false;

			if( !tag_name|| !att_name || !tag_name[0] ) 
				return def_value;

			if( ++nRecursionLevel > g_nMaxResursionLevel )
			{
				CString cstrErrorMessage;
				cstrErrorMessage.Format("Max recursion level reached for [%s]:%s.  Circular reference in Parent?",
					tag_name, att_name);
				GBM_DisplayError(cstrErrorMessage);

				--nRecursionLevel;
				return def_value;
			}

			const CAVector caResult = m_buteMgr.GetVector(tag_name,att_name, def_value);
			if( m_buteMgr.Success( ))
			{
				--nRecursionLevel;
				m_bSuccess = true;
				return caResult;
			}

			const CString parent_name = m_buteMgr.GetString(tag_name,"Parent",CString(""));
			if( stricmp(parent_name,tag_name)  == 0) 
			{
				--nRecursionLevel;
				return def_value;
			}

			const CAVector caRecurResult = GetVector( parent_name, att_name, def_value );

			--nRecursionLevel;
			return caRecurResult;
		}

		CAVector GetVector(const char * tag_name, const char * att_name)
		{
			static CAVector def_value(0,0,0);
			return GetVector(tag_name,att_name,def_value);
		}

		CARange GetRange(const char * tag_name, const char * att_name, CARange & def_value )
		{
			static int nRecursionLevel = 0;

			m_bSuccess = false;

			if( !tag_name|| !att_name || !tag_name[0] ) 
				return def_value;

			if( ++nRecursionLevel > g_nMaxResursionLevel )
			{
				CString cstrErrorMessage;
				cstrErrorMessage.Format("Max recursion level reached for [%s]:%s.  Circular reference in Parent?",
					tag_name, att_name);
				GBM_DisplayError(cstrErrorMessage);

				--nRecursionLevel;
				return def_value;
			}

			const CARange rgResult = m_buteMgr.GetRange(tag_name,att_name, def_value);
			if( m_buteMgr.Success( ))
			{
				--nRecursionLevel;
				m_bSuccess = true;
				return rgResult;
			}

			const CString parent_name = m_buteMgr.GetString(tag_name,"Parent",CString(""));
			if( stricmp(parent_name,tag_name)  == 0) 
			{
				--nRecursionLevel;
				return def_value;
			}

			const CARange rgRecurResult = GetRange( parent_name, att_name, def_value );

			--nRecursionLevel;
			return rgRecurResult;
		}

		CARange GetRange(const char * tag_name, const char * att_name)
		{
			static CARange def_value(0,0);
			return GetRange(tag_name,att_name,def_value);
		}

	protected:

		// Indicates previous call found real value rather than just default.
		bool	m_bSuccess;

};

#endif // __HIERARCHICAL_BUTE_MGR_H__