#if !defined General_h
#define General_h

#include <cassert>
#include <vector>
#include <string>

void ArgusTrace(const char* Format, ...);
unsigned long GetTempDir(unsigned long BufferLength, char* Buffer);

int RemoveStringInPlace(std::string& subject, char ch);
void ReplaceStringInPlace(std::string& subject, const char search, const char replace);
void QuickSortStringArray(std::vector<std::string> &s);
int BinSearchStringArray(std::vector<std::string> &s, std::string x, int nMissing, bool &IsMissing);

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

#endif // General_h
