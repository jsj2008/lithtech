cmake_minimum_required (VERSION 2.8)
project (ltmem)
export (PACKAGE ltmem)
add_library (ltmem STATIC 
	ltmem.cpp
	ltmemdebug.cpp
	ltmemheap.cpp
	ltmemstats.cpp
	ltmemtrack.cpp
	StdAfx.cpp)
add_definitions (-D_CRT_SECURE_NO_WARNINGS)
include_directories (../../sdk/inc
	../../runtime/shared/src
	../../runtime/kernel/src
	../stdlith
	../../runtime/kernel/mem/src)
