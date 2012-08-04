// ------------------------------------------------------------------------
// Console version of renderstyle packer. 
// ------------------------------------------------------------------------

#pragma warning(disable:4786)

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <vector>
#include <string>
//#include <ltbasedefs.h>
#include "renderstylepacker.h"


const float LT_VERSION = 3.0 ;

using namespace std;


void SetPlatform			(const char *str );
void PackIt					(string szInputFile[], const char* szOutputFile);
void SetProgress( unsigned int );
extern string	g_InputFile[MAX_RENDERSTYLE_INPUTS], g_OutputFile;



// ------------------------------------------------------------------------
// command line parse/object
// ------------------------------------------------------------------------
class SCmdLineParams 
{
public :

	enum EDirective { D3D, PS2 };

	EDirective		m_directive ; 
	bool			m_verbose ;
	bool			m_error ;
	vector<string>	m_input;
	string			m_output ;
	
	SCmdLineParams():m_directive(D3D), m_verbose(false), m_output("tmp.ltb"),m_error(false){}
	SCmdLineParams( int argc, char **argv ){ m_error = parse(argc,argv); }
	bool parse( int argc, char **argv);
};

// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
void usage() 
{

	cout << "Usage : " << endl;
	cout << "RenderStyle_Packer PLATFORM_DIRECTIVE -input filename.lta [ -input filename2.lta ]"<<endl;
	cout << "-output ltbfile.ltb -verbose " << endl;
	cout << endl;
	cout <<"Packer Directives : " << endl;
	cout <<"d3d : \n\tPack render style for use with d3d renderer" << endl;
	cout <<"ps2 : \n\tPack render style for use with ps2 renderer" << endl;
	cout << endl;
	cout << "-input with the -input flag more than one input file can be specified on the command line."<<endl;
	cout << "i.e : -input filea.lta -input fileb.lta -input filec.lta " << endl;
	cout << __DATE__ << " " <<"-["<< LT_VERSION << "}-"<<endl;


}

// ------------------------------------------------------------------------
// parse command line 
// command line optins 
// return false on fail
// ------------------------------------------------------------------------
bool SCmdLineParams::parse( int argc, char **argv )
{
	enum ENextType { INPUT, OUTPUT, IGNORE };

	ENextType next_type;

	if( argc <= 1 ) 
		return false ;

	if( strcmp( argv[1] , "d3d" ) == 0 || strcmp( argv[1] , "D3D" ) == 0 )
	{
		
		m_directive = D3D ;
		SetPlatform("d3d");
	}else
	if( strcmp( argv[1] , "ps2" ) == 0 || strcmp( argv[1] , "PS2" ) == 0 )
	{
		m_directive = PS2;
		SetPlatform("ps2");
	}
	else {
		cout << "Error: invalid platform directive " << endl;
		m_error = true ;
		return false ;// error
	}

	for ( int i = 2 ; i < argc ; ++i )
	{
		if( strcmp( argv[i], "-input" ) == 0 || strcmp( argv[i], "-i" ) == 0 )
		{
			next_type = INPUT ;
			continue ;
		}
		else
		if( strcmp( argv[i], "-output") == 0 || strcmp( argv[i], "-o" ) == 0 )
		{
			next_type = OUTPUT;
			continue ;
		}
		else
		if( strcmp( argv[i], "-verbose") == 0)
		{	m_verbose = true ; continue ; }

		switch( next_type ){
		case INPUT :
			m_input.push_back( strdup( argv[i] ));
			break;
		case OUTPUT:
			m_output = argv[i];
			break;
		}
	}
	
	if( m_input.size() < 1 ){
		cout << "Error: missing input files. " << endl;
		m_error = true ;
	}

	return true ;
}

// ------------------------------------------------------------------------
// windows progress bar
// ------------------------------------------------------------------------
void SetProgress( unsigned int percent )
{
	printf(".\r");
}


void ShutDownApp()
{
	printf("done\n");
}

int main(int argc, char* argv[])
{
	SCmdLineParams cmdlineparams( argc, argv );

	if( cmdlineparams.m_error == false)
	{
		usage();
		return 1;
	}
	else
	{
		for( int cnt = 0 ; cnt< cmdlineparams.m_input.size(); cnt++ )
			g_InputFile[cnt] = cmdlineparams.m_input[cnt];

		PackIt( g_InputFile, cmdlineparams.m_output.c_str() );
	}

	return 0;
}



void OutputMsg(char * fmt, ...)
{
    char buff[256]; va_list va;

    va_start( va, fmt ); sprintf( buff, fmt, va );
    va_end( va ); strcat(buff, "\r\n");
    cout << buff << endl;
}

void dprintf(char * fmt, ...)
{
    char buff[256]; va_list va;

    va_start( va, fmt ); sprintf( buff, fmt, va );
    va_end( va ); strcat(buff, "\r\n");
	cerr << buff << endl;
}