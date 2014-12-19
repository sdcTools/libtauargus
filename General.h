/*
* Argus Open Source
* Software to apply Statistical Disclosure Control techniques
* 
* Copyright 2014 Statistics Netherlands
* 
* This program is free software; you can redistribute it and/or 
* modify it under the terms of the European Union Public Licence 
* (EUPL) version 1.1, as published by the European Commission.
* 
* You can find the text of the EUPL v1.1 on
* https://joinup.ec.europa.eu/software/page/eupl/licence-eupl
* 
* This software is distributed on an "AS IS" basis without 
* warranties or conditions of any kind, either express or implied.
*/

#if !defined General_h
#define General_h

#include <cassert>
#include <vector>
#include <string>
#include "defines.h"

void ArgusTrace(const char* Format, ...);

int RemoveStringInPlace(char* subject, char ch);
int RemoveStringInPlace(std::string& subject, char ch);
void ReplaceStringInPlace(std::string& subject, const char search, const char replace);
void QuickSortStringArray(std::vector<std::string> &s);
int BinSearchStringArray(std::vector<std::string> &s, const std::string &x, int nMissing, bool &IsMissing);
void AddSpacesBefore(char *str, int len);
void AddSpacesBefore(std::string &str, int len);

#ifndef LPCTSTR
#define LPCTSTR const char*
#endif

#ifndef DEBUG_NEW
#define DEBUG_NEW new
#endif

#ifndef ASSERT
#define ASSERT assert
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef FAR
#define FAR
#endif

#ifndef TRACE
#define TRACE              ::ArgusTrace
#endif

// Visual C++ 6.0 fix
#if _MSC_VER == 1200 && !defined(_WINDEF_)
template <typename T> T min(T a, T b)
{
	return (a < b) ? a : b;
}

template <class T> T max(T a, T b)
{
	return (a > b) ? a : b;
}
#endif

#endif // General_h
