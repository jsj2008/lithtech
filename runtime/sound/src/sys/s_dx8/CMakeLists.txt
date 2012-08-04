cmake_minimum_required (VERSION 2.8)
project (DLL_dx8)
add_library (DLL_dx8 SHARED
	../../../../kernel/src/sys/win/counter.cpp
	parsewavefile.cpp
	s_dx8.cpp
	StdAfx.cpp)
add_definitions (-D_CRT_SECURE_NO_WARNINGS)
include_directories (../..
	../../../../../sdk/inc 
	../../../../kernel/src)
set_target_properties(DLL_dx8
	PROPERTIES OUTPUT_NAME SndDrv)
find_package (ltmem)
target_link_libraries(DLL_dx8
	dxguid.lib
	winmm.lib
	msacm32.lib
	eaxguid.lib
	ltmem)
	