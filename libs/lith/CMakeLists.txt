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
if(MSVC)
	set_target_properties(Lib_Lith PROPERTIES COMPILE_FLAGS "/wd4996")
endif(MSVC)
if(CMAKE_COMPILER_IS_GNUCXX)
	set_target_properties(Lib_Lith PROPERTIES COMPILE_FLAGS "-fpermissive")
endif(CMAKE_COMPILER_IS_GNUCXX)
