#ifndef __ASSOCIATEPACKER_H__
#define __ASSOCIATEPACKER_H__

class IPackerImpl;

//given a filename and a directory to look under, it will try and find the packer
//in that directory that associates with the file. If pszDirectory is NULL or it
//cannot find it in that file, it will look under .\packers, and if it cannot
//find it in there, it will look in the registry for LithTech's installation path
//and try and find it there. If it cannot find it anywhere, it will return NULL

IPackerImpl*	AssociatePacker(const char* pszFilename, const char* pszDirectory);

#endif
