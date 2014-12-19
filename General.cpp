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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <algorithm>

#include "General.h"

using namespace std;

void ArgusTrace(const char* Format, ...)
{
}

int RemoveStringInPlace(char* subject, char ch) {
    int occurences = 0;
    char *pos = subject;
    while ((pos = strchr(pos, ch)) != NULL) {
        memmove(pos, pos + 1, strlen(pos));
        occurences++;
    }
    return occurences;
}

int RemoveStringInPlace(string& subject, char ch) {
	int occurences = 0;
	size_t pos = 0;
	while((pos = subject.find(ch, pos)) != string::npos) {
		subject.erase(pos, 1);
		occurences++;
	}
	return occurences;
}

void ReplaceStringInPlace(string& subject, const char search, const char replace)
{
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != string::npos) {
         subject[pos] = replace;
         pos++;
    }
}

// sort strings
void QuickSortStringArray(vector<string> &s)
{
	sort(s.begin(), s.end());
}

// binary search to see if an string is in an array of strings 
// taking in to account weather to look at missings or not.
// returns the position if the string is found -1 if not.
// IsMissing on true if x = Missing1 or x = Missing2
int BinSearchStringArray(vector<string> &s, const string &x, int nMissing, bool &IsMissing)
{
	ASSERT(s.size() > nMissing);

	IsMissing = false;

	vector<string>::iterator it = lower_bound(s.begin(), s.end() - nMissing, x);

	if (it != s.end() - nMissing) {
		if (*it == x) {
			return it - s.begin();
		}
	}

	// equal to missing1 or -2? // code missing not always the highest

	for (int mis = s.size() - nMissing; mis < s.size(); mis++) {
		if (x == s[mis]) {
			IsMissing = true;
			return mis;
		}
	}

	return -1;
}

// add spaces before to make the string the right length
void AddSpacesBefore(char *str, int len)
{
    int lstr = strlen(str);

    if (lstr < len) {
        char *dest = str + len - lstr;
        memmove(dest, str, lstr);
        while (dest-- != str) {
            *dest = ' ';
        }
        str[len] = '\0';
    }
}

void AddSpacesBefore(string& str, int len)
{
	int width = str.length();

	if (width < len)
	{
		str.insert((size_t)0, (size_t)(len - width), ' ');
	}
}
