cmake_minimum_required (VERSION 2.8)
project (DLL_dx8)

if(CMAKE_CL_64)
	link_directories("C:\\Program Files (x86)\\Microsoft DirectX SDK (February 2006)\\Lib\\x64")
else(CMAKE_CL_64)
	link_directories("C:\\Program Files (x86)\\Microsoft DirectX SDK (February 2006)\\Lib\\x86")
endif(CMAKE_CL_64)

add_library (DLL_dx8 SHARED
	../../../../kernel/src/sys/win/counter.cpp
	parsewavefile.cpp
	s_dx8.cpp
	StdAfx.cpp)
include_directories (../..
	../../../../../sdk/inc 
	../../../../kernel/src
	../../../../shared/src/sys/win)
set_target_properties(DLL_dx8
	PROPERTIES OUTPUT_NAME SndDrv)
target_link_libraries(DLL_dx8
	dxguid
	winmm
	msacm32
	ltmem)
	
if(NOT CMAKE_CL_64)
	target_link_libraries(DLL_dx8 eaxguid)
endif(NOT CMAKE_CL_64)
	