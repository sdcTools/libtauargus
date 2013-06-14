// Variable.cpp : implementation file
//

#include "stdafx.h"
#include <vector>
#include <math.h>
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
	nCode = 0;
	nMissing = 0;
	Missing1 = "";
	Missing2 = "";
	TotalCode = "";
	MinValue = 0;
	MaxValue = 0;
	HasRecode = false;
	HasCodeList = false;
	nDigitSplit = 0;
	hCode = 0;
	nBogus = 0;
	Recode.hCode = 0;
	Recode.DestCode = 0;
	Recode.nBogus = 0;
	PositionSet =  false;
	IsPeeper = false;
	PeepCode1 = "";
	PeepCode2 = "";
	m_SubCodes = 0;
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
bool CVariable::SetType(bool bIsCategorical, bool bIsNumeric,
								bool bIsWeight, bool bIsHierarchical,
								bool bIsHolding, bool bIsPeeper)
{
	IsCategorical = bIsCategorical;
	IsNumeric = bIsNumeric;
	IsWeight = bIsWeight;
	IsHierarchical = bIsHierarchical;
	IsHolding = bIsHolding;
	IsPeeper = bIsPeeper;
	return true;
}

// set the missing strings for variable. note a variable need not always
// have missing strings
bool CVariable::SetMissing(LPCTSTR sMissing1, LPCTSTR sMissing2, long NumMissing)
{

	if (NumMissing > 0)	{
		Missing1 = sMissing1;
		Missing2 = sMissing2;

		if (Missing1.IsEmpty()) {
			Missing1 = Missing2;
		}

		if (Missing1.IsEmpty() ) {
			nMissing = 0;
		}
		else {
		if (Missing2.IsEmpty() ) nMissing = 1;
		else                     nMissing = 2;
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
				if (newcode < sCode[i] ) break;
			}
			if (i < n) {
				vector<CString>::iterator p = sCode.begin() + i;
				sCode.insert(p, newcode);
			}
			else {
				sCode.push_back(newcode);
			}
		}
	}

	return true;
}

// binary search to see if an string is in an array of
// strings taking in to account weather to look at missings or not.
// returns the position if the string is found -1 if not.
int CVariable::BinSearchStringArray(vector<CString> &s, CString x, int nMissing, bool &IsMissing)
{
	int mid, left = 0, right = s.size() - 1 - nMissing;
	int mis;

	ASSERT(left <= right);

	IsMissing = false;

	while (right - left > 1) {
		mid = (left + right) / 2;
		if (x < s[mid]) {
			right = mid;
		}
		else {
			if (x > s[mid]) {
				left = mid;
			}
			else {
				return mid;
			}
		}
	}

	if (x == s[right]) return right;
	if (x == s[left]) return left;

	// equal to missing1 or -2? // code missing not always the highest

	if (nMissing > 0) {
		mis = s.size() - nMissing;
		if (x == s[mis]) {
			IsMissing = true;
			return mis;
		}
		if (nMissing == 2) {
			if (x == s[mis + 1]) {
				IsMissing = true;
				return mis + 1;
			}
		}
	}

	return -1;
}

// compute the hierarchical codes where digit split is involved
bool CVariable::ComputeHierarchicalCodes()
{
	int i, j, n;
	CString s, t;

	if (nDigitSplit < 2) return false;

	for (i = 0; i < sCode.size() - nMissing; i++) {
		n = 0;
		t = sCode[i];
		if (t.GetLength() != nPos) continue; // no datafile code
		for (j = 0; j < nDigitSplit - 1; j++) {
			n += DigitSplit[j];
			s = t.Left(n);
			AddCode((LPCTSTR) s, false);
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
CString CVariable::GetCode(int i)
{
	CString s;

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
vector<CString> * CVariable::GetCodeList()
{
  if (HasRecode) {
		return &(Recode.sCode);
	}
	return &(sCode);
}

// returns hierarchical code
CCode* CVariable::GethCode()
{
  if (HasRecode) {
		return Recode.hCode;
	}
	return hCode;

}

// only for non hierarchical variables
bool CVariable::IsCodeBasic(int i)
{
	ASSERT(IsHierarchical == false);
	return (*GetCodeList())[i].GetLength() == GetCodeWidth();
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
void CVariable::GetGHMITERCode(int i, char *code)
{
	CString s;

	ASSERT(i >= 0 && i < GetCodeList()->GetSize() );
	s = GetCode(i);
	ASSERT(s.GetLength() < 9);
	sprintf(code, "%8s", (LPCTSTR) s);
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
			n = sCode[i].GetLength();
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
	ASSERT(hCode[c].IsParent == TRUE);

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
	CString s;



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
		s.TrimRight();
		s.TrimRight("\n\r");
		/// add a few checks.



		if (s.IsEmpty()) continue;
		strcpy(str, (LPCTSTR) s);

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
	hfCodeWidth = sCode[0].GetLength();


  // check and compute hLevelBasic
	for (i = 1; i < nCode; i++) {
		// compute max width code
		if (sCode[i].GetLength() > hfCodeWidth) {
			hfCodeWidth = sCode[i].GetLength();
		}

		// level oke?
		if (hLevel[i] > hLevel[i - 1] && hLevel[i] - hLevel[i - 1] != 1) return HC_LEVELINCORRECT;
		if (sCode[i].IsEmpty()) return HC_CODEEMPTY;

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
	if (!ret == 1) {
		return HC_CODETOOLONG;
	}
	return true;
}

// Finds a code in a hierarchical codelist
int CVariable::FindAllHierarchicalCode(LPCTSTR code)
{

	int i, n = sCode.size();
	CString stemp;
	for (i = 0; i < n; i++) {
		stemp = sCode[i];
		if (sCode[i] == code) break; // hebbes!
	}

	if (i >= n) return -1; // not found

	return i;  // found
}

// Finds only the basic codes in a hierarchical codelist
int CVariable::FindHierarchicalCode(LPCTSTR code)
{
	int i, n = sCode.size();
	CString stemp;
	for (i = 0; i < n; i++) {
		if (hLevelBasic[i] ) {
			stemp = sCode[i];// only basic codes, not parental
			if (sCode[i] == code) break; // hebbes!
		}
	}

	if (i >= n) return -1; // not found

	return i;  // found
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
	vector<CString> *CodeList = GetCodeList();
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
			PrintLevelCode(fd, (LPCTSTR) (*CodeList)[i], LevelString);
		}
	}

	fclose(fd);
	return true;
}

void CVariable::WriteBogusCodelist(FILE *fd, LPCTSTR LevelString, int index, int level, int boguslevel, int ncode, vector<CString> *CodeList)
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
			PrintLevelCode(fd, (LPCTSTR) (*CodeList)[i], LevelString);
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
	vector<BYTE> RLevel; // levels recoded hierarchical variable

	// free previous recode
	if (HasRecode) {
		if (Recode.hCode != 0) {
			delete [] Recode.hCode;
		}
		if (Recode.DestCode != 0) {
			free(Recode.DestCode);
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
			if (sCode[i].GetLength() > Recode.CodeWidth) {
				Recode.CodeWidth = sCode[i].GetLength();
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
	long lowestlevel = 0;
	CString s,temps;
	long rightlen;
	long n = sCode.size();
	for (i = 0; i < n; i++) {
		if (hLevel[i] > lowestlevel) {
			lowestlevel = hLevel[i];
		}
	}

	for (i=0;i<n; i++) {
		if (hLevel[i] == lowestlevel) {
			s = sCode[i];
			rightlen = nPos-s.GetLength();
			temps = s;
			if (s.GetLength() > nPos) {
				s.Left(nPos);
				temps.Right(rightlen);
				rightlen = temps.Remove(' ');
				if (!temps.IsEmpty()) {
					return HC_CODETOOLONG;
				}
			}

			if (s.GetLength() <nPos) {
			// add spaces.
				long width = s.GetLength();
				char tempstr[100];
				sprintf(tempstr, "%*s", nPos - width, " ");
				s.Insert(0, tempstr);
			}

			sCode[i] = s;
		}
	}
	return 1;
}
bool CVariable::SetPeepCodes(CString Peep1, CString Peep2)
{
	if (IsPeeper)	{
		if ((Peep1.IsEmpty()) &&  (Peep2.IsEmpty()))	{
			return false;
		}
		if (Peep1.IsEmpty())	{
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
long CVariable::GetDepthOfHerarchicalBoom()
{
	long maxdepth =0,i;
	if (!IsHierarchical)	{
		return maxdepth;
	}
	else	{
		// figure out what to do with recodes
		for (i=0; i<nCode; i++)	{
			maxdepth = __max(maxdepth,hCode[i].Level);
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
	vector<CString> subcodes;
	if (IsHierarchical)	{
		return false;
	}
	else	{

		subcodes.resize(sCode.size() -1);
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
	vector<CString> subcode;
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
bool CVariable::FindChildren(long NumChild, vector<CString> & Child, long CodeIndex,
									  long *Index)
{
	long i, j;
	long ParentLevel;
	long ChildLevel;
	CString tempCode;
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

/*void CVariable:: ShowhCode()
{
	FILE *fdshow;
	CString filename;
	filename = "E:/Temp/hCodeout.txt";

	fdshow = fopen(filename, "a");

	int i;
	fprintf(fdshow,"%s\n","var1");
	fprintf(fdshow, "%s\n"," Code IsParent, Level, nChilderen");
	for (i=0; i<nCode; i++)	{
		fprintf(fdshow,"%s	%d		%d		%d\n", sCode.GetAt(i),
															/*hCode->Active, hCode->IsBogus,
															hCode->IsParent, hCode->Level,
															hCode->nChildren);
	}
	fprintf(fdshow,"%s\n", "______________________");
	fclose(fdshow);
}
*/

