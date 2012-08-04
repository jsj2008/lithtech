cmake_minimum_required (VERSION 2.8)
project (ServerDir)

add_subdirectory(../WONAPI ${CMAKE_CURRENT_BINARY_DIR}/WONAPI)

add_library (ServerDir SHARED
	CServerDir_Titan.cpp
	stdafx.cpp
	TitanTest.cpp)
add_definitions (-D_CRT_SECURE_NO_WARNINGS)
include_directories (../../sdk/inc
	../WONAPI
	../../NOLF2/ClientRes/Shared)
target_link_libraries(ServerDir
	WONAPI)