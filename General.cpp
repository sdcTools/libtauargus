#include <cstring>
#include <cstdlib>
#include <algorithm>

#include "General.h"

using namespace std;

void ArgusTrace(const char* Format, ...)
{
}

unsigned long GetTempDir(unsigned long BufferLength, char* Buffer)
{
	const char* val = 0;

	(val = getenv("TMPDIR"      )) || // POSIX
	(val = getenv("TMP"         )) || // POSIX/Windows
	(val = getenv("TEMP"        )) || // POSIX/Windows
	(val = getenv("TEMPDIR"     )) || // POSIX
	(val = getenv("USERPROFILE" )) || // Windows
	(val = getenv("WINDIR"      )) || // Windows
   (val = "/tmp"                );

	strncpy(Buffer, val, BufferLength);

	return strlen(val);
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

