cmake_minimum_required (VERSION 2.8)
project (ButeMgr)
add_library (ButeMgr STATIC 
	arange.cpp 
	avector.cpp 
	butemgr.cpp 
	stdafx.cpp)
add_definitions (-D_NOMFC)
include_directories (../CryptMgr 
	../stdlith 
	../lith
	../MFCStub)
if(MSVC)
	set_target_properties(ButeMgr PROPERTIES COMPILE_FLAGS "/wd4996")
endif(MSVC)