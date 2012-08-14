cmake_minimum_required (VERSION 2.8)
project (Lib_RezMgr)
add_library (Lib_RezMgr STATIC 
	rezfile.cpp
	rezhash.cpp
	rezmgr.cpp
	rezutils.cpp)
include_directories (../../sdk/inc
	../lith)
	
if(MSVC)
	set_target_properties(Lib_RezMgr PROPERTIES COMPILE_FLAGS "/wd4996")
endif(MSVC)
