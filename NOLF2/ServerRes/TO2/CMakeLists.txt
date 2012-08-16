cmake_minimum_required (VERSION 2.8)
project (NOLF2_ServerRes)
add_library (NOLF2_ServerRes SHARED 
	main.cpp
	ServerRes.rc)
set_target_properties(NOLF2_ServerRes 
	PROPERTIES OUTPUT_NAME SRes)