cmake_minimum_required (VERSION 2.8)
project (ButeMgr)
add_library (ButeMgr STATIC 
	arange.cpp 
	avector.cpp 
	butemgr.cpp 
	stdafx.cpp)
add_definitions (-D_NOMFC -D_CRT_SECURE_NO_WARNINGS)
include_directories (../CryptMgr 
	../stdlith 
	../lith
	../MFCStub)