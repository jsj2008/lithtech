cmake_minimum_required (VERSION 2.8)
project (Lib_StdLith)
add_library (Lib_StdLith STATIC 
	abstractio.cpp
	dynarray.cpp
	fileio.cpp
	goodlinklist.cpp
	helpers.cpp
	l_allocator.cpp
	linklist.cpp
	lithexception.cpp
	memory.cpp
	memoryio.cpp
	stringholder.cpp
	struct_bank.cpp
	zfstream.cpp)
if(MSVC)
	set_target_properties(Lib_StdLith PROPERTIES COMPILE_FLAGS "/wd4996")
endif(MSVC)
