cmake_minimum_required (VERSION 2.8)
project (MFCStub)
add_library (MFCStub STATIC 
	mfcs_point.cpp
	mfcs_rect.cpp
	mfcs_string.cpp)
#	mfcstub.cpp)
add_definitions (-D_CRT_SECURE_NO_WARNINGS)
include_directories (../stdlith) 
#	../stdlith 
#	../lith)