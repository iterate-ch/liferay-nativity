#pragma once

#ifdef UTIL_DLL
#define UTIL_API __declspec(dllexport)
#else
#define UTIL_API __declspec(dllimport)
#endif
