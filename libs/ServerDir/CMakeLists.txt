cmake_minimum_required (VERSION 2.8)
project (ServerDir)

add_library (ServerDir SHARED
	CServerDir_Titan.cpp
	stdafx.cpp)
include_directories (../../sdk/inc
	../WONAPI
	../../NOLF2/ClientRes/Shared)
target_link_libraries(ServerDir
	WONAPI)
if(MSVC)
	set_target_properties(ServerDir PROPERTIES COMPILE_FLAGS "/wd4996")
endif(MSVC)