cmake_minimum_required (VERSION 2.8)
project (ClientRes)
add_library (ClientRes SHARED 
	TO2/main.cpp
	TO2/Lang/EN/ClientRes.rc)
add_definitions (-D_CRT_SECURE_NO_WARNINGS)
include_directories (Shared
	Shared/Lang/EN
	../Shared/Lang/EN)
set_target_properties(ClientRes 
	PROPERTIES OUTPUT_NAME CRes)