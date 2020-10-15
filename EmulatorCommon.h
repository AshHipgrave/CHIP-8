#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/// <summary>
/// Throws an exception if the specified HRESULT is a failure condition
/// </summary>
/// <param name="hr">The HRESULT to check</param>
inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}