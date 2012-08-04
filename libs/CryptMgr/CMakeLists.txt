cmake_minimum_required (VERSION 2.8)
project (CryptMgr)
add_library (CryptMgr STATIC 
	blowfish.cpp 
	cryptmgr.cpp)