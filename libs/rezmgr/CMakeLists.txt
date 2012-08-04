cmake_minimum_required (VERSION 2.8)
project (Lib_RezMgr)
add_library (Lib_RezMgr STATIC 
	rezfile.cpp
	rezhash.cpp
	rezmgr.cpp
	rezutils.cpp)
add_definitions (-D_CRT_SECURE_NO_WARNINGS)
include_directories (../../sdk/inc
	../../../Libs/lith)
