// --------------------------------------------
// small memory parsenode processing for models
// smaller memory method of reading in parsenodes.
// with this method, parsenodes are thrown away as
// soon as a part of the sub tree is complete.
// --------------------------------------------
#pragma warning (disable:4786)

#include "ltaStream.h"
#include "ltalex.h"
#include "ltaParseNode.h"
#include "ltaScene.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>


int g_byte_cnt = 0;

// ------------------------------------------------------------------------
// lta externs
// ------------------------------------------------------------------------

ILTAStream *LTAStreamFrom_istream( istream & strm );
int BuildParseTree( ILTAStream *str, CParseNode *root  );
void PrettyPrint( ostream & os,  CParseNode * node, int depth );



// --------
class CLTACommand {
public :
	
	MetaModel *m_MetaModel ; // data base

    virtual void Exec( CParseNode *pn ) = 0 ;

};

// -----------
class CLTANULLCmd : public CLTACommand {
public :
	void Exec( CParseNode *pn )
	{
		// do nothing !!!! 
	}
};

// --------
class CLTAAnimSetCmd  : public CLTACommand 
{
public :
    void Exec( CParseNode *pn ) 
	{
		CLTATranslStatus status ;
		status.AddToLog( "animset:");
		
		//LTA::AnimSetFromPN(	*m_MetaModel, pn, status );
		pn->deleteChildren();
		status.AddToLog( status.m_msg );
		status.AddToLog("\r\n");
	}
	
	
};

// ------------------
class CLTAHierarchyCmd : public CLTACommand {
public :
	void Exec( CParseNode *pn )
	{
		CLTATranslStatus status ;
		
		status.AddToLog( "hierarchy:");
		
		//LTA::HierarchyFromPN( *m_MetaModel, pn, status );
		pn->deleteChildren();
		status.AddToLog( status.m_msg );
		status.AddToLog("\r\n");

	}
};


class CLTAShapeCmd : public CLTACommand {
public :
	
	void Exec( CParseNode *pn )
	{
		CLTATranslStatus status ;
			status.AddToLog( "shape:");
			//LTA::ShapeFromPN( *m_MetaModel, pn );
		status.AddToLog( status.m_msg );
		status.AddToLog("\r\n");
		
		//cout << " msg : " << status.m_msg << endl;
	}
};

// ------------------------------------------------------------------------
// Executing ParseNode
// This parse node possibly executes a command for a parsed node.
// ------------------------------------------------------------------------
class CExecPN : public CParseNode {
	static int s_id ;
	int m_id ;
	CLTACommand *m_cmd ;
public :
	typedef std::map< string, CLTACommand * , less<string> > CProcFnTable ;
	// map of node-types to functions
	static CProcFnTable s_ProcFnTable ;
	
	// list constructor
	CExecPN():m_cmd(NULL)
	{
		g_byte_cnt += sizeof( CExecPN );
		m_id = s_id++ ;
	}

	// value constructor 
	// the bool isString indicates whether the value is by origin a 
	// string. Since we can have either a string or a symbol as the input value.
	CExecPN( const char * pstr, int bIsString = 0 )
		:m_cmd(NULL), CParseNode(pstr,bIsString)
	{ 
		g_byte_cnt += sizeof( CExecPN );
		m_id = s_id++ ; 
	}
	
	 ~CExecPN()
	 { 
		 g_byte_cnt -= sizeof( CExecPN );
		 //cout << " deleting : " << m_id ; 
	 }
	
	
	// append either a new list or an element to an existing list
	 void append( CParseNode *new_node );

	// called by the lexer when a close paren is encountered. 
	// this indicates that the end of a list has been encountered.
	// this could be used to evaluate the list just formed..
	 void pop() ;

	
};


CExecPN::CProcFnTable CExecPN::s_ProcFnTable;
int CExecPN::s_id = 0 ;


// look for a name 
void CExecPN::append( CParseNode *new_node )
{
	if(  new_node->isAtom() && !new_node->isString())
	{
		string word = new_node->getValue();
		CProcFnTable::iterator it ;
		it = s_ProcFnTable.find( word );
		if( it != s_ProcFnTable.end() )
		{
			//cout <<"( " << m_id << " found : " << (*it).first.c_str() << endl;
			m_cmd = (*it).second ;

			//m_cmd->Process( new_node );
		}
	}

	CParseNode::append( new_node );	
}


void CExecPN::pop()
{
	if( m_cmd != NULL )
	{
		//cout << m_id << " " ;
		// execute the command
		m_cmd -> Exec( this ) ;
		// delete decendents
		deleteChildren();
		
		//cout << " **-> " << g_byte_cnt << endl;
	}
}



static int g_done = 0;
static int g_prep_done = 0;


// --------------------------------------------
// 
// --------------------------------------------	
static 
int _build_parse_tree(CLTALexAnalizer& lexan, ILTAStream * istr , CParseNode *root )
{
	int lexRet ;
	int done = 0;
	while(!done )
	{
		int bIsString = 0;
		string cur_str ;
		lexRet = 	lexan.analize_stream( *istr , cur_str );
		switch( lexRet )
		{
		case CLTALexAnalizer::LEX_OTHER :
			root->append( new CExecPN( cur_str.c_str(), bIsString ));
			break ;
		case CLTALexAnalizer::LEX_NONE :
			break ;
		case CLTALexAnalizer::LEX_PUSH :
			{
				// create a new list
				CParseNode *pn = new CExecPN ;
				root->append(pn) ;
				
				_build_parse_tree(lexan, istr, pn );
			}
			break ;
		case CLTALexAnalizer::LEX_POP :
			// root is 'pn' from the LEX_PUSH case here so we call pop on the node
			// knowing that its the close paren.
			root->pop();
			return 0;
			break;
		case CLTALexAnalizer::LEX_STRING :
			{
				// strip out the quotes
				string new_string ;
				if(cur_str.size() > 2 )
				{
					new_string = cur_str.substr(1,cur_str.size()-2);
					cur_str = new_string ;
				}
				bIsString = 1;
				root->append( new CExecPN( cur_str.c_str(), bIsString ));
			}	
			break;
		case CLTALexAnalizer::LEX_EOF :
			done = 1;
		
			break ;
		default :
			;
			//(*cur_strm) << " error " << endl;
		}
	}
	
	return 1;
}


static
int __BuildParseTree( ILTAStream *str, CParseNode *root  )
{
	g_done = 0;
	g_prep_done = 0;

	CLTALexAnalizer lexan ;
	return _build_parse_tree(lexan, str, root );
}

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

_timeb lta_time ;
time_t lta_start_time ;

void start_time()
{
	//cout << " start time : " << endl;
	time( &lta_start_time );
	_ftime( &lta_time );
}

void end_time()
{
	char tmpbuf[256];
	time_t tmp ;
	_timeb tmpTB ;

	time( &tmp );
	_ftime( & tmpTB );
	tmp = tmp - lta_start_time ;
	
	
	double val1 = tmpTB.time + ( tmpTB.millitm / 1000.0 ) ;
	double val2 = lta_time.time + ( lta_time.millitm / 1000.0 ) ;

	sprintf( tmpbuf ," secs : %f\r\nbyte cnt : %d\r\n" , ( val1 - val2  ) , g_byte_cnt );
	
	CLTATranslStatus::AddToLog( tmpbuf );

}


static CLTAAnimSetCmd   s_animsetcmd ;
static CLTAHierarchyCmd s_hierarchycmd ;
static CLTAShapeCmd     s_shapecmd ;
// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
void SmInitMetaModelFromPN( ILTAStream *strm , MetaModel &metaModel, 
						    CParseNode *pnroot, 
							CLTATranslStatus & status  )
{
	// give pnode processors a target
	s_animsetcmd.m_MetaModel   = & metaModel ;
	s_hierarchycmd.m_MetaModel = & metaModel ;
	s_shapecmd.m_MetaModel     = & metaModel ;
	// set up symbol table
	CExecPN::s_ProcFnTable["animset"] = & s_animsetcmd  ;
	CExecPN::s_ProcFnTable["hierarchy"] = & s_hierarchycmd  ;
	CExecPN::s_ProcFnTable["shape"] = & s_shapecmd ;
	
	// apply cmds to tree.
	start_time();
	__BuildParseTree( strm,  pnroot );
	end_time();
}



void PostCC()
{
	end_time();
	cout << " ok " << g_byte_cnt << endl;
}

