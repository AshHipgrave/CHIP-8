/* Central place for common #include headers. Prevents spreading them out across the entire program (e.g. Redefining 'WIN32_LEAN_AND_MEAN' every time a class needs Windows.h etc) */
#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdint.h>

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>