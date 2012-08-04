cmake_minimum_required (VERSION 2.8)
project (Lib_Lith)
add_library (Lib_Lith STATIC 
	basehash.cpp
	baselist.cpp
	baselistcounter.cpp
	dprintf.cpp
	lith.cpp
	lithbaselist.cpp
	lithchunkallocator.cpp
	lithsimpary.cpp
	lithsimparystat.cpp
	virtlist.cpp)
add_definitions (-D_CRT_SECURE_NO_WARNINGS)
