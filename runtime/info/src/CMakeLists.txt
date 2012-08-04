add_library(Lib_Info STATIC
	outputredir.cpp
	sys/win32/outputredir_win32.cpp)
set_target_properties(Lib_Info
	PROPERTIES OUTPUT_NAME libltinfo)