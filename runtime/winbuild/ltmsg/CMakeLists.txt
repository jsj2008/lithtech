cmake_minimum_required (VERSION 2.8)
project (DLL_LTMsg)
add_library (DLL_LTMsg SHARED 
	Lang/EN/ltmsg.rc
	init.cpp)
set_target_properties(DLL_LTMsg
	PROPERTIES OUTPUT_NAME LTMsg)