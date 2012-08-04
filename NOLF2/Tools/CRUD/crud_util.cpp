//////////////////////////////////////////////////////////////////////////////
// crud copy utility procedures

#include <locale>
#include "crud_util.h"

// Remove double "\\" & non-printables
char *CleanPath(char *string)
{
    char *stripread = string;
	char *stripwrite = string;
    while(*stripread) 
	{
        if(((stripread[0] == '\\') && (stripread[1] == '\\')) ||
			(!isprint(stripread[0])))
		{
			// Skip over this character
			++stripread;
        } else {
			// Copy the character
			*stripwrite = tolower(*stripread);
            ++stripread;
			++stripwrite;
        }
    }
	// Remember to terminate the string...
	*stripwrite = 0;

	return string;
}

