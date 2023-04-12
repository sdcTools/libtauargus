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

#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <vector>

#include "General.h"
#include "Variable.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVariable

// Variable constructor
CVariable::CVariable()
{
	bPos= -1;
        nPos=0; //Added
        nDec=0; //Added
        
	IsPeeper = false;
        IsCategorical = false; //Added
        IsNumeric = false; //Added
        IsWeight = false; //Added
        IsHierarchical = false; //Added
        IsHolding = false; //Added
        IsRecordKey = false; //Added
        
	nMissing = 0;
	Missing1 = "";
	Missing2 = "";
	TotalCode = "";

	MinValue = 0;
	MaxValue = 0;

	nCode = 0;

	nDigitSplit = 0;
        memset(DigitSplit, 0, sizeof(int) * MAXDIGITGROUP); //Added
	hCode = 0;

        TableIndex = -1; //Added
        ValueToggle = -1; //Added
        Value = 0; //Added
        HasRecode = false;
	HasCodeList = false;
        Recode.nCode = 0; //Addedd
        Recode.Missing1 = ""; //Addedd
        Recode.Missing2 = ""; //Addedd
        Recode.nMissing = 0; //Addedd
	Recode.DestCode = 0;
        Recode.CodeWidth = 0; //Addedd
	Recode.nBogus = 0;
	Recode.hCode = 0;
	PeepCode1 = "";
	PeepCode2 = "";

	PositionSet =  false;
        NumSubCodes = 0; // Added
	m_SubCodes = 0;

       	nBogus = 0;
        hfCodeWidth = 0; //Added
}

// memory allocated has to be destroyed
CVariable::~CVariable()
{
	if (HasRecode) {
		UndoRecode();
	}

	if (hCode != 0) {
		delete [] hCode;
	}
	if (m_SubCodes != 0)	{
		delete [] m_SubCodes;
	}

}

/*
BEGIN_MESSAGE_MAP(CVariable, CWnd)
	//{{AFX_MSG_MAP(CVariable)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
*/

// Set position for variables.
bool CVariable::SetPosition(long lbPos, long lnPos, long lnDec)
{
    bPos = lbPos - 1;
    nPos = lnPos;
    nDec = lnDec;
    PositionSet = true;
    return true;
}


//Set number of decimal places
bool CVariable::SetDecPosition(long lnDec)
{
	if (lnDec < 0) return false;
	nDec = lnDec;
	return true;
}

// Set what type the variable is.
// Categorical, Numerical etc
bool CVariable::SetType(bool bIsCategorical, bool bIsNumeric, bool bIsWeight, bool bIsHierarchical,
			bool bIsHolding, bool bIsPeeper, bool bIsRecordKey)
{
	IsCategorical = bIsCategorical;
	IsNumeric = bIsNumeric;
	IsWeight = bIsWeight;
	IsHierarchical = bIsHierarchical;
	IsHolding = bIsHolding;
	IsPeeper = bIsPeeper;
        IsRecordKey = bIsRecordKey;
	return true;
}

// set the missing strings for variable. note a variable need not always
// have missing strings
bool CVariable::SetMissing(LPCTSTR sMissing1, LPCTSTR sMissing2, long NumMissing)
{
    if (NumMissing > 0)	{
	Missing1 = sMissing1;
	Missing2 = sMissing2;

	if (Missing1.empty()) {
            Missing1 = Missing2;
	}

	if (Missing1.empty()) {
            nMissing = 0;
	}
	else if (Missing2.empty()) {
            nMissing = 1;
	}
	else {
            nMissing = 2;
	}
    }
    else {
	nMissing = 0;
    }
    return true;
}


bool CVariable::SetTotalCode(LPCTSTR sTotalCode)
{
    TotalCode = sTotalCode;
	return true;
}


// code is added to the code list.
// if code already in code list it is not added
bool CVariable::AddCode(const char *newcode, bool tail)
{
	int i, n;
	bool IsMissing;

	n = sCode.size();

	// add at the end of array if not found
	if (tail) {
		// not already in list?
		for (i = 0; i < n; i++) {
			if (sCode[i] == newcode) break;  // found!
		}
		if (i == n) {  // not found
			sCode.push_back(newcode);
		}
                return true;
	}

	// keep list sorted
	if (n == 0) {
		sCode.push_back(newcode);
	}
	else {
		int res = BinSearchStringArray(sCode, newcode, 0, IsMissing);
		if (res < 0) { // not found
			for (i = 0; i < n; i++) {
				if (sCode[i] > newcode) break;
			}
			if (i < n) {
				vector<string>::iterator p = sCode.begin() + i;
				sCode.insert(p, newcode);
			}
			else {
				sCode.push_back(newcode);
			}
		}
	}

	return true;
}

// compute the hierarchical codes where digit split is involved
bool CVariable::ComputeHierarchicalCodes()
{
	int i, j, n;
	string s, t;

	if (nDigitSplit < 2) return false;

	for (i = 0; i < (int) sCode.size(); i++) {
		n = 0;
		t = sCode[i];
		if (((int) t.length()) != nPos) continue; // no datafile code
		for (j = 0; j < nDigitSplit - 1; j++) {
			n += DigitSplit[j];
			s = t.substr(0, n);
			AddCode(s.c_str(), false);
		}
	}

	return true;
}

// Returns the number of codes
int CVariable::GetnCode()
{
  if (HasRecode) {
		return Recode.nCode;
	}
	return nCode;
}

// returns the number of Bogus codes
int CVariable::GetnBogus()
{
  if (HasRecode) {
		return Recode.nBogus;
	}
	return nBogus;
}

// returns number of active codes
int CVariable::GetnCodeActive()
{
	int i, n, k;

	if (hCode == 0) {    // not hierarchical
	  return GetnCode(); // so all active
	}

	// hierarchical
	k = GetnCode();
	for (i = n = 0; i < k; i++) {
		if (GethCode()[i].Active) n++;
	}
	return n;
}

// get number of inactive codes. This is used in recodes to see if
// you can apply recodes
int CVariable::GetnCodeInActive()
{
	int i, n, k;

	if (hCode == 0) { // not hierarchical
		return 0;       // so no inactives
	}

	// hierarchical
	k = GetnCode();
	for (i = n = 0; i < k; i++) {
		if (!GethCode()[i].Active) n++;
	}
	return n;
}

// Given a position i returns the code for that position. Can only
// be done for a categorical variable
std::string CVariable::GetCode(int i)
{
    //string s; // not used?
    if (HasRecode) {
	return Recode.sCode[i];
    }
    return sCode[i];
}


int CVariable::GetLevel(int i)
{
	int l;
    if (IsHierarchical) {
     l = hLevel[i];
     return l;
	}
	return 0;
}

// returns number of missing
int CVariable::GetnMissing()
{
    if (HasRecode) {
	return Recode.nMissing;
    }
    return nMissing;
}

// returns codelist
vector<string> * CVariable::GetCodeList()
{
    if (HasRecode) {
	return &(Recode.sCode);
    }
    return &(sCode);
}

// returns hierarchical code
CCode* CVariable::GethCode()
{
    if (HasRecode){
	return Recode.hCode;
    }
    return hCode;
}

// only for non hierarchical variables
bool CVariable::IsCodeBasic(int i)
{
	ASSERT(IsHierarchical == false);
	return ((int)(*GetCodeList())[i].length() == GetCodeWidth());
}

// returns code width
int CVariable::GetCodeWidth()
{
	if (HasRecode) {
		return Recode.CodeWidth;
	}

	if (IsHierarchical) {
		if (nDigitSplit != 0) {
			return nPos;
		}
		else {
			return hfCodeWidth;
		}
	}
	else {
		return nPos;
	}
}


// always with width of 8 characters
// not needed: code is original code, not the one used to feed GHMIter
//void CVariable::GetGHMITERCode(int i, char *code)
void CVariable::GetGHMITERCode(int i, std::string &code)
{
	//string s;

	ASSERT(i >= 0 && i < (int) GetCodeList()->size() );
	//s = GetCode(i);
        code = GetCode(i);
	//ASSERT(s.length() < 9);
        //ASSERT(s.length() < 64);
	//sprintf(code, "%8s", s.c_str());
        //sprintf(code, "%s", s.c_str());
}

// set the hierarchy. Very important and used by explore file and
// completed codelist
// In this functions the hierarchies, levels and number of children are calculated.
bool CVariable::SetHierarch()
{
	int i, j, n, lev[MAXDIGITGROUP + 1];

	if (hCode != 0) {
		delete [] hCode;
	}

	hCode = new CCode[nCode];
	if (hCode == 0) {
		return false; // not enough memory
	}

	// assume DigitSplit = {2,1,2}, then
	// lev[] = {0, 2, 3, 5}
	if (nDigitSplit > 0) {
		lev[0] = 0;
		for (i = n = 0; i < nDigitSplit; i++) {
			n += DigitSplit[i];
			lev[i + 1] = n;
		}
	}

	// set Level, IsParent
	for (i = 0; i < nCode; i++) {
		if (nDigitSplit > 0) {
			n = sCode[i].length();
			for (j = 0; j <= nDigitSplit; j++) {
				if (n == lev[j]) break;
			}
		}
		else {
			j = hLevel[i];
		}
		hCode[i].Level = j;

		if (i > 0) {
			if (hCode[i].Level > hCode[i - 1].Level) {
  				hCode[i - 1].IsParent = true;
			}
			else {
  				hCode[i - 1].IsParent = false;
			}
		}
	}
	hCode[nCode - 1].IsParent = false;


	// set nChildren and IsBogus and nBogus
	nBogus = 0;
	for (i = 0; i < nCode - nMissing; i++) {
		n = 0;
		int level = hCode[i].Level;
		for (j = i + 1; j < nCode - nMissing && hCode[j].Level > level; j++) {
			if (hCode[j].Level == level + 1) n++; // that's a child
		}
		hCode[i].nChildren = n;
		if (n == 1) {
			hCode[i + 1].IsBogus = true;
			nBogus++;
		}
	}

	for (; i < nCode; i++) { // Missing
  		hCode[i].nChildren = 0;
		hCode[i].Level = 1;
	}

	return true;
}

// set all descendants on active / not active
void CVariable::SetActive(long CodeIndex, bool active)
{
	int c = CodeIndex, level;

	ASSERT(c >= 0 && c < nCode - nMissing);
	ASSERT(hCode != 0);
	ASSERT(hCode[c].IsParent);

	level = hCode[c].Level;

	// set all descendants on active / not active
	for (c = CodeIndex + 1; c < nCode - nMissing; c++) {
		if (hCode[c].Level <= level) break;
		hCode[c].Active = active;
	}
}

// from a file the code list is created. This is used for hierarchical codelists
// without digit split
int CVariable::SetCodeList(LPCTSTR FileName, LPCTSTR LevelString)
{
	FILE *fd;
	int i, LenLevelString;
	string s;
        


	if (LenLevelString = strlen(LevelString), LenLevelString < 1) return HC_LEVELSTRINGEMPTY;

	hLevel.clear();
	hLevel.reserve(20);
	hLevel.push_back(0); // for total

	sCode.clear();
	sCode.reserve(20);
	sCode.push_back(""); // code for total

	hLevelBasic.clear();
	hLevelBasic.reserve(20);
	hLevelBasic.push_back(false); // for total

	fd = fopen(FileName, "r");
	if (fd == 0) return HC_FILENOTFOUND;

//	ftemp = fopen(tempfilename, "w");


	while (!feof(fd) ) {
		char str[200];
		str[0] = 0;
		fgets(str, 200, fd);
		if (str[0] == 0) break;
		s = str;
		// trim right
		size_t found = s.find_last_not_of(" \n\r\t");
		if (found != string::npos)
			s.erase(found + 1);
		else
			s.resize(0); // str is all whitespace

		/// add a few checks.

		if (s.empty()) continue;
		strcpy(str, s.c_str());

		// count number of LevelStrings
		i = 0;
		while (strncmp(LevelString, &str[i * LenLevelString], LenLevelString) == 0) i++;
		hLevel.push_back(i + 1);
//		fprintf(ftemp,"%s","hLevel   ");
//		fprintf(ftemp,"%i\n",hLevel.GetAt(num));
		s = &str[i * LenLevelString];
		if (s == Missing1 || (nMissing == 2 && s == Missing2) ) {
			return HC_CODEISMISSING;
		}

		sCode.push_back(s);
//		fprintf(ftemp,"%s","sCode   ");
//		fprintf(ftemp,"%s\n",sCode.GetAt(num));
//		num++;
	}
	fclose(fd);

 // num = 1;

	// set nCode
	nCode = sCode.size();
	hfCodeWidth = sCode[0].length();


  // check and compute hLevelBasic
	for (i = 1; i < nCode; i++) {
		// compute max width code
		if (((int)sCode[i].length()) > hfCodeWidth) {
			hfCodeWidth = sCode[i].length();
		}

		// level oke?
		if (hLevel[i] > hLevel[i - 1] && hLevel[i] - hLevel[i - 1] != 1) return HC_LEVELINCORRECT;
		if (sCode[i].empty()) return HC_CODEEMPTY;

		if (i == nCode - 1) {
			hLevelBasic.push_back(true); // last one always basic
		}
		else {
			if (hLevel[i + 1] <= hLevel[i]) {
				hLevelBasic.push_back(true);
			}
			else {
				hLevelBasic.push_back(false);
			}
		}

//		fprintf(ftemp,"%s","hLevelBasic   ");
//		if (hLevelBasic.GetAt(num)) {fprintf(ftemp,"%s\n","TRUE");}
//		else {fprintf(ftemp,"%s\n","FALSE");}
//		num ++;
	}

//  fclose(ftemp);
	long ret;
//	ret = OrganizeCodelist();
//  Maar even niet; slaat tenslotte nergens op; AHNL 9 juni 2005
	ret = 1;
	if (!(ret == 1)) {
		return HC_CODETOOLONG;
	}
	return true;
}

// Finds a code in a hierarchical codelist
int CVariable::FindAllHierarchicalCode(LPCTSTR code)
{
	int n = sCode.size();

	for (int i = 0; i < n; i++) {
		if (sCode[i] == code) {
			return i; // found
		}
	}

	return -1; // not found
}

// Finds only the basic codes in a hierarchical codelist
int CVariable::FindHierarchicalCode(LPCTSTR code)
{
	int n = sCode.size();

	for (int i = 0; i < n; i++) {
		if (hLevelBasic[i] && sCode[i] == code) {
			return i; // found
		}
	}

	return -1; // not found
}

//
bool CVariable::SetHierarchicalDigits(long nDigitPairs, long *nDigits)
{
	int i, npos = 0;

	if (nDigitPairs > MAXDIGITGROUP) return false;

	// save array, count digits
	for (i = 0; i < nDigitPairs; i++) {
		if (nDigits[i] < 1) return false;
		DigitSplit[i] = nDigits[i];
		npos += nDigits[i];
	}

	// sum digits should be equal to number of positions in data file
	if (npos != nPos) return false;

	nDigitSplit = nDigitPairs;
	return true;
}

bool CVariable::WriteCodelist(LPCTSTR FileName, LPCTSTR LevelString, LPCTSTR Name, bool bogus)
{
	vector<string> *CodeList = GetCodeList();
	CCode *phCode = GethCode();
	int i, n = CodeList->size();
	FILE *fd;

	if (IsHierarchical && phCode == 0) return false; // not yet set complete

	fd = fopen(FileName, "w");
	if (fd == 0) return false;

	// write name, only if specified
	if (Name != 0 && Name[0] != 0) {
		fprintf(fd, "%s\n", (LPCTSTR) Name);
	}

	if (IsHierarchical && bogus) {
		// Als de eerste een bogus is, dan gaat dit mis.
		WriteBogusCodelist(fd, LevelString, 1, 1, 1, n, CodeList);
	}
	else {
		for (i = 1; i < n; i++) { // ignore total (always index 0)
			if (IsHierarchical) {
				PrintLevelStrings(fd, phCode[i].Level - 1, LevelString);
			}
			PrintLevelCode(fd, (*CodeList)[i].c_str(), LevelString);
		}
	}

	fclose(fd);
	return true;
}

void CVariable::WriteBogusCodelist(FILE *fd, LPCTSTR LevelString, int index, int level, int boguslevel, int ncode, vector<string> *CodeList)
{
	int i, a;
	CCode *phCode = GethCode();

//    if ( index==1 && phCode[i].nChildren = 1 ) {
//		index = index + 1} // Als testje toegevoegd voor het geval dat de hoogste code bogus is.

	for (i = index; i < ncode; i++) {
		if (phCode[i].Level < level) break;
		if (phCode[i].Level > level) continue;
		if (phCode[i].nChildren != 1) {  // no bogus
			PrintLevelStrings(fd, boguslevel - 1, LevelString);
			PrintLevelCode(fd, (*CodeList)[i].c_str(), LevelString);
		}
		if (phCode[i].IsParent) {
			if (phCode[i].nChildren == 1) a = 0;
			else                         a = 1;
			WriteBogusCodelist(fd, LevelString, i + 1, level + 1, boguslevel + a, ncode, CodeList);
		}
	}
}

void CVariable::PrintLevelStrings(FILE *fd, int nLevel, LPCTSTR LevelString)
{
	for (int i = 0; i < nLevel; i++) { // print LevelStrings
		fprintf(fd, "%s", LevelString);
	}
}


void CVariable::PrintLevelCode(FILE *fd, LPCTSTR code, LPCTSTR LevelString)
{
	fprintf(fd,"%s%s\n", LevelString, code);
}


bool CVariable::SetHierarchicalRecode()
{
	int i, j;
	vector<unsigned char> RLevel; // levels recoded hierarchical variable

	// free previous recode
	if (HasRecode) {
		if (Recode.hCode != 0) {
                    delete [] Recode.hCode;
                    Recode.hCode = 0; //PWOF
		}
		if (Recode.DestCode != 0) {
                    free(Recode.DestCode);
                    Recode.DestCode = 0; // PWOF
		}
	}

	// initialize new recode
	Recode.DestCode = (int *) malloc(nCode * sizeof(int) );
	if (Recode.DestCode == 0) {
		return false;
	}
	Recode.sCode.clear();
	Recode.CodeWidth = 0;
	Recode.nMissing = nMissing;
	Recode.Missing1 = Missing1;
	Recode.Missing2 = Missing2;
	RLevel.reserve(20);

	// set new codelist, remember RLevel
	for (i = j = 0; i < nCode; i++) {
		if (hCode[i].Active) {
			Recode.sCode.push_back(sCode[i]);
			// compute max width code
			if (((int) sCode[i].length()) > Recode.CodeWidth) {
				Recode.CodeWidth = sCode[i].length();
			}
			Recode.DestCode[i] = j++;
			RLevel.push_back(hCode[i].Level);
		}
		else {
			Recode.DestCode[i] = -1;
		}
	}

	Recode.nCode = Recode.sCode.size();

	// initialize hCode
	Recode.hCode = new CCode[Recode.nCode];
	if (Recode.hCode == 0) {
		return false;
	}

	// compute hCode
	for (i = 0; i < Recode.nCode; i++) {
		Recode.hCode[i].Active = true;
		Recode.hCode[i].Level = RLevel[i];
	}

	Recode.nBogus = 0;
	for (i = 0; i < Recode.nCode - Recode.nMissing; i++) {
		int n = 0;
		for (j = i + 1; j < Recode.nCode; j++) {
			int LevelDiff = Recode.hCode[i].Level - Recode.hCode[j].Level;
			if (LevelDiff >= 0) break;
			if (LevelDiff == -1) n++;
		}
		Recode.hCode[i].IsParent = (n > 0);
		Recode.hCode[i].nChildren = n;
		if (n == 1) {
			Recode.hCode[i + 1].IsBogus = true;
			Recode.nBogus++;
		}
	}

	for (; i < Recode.nCode; i++) { // Missing
  		Recode.hCode[i].nChildren = 0;
		Recode.hCode[i].Level = 1;
	}

	HasRecode = true;
	return true;
}


void CVariable::UndoRecode()
{
  if (HasRecode) {
		if (Recode.DestCode != 0) {
                    free(Recode.DestCode);
                    Recode.DestCode = 0;
		}

		if (IsHierarchical) {
			if (Recode.hCode != 0) {
				delete [] Recode.hCode;
				Recode.hCode = 0;
			}
			SetActive(0, true); // at toplevel, so every code is set at Active
		}

		HasRecode = false;
	}
}

long CVariable::OrganizeCodelist()
{
	long i;
	long n = sCode.size();
	long lowestlevel = 0;

	for (i = 0; i < n; i++) {
		if (hLevel[i] > lowestlevel) {
			lowestlevel = hLevel[i];
		}
	}

	for (i = 0; i < n; i++) {
		if (hLevel[i] == lowestlevel) {
			string s = sCode[i];
			if (((int)s.length()) > nPos) {
				string temps = s;
				s.substr(0, nPos);
				temps.substr(nPos);
				RemoveStringInPlace(temps, ' ');
				if (!temps.empty()) {
					return HC_CODETOOLONG;
				}
			}

			if (((int)s.length()) < nPos) {
				// add spaces.
				s.insert((size_t)0, (size_t)(nPos - s.length()), ' ');
			}

			sCode[i] = s;
		}
	}
	return 1;
}

bool CVariable::SetPeepCodes(const string &Peep1, const string &Peep2)
{
	if (IsPeeper)	{
		if ((Peep1.empty()) &&  (Peep2.empty()))	{
			return false;
		}
		if (Peep1.empty())	{
			PeepCode1 = Peep2;
		}
		else	{
			PeepCode1 = Peep1;
			PeepCode2 = Peep2;
		}
	}
	return true;
}

// returns the depth of Hierarchical codelist. This is used
// to calculate marginals.
long CVariable::GetDepthOfHerarchicalBoom(bool Recoded)
{
	if (!IsHierarchical)	{
		return 0;
	}
	else
	{
            int maxdepth = 0;
		// figure out what to do with recodes
            if (HasRecode && Recoded) {
		for (int i = 0; i < Recode.nCode; i++)	{
			maxdepth = max(maxdepth, Recode.hCode[i].Level);
		}
            }
            else
            {
		for (int i = 0; i < nCode; i++)	{
			maxdepth = max(maxdepth, hCode[i].Level);
		}
            }
            return maxdepth;
	}
}

// To find the number of sub codes. Every parent has a sub code list
long CVariable::NumberOfSubCodeList()
{
	long i;
	long num;

	if (!IsHierarchical)	{
		num = 1;
		return num;
	}
	else	{
		num = 0;
		for (i=0; i<nCode; i++)	{
			if (hCode[i].IsParent)	{	// for every parent there is a code list
				num ++;
			}
		}
		return num;
	}
}

// allocate memory for subcodelist
bool CVariable::PrepareSubCodeList()
{
	long numsubcodes;

	if (m_SubCodes != 0)	{
		delete [] m_SubCodes;
	}

	numsubcodes = NumberOfSubCodeList();
	m_SubCodes = new CSubCodeList[numsubcodes];
	if (m_SubCodes == 0)	{
		return false;
	}
	NumSubCodes = numsubcodes;
	return true;

}

// for a variable make a collection of subcodelist
bool CVariable::FillSubCodeList()
{
	long SeqNum,ind;
	if (m_SubCodes == 0)	{
		return false;  // memory not allocated
	}
	if (!IsHierarchical)	{
		if (!CreateSubCodeForNonHierarchicalCode())	{
			return false;
		}
	}
	else	{
		SeqNum = 0;
		for (ind = 0; ind<nCode; ind++)	{
			if (hCode[ind].IsParent)	{
				CreateSubCodeForHierarchicalCode(ind,SeqNum);
				SeqNum++;
			}
		}
	}
	return true;
}

// for non-hierarchical codes is the subcodelist all the codes.
bool CVariable::CreateSubCodeForNonHierarchicalCode()
{
	CSubCodeList *subcodelist;
	long *indices;
	long i;
	vector<string> subcodes;
	if (IsHierarchical)	{
		return false;
	}
	else	{

		subcodes.resize(sCode.size()-1);
		indices = new long [sCode.size()-1];
		subcodelist = &(m_SubCodes[0]);
		for (i=1; i<nCode; i++)	{
			subcodes[i-1] = sCode[i];
			indices[i-1] = i;
		}
		subcodelist->SetParentCode(sCode[0]);
		subcodelist->SetParentIndex(0);
		subcodelist->SetSequenceNumber(0);
		subcodelist->FillSubCodes(subcodes, indices);
		delete [] indices;
		return true;
	}
}

// for hierarchical codes given a parent, the list of children form a subcodelist
bool CVariable::CreateSubCodeForHierarchicalCode(long CodeIndex,
																 long SubCodeSequenceNumber)
{
	long  NumChild;
	vector<string> subcode;
	CSubCodeList *subcodelist;
	long *indices;
	if (!IsHierarchical)	{
		return false;
	}
	else	{
		NumChild = FindNumberOfChildren(CodeIndex);
		if (NumChild <= 0)	{
			return false;
		}
		else	{
			subcode.resize(NumChild);
			indices = new long [NumChild];
			if (!FindChildren(NumChild,subcode,CodeIndex, indices))	{
				return false;

			}
		}
	}

	subcodelist = &(m_SubCodes[SubCodeSequenceNumber]);
	subcodelist->SetParentCode(sCode[CodeIndex]);
	subcodelist->SetParentIndex(CodeIndex);
	subcodelist->SetSequenceNumber(SubCodeSequenceNumber);
	subcodelist->FillSubCodes(subcode, indices);
	delete [] indices;

	return true;

}

//Find the number of children for a code.
long CVariable::FindNumberOfChildren(long CodeIndex)
{
	long num = 0;
	long ParentLevel,ChildLevel,i;
	if (!IsHierarchical)	{
		return 0;
	}
	else	{
		if (!hCode[CodeIndex].IsParent)	{
			return 0;
		}
		else	{
			ParentLevel = hCode[CodeIndex].Level;
			for (i = CodeIndex+1; i<nCode; i++)	{
				ChildLevel = hCode[i].Level;
				if (ChildLevel <= ParentLevel)	{
					break;
				}
				if (ChildLevel == ParentLevel +1)	{
					num ++;
				}
			}
			return num;
		}
	}
}

// given a parent find list of children.
bool CVariable::FindChildren(long NumChild, vector<string> & Child, long CodeIndex,
									  long *Index)
{
	long i, j;
	long ParentLevel;
	long ChildLevel;
	string tempCode;
	if (!IsHierarchical)	{
		return false;
	}
	else {
		if (!hCode[CodeIndex].IsParent)	{
			return false;
		}
		else	{
			j = 0;
			ParentLevel = hCode[CodeIndex].Level;
			for (i= CodeIndex+1; i<nCode; i++)	{
				ChildLevel = hCode[i].Level;
				if (ChildLevel <= ParentLevel)	{
					break;
				}
				if (ChildLevel == ParentLevel +1)	{
					Child[j] = sCode[i];
					Index[j] = i;
					j++;
				}

			}
			return true;
		}
	}

}

/* 
 * Note for change: Don't write beyond the original '\0' terminator.
 */
bool CVariable::NormaliseCode(char *Code)
{
	// trim left and right
   char *s = Code;
	while (*s == ' ' || *s == '\t') {
		s++;
	}
    
	if (*s == '\0') {
		*Code = '\0';
	} 
	else {
		char *e = s + strlen(s);
		while (e != s
				&& (*(e-1) == ' ' || *(e-1) == '\t')) {
			e--;
		}
		int length = e - s;
		memmove(Code, s, length);
		Code[length] = '\0';
		//int inrem = RemoveStringInPlace(Code, '"');
		ASSERT ((RemoveStringInPlace(Code, '"') == 2) || (RemoveStringInPlace(Code, '"') == 0));
		AddSpacesBefore(Code, nPos);
	}
	return true;
}

/*void CVariable:: ShowhCode()
{
	FILE *fdshow;
	string filename;
	filename = "E:/Temp/hCodeout.txt";

	fdshow = fopen(filename, "a");

	int i;
	fprintf(fdshow,"%s\n","var1");
	fprintf(fdshow, "%s\n"," Code IsParent, Level, nChilderen");
	for (i=0; i<nCode; i++)	{
		fprintf(fdshow,"%s	%d		%d		%d\n", sCode.GetAt(i),
		hCode->Active, hCode->IsBogus,
		hCode->IsParent, hCode->Level,
                hCode->nChildren);
	}
	fprintf(fdshow,"%s\n", "______________________");
	fclose(fdshow);
}
*/

