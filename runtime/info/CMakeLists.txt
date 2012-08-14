cmake_minimum_required (VERSION 2.8)
project (Lib_Info)

add_library(Lib_Info STATIC
	src/outputredir.cpp
	src/sys/win32/outputredir_win32.cpp)
include_directories(src
	../../sdk/inc
	../kernel/src
	../shared/src)
set_target_properties(Lib_Info
	PROPERTIES OUTPUT_NAME libltinfo)