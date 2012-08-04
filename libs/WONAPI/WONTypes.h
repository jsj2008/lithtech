#ifndef __WONTYPES_H__
#define __WONTYPES_H__
#include "WONShared.h"

#include <list>

namespace WONTypes
{

typedef std::list<bool> BoolList;
typedef std::list<int> IntList;
typedef std::list<std::string> StringList;
typedef std::list<std::wstring> WStringList;

typedef std::list<BoolList> BoolListList;
typedef std::list<IntList> IntListList;
typedef std::list<StringList> StringListList;
typedef std::list<WStringList> WStringListList;


}; // namespace WONTypes

#endif
