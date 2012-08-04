cmake_minimum_required (VERSION 2.8)
project (ServerRes)
add_library (ServerRes SHARED 
	main.cpp
	ServerRes.rc)
add_definitions (-D_CRT_SECURE_NO_WARNINGS)
#include_directories (../CryptMgr 
#	../stdlith 
#	../lith
#	../MFCStub)
set_target_properties(ServerRes 
	PROPERTIES OUTPUT_NAME SRes)