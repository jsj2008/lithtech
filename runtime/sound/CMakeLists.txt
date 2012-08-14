cmake_minimum_required (VERSION 2.8)
project (Lib_ILTSound)

include_directories(src
	../../sdk/inc)

add_library(Lib_ILTSound STATIC
	src/iltsound.cpp)
	
if(MSVC)
	set_target_properties(Lib_ILTSound PROPERTIES COMPILE_FLAGS "/wd4996")
endif(MSVC)