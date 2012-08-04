
// The ConParse module parses console strings into argument lists.
// The syntax is very simple: arguments separated by spaces.
// The arguments can be 
// There can be multiple argument lists per string, each separated
// by a semicolon.

// Here's an example of a valid console string:
// playsound "sounds/my sound.wav" ; preloadmodel models/model1.abc

// The first call to cp_Parse would give you a list with 2 arguments
// and return 1 indicating there are more strings to parse:
//    "playsound" and "sounds/my sound.wav"
// The second call would give 2 more strings and return 0:
//    "preloadmodel" and "models/model1.abc"

#ifndef __CONPARSE_H__
#define __CONPARSE_H__


	// Set pNewCommandPos to pCommands to start.
	// Always fills in nArgs and argPointers appropriately.
	// Returns 0 if there are no more strings in here to parse.
	// Returns 1 if there are more strings to parse.
	// If there are more strings to parse, pNewCommandPos is set to what you
	// should set pCommands to on the next call.
	int cp_Parse(char *pCommands, char **pNewCommandPos, char *argBuffer, char **argPointers, int *nArgs);


#endif  // __CONPARSE_H__




