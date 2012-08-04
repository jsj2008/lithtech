cmake_minimum_required (VERSION 2.8)
project (RegMgr)
add_library (RegMgr STATIC 
	regmgr.cpp)
add_definitions (-D_CRT_SECURE_NO_WARNINGS)
include_directories (../lith) 
#	../stdlith 
#	../lith)