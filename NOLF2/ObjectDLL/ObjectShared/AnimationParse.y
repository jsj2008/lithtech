
%{

#include "StdAfx.h"
#include "AnimationLex.h"
#include "AnimationParser.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE

void yyerror(const char* szError);

%}

%union 
{
	char	szString[256];
}

%token ERROR_TOKEN
%token PROPERTIES ANIMATIONS TRANSITIONS STRING BRACE_LEFT BRACE_RIGHT SET_ADD SET_REMOVE SET_CONSTANT SET_NOT COMMA
%type <szString> string

%%

input
	:
	| input line
	;

line
	: PROPERTIES											{ g_pParser->BeginProperties(); }
		properties
	  ANIMATIONS											{ g_pParser->BeginAnimations(); }
		animations
	  TRANSITIONS											{ g_pParser->BeginTransitions(); }
		transitions
	;

properties
	: property
	  properties
	|
	;

property
	: string												{ g_pParser->AddPropertyGroup($1); }
		BRACE_LEFT property_elements BRACE_RIGHT
	;

property_elements
	: property_element										
	| property_elements property_element
	;

property_element
	: string												{ g_pParser->AddPropertyToCurrentGroup($1); }
	;

animations
	: animation
	  animations
	|
	;

animation
	: string string												{ g_pParser->AddAnimation($1, $2); }
		BRACE_LEFT property_set BRACE_RIGHT
	;

transitions
	: transition
	  transitions
	|
	;

transition													
	: string string												{ g_pParser->AddTransition($1, $2); }
		BRACE_LEFT transition_current_set BRACE_RIGHT				
		SET_ADD BRACE_LEFT transition_add_set BRACE_RIGHT		
		SET_REMOVE BRACE_LEFT transition_remove_set BRACE_RIGHT	
		SET_CONSTANT BRACE_LEFT transition_constant_set BRACE_RIGHT
		SET_NOT BRACE_LEFT transition_not_set BRACE_RIGHT
	;

transition_current_set
	: transition_current_set
	  string												{ g_pParser->AddPropertyToCurrentTransitionInitialSet($2); }
	|
	;

transition_add_set
	: transition_add_set
	  string												{ g_pParser->AddPropertyToCurrentTransitionAddSet($2); }
	|
	;

transition_remove_set
	: transition_remove_set
	  string												{ g_pParser->AddPropertyToCurrentTransitionRemoveSet($2); }
	|
	;

transition_constant_set
	: transition_constant_set
	  string												{ g_pParser->AddPropertyToCurrentTransitionConstantSet($2); }
	|
	;

transition_not_set
	: transition_not_set
	  string												{ g_pParser->AddPropertyToCurrentTransitionNotSet($2); }
	|
	;

property_set
	: property_set
	  string												{ g_pParser->AddPropertyToCurrentAnimation($2); }
	|
	;

string
	: STRING
		{
		}
	;

%%

