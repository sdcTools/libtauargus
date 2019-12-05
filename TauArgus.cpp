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

#include "TauArgus.h"

#include <map>
#include <cstdio>
#include <cstring>
#include <cfloat>
#include <cmath>
#include <cstdarg>
#include <algorithm>
#include <vector>
#include <sstream>

#include "General.h"
#include "Globals.h"
#include "AMPL.h"
#include "Properties.h"
#include "PTable.h"
#include "PTableCont.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

double mysign(double x){
    return (x >= 0.0) ? 1.0 : -1.0; // 1 in case x >= 0, -1 otherwise
}

static Properties TauArgusErrors("TauArgus.properties");

//extern int CurrentHoldingNr;

/////////////////////////////////////////////////////////////////////////////
// TauArgus

#ifdef _DEBUG
	static int DEBUGprintf(const char *fmt, ...)
	{
		int retval = -1;
//		FILE *f = fopen("C:\\Users\\Gebruiker\\Documents\\debug.txt","a");
//		if (f) {
			va_list arg; /* argument pointer */
			va_start(arg, fmt); /* make arg point to first unnamed argument */
			retval = vfprintf(stderr, fmt, arg);
//			retval = vfprintf(f, fmt, arg);
//			fclose(f);
//		}	
		return retval;
	}
#else
	inline static int DEBUGprintf(const char *fmt, ...) { return 0; }
#endif

void TauArgus::SetProgressListener(IProgressListener* ProgressListener)
{
	m_ProgressListener = ProgressListener;
}

void TauArgus::FireUpdateProgress(int Perc)
{
	if (m_ProgressListener != NULL) {
		m_ProgressListener->UpdateProgress(Perc);
	}
}

// cells that are set as secondary unsafe to be undone
bool TauArgus::UndoSecondarySuppress(long TableIndex, long SortSuppress)
{
   if (TableIndex < 0 || TableIndex >= m_ntab) {
 		return false;
 	}
	if ((SortSuppress < 1) || (SortSuppress > 3)) {
		return false;
	}

   CTable *tab = GetTable(TableIndex);
   tab->UndoSecondarySuppress(SortSuppress);

 	return true;
}

// Set number of Variables
bool TauArgus::SetNumberVar(long nVar)
{
	if (nVar < 1) {
		return false;
	}

   CleanUp();  // in case of a second call very usefull

	m_nvar = nVar;
	m_var = new CVariable[m_nvar+1]; // Last variable for the empty variables
												// if the table is a freq table etc
	if (m_var == 0) {
		return false;
	}
	m_ntab = 0;

	m_var[m_nvar].nDec =0;   //Anders gaat het schrijven van JJ-files later mis!! Anco 1-6-2004

	return true;
}

// set number of tables
bool TauArgus::SetNumberTab(long nTab)
{
	// Not the right moment, first call SetNumberVar
	if (m_nvar == 0 || nTab < 1) {
		return false;
	}

	// ev. delete previous tables
	CleanTables();

	m_ntab = nTab;
	m_tab = new CTable[nTab + nTab];  // second part for recoded tables
	if (m_tab == 0) {
		return false;
	}

	return true;
}

// Compute the Tabels. In this function all the tables are filled.
bool TauArgus::ComputeTables(long *ErrorCode, long *TableIndex)
{
	// long MemSizeAll = 0, MemSizeTable;

	// initialize errorcodes
	*ErrorCode = -1;   // na
	*TableIndex = -1;  // na

	// Not the right moment, first call SetNumberVar
	if (m_nvar == 0) {
		*ErrorCode = NOVARIABLES;
		return false;
	}

	// Not the right moment, first call SetNumberTab
	if (m_ntab == 0) {
		*ErrorCode = NOTABLES;
		return false;
	}

	// First do ExploreFile
	/* if (m_UsingMicroData)  {
	  if (m_fname[0] == 0) {
		 *ErrorCode = NODATAFILE;
		 return false;
	  }
  }*/

	// First do Explore File
	if (!m_CompletedCodeList)  {
		*ErrorCode = NODATAFILE;
		return false;
	}


	// All tables set?
	int i;
	for (i = 0; i < m_ntab; i++) {
		if (m_tab[i].nDim == 0) {
			*ErrorCode = TABLENOTSET;
			return false;
		}
		/*if (m_tab[i].SafetyRule == 0) {
			*ErrorCode = TABLENOTSET;
			return false;
		}*/
  }

/* Is nu een aparte functie zie boven

  // compute memory size for each table
  for (i = 0; i < m_ntab; i++) {
    MemSizeTable = m_tab[i].GetMemSizeTable();
    MemSizeAll += MemSizeTable;
  }

  // check total memory to use
  if (MemSizeAll > MAXMEMORYUSE) {
    *ErrorCode = NOTENOUGHMEMORY;
    return false;
  }
*/
	// Prepare each table
	// Allocate memory for cells in tables
	for (i = 0; i < m_ntab; i++) {
		if (!m_tab[i].PrepareTable() ) {
			*ErrorCode = NOTABLEMEMORY;
			*TableIndex = i;
			return false;
		}
		// m_tab[i].nCell = m_tab[i].GetSizeTable();
  }


	// do datafile
	FILE *fd = fopen(m_fname, "r");
	if (fd == 0) {
		*ErrorCode = FILENOTFOUND;
		return false;
	}


	int recnr = 0;
	while (!feof(fd) ) {
		char str[MAXRECORDLENGTH];
		int res = ReadMicroRecord(fd, str);
		if (++recnr % FIREPROGRESS == 0) {
			FireUpdateProgress((int)(ftell(fd) * 100.0 / m_fSize));  // for progressbar in container
		}
		switch (res) {
			case -1:  // error
				goto error;
				break;
			case 0:  // eof
				goto oke;
				break;
			case 1:
				// here is where tables are filled
				FillTables(str);
				break;
		}
	}

oke:
	fclose(fd);
	FireUpdateProgress(100);  // for progressbar in container
	// Once more add freq
	// For holding tables the last holding has to be
	// corrected
	MergeLastTempShadow();


	// compute Status Cells for all base tables
	for (i = 0; i < m_ntab; i++) {
            ComputeCellStatuses(m_tab[i]);
	}

	// compute protection levels
	for (i=0; i<m_ntab; i++)	{
            SetProtectionLevels(m_tab[i]);
	}
        
        // compute cellkey from added recordkeys
        // only if record key variable is present
        for (i=0; i<m_ntab;i++){
            if (m_tab[i].CellKeyVarnr>=0){ ComputeCellKeys(m_tab[i]); }
        }


#ifdef _DEBUGG
{
	for (int i = 0; i < m_ntab; i++) {
		string fname;
		fname.Format("tab%02d.txt", i);
		ShowTable((LPCTSTR) fname, m_tab[i]);
	}
}
#endif // _DEBUG

	for (i = 0; i < m_nvar; i++) {
		if (m_var[i].IsHierarchical && m_var[i].nDigitSplit == 0) {
			// empty no longer needed arrays
			m_var[i].hLevel.clear();
			m_var[i].hLevelBasic.clear();
		}
	}
	return true;

error:
	// You could change the errors with throws.
	fclose(fd);
	return false;

}

bool TauArgus::DoRecode(long VarIndex, const char* RecodeString, long nMissing, const char* eMissing1, const char* eMissing2,
												long *ErrorType, long *ErrorLine, long *ErrorPos,
												const char** WarningString)
{

	// RecodeStrings are changed to string. Check this out
	int i, v = VarIndex, oke, maxwidth = 0;
	*ErrorType = *ErrorLine = *ErrorPos = -1;

	string Missing1 = eMissing1;
	string Missing2 = eMissing2;

	// for warnings
	m_nOverlap = 0;
	m_nUntouched = 0;
	m_nNoSense = 0;

	m_WarningRecode.resize(0);

	  // too early?
	  /*
	  if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
		 *ErrorType =  E_NOVARTABDATA;
		 return false;
	  }
	  */

	if (m_nvar == 0 || m_ntab == 0 || !m_CompletedCodeList) {
		*ErrorType =  E_NOVARTABDATA;
		return false;
	}

	// wrong VarIndex
	if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical) {
		*ErrorType =  E_VARINDEXWRONG;
		return false;
	}

	// remove existing one, if needed
	// have problems here
	/*
	if (m_var[v].HasRecode) {
			UndoRecode(VarIndex);
		}*/

  // only check syntax, phase = CHECK
	oke = ParseRecodeString(v, RecodeString, ErrorType, ErrorLine, ErrorPos, CHECK);
	if (!oke) {
		return false;
	}

	// recode oke

	// remove old recode
	if (m_var[v].Recode.DestCode != 0) {
		free(m_var[v].Recode.DestCode);
		m_var[v].Recode.DestCode = 0;
	}

	// initialize new recode
	m_var[v].Recode.DestCode = (int *) malloc(m_var[v].nCode * sizeof(int) );
	if (m_var[v].Recode.DestCode == 0) {
		*ErrorType = NOTENOUGHMEMORY;
		return false;
	}

	for (i = 0; i < m_var[v].nCode; i++) {
		if (m_var[v].IsCodeBasic(i) ) m_var[v].Recode.DestCode[i] = -1;  // no (sub)total
		else                          m_var[v].Recode.DestCode[i] = -2;  // (sub)total
	}

	// Missing2 specified but not Missing1? Swap!
	if (Missing1.empty() && !Missing2.empty()) {
		Missing1 = Missing2;
	}

	// no missings for recode specified? Take the missing(s) of the source
	if (Missing1.empty() && Missing2.empty()) {
		Missing1 = m_var[v].Missing1;
		if (m_var[v].nMissing == 2) {
			Missing2 = m_var[v].Missing2;
		}
		else {
			Missing2 = Missing1;
		}
	}

	m_var[v].Recode.sCode.clear();
	m_var[v].Recode.nCode = 0;
	m_var[v].Recode.CodeWidth = 0;
	m_var[v].Recode.nMissing = 0;

	m_var[v].Recode.sCode.push_back(""); // for Total

	// another time, now compute list of dest codes
	ParseRecodeString(v, RecodeString, ErrorType, ErrorLine, ErrorPos, DESTCODE);
	if (m_var[v].Recode.sCode.size() < 2) {
		*ErrorType = E_EMPTYSPEC;
		*ErrorLine = 1;
		*ErrorPos = 1;
		return false;
	}
	// sort list of dest codes, still without missing values (coming soon)
	QuickSortStringArray(m_var[v].Recode.sCode);


	// now the number of codes is known, but not the not mentioned ones
	// m_var[v].Recode.nCode = m_var[v].Recode.sCode.GetSize();

	// again, now compute dest codes and link between dest and src
	oke = ParseRecodeString(v, RecodeString, ErrorType, ErrorLine, ErrorPos, SRCCODE);
	if (!oke) {
		return false; // missing to valid codes, a terrible shame
	}

	// compute untouched codes, add them to the recode codelist
	m_nUntouched = 0;
	for (i = 0; i < m_var[v].nCode - m_var[v].nMissing; i++) {
		if (m_var[v].Recode.DestCode[i] == -1) { // not touched
			m_nUntouched++;
			AddRecode(v, m_var[v].sCode[i].c_str());
		}
	}

	// make all recode codes same width
	maxwidth = MakeRecodelistEqualWidth(v, Missing1.c_str(), (LPCTSTR) Missing2.c_str());

	// remove the missing codes,
	{
		if (!((nMissing == 0) && (m_var[v].nMissing ==0)))	{
			int n;
			bool IsMissing;
			string mis = Missing1;
			AddSpacesBefore(mis, maxwidth);
			if (n = BinSearchStringArray(m_var[v].Recode.sCode, mis, 0, IsMissing), n >= 0) {
				vector<string>::iterator p = m_var[v].Recode.sCode.begin() + n;
				m_var[v].Recode.sCode.erase(p);
			}
			mis = Missing2;
			AddSpacesBefore(mis, maxwidth);
			if (n = BinSearchStringArray(m_var[v].Recode.sCode, mis, 0, IsMissing), n >= 0) {
				vector<string>::iterator p = m_var[v].Recode.sCode.begin() + n;
				m_var[v].Recode.sCode.erase(p);
			}
		}
	}

	// sort list of dest codes, still without missing values (coming soon)
	QuickSortStringArray(m_var[v].Recode.sCode);

	// ADD MISSING1 AND -2
	// both empty impossible, see start of function
	// swap missings if missing1 empty

	if (!((nMissing == 0) && (m_var[v].nMissing ==0)))	{

		if (Missing1.empty() && Missing2.empty()) {             // no missing specified?
			m_var[v].Recode.Missing1 = m_var[v].Missing1;  // take the missing of source variable
			m_var[v].Recode.Missing2 = m_var[v].Missing2;
		}
		else {
			if (Missing1.empty()) {  // at least one missing specified
				m_var[v].Recode.Missing1 = Missing2;
				m_var[v].Recode.Missing2 = Missing1;
			}
			else {
				m_var[v].Recode.Missing1 = Missing1;
				m_var[v].Recode.Missing2 = Missing2;
			}
		}

		// second empty?
	if (m_var[v].Recode.Missing2.empty()) {
		m_var[v].Recode.Missing2 = m_var[v].Recode.Missing1;
		m_var[v].Recode.nMissing = 1;
	}
	else {
		// equal?
		if (m_var[v].Recode.Missing1 == m_var[v].Recode.Missing2) {
			m_var[v].Recode.nMissing = 1;
		}
		else {
			m_var[v].Recode.nMissing = 2;
		}
	}

	// put in list, last one or two
	AddSpacesBefore(m_var[v].Recode.Missing1, maxwidth);
	AddRecode(v, m_var[v].Recode.Missing1.c_str());
	if (m_var[v].Recode.nMissing == 2) {
		 AddSpacesBefore(m_var[v].Recode.Missing2, maxwidth);
		 AddRecode(v, m_var[v].Recode.Missing2.c_str());
	}
}
  // last time, now compute dest codes and link between dest and src
  // for warnings
	m_nOverlap = 0;
	m_nNoSense = 0;
	for (i = 0; i < m_var[v].nCode; i++) {
		if (m_var[v].IsCodeBasic(i) ) m_var[v].Recode.DestCode[i] = -1;  // no (sub)total
		else                          m_var[v].Recode.DestCode[i] = -2;  // (sub)total
	}

	oke = ParseRecodeString(v, RecodeString, ErrorType, ErrorLine, ErrorPos, SRCCODE);
	if (!oke) {
		return false; // missing to valid codes, a terrible shame
	}

	// yep, the number of codes is known and the codes are sorted (except one or two MISSINGs at the end of te list)
	m_var[v].Recode.nCode = m_var[v].Recode.sCode.size();


	// do the same for MISSINGS, more complicated
  {
		RECODE *c = &(m_var[v].Recode); // for easier reference

		i = m_var[v].nCode - m_var[v].nMissing;     // first missing
		if (c->DestCode[i] == -1) {                 // missing 1 not specified
			c->DestCode[i] = c->nCode - c->nMissing;  // make missing1 equal to missing1 source
		}
		if (m_var[v].nMissing == 2) {               // two missings in source?
			i++;
			if (c->nMissing == 2) {                   // two missings in dest?
				if (c->DestCode[i] == -1) {                     // missing 2 not specified
					c->DestCode[i] = c->nCode - c->nMissing + 1;  // make missing2 equal to missing2 source
				}
			}
			else {
				if (c->DestCode[i] == -1) {                 // missing 2 not specified
					c->DestCode[i] = c->nCode - c->nMissing;  // make missing2 equal to missing1 source
				}
			}
		}
	}

	// set untouched codes on right index
  {
		RECODE *c = &(m_var[v].Recode); // for easier reference
		bool IsMissing;
		int index, ncode = m_var[v].nCode - m_var[v].nMissing;
		for (i = 0; i < ncode; i++) {
			if (c->DestCode[i] == -1) {
				string str = m_var[v].sCode[i];
				AddSpacesBefore(str, c->CodeWidth);
				index = BinSearchStringArray(c->sCode, str, c->nMissing, IsMissing);
				ASSERT(index >= 0 && index < c->nCode);
				c->DestCode[i] = index;
			}
		}
	}

	// WARNINGS in recode:

  // show untouched codes:
  ostringstream ss;
	if (m_nUntouched > 0) {
		ss << "Number of untouched codes: " << m_nUntouched << "\r\n";
	}

	// show warnings
	if (m_nOverlap > 0) {
		ss << "Number of overlapping codes: " << m_nOverlap << "\r\n";
	}

	if (m_nNoSense > 0) {
		ss << "Number of \"no sense\" codes: " << m_nNoSense << "\r\n";
	}
	m_WarningRecode = ss.str();

	if (m_WarningRecode.empty() ) {
		m_WarningRecode = "Recode OK";
	}


	*WarningString = m_WarningRecode.c_str();

	m_var[v].HasRecode = true;

	m_var[v].Recode.nCode = m_var[v].Recode.sCode.size();

	m_var[v].Recode.sCode[0] = "";

	return true;
}

// Apply Recoding. You need this for recoding tables
void TauArgus::ApplyRecode()
{
	ComputeRecodeTables();
}

// Clean all allocated memory. Destructor does this
void TauArgus::CleanAll()
{
	CleanUp();
}

// Used for setting Hierarchical Variables with digit Split
bool TauArgus::SetHierarchicalDigits(long VarIndex, long nDigitPairs, long *nDigits)
{
	if (VarIndex < 0 || VarIndex >= m_nvar || !m_var[VarIndex].IsHierarchical) {
		return false;
	}

	if (m_var[VarIndex].hLevel.size() != 0) {
		return false;
	}

	// return m_var[VarIndex].SetHierarchicalDigits(nDigitPairs, nDigits);
	if (m_var[VarIndex].SetHierarchicalDigits(nDigitPairs, nDigits) == 0)	{
			return false;
	}
	else	{
		return true;
	}
}

// For every row get all cells with corresponding information
bool TauArgus::GetTableRow(long TableIndex, long *DimIndex, double *Cell,
													long *Status, long CountType)
{
	int coldim = -1, nCodes;
	long DimNr[MAXDIM];
	long DoAudit, CountTypeLocal;

	// check parameters
	if (TableIndex < 0 || TableIndex >= m_ntab) {
		return false;
	}
    DoAudit = 0;
	CountTypeLocal = CountType;
	if (CountTypeLocal < 0 ) {
		DoAudit = 1;
        CountTypeLocal = -CountTypeLocal;
	}

	CTable *Table = GetTable(TableIndex);

	// check DimIndices
	for (int i = 0; i < Table->nDim; i++) {
		CVariable *var = &(m_var[Table->ExplVarnr[i]]);
		nCodes = var->GetnCode();
		DimNr[i] = DimIndex[i];
		switch (DimIndex[i]) {
			case -1:
				if (coldim == -1) {
					coldim = i;
				}
				else {
					return false;  // more than one coldim specified
				}
				break;
			default:
				if (DimIndex[i] < 0 || DimIndex[i] >= nCodes) {
					return false;
				}
				break;
		}
	}

	if (coldim == -1) { // no coldim specified
		return false;
	}

	// parameters oke
	CDataCell *dc;
	nCodes = m_var[Table->ExplVarnr[coldim]].GetnCode();
	for (int c = 0; c < nCodes; c++) {
		// fill table row cells
		DimNr[coldim] = c;
		dc = Table->GetCell(DimNr); // get pointer to cell from table
		switch (CountTypeLocal) {
			case CT_RESPONS:
				Cell[c] = dc->GetResp();
				break;
			case CT_SHADOW:
				Cell[c] = dc->GetShadow();
				break;
			case CT_COST:
				Cell[c] = dc->GetCost(Table->Lambda);
				break;
			case CT_ROUNDEDRESP:
				 Cell[c] = dc->GetRoundedResponse();
				 break;
			case CT_CTA:
				 Cell[c] = dc->GetCTAValue();
				 break;
			default:
				return false;
		}

		// set Status
		Status[c] = dc->GetStatus();
		if (DoAudit==1) {
		  double 	XRL, XRU, XPL, XPU, XC;
		  if ((Status[c] >= CS_UNSAFE_RULE) && (Status[c] <= CS_UNSAFE_MANUAL )) {
			XRL = dc->GetRealizedLowerValue();
			XRU = dc->GetRealizedUpperValue();
			XPL = dc->GetLowerProtectionLevel();
			XPU = dc->GetUpperProtectionLevel();
			XC =  dc->GetResp();

			if ( (XRU < (XC + XPU)) || (XRL > (XC-XPL)) ){
				Status[c] = -Status[c];
			}
		  }//prim
		  if ((Status[c] == CS_SECONDARY_UNSAFE) || (Status[c] == CS_SECONDARY_UNSAFE_MANUAL )) {
  			XRL = dc->GetRealizedLowerValue();
			XRU = dc->GetRealizedUpperValue();
			XC =  dc->GetResp();
			if ((XRU==XC) && (XRL==XC)) {
				Status[c]=-Status[c];
			}
		  } // sec
		} //if audit
	} //for

	return true;
}

// return information about Unsafe Variable
bool TauArgus::UnsafeVariable(long VarIndex, long *Count, long *UCArray)
{
	int t;

	if (VarIndex < 0 || VarIndex >= m_nvar) {
		return false;
	}

	// compute count
	*Count = 0;
	for (t = 0; t < m_ntab; t++) {
		CTable *tab = GetTable(t);
		if (tab->nDim > *Count) {
			*Count = tab->nDim;
		}
	}

	memset(UCArray, 0, *Count * sizeof(long) );

	// compute nUnsafe for variable VarIndex, add to UCArray
	long nUnsafe[MAXDIM];
	for (t = 0; t < m_ntab; t++) {
		CTable *tab = GetTable(t);
		int var;
		for (var = 0; var < tab->nDim; var++) {
			if (tab->ExplVarnr[var] == VarIndex) break;  // hebbes
		}
		if (var < tab->nDim) {
			tab->GetUnsafeCells(VarIndex, nUnsafe);
			for (int i = 0; i < tab->nDim; i++) {
				UCArray[i] += nUnsafe[i];
			}
		}
	}

	TRACE("Var %d unsafe %d %d %d\n", VarIndex, UCArray[0], UCArray[1], UCArray[2]);

	return true;
}

// In this function the input file is read and the code list is built
bool TauArgus::ExploreFile(const char* FileName, long *ErrorCode, long *LineNumber, long *ErrorVarIndex)
{
   char str[MAXRECORDLENGTH];
   int i, length, recnr = 0, Result;

   *ErrorCode = *LineNumber = 0;
   *ErrorVarIndex = -1;

   if (m_nvar == 0) {
		*ErrorCode = NOVARIABLES;
		return false;
   }

	for (i = 0; i < m_nvar; i++) {
		if (InFileIsFixedFormat) {
			if (m_var[i].bPos < 0) {
				*ErrorCode = VARIABLENOTSET;
				return false;
			}
		}
		else {
			if (!m_var[i].PositionSet) {
				*ErrorCode = VARIABLENOTSET;
				return false;
			}
		}
	}

	FILE *fd = fopen(FileName, "r");
	if (fd == 0) {
		*ErrorCode = FILENOTFOUND;
		return false;
	}

	fseek(fd, 0, SEEK_END);
	m_fSize = ftell(fd);
	rewind(fd);


	// read first record to determine the fixed recordlength
	str[0] = 0;
	fgets((char *)str, MAXRECORDLENGTH, fd);
	if (str[0] == 0) {
		*ErrorCode = EMPTYFILE;
		goto error;
	}

	length = strlen((char *)str) - 1;
	while (length > 0 && str[length] < ' ') length--;
	m_fixedlength = length + 1;
	if (length == 0) {
		*ErrorCode = EMPTYFILE; // first record empty
		goto error;
	}

	if (!InFileIsFixedFormat) {
		m_maxBPos = -1;
		for (i = 0; i < m_nvar; i++) {
			if (m_var[i].bPos > m_maxBPos) {
				m_maxBPos = m_var[i].bPos;            
			}
		}
   }

	// record length oke? hierachie oke?
	for (i = 0; i < m_nvar; i++) {
		if (InFileIsFixedFormat) {
			if (m_var[i].bPos + m_var[i].nPos > m_fixedlength) {
				*ErrorCode = RECORDTOOSHORT;
				goto error;
			}
		}

		if (m_var[i].IsHierarchical) {
			if (m_var[i].nDigitSplit == 0 && m_var[i].hLevel.size() == 0) {
				*ErrorCode = WRONGHIERARCHY;
				goto error;
			}
		}

		// initialize Min and Max Value for Numerics
		if (m_var[i].IsNumeric) {
			m_var[i].MaxValue = -DBL_MAX;
			m_var[i].MinValue = DBL_MAX;
		}
	}

	// read micro data to make codelists for categorical variables
	rewind(fd);
	while (!feof(fd) ) {
		int res, varindex;
		res = ReadMicroRecord(fd, str);
		switch (res) {
			case -1: // error
				recnr++;
				*ErrorCode = WRONGLENGTH;
				*LineNumber = recnr;
				goto error;
			case  0: // eof
				goto oke;
			case  1: // oke
				recnr++;
				if (recnr % FIREPROGRESS == 0) {
					FireUpdateProgress((int)(ftell(fd) * 100.0 / m_fSize));  // for progressbar in container
				}
				// Here is where the code lists are built
				if (Result = DoMicroRecord(str, &varindex), Result >= 1000) { // error?
					*ErrorCode = Result;
					*LineNumber = recnr;
					*ErrorVarIndex = varindex;
					goto error;
				}
				break;
		}
	}

	oke:
	fclose(fd);
	m_nRecFile = recnr;
	FireUpdateProgress(100);  // for progressbar in container

	// add hierarchy, Missing1 and -2
	for (i = 0; i < m_nvar; i++) {
		// add hierarchy for split levels
		if (m_var[i].nDigitSplit > 0) {
			if (!m_var[i].ComputeHierarchicalCodes() ) {
				*ErrorCode = WRONGHIERARCHY;
				*LineNumber = -1;
				return false;
			}
		}

		// add total
		if (!m_var[i].IsHierarchical || m_var[i].nDigitSplit > 0) {
			m_var[i].AddCode("", false);
		}

		// add Missing1 and -2
		if (m_var[i].IsCategorical) {
			if (m_var[i].nMissing != 0)	{
				m_var[i].AddCode(m_var[i].Missing1.c_str(), true);
				if (m_var[i].IsHierarchical && m_var[i].nDigitSplit == 0) {
					m_var[i].hLevel.push_back(1);
					m_var[i].hLevelBasic.push_back(true);
				}
				if (m_var[i].nMissing == 2) {
					m_var[i].AddCode(m_var[i].Missing2.c_str(), true);
					if (m_var[i].IsHierarchical && m_var[i].nDigitSplit == 0) {
						m_var[i].hLevel.push_back(1);
						m_var[i].hLevelBasic.push_back(true);
					}
				}
			}
		}

		m_var[i].nCode = m_var[i].sCode.size();
		// allocate and initialize memory
		if (m_var[i].IsHierarchical) {
			if (!m_var[i].SetHierarch() ) {
				*ErrorCode = WRONGHIERARCHY;
				*LineNumber = -1;
				return false;
			}
		}
	}

	strcpy(m_fname, FileName);  // Save name for use in ComputeTables
	m_CompletedCodeList = true;
	// ShowCodeLists();  // in output pane

	return true;

error:
	fclose(fd);

	return false;
}

// get maximum unsafe Combination
long TauArgus::GetMaxnUc()
{
	if (m_nvar == 0  ||  m_ntab  == 0  || !m_CompletedCodeList)  {
		return -1;
	}

	int max = 0;
	for (int t = 0; t < m_ntab; t++) {
		CTable *tab;
		tab = GetTable(t);
		for (int c = 0; c < tab->nCell; c++) {
			long tempstatus = tab->GetCell(c)->GetStatus();
			if ((tempstatus == CS_UNSAFE_FREQ) || (tempstatus == CS_UNSAFE_PEEP) ||
				( tempstatus == CS_UNSAFE_RULE) || (tempstatus == CS_UNSAFE_SINGLETON) ||
				(tempstatus == CS_UNSAFE_ZERO)) max++;
		}
	}

	return max;
}

// Undo recode. Undo recodes for a variable. This is used when a table is
// created to be recoded
bool TauArgus::UndoRecode(long VarIndex)
{
	/*if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
    return false;
	}*/

	if (m_nvar == 0  || m_ntab == 0  || !m_CompletedCodeList)  {
		return false;
	}

	// wrong VarIndex
	if (VarIndex < 0 || VarIndex >= m_nvar || !m_var[VarIndex].IsCategorical) {
		return false;
	}

	m_var[VarIndex].UndoRecode();

	// recomputes for all tables the flag HasRecode
	SetTableHasRecode();

	return true;
}


//Sets the status of a cell to a given status
bool TauArgus::SetTableCellStatus(long TableIndex, long *DimIndex, long CelStatus)
{

	// redo protection levels
   int iOriginalStatus;
	CDataCell *dc;

	// check parameters
   if (TableIndex < 0 || TableIndex >= m_ntab) {
		return false;
	}
	//if (m_fname[0] == 0)      		return false;
   if (!m_CompletedCodeList) {
		return false;
	}

	// check CellStatus; status EMPTY cannot be set
	// Note empty can be set. empty means freq is nul
   if (CelStatus < CS_SAFE || CelStatus >= CS_EMPTY) {
		return false;
	}

	CTable *table = GetTable(TableIndex);
	//dc = table ->GetCell(DimIndex);
	//iTemporaryStatus = table->ComputeCellSafeCode(*dc);

	// check DimIndices
	for (int i = 0; i < table->nDim; i++) {
		int nCodes = m_var[table->ExplVarnr[i]].GetnCode();
		ASSERT(DimIndex[i] >= 0 && DimIndex[i] < nCodes);
		if (DimIndex[i] < 0 || DimIndex[i] >= nCodes)  {
			return false;
		}
	}

	// set CellStatus
	// if manual unsafe or manual safe then check rules.
	// So that you set the status to the original status.
	// Manual unsafe is set as unsafe if the cel was already unsafe
	//	Manual safe is set as safe if the cel was already safe
		dc = table ->GetCell(DimIndex);
		iOriginalStatus = table->ComputeCellSafeCode(*dc);
	if (CelStatus == CS_UNSAFE_MANUAL)	{
		dc = table ->GetCell(DimIndex);
		iOriginalStatus = table->ComputeCellSafeCode(*dc);
		if(! ((iOriginalStatus == CS_UNSAFE_FREQ) || (iOriginalStatus == CS_UNSAFE_PEEP) ||
			(iOriginalStatus == CS_UNSAFE_RULE) || (iOriginalStatus == CS_UNSAFE_SINGLETON) ||
			(iOriginalStatus == CS_UNSAFE_ZERO)))	{
			dc->SetStatus(CelStatus);
			// redo protectionlevel
			table->SetProtectionLevelCell(*dc);
		}
		else	{
			dc->SetStatus(iOriginalStatus);
			table->SetProtectionLevelCell(*dc);
		}

   }
	else if (CelStatus == CS_SAFE_MANUAL)	{
		dc = table ->GetCell(DimIndex);
      iOriginalStatus = table->ComputeCellSafeCode(*dc);
      if (!(iOriginalStatus == CS_SAFE))	{
			dc ->SetStatus(CelStatus);
			table->SetProtectionLevelCell(*dc);
      }
		else	{
			dc	-> SetStatus(iOriginalStatus);
			table->SetProtectionLevelCell(*dc);
		}
   }
	else if (CelStatus == CS_EMPTY_NONSTRUCTURAL)	{
		dc = table ->GetCell(DimIndex);
      iOriginalStatus = table->ComputeCellSafeCode(*dc);
		if (iOriginalStatus == CS_EMPTY)	{
			table->GetCell(DimIndex)->SetStatus(CelStatus);
			table->SetProtectionLevelCell(*dc);
		}
	}
   else
   {
      table->GetCell(DimIndex)->SetStatus(CelStatus);
   }

	return true;
}

// Set the cost function for a cell
bool TauArgus::SetTableCellCost(long TableIndex, long *DimIndex, double Cost)
{

	// check parameters
   if (TableIndex < 0 || TableIndex >= m_ntab) {
		return false;
	}
	//if (m_fname[0] == 0)      		return false;
   if (!m_CompletedCodeList) {
		return false;
	}

	CTable *table = GetTable(TableIndex);
	//dc = table ->GetCell(DimIndex);
	//iTemporaryStatus = table->ComputeCellSafeCode(*dc);

	// check DimIndices
	for (int i = 0; i < table->nDim; i++) {
		int nCodes = m_var[table->ExplVarnr[i]].GetnCode();
		ASSERT(DimIndex[i] >= 0 && DimIndex[i] < nCodes);
		if (DimIndex[i] < 0 || DimIndex[i] >= nCodes)  {
			return false;
		}
	}
	CDataCell *dc = table ->GetCell(DimIndex);
	dc->SetCost(Cost);
	return true;
}

// Get Status and Cost per dimensie
bool TauArgus::GetStatusAndCostPerDim(long TableIndex, long *Status, double *Cost)
{
	if (TableIndex < 0 || TableIndex >= m_ntab) {
		return false;
	}

	CTable *tab = GetTable(TableIndex);
	tab->GetStatusAndCostPerDim(Status, Cost);
	return true;
}

// Set a Variable code as Active, This is important for
// recoding tables. This gives that the respective code is active
// and will be used in recoding
bool TauArgus::SetVarCodeActive(long VarIndex, long CodeIndex, bool Active)
{
	int v = VarIndex, c = CodeIndex;

	if (v < 0 || v >= m_nvar) {
		return false;
	}
	if (!m_var[v].IsHierarchical) {
		return false;
	}
	if (c < 0 || c >= m_var[v].nCode - m_var[v].nMissing)	{
		return false;
	}
	if (!m_var[v].hCode[c].IsParent)  {
		return false;
	}

	m_var[v].SetActive(c, Active);

	return true;
}

// Get the number of codes for the given variable
bool TauArgus::GetVarNumberOfCodes(long VarIndex, long *NumberOfCodes,
															long *NumberOfActiveCodes)
{
	if (VarIndex < 0 || VarIndex >= m_nvar) {
		return false;
	}

	*NumberOfCodes = m_var[VarIndex].GetnCode();
	*NumberOfActiveCodes = m_var[VarIndex].GetnCodeActive();

	return true;
}

// Do recode for all active codes
bool TauArgus::DoActiveRecode(long VarIndex)

{
	// too early?
 /* if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
    return false;
  }*/

	if  (m_nvar == 0 || m_ntab == 0 || !m_CompletedCodeList)  {
		return false;
	}

  // wrong VarIndex, not hierarchical
	if (VarIndex < 0 || VarIndex >= m_nvar || !m_var[VarIndex].IsCategorical || !m_var[VarIndex].IsHierarchical) {
		return false;
	}

	// nothing to do?
	if (m_var[VarIndex].hCode == 0 || m_var[VarIndex].GetnCodeInActive() == 0) {
		return false;
	}

	return m_var[VarIndex].SetHierarchicalRecode();;
}

// Set Variable. All information to set in the variable object is given
bool TauArgus::SetVariable(long VarIndex, long bPos,
				long nPos, long nDec, long nMissing, const char* Missing1,
				const char* Missing2, const char* TotalCode, bool IsPeeper,
				const char* PeeperCode1, const char* PeeperCode2,
				bool IsCategorical,
				bool IsNumeric, bool IsWeight,
				bool IsHierarchical,
				bool IsHolding,
                                bool IsRecordKey)
{
	// index oke?
	if (VarIndex < 0 || VarIndex >= m_nvar+1) {
		return false;
	}
        
	// holding?
	if (IsHolding) {
		if (m_VarNrHolding >= 0)	{
			return false;  // holding variable already given
		}
		if (IsCategorical || IsNumeric || IsWeight || IsHierarchical) {
			return false;
		}
		m_VarNrHolding = VarIndex;
	}

	// weight?
	if (IsWeight) {
		if (!IsNumeric)	{
			return false;
		}
		if (m_VarNrWeight >= 0) {
			return false;
		}// weight variable already given
		if (IsCategorical || IsHierarchical)	{
			return false;
		}
		m_VarNrWeight = VarIndex;
	}

	// Categorical variables should have at least one Missing
/*	if (IsCategorical && Missing1[0] == 0 && Missing2[0] == 0)	{
		return false;
	}*/
        
	// save properties
	if (!m_var[VarIndex].SetPosition(bPos, nPos, nDec) )	{
		return false;
	}
	if (!m_var[VarIndex].SetType(IsCategorical, IsNumeric, IsWeight, IsHierarchical,
	  IsHolding, IsPeeper, IsRecordKey) ) {
		return false;
	}
	if (nMissing < 0 || nMissing > 2) {
		return false;
	}
	if (!m_var[VarIndex].SetMissing(Missing1, Missing2, nMissing) )	{
		return false;
	}
	if (!m_var[VarIndex].SetTotalCode(TotalCode) ) {
		return false;
	}
	if (!m_var[VarIndex].SetPeepCodes(PeeperCode1, PeeperCode2)) {
		return false;
	}
        
        if (IsRecordKey){
		if (m_VarNrRecordKey >= 0) {
			return false;
		}// recordkey variable already given
		if (IsCategorical || IsWeight || IsHierarchical || IsHolding) {
			return false;
		}
                m_VarNrRecordKey = VarIndex;
        }
	return true;
}

// Sets all the information for the Table object this together with
// SetTableSafety does the trick.
bool TauArgus::SetTable(long Index, long nDim, long *ExplanatoryVarList,
                        bool IsFrequencyTable,
			long ResponseVar, long ShadowVar, long CostVar, long CellKeyVar,
                        const char* CKMType, 
                        long CKMTopK,
			double Lambda,
			double MaxScaledCost,
			long PeepVarnr,
			bool SetMissingAsSafe)
{
	int i = Index, j;
	long nd;

	// check TableIndex
	if (m_nvar == 0 || i < 0 || i >= m_ntab) {
		return false;
	}

	// check number of dimensions
	if (nDim < 1 || nDim > MAXDIM) {
		return false;
	}

	if (!IsFrequencyTable)	{
		if (ResponseVar < 0 || ResponseVar >= m_nvar || !m_var[ResponseVar].IsNumeric) {
			return false;
		}
		nd = - m_var[ResponseVar].nDec;
		m_tab[i].MinLPL = pow(10.0, nd);

		if (ShadowVar < 0 || ShadowVar >= m_nvar || !m_var[ShadowVar].IsNumeric) {
			return false;
		}

		// -1 : Count every record for 1 (= Freq)
		// -2 : Eacht table cell, incl. (sub)totals, fixed value 1
		if (CostVar != -1 && CostVar != -2 && CostVar != -3) {
			if (CostVar < 0 || CostVar >= m_nvar || !m_var[CostVar].IsNumeric) {
				return false;
			}
		}
	}
	else	{
		m_tab[i].IsFrequencyTable = true;
		ResponseVar = m_nvar;
	}

  // ExploreFile done?
	/*if (m_UsingMicroData)  {
		if (m_fname[0] == 0) {
			return false;
		}
	}*/

	// Check if explore file is done
	if (!m_CompletedCodeList)	{
		return false;
	}


	// check variable indices, variable should be categorical
	for (j = 0; j < nDim; j++) {
		int d = ExplanatoryVarList[j]; // index of variable, 1 .. m_nvar is oke
		if (d < 0 || d >= m_nvar) {
			return false;
		}
		if (!m_var[d].IsCategorical) { // property set in SetVariable(...)
			return false;
		}
	}

	// set MissingAsSafe
	if(SetMissingAsSafe)	{
		m_tab[i].SetMissingAsSafe = true;
	}
	else	{
		m_tab[i].SetMissingAsSafe = false;
	}

	// set Lambda
	if (Lambda<0 || Lambda >1) {
		return false;
	}
	m_tab[i].Lambda = Lambda;
	if (MaxScaledCost <1)	{
		return false;
	}
	m_tab[i].MaxScaledCost = MaxScaledCost;
	// set table variables
        m_tab[i].SetVariables(nDim, ExplanatoryVarList, ResponseVar, ShadowVar, CostVar, CellKeyVar, PeepVarnr);

	// add SizeDim to tab
	for (int d = 0; d < nDim; d++) {
		m_tab[i].SetDimSize(d, m_var[ExplanatoryVarList[d]].nCode);
	}
        
        m_tab[i].CKMType = CKMType;
        m_tab[i].CKMTopK = CKMTopK;
	return true;
}


bool TauArgus::GetTableCellValue(long TableIndex, long CellIndex, double *CellResponse)
{	
	if (!m_CompletedCodeList)  {
		return false;
	}

	CTable *table = GetTable(TableIndex);

	CDataCell *dc = table->GetCell(CellIndex);

	*CellResponse = dc->GetResp();

	return true;
}


// Returns the information in a cell.
bool TauArgus::GetTableCell(long TableIndex, long *DimIndex, double *CellResponse, double *CellRoundedResp, double *CellCTAResp,
                                double *CellCKMResp,
                                double *CellShadow, double *CellCost, double *CellKey, double *CellKeyNoZeros,
				long *CellFreq, long *CellStatus,
				double *CellMaxScore,double *CellMAXScoreWeight,
				long *HoldingFreq,
				double *HoldingMaxScore, long *HoldingNrPerMaxScore,
				double *PeepCell, double *PeepHolding, long *PeepSortCell, long *PeepSortHolding,
				double *Lower, double *Upper,
				double *RealizedLower,double *RealizedUpper)
{
	int i;

	// check parameters
	if (TableIndex < 0 || TableIndex >= m_ntab) {
		return false;
	}

  /*if (m_UsingMicroData)  {
	if (m_fname[0] == 0) {
			return false;
		}
  }
  */

	if (!m_CompletedCodeList)  {
		return false;
	}

	CTable *table = GetTable(TableIndex);

	// check DimIndices
	for (i = 0; i < table->nDim; i++) {
		int nCodes = m_var[table->ExplVarnr[i]].GetnCode();
		ASSERT(DimIndex[i] >= 0 && DimIndex[i] < nCodes);
		if (DimIndex[i] < 0 || DimIndex[i] >= nCodes) {
			return false;
		}
	}

	CDataCell *dc = table->GetCell(DimIndex);

	*CellResponse = dc->GetResp();
	*CellRoundedResp = dc->GetRoundedResponse();
	*CellCTAResp = dc->GetCTAValue();
        *CellCKMResp = dc->GetCKMValue();
	*CellShadow  = dc->GetShadow();
	*CellCost    = dc->GetCost(table->Lambda);
        *CellKey     = dc->GetCellKey();
        *CellKeyNoZeros = dc->GetCellKeyNoZeros();
	*CellFreq    = dc->GetFreq();
	*CellStatus  = dc->GetStatus();
	*RealizedUpper = dc->GetRealizedUpperValue();
	*RealizedLower = dc->GetRealizedLowerValue();
	*Lower = dc->GetLowerProtectionLevel();
	*Upper = dc->GetUpperProtectionLevel();
	*HoldingFreq = dc->GetFreqHolding();
	*PeepCell = dc->GetPeepCell();
	*PeepHolding = dc->GetPeepHolding();
	*PeepSortCell = dc->GetPeepSortCell();
	*PeepSortHolding = dc->GetPeepSortHolding();

	for (i = 0; i < dc->nMaxScoreCell && i < *CellFreq; i++) {
		CellMaxScore[i] = dc->MaxScoreCell[i];
		if (table->ApplyWeight)	{
			CellMAXScoreWeight[i] = dc->MaxScoreWeightCell[i];
		}
	}
	for (; i < dc->nMaxScoreCell; i++) {
		CellMaxScore[i] = 0;
		if (table->ApplyWeight)	{
			CellMAXScoreWeight[i] = 0;
		}
	}
	if ((table->ApplyHolding) && (dc->nMaxScoreHolding >0))	{
		for (i= 0; i < dc->nMaxScoreHolding; i++)	{
			HoldingMaxScore[i] = dc->MaxScoreHolding[i];
			HoldingNrPerMaxScore[i] = dc->HoldingnrPerMaxScore[i];
		}
	}

	return true;
}

// Set information necessary to build a tableobject. This functions works together with SetTable.
bool TauArgus::SetTableSafety( long Index, bool DominanceRule,
														long * DominanceNumber,
														long * DominancePerc,
														bool PQRule,
														long * PriorPosteriorP,
														long * PriorPosteriorQ,
														long * PriorPosteriorN,
														long * SafeMinRecAndHoldings,
														long * PeepPerc,
														long * PeepSafetyRange,
														long * PeepMinFreqCellAndHolding,
														bool ApplyPeep,
														bool ApplyWeight, bool ApplyWeightOnSafetyRule,
														bool ApplyHolding,bool ApplyZeroRule,
														bool EmptyCellAsNonStructural, long NSEmptySafetyRange,
														double ZeroSafetyRange,	long ManualSafetyPerc,
														long * CellAndHoldingFreqSafetyPerc)
{
	int i = Index;
	// check TableIndex
	if (m_nvar == 0 || i < 0 || i >= m_ntab) {
		return false;
	}

	// Holding and weights cannot be applied at the same time
	if (ApplyHolding && ApplyWeight)	{
		return false;
	}
	if (ApplyHolding && m_VarNrHolding < 0)  {
		return false;
	}
	if (ApplyWeight && m_VarNrWeight < 0)  {
		return false;
	}
	if (ApplyWeightOnSafetyRule && !ApplyWeight) {
		return false;
	}
	if (ManualSafetyPerc < 0 || ManualSafetyPerc > 100)	{
		return false;
	}

	if (CellAndHoldingFreqSafetyPerc[0] < 0 || CellAndHoldingFreqSafetyPerc[0] > 100) {
		return false;
	}

	if (ApplyHolding)	{
		if (CellAndHoldingFreqSafetyPerc[1] < 0 || CellAndHoldingFreqSafetyPerc[1] > 100) {
			return false;
		}
	}
	if (ApplyHolding)	{
		m_tab[i].ApplyHolding = true;
	}
	else	{
		m_tab[i].ApplyHolding = false;
	}
	// set safety
	if (DominanceRule) {
		if (!m_tab[i].SetDominance(DominanceNumber,DominancePerc)) {
			//*ERRORCODECANNOTAPPLYDOMINANCERULE,
			return false;
		}
	}
		//m_tab[i].nMaxCellValues = DominanceNumber;
	if (PQRule)	{
		if (!m_tab[i].SetPQRule(PriorPosteriorP, PriorPosteriorQ, PriorPosteriorN)) {
			return false;
		}
	}
	//m_tab[i].nMaxCellValues = PriorPosteriorN + 1;

	// Not too sure about this
	/*default:
		*pVal = false;
	 return S_OK;
	 */
	if (EmptyCellAsNonStructural)	{
		if (NSEmptySafetyRange < 0 || NSEmptySafetyRange >= 100)	{
			return false;
		}
		m_tab[i].EmptyCellsAsNSEmpty = true;
		m_tab[i].NSEmptySafetyRange = NSEmptySafetyRange;
	}

	// set min rec
	if (!m_tab[i].SetSafeMinRecAndHold(SafeMinRecAndHoldings[0], SafeMinRecAndHoldings[1]) ) {
		return false;
	}

	// set other properties
	// m_tab[i].SafetyRule = SafetyRule;
	m_tab[i].ApplyWeight = ApplyWeight;
	m_tab[i].ApplyWeightOnSafetyRule = ApplyWeightOnSafetyRule;

	m_tab[i].ManualSafetyPerc = ManualSafetyPerc;
	m_tab[i].CellFreqSafetyPerc = CellAndHoldingFreqSafetyPerc[0];
	m_tab[i].HoldingFreqSafetyPerc = CellAndHoldingFreqSafetyPerc[1];
	m_tab[i].ApplyZeroRule = ApplyZeroRule;
	m_tab[i].ZeroSafetyRange = ZeroSafetyRange;

	if (ApplyPeep)	{
		m_tab[i].ApplyPeeper = true;
		if ((m_tab[i].PeepVarnr <0) || (m_tab[i].PeepVarnr >= m_nvar))	{
			return false;
		}
	}
	//check he does not give nonsense
	m_tab[i].PeepPercCell_1 = PeepPerc[0];
	m_tab[i].PeepPercCell_2 = PeepPerc[1];
	m_tab[i].PeepPercHolding_1 = PeepPerc[2];
	m_tab[i].PeepPercHolding_2 = PeepPerc[3];
	m_tab[i].PeepMinFreqCell = PeepMinFreqCellAndHolding[0];
	m_tab[i].PeepMinFreqHold = PeepMinFreqCellAndHolding[1];
	m_tab[i].PeepSafetyRangePercCell  = PeepSafetyRange[0];
	m_tab[i].PeepSafetyRangePercHolding  = PeepSafetyRange[1];

	// Now Set HoldingVarnr

	return true;
}

// Prepare the file for Hitas. The table is written in a file that could be used by
//Hitas for secondary supressions
bool TauArgus::PrepareHITAS(long TableIndex, const char* NameParameterFile, const char* NameFilesFile, const char* TauTemp)
{
    m_hitas.TempPath = TauTemp; // Temp doorgeven vanuit de TAU ipv zlf bepalen.

	long t = TableIndex;

	if (t < 0 || t >= m_ntab)	{
		return false;
	}
	if (m_tab[t].HasRecode) t += m_ntab;

	FILE *fdParameter = fopen(NameParameterFile, "w");
	if (fdParameter == 0) {
		return false;
	}

	FILE *fdFiles = fopen(NameFilesFile, "w");
	if (fdFiles == 0) {
		fclose(fdParameter);
		remove(NameParameterFile);
		return false;
	}

	if (!m_hitas.WriteParameterFile(fdParameter, m_tab[t]) ) goto error;
	if (!m_hitas.WriteFilesFile(fdFiles, m_tab[t], m_var) ) goto error;

	fclose(fdParameter);
	fclose(fdFiles);
	return true;

error:
	fclose(fdParameter);
	fclose(fdFiles);
	remove(NameParameterFile);
	remove(NameFilesFile);
	return false;
}

// Cells that are found to be secondary unsafe by Hitas is set as Unsafe in the table
bool TauArgus::SetSecondaryHITAS(long TableIndex, long *nSetSecondary)
{
	string SecFileName = m_hitas.TempPath + m_hitas.NameSecFile;
	int t = TableIndex;

	if (t < 0 || t >= m_ntab)	{
		return false;
	}

	if (m_tab[t].HasRecode) t += m_ntab;

	FILE *fd = fopen(SecFileName.c_str(), "r");
	if (fd == 0)	{
		return false;
	}

	bool res = m_tab[t].SetSecondaryHITAS(fd, m_var, nSetSecondary);

	fclose(fd);

	#ifdef _DEBUGG
		ShowTable("c:\\temp\\hitasres.txt", m_tab[t]);
	#endif

	return res; 
}

// sets a Hierarchical codelist (i.e a codelist given through a file not
// through digit splits)
long TauArgus::SetHierarchicalCodelist(long VarIndex, const char* FileName, const char* LevelString)
{
	if (VarIndex < 0 || VarIndex >= m_nvar || !m_var[VarIndex].IsHierarchical) {
		return HC_NOTHIERARCHICAL;
	}
	if (m_var[VarIndex].nDigitSplit != 0) {
		return HC_HASSPLITDIGITS;
	}

	return m_var[VarIndex].SetCodeList(FileName, LevelString);
}

// Gets a code if given an index and a variable number
//bool TauArgus::GetVarCode(long VarIndex, long CodeIndex, long *CodeType, const char **CodeString,
//                          long *IsMissing, long *Level)
bool TauArgus::GetVarCode(long VarIndex, long CodeIndex, long *CodeType, std::string *CodeString,
                          long *IsMissing, long *Level)
{
    int v = VarIndex, nCodes;

    if (v < 0 || v >= m_nvar)	{
	return false;
    }

    nCodes = m_var[v].GetnCode();
    if (CodeIndex < 0 || CodeIndex >= nCodes)	{
	return false;
    }

  /*if (m_Using MicroData)  {
	if (m_fname[0] == 0) return false;
  }*/
    if (!m_CompletedCodeList)	{
	return false;
    }

    if (m_var[v].IsHierarchical) {
        if (m_var[v].GethCode()[CodeIndex].IsParent) *CodeType = CTY_TOTAL;
        else                                         *CodeType = CTY_BASIC;
        *Level = m_var[v].GethCode()[CodeIndex].Level;
    } 
    else {
        if (m_var[v].IsCodeBasic(CodeIndex) ) *CodeType = CTY_BASIC;
	else                                  *CodeType = CTY_TOTAL;
        *Level = (CodeIndex == 0 ? 0 : 1);
    }

    // Gets a pointer to an internal buffer. It is still safe because the referenced object keeps living
    //*CodeString = m_var[v].GetCode(CodeIndex).c_str();
    *CodeString = m_var[v].GetCode(CodeIndex);
    *IsMissing = (CodeIndex >= m_var[v].GetnCode() - m_var[v].GetnMissing());

    return true;
}

long TauArgus::GetVarHierarchyDepth(long VarIndex, bool Recoded)
{
    if (VarIndex < 0 || VarIndex >= m_nvar || !m_var[VarIndex].IsHierarchical)
        return -1;
    else
    {
        return m_var[VarIndex].GetDepthOfHerarchicalBoom(Recoded);
    }
}

// return the codes for unsafe variables
//bool TauArgus::UnsafeVariableCodes(long VarIndex, long CodeIndex, long *IsMissing, long *Freq, 
//                                    const char **Code, long *Count, long *UCArray)
bool TauArgus::UnsafeVariableCodes(long VarIndex, long CodeIndex, long *IsMissing, long *Freq, 
                                    std::string *Code, long *Count, long *UCArray)
{
	int t;

	if (VarIndex < 0 || VarIndex >= m_nvar) {
		return false;
	}

	int nCodes = m_var[VarIndex].GetnCode();
	if (CodeIndex < 0 || CodeIndex >= nCodes) {
		return false;
	}

	// compute count
	*Count = 0;
	for (t = 0; t < m_ntab; t++) {
		CTable *tab = GetTable(t);
		if (tab->nDim > *Count) {
			*Count = tab->nDim;
		}
	}

	memset(UCArray, 0, *Count * sizeof(long) );

	// compute Freq
	for (t = 0; t < m_ntab; t++) {
		CTable *tab = GetTable(t);
		int var;
		for (var = 0; var < tab->nDim; var++) {
			if (tab->ExplVarnr[var] == VarIndex) break;  // hebbes
		}
		if (var < tab->nDim) {
			long DimList[MAXDIM];
			memset(DimList, 0, sizeof(DimList) );
			DimList[var] = CodeIndex;
			*Freq = tab->GetCell(DimList)->GetFreq();
			break;
		}
	}

	// compute nUnsafe for variable v, code CodeIndex, add to UCArray, only if Freq != 0
	if (*Freq != 0) {
		for (t = 0; t < m_ntab; t++) {
  			CTable *tab = GetTable(t);
			int var;
			for (var = 0; var < tab->nDim; var++) {
				if (tab->ExplVarnr[var] == VarIndex) break;  // hebbes
			}
			if (var < tab->nDim) {
				long nUnsafe[MAXDIM];
				tab->GetUnsafeCells(VarIndex, CodeIndex, nUnsafe);
				for (int i = 0; i < tab->nDim; i++) {
					UCArray[i] += nUnsafe[i];
				}
			}
		}
	}

	//*Code = m_var[VarIndex].GetCode(CodeIndex).c_str();
        *Code = m_var[VarIndex].GetCode(CodeIndex);
	*IsMissing = (CodeIndex >= nCodes - m_var[VarIndex].GetnMissing() );

	// TRACE("Var %d Code [%s] Freq = %d, unsafe %d %d %d\n", VarIndex + 1, (LPCTSTR) m_var[VarIndex].GetCode(CodeIndex), *Freq, UCArray[0], UCArray[1], UCArray[2]);

	return true;
}

// return properties given a Variable and Code Index
bool TauArgus::GetVarCodeProperties(long VarIndex, long CodeIndex, long *IsParent, 
                                    long *IsActive, long *IsMissing, long *Level, 
                                    long *nChildren, const char** Code)
{
	int v = VarIndex, c = CodeIndex;

	if (v < 0 || v >= m_nvar)	{
		return false;
	}
	if (!m_var[v].IsHierarchical)	{
		return false;
	}
	if (c < 0 || c >= m_var[v].nCode)	{
		return false;
	}

	CCode *phCode = m_var[v].hCode;
	vector<string> *psCode = &(m_var[v].sCode);

	*IsParent = phCode[c].IsParent;
	*IsActive = phCode[c].Active;
	*IsMissing = (c >= m_var[v].nCode - m_var[v].nMissing);
	*Level = phCode[c].Level;
	*nChildren = phCode[c].nChildren;
	*Code = (*psCode)[c].c_str();

	return true;
}

// Write Table in GHmiter file. This is used in secondary supressions
long TauArgus::WriteGHMITERSteuer(const char* FileName, const char* EndString1,
								  const char* EndString2, long TableIndex)
{
	// check table index
	if (TableIndex < 0 || TableIndex >= m_ntab) {
		return GHM_TABLEINDEXWRONG;
	}

	CTable *tab = GetTable(TableIndex);

	ostringstream ss;
	ss << "Table " << TableIndex + 1;

	if (m_ghmiter.ControlDataTable(FileName, ss.str().c_str(), EndString1, EndString2, tab->nDim, tab->ExplVarnr, m_var) == 0) {
		return GHM_STEUERINCORRECT;
	}

	return 1;
}

// Write data Cells for GHMiter.
long TauArgus::WriteGHMITERDataCell(const char* FileName, long TableIndex, bool IsSingleton)
{
	// check table index
	if (TableIndex < 0 || TableIndex >= m_ntab) {
		return GHM_TABLEINDEXWRONG;
	}

	CTable *tab = GetTable(TableIndex);

	if (m_ghmiter.CellsTable(FileName, tab, m_var, IsSingleton) == 0) {
		return GHM_EINGABEINCORRECT;
	}
	return 1;
}

// Cells that are given as secondary Unsafe by GHmiter
// to be set in the table
long TauArgus::SetSecondaryGHMITER(const char* FileName, long TableIndex,
								   long *nSetSecondary, bool IsSingleton)
{
	// check table index
	if (TableIndex < 0 || TableIndex >= m_ntab) {
		return GHM_TABLEINDEXWRONG;
	}

	CTable *tab = GetTable(TableIndex);
	int ErrorCode;

	int result = m_ghmiter.SetSecondaryUnsafe(FileName, tab, nSetSecondary, &ErrorCode, IsSingleton);

	// if secondary supress does not wiork for some reason
	if (result == 0) {// Not too sure about this
		if (IsSingleton) {
			tab->UndoSecondarySuppress(WITH_SINGLETON);
		}
		else {
			tab->UndoSecondarySuppress(WITHOUT_SINGLETON);
		}
		return ErrorCode;
	}

	return 1;
}

// Get information per status
bool TauArgus::GetCellStatusStatistics(long TableIndex,
									   long *StatusFreq,
									   long *StatusCellFreq,
									   long *StatusHoldingFreq,
									   double *StatusCellResponse,
									   double *StatusCellCost)
{
	if (TableIndex < 0 || TableIndex >= m_ntab) {
	  	return false;
	}

	CTable *tab = GetTable(TableIndex);
	tab->GetStatusStatistics(StatusFreq, StatusCellFreq, StatusHoldingFreq, StatusCellResponse, StatusCellCost);

	return true;
}


// This function is not usedb at the moment
bool TauArgus::PrepareCellDistance(long TableIndex)
{
	if (TableIndex < 0 || TableIndex >= m_ntab)	{
		return false;
	}

	CTable *tab = GetTable(TableIndex);
	tab->PrepareComputeDistance();

	return true;
}

// This function is not used either at the moment
bool TauArgus::GetCellDistance(long TableIndex, long *DimIndex, long *Distance)
{
	if (TableIndex < 0 || TableIndex >= m_ntab) {
		return false;
	}

	CTable *tab = GetTable(TableIndex);
	tab->GetCellDistance(tab->GetCellNrFromIndices(DimIndex), Distance);
	
	return true;
}

// Write a table as Comma seperated file
bool TauArgus::WriteCSV(long TableIndex, const char* FileName, bool EmbedQuotes, 
						long *DimSequence, long RespType)
{
	if (TableIndex < 0 || TableIndex >= m_ntab)	{
		return false;
	}

	// check DimSequence
	// correct and unique?
	for (int d = 0; d < m_tab[TableIndex].nDim; d++) {
		if (DimSequence[d] < 0 || DimSequence[d] >= m_tab[TableIndex].nDim){
			return false;
		}
		for (int e = d + 1; e < m_tab[TableIndex].nDim; e++) {
			if (DimSequence[d] == DimSequence[e])	{
				return false;
			}
		}
	}

	// open result file
	FILE *fd = fopen(FileName, "w");
	if (fd == 0)	{
		return false;
	}

	// get pointer to (recoded) table
	CTable *tab = GetTable(TableIndex);
	long Dims[MAXDIM];
	WriteCSVTable(fd, tab, EmbedQuotes, DimSequence, Dims, 0, m_ValueSeparator, RespType);

	fclose(fd);

	return true;
}

// Write a table as Comma seperated file
// Not used
/*bool TauArgus::WriteCSVBasic(long TableIndex, const char* FileName,
							 long *DimSequence, long RespType)
{
	if (TableIndex < 0 || TableIndex >= m_ntab)	{
		return false;
	}

	for (int d = 0; d < m_tab[TableIndex].nDim; d++) {
		DimSequence[d]--;
	}

	return WriteCSV(TableIndex, FileName, DimSequence, RespType);
}*/

// Write Table in JJ Format
bool TauArgus::WriteJJFormat(long TableIndex, const char* FileName,
													double LowerBound, double UpperBound,
													bool WithBogus, bool AsPerc,
													bool ForRounding)
{
	string sFileNameFreq = FileName;
	sFileNameFreq += ".frq";
	FILE *fd, *fdFrq;
	double MaxCost, MaxResp, x;
	int i;
	CDataCell *dc;

	if (TableIndex < 0 || TableIndex >= m_ntab)	{
		return false;
	}
	CTable *tab = GetTable(TableIndex);

	fd = fopen(FileName, "w");
	if (fd == 0)	{
		return false;
	}
	fdFrq = fopen(sFileNameFreq.c_str(), "w");
	if (fdFrq == 0)	{
		return false;
	}
	MaxCost = 0;
	MaxResp = 0;
  	for (i = 0; i < tab->nCell; i++) {
		dc = tab->GetCell(i);
		x = dc->GetCost(tab->Lambda);
		if (x > MaxCost) MaxCost = x;
		x = dc->GetResp();
		x=fabs(x);
		if (x > MaxResp) MaxResp = x;
	}

	if (!m_jjformat.WriteCells(fd, fdFrq, LowerBound, UpperBound, tab, m_var,
		        m_var[tab->ResponseVarnr].nDec, WithBogus, AsPerc, ForRounding, MaxCost, MaxResp) ) goto error;
	if (!m_jjformat.WriteRestrictions(fd, tab, m_var, WithBogus) ) goto error;

	fclose(fd);
	fclose (fdFrq);
	return true;
error:
	fclose(fd);
	fclose (fdFrq);
	return false;
}

// Cells that are give as secondary unsafe by JJ to be set in the table
long TauArgus::SetSecondaryJJFORMAT(long TableIndex, const char* FileName, bool WithBogus, long *nSetSecondary)
{
	if (TableIndex < 0 || TableIndex >= m_ntab) {
		return JJF_TABLEINDEXWRONG;
	}

	CTable *tab = GetTable(TableIndex);
	long ErrorCode;

	if (!m_jjformat.SetSecondaryUnsafe(FileName, tab, m_var, nSetSecondary, &ErrorCode, WithBogus)) {
		return ErrorCode;
	}
	return 1;
}

// Get Size of a tabel. This function has to be changed
void TauArgus::GetTotalTabelSize(long TableIndex, long* nCell, long * SizeDataCell)
{
	*nCell = m_tab[TableIndex].nCell;
	int lenarrcell = m_tab[TableIndex].NumberofMaxScoreCell;
	int lenarrhold = m_tab[TableIndex].NumberofMaxScoreHolding;
	int applyhold = m_tab[TableIndex].ApplyHolding;
	int applyweight = m_tab[TableIndex].ApplyWeight;
	*SizeDataCell = sizeof(CDataCell(lenarrcell, lenarrhold, applyhold, applyweight));
}

// Write Cell in file
bool TauArgus::WriteCellRecords(long TableIndex, const char* FileName,
								long SBS,
								bool SBSLevel,
								bool SuppressEmpty, 
								const char* FirstLine,
								bool ShowUnsafe,
								bool EmbedQuotes,
                                                                long RespType)
{
	if (TableIndex < 0 || TableIndex >= m_ntab) {
		return false;
	}
	FILE *fd = fopen(FileName, "w");
	if (fd == 0) {
		return false;
	}

	CTable *tab = GetTable(TableIndex);
    WriteFirstLine(fd, FirstLine);
// SBSCode: 0 doet niet mee
//          1 geen topn-info aanwezig
//			2 een topn individueel
//			3 meer dan een topn individueel
//			4 een topn Holding
//			5 meer dan een topn Holding

	int SBSCode = 0;
	if (SBS > 0) {
		if( (tab->NumberofMaxScoreCell == 0) && (tab->NumberofMaxScoreHolding == 0) )  {
			SBSCode = 1;}
    	if( (tab->NumberofMaxScoreCell == 1))  {
			SBSCode = 2;}
    	if( (tab->NumberofMaxScoreCell > 1))  {
			SBSCode = 3;}
    	if( (tab->NumberofMaxScoreHolding == 1))  {
			SBSCode = 4;}
    	if( (tab->NumberofMaxScoreHolding > 1))  {
			SBSCode = 5;}
	}
	long Dims[MAXDIM];
	WriteCellRecord(fd, tab, Dims, 0, m_ValueSeparator, SBSCode, SBSLevel, SuppressEmpty, ShowUnsafe, EmbedQuotes, RespType);

	fclose(fd);

	return true;
}

// Code list to be created. This is a sibling of the explore file. This is needed
// for crerating a codelist when a table is directly given instead of being created from
// micro data.
bool TauArgus::SetInCodeList(long NumberofVar, long *VarIndex,
													char *sCode[], long *ErrorCode,
													long *ErrorInVarIndex)
{
	long i;
 // check if number of variables = length of array codes

	*ErrorInVarIndex = -1;
	if (m_nvar == 0) {
		*ErrorCode = NOVARIABLES;
		return false;
	}

	for (i = 0; i < m_nvar; i++) {
		if (m_var[i].IsHierarchical) {
			if (m_var[i].nDigitSplit == 0 && m_var[i].hLevel.size() == 0) {
				*ErrorCode = WRONGHIERARCHY;
				*ErrorInVarIndex = i;
				return false;
			}
		}
	}

	// Now start reading the codes in Not if the code is in a hierarchical
	// codelist without digit split don't bother.

	for (i=0; i<NumberofVar; i++)  {
		long lvarindex = VarIndex[i];
		CVariable *var = &(m_var[lvarindex]);
		// To get a string from an array of strings. Use SafeArray
		string tempcode = sCode[i];
		if (tempcode != "" ) {
		if (var->IsCategorical)  {
			if ((var ->IsHierarchical) && (var ->nDigitSplit == 0))  {
				if (var ->FindAllHierarchicalCode(tempcode.c_str()) < 0)  {
					*ErrorCode = CODENOTINCODELIST;
					*ErrorInVarIndex = i;
					 // add some info here
					return false;

				}
			}
			else {
				if (!var->AddCode(tempcode.c_str(), false) ) {   // adds if new, else does nothing
					*ErrorCode = PROGRAMERROR;
					return false;
				}
			}
		}
		}
	}

	return true;
}

// Once all code list a finished. Set Hierarchies and totals. This happens automatically in
// the end of explore file
bool TauArgus::SetTotalsInCodeList(long NumberofVariables, long *VarIndex, long *ErrorCode, long *ErrorInVarIndex)
{

	for (long i=0; i<NumberofVariables; i++)  {
		long lvarindex = VarIndex[i];
		CVariable *var = &(m_var[lvarindex]);

		if (var->IsCategorical)  {

			if ((var->nDigitSplit > 0)) {
				if (!(var->ComputeHierarchicalCodes()) ) {
					*ErrorCode = WRONGHIERARCHY;
					return false;
				}
			}

			// Add Totals to the Code
			if (!var->IsHierarchical || var->nDigitSplit > 0) {
				var->AddCode("", false);
			}


			if (var->nMissing > 0)	{
				var->AddCode(var->Missing1.c_str(), true);
				if (var->IsHierarchical && var->nDigitSplit == 0) {
					var->hLevel.push_back(1);
					var->hLevelBasic.push_back(true);
				}

				if (var->nMissing == 2)  {
					var->AddCode(var->Missing2.c_str(), true);
					if  (var->IsHierarchical && var->nDigitSplit == 0)   {
						var->hLevel.push_back(1);
						var->hLevelBasic.push_back(true);
					}
				}
			}

//			m_var[i].nCode = m_var[i].sCode.GetSize();
			long n = m_var[lvarindex].sCode.size();
			m_var[lvarindex].nCode = n;
			if (var->IsHierarchical)  {
				if (!var->SetHierarch()) {
					*ErrorCode = WRONGHIERARCHY;
					return false;
				}
			}
		}

	}

	m_CompletedCodeList = true;

	return true;
}

// Tells you that you a given a table directly and not to
//create one from micro data
void TauArgus::ThroughTable()
{
	m_UsingMicroData = false;
}

// Set data in table. Since table can be given in many ways.
// See SetTableSafetyInfo, not all input is valid input.
bool TauArgus::SetInTable(long Index, char *sCode[],
			  double Shadow, double Cost,
			  double Resp, long Freq,
			  double *MaxScoreCell, double *MaxScoreHolding,
			  long Status, double LowerProtectionLevel, double UpperProtectionLevel,
			  long *ErrorCode, long *ErrVNum)
{
	*ErrVNum = 0;

	// check if table is defined
	if (m_tab[Index].nDim == 0) {
		*ErrorCode = TABLENOTSET;
		return false;
	}

	// Memory allocated for table. If not do it
	if (!m_tab[Index].Prepared)  {
		if (!m_tab[Index].PrepareTable() ) {
			*ErrorCode = NOTABLEMEMORY;
			return false;
		}
	}

	int dim = m_tab[Index].nDim;
	string *sCodes = new string[dim];

	for (int i=0; i<dim; i++) {
		sCodes[i] = sCode[i];
	}

	// fill it in the table cell
	if (!FillInTable(Index, sCodes, Cost, Resp, Shadow, Freq, MaxScoreCell, 
                        MaxScoreHolding, LowerProtectionLevel, UpperProtectionLevel, Status, *ErrorCode, *ErrVNum))  {
		delete [] sCodes;
		return false;
	}


	delete [] sCodes;

	return true;
}

// To state all the cells have been read and the table has to be built.
// In some case the marginals (or sub totals are given ) in other cases they have to be
// calculated
bool TauArgus::CompletedTable(long Index, long *ErrorCode,
										 const char* FileName,
										 bool CalculateTotals,
										 bool SetCalculatedTotalsAsSafe,
										 bool ForCoverTable)
{
//	string sFileName;
//	sFileName = FileName;
	CDataCell *dc ;

//	CDataCell *dcempty = new CDataCell(m_tab[Index].NumberofMaxScoreCell,m_tab[Index].NumberofMaxScoreHolding,m_tab[Index].ApplyHolding);

	// if status for each cel is given
	bool IsAdditive = true;
	int i = 1;
	if (m_HasStatus)  {

//  WriteJJFormat(Index, FileName, -1000, 1000, true, false, false);
//													double LowerBound, double UpperBound,
//													bool WithBogus, bool AsPerc,
//													bool ForRounding,

		for (i=0; i <m_tab[Index].nCell; i++)  {
			// if not safe or unsafe set empty; all not entered cells
			dc = m_tab[Index].GetCell(i);
			if (dc->GetStatus() == 0)  {
			//	m_tab[Index].CellPtr.SetAt(i,dcempty);
			//	dc = m_tab[Index].GetCell(i);
				dc->SetStatus(CS_EMPTY);
				dc->IsFilled = false; //Why false,AncoJuly 2012
			}
		}
		// Sub totals are given. Check if table is additive
		// CoverTabel always has a status
		if (!ForCoverTable){
			if (!CalculateTotals)	{
	    		if (!IsTable(&(m_tab[Index]))) {
		    		*ErrorCode = TABLENOTADDITIVE;
     			    WriteJJFormat(Index, FileName, -1000, 1000, false, false, false);
					IsAdditive = false;
//					return false;
				}
			}

			// other wise make sub totals
			else
			{
				long tel = 0;
				long maxdiepte = MaxDiepteVanSpanVariablen(&(m_tab[Index]));

				while (!IsTable(&(m_tab[Index])))	{
					AdjustTable(&(m_tab[Index]));
					// To preven an unending loop
					tel++;
					if (tel > maxdiepte + 6 )	{

						break;
					}
				}


				if (tel > maxdiepte + 6)	{
					*ErrorCode = CANNOTMAKETOTALS;
					IsAdditive = false;
//					return false;
				}
			}
		}
		// Calculated totals to be set as safe
		if (SetCalculatedTotalsAsSafe)	{
			for (i=0; i <m_tab[Index].nCell; i++)  {
			// if not safe or unsafe set empty
				dc = m_tab[Index].GetCell(i);
				if (dc->GetStatus() == 0)  {
					dc->SetStatus(CS_SAFE_MANUAL);
					dc->IsFilled = false;  //Why false,AncoJuly 2012
				}
			}
		}
		// if status is not given. Either frequency or max score is given.
		// to apply rules
		else {
			if (!((m_HasFreq) || (m_HasMaxScore)))	{
				*ErrorCode = CANNOTCALCULATESAFETY;
				return false;
			}
			else
			{
				for (i=0; i <m_tab[Index].nCell; i++)  {
			// if not safe or unsafe set empty
					dc = m_tab[Index].GetCell(i);
					if (dc->GetStatus() == 0)  {
						//	m_tab[Index].CellPtr.SetAt(i,dcempty);
						//	dc = m_tab[Index].GetCell(i);
						dc->SetStatus(m_tab[Index].ComputeCellSafeCode(*dc));
						m_tab[Index].SetProtectionLevelCell(*dc);
						dc->IsFilled = false;  //Why false,AncoJuly 2012
					}
				}
			}
		}

		/// check if is table
		return IsAdditive;

	} // End loop status is given

	if ((m_HasFreq) || (m_HasMaxScore))  {
		//ComputeCellStatuses(m_tab[Index]);

		//Once more check all statuses are filled
		for (i=0; i <m_tab[Index].nCell; i++)  {
			// if not safe or unsafe set empty
			dc = m_tab[Index].GetCell(i);
			if (dc->GetStatus() == 0)  {
				dc->SetStatus(CS_EMPTY);
			}
		}
		if (!CalculateTotals)	{
			if (!IsTable(&(m_tab[Index]))) {
				// maybe error code shouls show that table is not ok.
				*ErrorCode = TABLENOTADDITIVE;
   			    WriteJJFormat(Index, FileName, -1000, 1000, false, false, false);
                IsAdditive = false;
//				return false;
			}
		}
		else
		{
			long tel = 0;
			long maxdiepte = MaxDiepteVanSpanVariablen(&(m_tab[Index]));
			while (!IsTable(&(m_tab[Index])))	{
				AdjustTable(&(m_tab[Index]));
				// To prevent an unending loop
				tel++;
				if (tel > maxdiepte+6 )	{
					break;
				}
			}


			if (tel > maxdiepte +6)	{
				*ErrorCode = CANNOTMAKETOTALS;
				IsAdditive = false;
//				return false;
			}
		}
		//for each cell create safe code
		ComputeCellStatuses(m_tab[Index]);
		SetProtectionLevels(m_tab[Index]);
	} //end loop HasFreq or HasMaxScore

	// finale check: alles met status = 0  wordt empty als leeg or safe als niet leeg
    // Bijvoorbeeld als er niets bekend is over de status
	for (i=0; i <m_tab[Index].nCell; i++)  {
		// if not safe or unsafe set empty
		dc = m_tab[Index].GetCell(i);
		if (dc->GetStatus() == 0)  {
		//	m_tab[Index].CellPtr.SetAt(i,dcempty);
		//	dc = m_tab[Index].GetCell(i);
			if (dc->GetFreq() == 0) {
				dc->SetStatus(CS_EMPTY);
				dc->IsFilled = false;
			}
			else
			{
				dc->SetStatus(CS_SAFE_MANUAL);
			}
		}
	}

	return IsAdditive;
}

// variable set. This is incase table is given
//directly and not built from micro data
bool TauArgus::SetVariableForTable(long Index, long nMissing, const char* Missing1,
					const char* Missing2, const char* TotalCode,
                                        long nDec, bool IsPeeper,
					const char* PeeperCode,
					bool IsHierarchical,
					bool IsNumeriek, long nPos)
{
	if (Index < 0 || Index >= m_nvar) {
		return false;
	}
	if ((nMissing < 0) || (nMissing  > 2)) {
		return false;
	}
	if (!m_var[Index].SetMissing(Missing1, Missing2, nMissing)) {
		return false;
	}
        if (!m_var[Index].SetTotalCode(TotalCode) ) {
		return false;
	}
        
        m_var[Index].nPos = nPos;
	//variable ia always categorical, weight and holdings don't come into play
	if (IsNumeriek) {
		if (!m_var[Index].SetDecPosition(nDec))	{
			return false;
		}
		if (!m_var[Index].SetType(false, IsNumeriek, false, IsHierarchical, false, false, false)) {
			return false;
		}
	}
	else {
		if (!m_var[Index].SetType(true, false, false, IsHierarchical, false, false, false)) {
			return false;
		}
	}

	return true;
}

// set safety information. this function is used along with set table.
// this is the sibling of settablesafety. for instance holding and weight cannot be made
bool TauArgus::SetTableSafetyInfo(long TabIndex,
															bool HasMaxScore,
															bool DominanceRule,
															long * DominanceNumber, long * DominancePerc,
															bool PQRule,
															long * PriorPosteriorP, long * PriorPosteriorQ, long * PriorPosteriorN,
															bool HasFreq,
															long CellFreqSafetyPerc, long SafeMinRec,
															bool HasStatus, long ManualSafetyPerc,
															bool ApplyZeroRule, double ZeroSafetyRange,
															bool EmptyCellAsNonStructural, long NSEmptySafetyRange,
															long *ErrorCode)
{
	int i = TabIndex;

	m_HasMaxScore = HasMaxScore;
	m_HasStatus = HasStatus;
	m_HasFreq = HasFreq;

	// check TableIndex
	if (m_nvar == 0)  {
		*ErrorCode = VARIABLENOTSET;
		return false;
	}

	if (i < 0 || i >= m_ntab) {
		*ErrorCode = TABLENOTSET;
		return false;
	}

	if (HasFreq)  {
		if ((CellFreqSafetyPerc < 0 ) || (CellFreqSafetyPerc > 100))  {
			*ErrorCode = FREQPERCOUTOFRANGE;
			return false;
		}
		//!!!!!!!!!!!!!!!!!!
		if (!m_tab[i].SetSafeMinRecAndHold(SafeMinRec, 0) ) {
			*ErrorCode = PROGRAMERROR;
			return false;
		}
	}

	if (HasStatus)  {
		if ((ManualSafetyPerc < 0) || (ManualSafetyPerc > 100)) {
			*ErrorCode = STATUSPERCOUTOFRANGE;
			return false;
		}
	}


	// Set safety Rule
	if (DominanceRule)	{
		if (!m_tab[i].SetDominance(DominanceNumber,DominancePerc )) {
			*ErrorCode = PROGRAMERROR;
			return false;
		}
	}
	if (PQRule)	{
		if (!m_tab[i].SetPQRule(PriorPosteriorP,PriorPosteriorQ,PriorPosteriorN) ) {
			*ErrorCode = PROGRAMERROR;
			return false;
		}
	}

	if (EmptyCellAsNonStructural)	{
		if (NSEmptySafetyRange < 0 || NSEmptySafetyRange >= 100)	{
			return false;
		}
		m_tab[i].EmptyCellsAsNSEmpty = true;
		m_tab[i].NSEmptySafetyRange = NSEmptySafetyRange;
	}

	// Should I do something with Set Safe Min Rec

	// set min rec

	//	m_tab[i].SafetyRule = SafetyRule;
	m_tab[i].ApplyWeight = false;
	m_tab[i].ApplyWeightOnSafetyRule = false;
	m_tab[i].ApplyHolding = false;
	m_tab[i].ManualSafetyPerc = ManualSafetyPerc;
	m_tab[i].CellFreqSafetyPerc = CellFreqSafetyPerc;
	m_tab[i].ApplyZeroRule = ApplyZeroRule;
	m_tab[i].ZeroSafetyRange = ZeroSafetyRange;
	//	!!!!
	// m_tab[i].HoldingFreqSafetyPerc = HoldingFreqSafetyPerc;

	return true;
}

//CTA-value
bool TauArgus::SetCTAValues(long TabIndex, long CelNr,
							double OrgVal, double CTAVal, long *Sec)
{
	if (TabIndex < 0 || TabIndex >= m_ntab) {
		return false;
	}
	if (m_tab[TabIndex].HasRecode) TabIndex += m_ntab;

	if (CelNr < 0 || CelNr > m_tab[TabIndex].GetSizeTable()) {
		return false;
	}

	CDataCell *dc = m_tab[TabIndex].GetCell(CelNr);
	long status = dc->GetStatus();
	dc->SetCTAValue (CTAVal);
	if (OrgVal != CTAVal){
	 if (status == CS_SAFE) {
		 dc->SetStatus (CS_SECONDARY_UNSAFE); *Sec= *Sec + 1;}
	 if ((status == CS_SAFE_MANUAL)){
			dc->SetStatus (CS_SECONDARY_UNSAFE_MANUAL); *Sec= *Sec + 1;}
	}

	return true;
}

// Set Lower and upper protection level. This is the one calculated by Anneke
bool TauArgus::SetRealizedLowerAndUpper(long TabNr, long CelNr, double RealizedUpper, double RealizedLower)
{
	//	CTable *tab;
	if (TabNr < 0 || TabNr >= m_ntab) {
		return false;
	}

	long TabIndex = TabNr;
	if (m_tab[TabIndex].HasRecode) TabIndex += m_ntab;

	if ((CelNr < 0) || (CelNr > m_tab[TabIndex].GetSizeTable())) {
		return false;
	}

	CDataCell *dc = m_tab[TabIndex].GetCell(CelNr);
	if ((dc->GetStatus() == CS_UNSAFE_FREQ) || (dc->GetStatus() == CS_UNSAFE_PEEP) ||
		(dc->GetStatus() == CS_UNSAFE_RULE) || (dc->GetStatus() == CS_UNSAFE_SINGLETON) ||
		(dc->GetStatus() == CS_UNSAFE_ZERO) ||
		(dc->GetStatus() == CS_SECONDARY_UNSAFE) ||
		(dc->GetStatus() == CS_UNSAFE_MANUAL)  || (dc->GetStatus() == CS_SECONDARY_UNSAFE_MANUAL)) {
		dc->SetRealizedLowerValue(RealizedLower);
		dc->SetRealizedUpperValue(RealizedUpper);
		return true;
	}
	else  {
		return false;
	}
}

// Check if Realized Upper and Lower bound are a better
// approximation than the Protection Level.

long TauArgus::CheckRealizedLowerAndUpperValues(long TabIndex)
{
	//double Sliding;
	//double Capacity;

	long NumFaults = 0;
	for (long i = 0; i < m_tab[TabIndex].nCell; i++)  {

		// get UPL and LPL
		CDataCell *dc = m_tab[TabIndex].GetCell(i);
		double LPL = dc->GetLowerProtectionLevel();
		double UPL = dc->GetUpperProtectionLevel();
		if ((UPL > dc->GetRealizedUpperValue()) || (LPL < dc->GetRealizedLowerValue()))  {
			NumFaults ++;
		}
	}

	return NumFaults;
}

// given an array of codes, calculate the corresponding indexes or cell number
bool TauArgus::ComputeCodesToIndices(long TableIndex, char* sCode[], long *DimIndex)
{
	CTable *tab = &(m_tab[TableIndex]);
	long dim = tab->nDim;

	for (int j=0; j<dim; j++)  {
		CVariable *var = &(m_var[tab->ExplVarnr[j]]);
		string tempcode = sCode[j];
		if (tempcode == "") {
			var->TableIndex = 0;
		}
		else {
	  		if (var->IsHierarchical && var->nDigitSplit == 0) {
				var->TableIndex = var->FindAllHierarchicalCode(tempcode.c_str());
			}
			else {
				bool IsMissing;
				var->TableIndex = BinSearchStringArray(var->sCode, tempcode, var->nMissing,
					IsMissing);
			}
      }
		if (var->TableIndex < 0) {
			return false;
		}
		DimIndex[j] = var->TableIndex;
	}

	return true;
}

//Give input file information. This important when you are reading a file with free format
void TauArgus::SetInFileInfo(bool IsFixedFormat, const char* Separator)
{
	InFileIsFixedFormat = IsFixedFormat;
	InFileSeperator = Separator;
}

// Write Table in AMPL format. This is used in networking
bool TauArgus::WriteTableInAMPLFormat(const char* AMPLFileName, long TableIndex)
{
	CTable *tab = GetTable(TableIndex);

	if (tab->nDim != 2) {
		return false;
	}
	for (int i=0; i<2; i++) {
		CVariable *var = &(m_var[tab->ExplVarnr[i]]);
		 if (var ->IsHierarchical) {
			 return false;
		 }
	}

	CAMPL oampl;
	FILE *fdampl = fopen(AMPLFileName, "w");
	oampl.WriteTableInAMPL(tab, m_var, fdampl);
	fclose(fdampl);
	return true;
}


bool TauArgus::SetProtectionLevelsForFrequencyTable(long TableIndex, long Base, long K)
{
	if (TableIndex < 0 || TableIndex >= m_ntab)	{
		return false;
	}
	CTable *tab = &m_tab[TableIndex];
	if (!tab->IsFrequencyTable)	{
		return false;
	}

	for (long c = 0; c < tab->nCell; c++) {
		CDataCell *dc = tab->GetCell(c);
		if (!tab->SetProtectionLevelCellFrequency(*dc, Base, K)) {
			return false;
		}
	}

	return true;
}

bool TauArgus::SetProtectionLevelsForResponseTable(long TableIndex, long *DimIndex,
															   double LowerBound, double UpperBound)
{
	// check parameters
   if (TableIndex < 0 || TableIndex >= m_ntab) {
		return false;
	}
	//if (m_fname[0] == 0)      		return false;
   if (!m_CompletedCodeList) {
		return false;
	}

	CTable *table = GetTable(TableIndex);
	//dc = table ->GetCell(DimIndex);
	//iTemporaryStatus = table->ComputeCellSafeCode(*dc);

	// check DimIndices
	for (int i = 0; i < table->nDim; i++) {
		int nCodes = m_var[table->ExplVarnr[i]].GetnCode();
		ASSERT(DimIndex[i] >= 0 && DimIndex[i] < nCodes);
		if (DimIndex[i] < 0 || DimIndex[i] >= nCodes)  {
			return false;
		}
	}
	CDataCell *dc = table ->GetCell(DimIndex);
//	dc->SetCost(Cost);
//	return true;
//}

	dc->SetLowerProtectionLevel (LowerBound);
	dc->SetUpperProtectionLevel (UpperBound);
//	if (dc->GetResp() < LowerBound) {
//		if (!m_tab[i].SetProtectionLevelCellFrequency(*dc, Base, K))	{
//		return false;
//	}

	return true;
}


double TauArgus::MaximumProtectionLevel(long TableIndex)
{
	CTable *tab = &m_tab[TableIndex];
	double maxprot = 0;

	for (long c = 0; c < tab->nCell; c++)	{
		CDataCell *dc = tab->GetCell(c);
		long St = dc->GetStatus();
		// 10 juni 2005 toegevoegd: alleen voor onveilige cellen AHNL
		if (St >= CS_UNSAFE_RULE && St <= CS_UNSAFE_MANUAL) {
			maxprot = max(maxprot,dc->GetLowerProtectionLevel() + dc ->GetUpperProtectionLevel());
		}
	}

	return maxprot;
}

//GetMinimumCellValue
double TauArgus::GetMinimumCellValue(long TableIndex, double *Maximum)
{

	double mincell = 10E+60, maxcell = -10E+60;

	for (long c = 0; c < m_tab[TableIndex].nCell; c++)	{
		CDataCell *dc = m_tab[TableIndex].GetCell(c);
		long St = dc->GetStatus();
		// 10 juni 2005 toegevoegd: alleen voor onveilige cellen AHNL
		if ( St < CS_EMPTY) {
			mincell = min(mincell, dc->GetResp());
			maxcell = max(maxcell, dc->GetResp());
		}

	}

	*Maximum = maxcell;
	return mincell;
}

bool TauArgus::SetRoundedResponse(const char* RoundedFile, long TableIndex)
{
	if (TableIndex < 0 || TableIndex >= m_ntab)	{
		return false;
	}
	CTable *tab = GetTable(TableIndex);

	FILE *frounded = fopen(RoundedFile, "r");
	if (frounded == NULL)	{
		return false;
	}
	// Go to the beginning
	fseek( frounded, 0L, SEEK_SET );
	long itel = 0;
	while (!feof(frounded) ) {
		if (itel == tab->nCell)	{ //was nCell - 1
			break;
		}
		long CellNum;
		char s[30];
		double d;
		double RoundedResp; //was float
		fscanf( frounded, "%ld", &CellNum );
		fscanf( frounded, "%s", s);
		fscanf ( frounded, "%lf", &d); // original value
		fscanf (frounded,"%s", s); // to
		//fscanf (frounded, "%f", &RoundedResp);
		fscanf (frounded, "%s", s);
		RoundedResp = atof(s);
		//fscanf( frounded, "%s", s);

		if ((CellNum < 0) || CellNum >= tab->nCell)	{
			fclose(frounded);
			return false;
		}
		CDataCell *dc = tab->GetCell(CellNum);
		if ((RoundedResp != 0) && ((dc->GetStatus() == CS_EMPTY) ||
				(dc->GetStatus() == CS_EMPTY_NONSTRUCTURAL)))	{

			CDataCell *newcell = new CDataCell(tab->NumberofMaxScoreCell,tab->NumberofMaxScoreHolding,tab->ApplyHolding, tab->ApplyWeight);
			tab->CellPtr[CellNum] = newcell;
			dc = tab->GetCell(CellNum);
			dc->SetStatus(CS_SAFE);

		}

		//dc->SetRoundedResponse((long) RoundedResp);
                dc->SetRoundedResponse(RoundedResp);

		itel++;
	}

	fclose(frounded);
	return true;

}


bool TauArgus::WriteHierarchicalTableInAMPLFormat(const char* AMPLFilename, const char* TempDir,
															 long Tableindex, double MaxScale, long *ErrorCode)
{
	if ((Tableindex <0) || (Tableindex >= m_ntab))	{
		*ErrorCode = NOTABLE;
		return false;
	}

	CTable *tab = GetTable(Tableindex);
	//tab = &(m_tab[tabind]);

	if (tab->nDim >2)	{
		*ErrorCode = TABLEWITHMORETHAN2VARIABLES;
		return false;
	}

	CVariable * var;
	// Check if 1st variable is Hierarchical and the last is not;
	var = &(m_var[tab->ExplVarnr[0]]);
	if (!var->IsHierarchical)	{
		*ErrorCode = FIRSTVARNOTHIERARCHICAL;
		return false;
	}
	var = &(m_var[tab->ExplVarnr[1]]);
	if  (var->IsHierarchical)	{
		*ErrorCode =SECONDVARNOTFLAT;
		return false;
	}

	for (int j = 0; j<tab->nDim; j++)	{
		var = &(m_var[tab->ExplVarnr[j]]);
		if (!var->PrepareSubCodeList())	{
			*ErrorCode = NOMEMORYFORSUBCODES;
			return false;
		}
		//create subcodelist for table
		if (!var->FillSubCodeList())	{
			*ErrorCode = COULDNOTCREATESUBCODES;
			return false;
		}
	}
	FILE *fd = fopen(AMPLFilename,"w");
	if (fd == 0)	{
		*ErrorCode = COULDNOTOPENFILE;
		return false;
	}
	if (!WriteHierTableInAMPL(fd, Tableindex, TempDir, MaxScale))	{
		*ErrorCode = COULDNOTCREATEAMPL;
		fclose(fd);
		return false;
	}
	fclose(fd);
	return true;
}


bool TauArgus::SetSecondaryFromHierarchicalAMPL(const char* FileName, long TableIndex, long *ErrorCode)
{
	if ((TableIndex < 0) || (TableIndex >= m_ntab))	{
		return false;
	}

	FILE *fd = fopen(FileName, "r");
//	fd = fopen("E:/Temp/NetSolHH.tmp","r");
	if (fd == 0)	{
		return false;
	}

	CTable *tab = GetTable(TableIndex);
	//tab = &(m_tab[TableIndex]);
	long *SubTableCellIndexTuple = new long [tab->nDim];
	long *SubTableTuple = new long [tab->nDim];
	long *TableCellIndex = new long [tab->nDim];

	fseek( fd, 0L, SEEK_SET );
	long nSuppr;
	fscanf(fd, "%ld", &nSuppr);
	//while (!feof(fd) ) {
	for (long j = 0; j < nSuppr; j++)	{
		long subtablenum;
		char s[30];
		fscanf(fd, "%ld", &subtablenum );
		fscanf(fd, "%c", s);
		long i;
		for (i = 0; i < tab->nDim; i++)	{
			fscanf(fd, "%ld", &SubTableCellIndexTuple[i]);
			if (i == 0)	{
				fscanf(fd, "%c", s);
			}
		}
		SubTableTupleForSubTable(TableIndex,subtablenum,SubTableTuple);
		for (i= 0; i<tab->nDim; i++)	{
			CVariable *var = &(m_var[tab->ExplVarnr[i]]);
			if (SubTableCellIndexTuple[i] < var->m_SubCodes[SubTableTuple[i]].NumberOfSubCodes() )	{
				TableCellIndex[i] = var->m_SubCodes[SubTableTuple[i]].GetSubCodeIndex(SubTableCellIndexTuple[i]);
			}
			else	{
				TableCellIndex[i] = var->m_SubCodes[SubTableTuple[i]].GetParentIndex();
			}
		}

		long Cellnum = tab->GetCellNrFromIndices(TableCellIndex);

		CDataCell *dc = tab->GetCell(Cellnum);
		if ((dc->GetStatus() != CS_EMPTY) && (dc->GetStatus() != CS_EMPTY_NONSTRUCTURAL))	{
			dc->SetStatus(CS_SECONDARY_UNSAFE);
		}
		else {
			return false;
		}
	}
	//}
	delete [] SubTableCellIndexTuple;
	delete [] SubTableTuple;
	delete [] TableCellIndex;

	return true;
}

bool TauArgus::SetAllEmptyNonStructural(long TableIndex)
{
	if ((TableIndex <0) || (TableIndex >= m_ntab))	{
		return false;
	}

	CTable *tab = GetTable(TableIndex); //&(m_tab[index]);
	for (long i = 0; i < tab->nCell; i++) {
		CDataCell *dc = tab->GetCell(i);
		if (dc->GetStatus() == CS_EMPTY) {
			CDataCell *dcempty = new CDataCell(tab->NumberofMaxScoreCell,tab->NumberofMaxScoreHolding,tab->ApplyHolding, tab->ApplyWeight);
			tab->CellPtr[i] = dcempty;
			dc = tab->GetCell(i);
			dc->SetStatus(CS_EMPTY_NONSTRUCTURAL);
//			tab->ComputeCellSafeCode (dc);
		}
	}
	return true;
}

bool TauArgus::SetSingleEmptyAsNonStructural(long TableIndex, long *DimIndex)
{
	/*
	if (TableIndex < 0 || TableIndex >= m_ntab) {
		return false;
	}

	CTable *tab = &(m_tab[TableIndex]);
	for (long i = 0; i < tab->nDim; i++) {
		int nCodes = m_var[tab->ExplVarnr[i]].GetnCode();
		ASSERT(DimIndex[i] >= 0 && DimIndex[i] < nCodes);
		if (DimIndex[i] < 0 || DimIndex[i] >= nCodes) {
			return false ;
		}
	}

	CDataCell *dc = tab->GetCell(DimIndex);
	if (dc->GetStatus() == CS_EMPTY) {
		CDataCell *dcempty = new CDataCell(tab->NumberofMaxScoreCell,tab->NumberofMaxScoreHolding,tab->ApplyHolding, tab->ApplyWeight);
		tab->CellPtr.SetAt(tab->GetCellNrFromIndices(DimIndex),dcempty);
		dc = tab->GetCell(tab->	GetCellNrFromIndices(DimIndex));
		dc->SetStatus(CS_EMPTY_NONSTRUCTURAL);
	}
	else {
		return false;
	}
	return true;
*/
	return true;
}


bool TauArgus::SetSingleNonStructuralAsEmpty(long TableIndex, long *DimIndex)
{
/*
	if (TableIndex < 0 || TableIndex >= m_ntab) {
		return false;
	}

	CTable *tab = &(m_tab[TableIndex]);
	for (int i = 0; i < tab->nDim; i++) {
		int nCodes = m_var[tab->ExplVarnr[i]].GetnCode();
		ASSERT(DimIndex[i] >= 0 && DimIndex[i] < nCodes);
		if (DimIndex[i] < 0 || DimIndex[i] >= nCodes) {
			return false;
		}
	}

	CDataCell *dc = tab->GetCell(DimIndex);
	if (dc->GetStatus() == CS_EMPTY_NONSTRUCTURAL)	{
		//dcempty = new CDataCell(tab->NumberofMaxScoreCell,tab->NumberofMaxScoreHolding,tab->ApplyHolding, tab->ApplyWeight);
		dc = tab->GetCell(tab->	GetCellNrFromIndices(DimIndex));
		delete dc;
		tab->CellPtr.SetAt(tab-> GetCellNrFromIndices(DimIndex),NULL);
	}
	else {
		return false;
	}
*/
	return true;
}



bool TauArgus::SetAllNonStructuralAsEmpty(long TableIndex)
{
	if (TableIndex < 0 || TableIndex >= m_ntab) {
		return false;
	}

	CTable *tab = GetTable(TableIndex); // &(m_tab[index]);
	for (long i=0; tab->nCell; i++)	{
		CDataCell *dc = tab->GetCell(i);
		if (dc->GetStatus() == CS_EMPTY_NONSTRUCTURAL)	{
			delete dc;
			tab->CellPtr[i] = NULL;
		}
	}

	return true;
}

string TauArgus::GetErrorString(long ErrorNumber)
{
	char ErrorNumberString[10];

	sprintf(ErrorNumberString, "%ld", ErrorNumber);

	string Error = TauArgusErrors.getProperty(ErrorNumberString);

	if (Error.empty()) {
		Error = string("UnKnown Error (") + ErrorNumberString + ")";
	}

	return Error;
}

///////////////////////////////////////////////////////////////////////////
/////////// end export functions
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
/////////// other functions
///////////////////////////////////////////////////////////////////////////

// Clean up all allocated memory
void TauArgus::CleanUp()
{
	if (m_nvar > 0) {
		delete [] m_var;
		m_nvar = 0;
	}
        
	CleanTables();

	m_fname[0] = 0;
	m_ValueSeparator = ',';
	m_ValueTotal = "Total";
	m_VarNrHolding = -1;
	m_VarNrWeight = -1;
        m_VarNrRecordKey = -1;

}

// return -1: error
// returns 0: EOF
// return 1: oke
// a line in the micro record is read and set in the string
int TauArgus::ReadMicroRecord(FILE *fd, char *str)
{
	int length = 0;

	while (length == 0) {
		str[0] = 0;
		fgets((char *)str, MAXRECORDLENGTH, fd);
		if (str[0] == 0) return 0;  // EOF
		length = strlen((char *)str) - 1;
		while (length > 0 && str[length] < ' ') length--;
		if (length == 0) continue;  // skip empty records
		str[length + 1] = 0;
		if ( InFileIsFixedFormat) {
			if (length  + 1 != m_fixedlength) {
				return -1; // error
			}
		}
	}

	return 1; // oke
}

// with a string fill the code lists
int TauArgus::DoMicroRecord(char *str, int *varindex)
{
	string tempcode;
	vector<char *> VarCodes;

	if (!InFileIsFixedFormat) {
            if (!ReadVariablesFreeFormat(str, VarCodes)) {
                return PROBLEMSREADINGFREEFORMAT;
            }
        }
	for (int i = 0; i < m_nvar; i++) {
		*varindex = i;
		CVariable *var = &(m_var[i]);
		if (var->IsCategorical || var->IsNumeric) {
        		char code[MAXCODEWIDTH];
			if (InFileIsFixedFormat) {
				int bp = var->bPos;         // startposition
				int ap = var->nPos;         // number of positions
				strncpy(code, (const char *)&str[bp], ap); // get code from record
				code[ap] = 0;
			}
			else {
				int ap = var->nPos;         // number of positions
				strcpy(code, VarCodes[var->bPos]);
				var->NormaliseCode(code);
				code[ap] = 0;
			}
                        // exclude missing codes
                        if (var->Missing1 != code && var->Missing2 != code) {
                                if (var->IsCategorical) { // only a categorical var has a codelist
                                        if (var->IsHierarchical && var->nDigitSplit == 0) {
                                                // code should be in already defined list of codes
                                                if (var->FindHierarchicalCode(code) < 0) { // not found by normal basis ones
                                                        return CODENOTINCODELIST;
                                                }
                                        }
                                        else {
                                                if (!var->AddCode(code, false) ) {   // adds if new, else does nothing
                                                        return PROGRAMERROR;
                                                }
                                        }
                                }

                                if (var->IsNumeric) {
                                        double d;
                                        if (!ConvertNumeric(code, d) ) {
                                                return ISNOTNUMERIC;   // is not numeric! (variabelenummer meegeven voor gebruiker)
                                        }
                                        if (d > var->MaxValue) var->MaxValue = d;
                                        if (d < var->MinValue) var->MinValue = d;
                                }
                        }
		}
	}

	return true;
}

/*
void TauArgus::QuickSortMaxScore(double &doub, int &intg, int first, int last)
{

  int i, j;
  string mid, temp;

  ASSERT(first >= 0 && last >= first);

  do {
    i = first;
    j = last;
    mid = doub[(i + j) / 2];
    do {
      while (doub[i] < mid) i++;
      while (doub[j] > mid) j--;
      if (i < j) {
		itemp = intg[i]
        dtemp = doub[i];
        doub[i] = doub[j];
        doub[j] = dtemp;
		intg[j] =itemp;
      } else {
        if (i == j) {
          i++;
          j--;
        }
        break;
      }
    } while (++i <= --j);

    if (j - first < last - i) {
      if (j > first) {
        QuickSortMaxScore(doub,intg, first, j);
      }
      first = i;
    } else {
      if (i < last) {
        QuickSortMaxScore(doub,intg, i, last);
      }
      last = j;
    }
  } while (first < last);

}

*/

// converts code to double; leading and trailing spaces are ignored;
// the last chararter should be a zero-byte
bool TauArgus::ConvertNumeric(char *code, double &d)

{
	char *stop;

	d = strtod(code, &stop);

	if (*stop != 0) {
		while (*(stop) == ' ') stop++;
		if (*stop != 0) {
			return false;
		}
	}

	return true;
}

// fill code lists in a table. This is the sibling of do micro record

bool TauArgus::FillInTable(long Index, string *sCodes, double Cost,
			   double Resp,double Shadow, long Freq,
			   double *TopNCell, double *TopNHolding,
                           double LPL, double UPL,
                           long Status, long & error, long & ErrorVarNo)
{
	CTable *tab = &(m_tab[Index]);
	long *tabind = new long [tab->nDim];

	// Find Variable indexes for a code
	ErrorVarNo = 0;
	int j;
	for (j = 0; j < tab->nDim; j++)  {
		CVariable *var = &(m_var[tab->ExplVarnr[j]]);
		if (var->IsHierarchical && var->nDigitSplit == 0) {
			var->TableIndex = var->FindAllHierarchicalCode(sCodes[j].c_str());
		}
		else {
			bool IsMissing;
			var->TableIndex = BinSearchStringArray(var->sCode, sCodes[j], var->nMissing, IsMissing);
		}

		if (var->TableIndex < 0) {
			error = CODEDOESNOTEXIST;
			ErrorVarNo = j+1;
	        delete [] tabind;
			return false;
		}
		tabind[j] = var->TableIndex;
	}
	//find cell
	CDataCell *dc = tab ->GetCell(tabind);
	// if cell already exists
	if ((dc->IsFilled) )  {
		error = CELLALREADYFILLED;
		return false;
	}
	// create new cell
	else {
		CDataCell *dcempty = new CDataCell(tab->NumberofMaxScoreCell,tab->NumberofMaxScoreHolding,tab->ApplyHolding, tab->ApplyWeight);
		long CellIndex = tab->GetCellNrFromIndices(tabind);

		// check if cell index is valid
		tab->CellPtr[CellIndex] = dcempty;
		dc = tab ->GetCell(tabind);
		dc->IsFilled = true;

	}

	// NOT too sure about this
	// set value
	dc->SetResp(Resp);
	dc->SetShadow(Shadow);
	dc->SetCost(Cost);
	dc->SetStatus(Status);

	// if frequency is given
//	if (m_HasFreq)		{
//		dc->SetFreq(Freq);
//	}
//	else {
//		dc->SetFreq(1);
//	}
// Als de freq gegeven is, invullen andser  op 1 zetten
	if (Freq > 1)		{
		dc->SetFreq(Freq);
	}
	else {
		dc->SetFreq(1);
	}

	// if status is given
	if (m_HasStatus) {
		dc->SetStatus(Status);
		if (Status == CS_UNSAFE_MANUAL)	{
			dc->SetLowerProtectionLevel(LPL);
			dc->SetUpperProtectionLevel(UPL);
		}
	}
	// if top n is given
	if (m_HasMaxScore)  {
		for (j=0; j<dc->nMaxScoreCell; j++)  {
			dc->MaxScoreCell[j] = TopNCell[j];
		}
		// sort max score.
		//dc->SortMaxScore();
		for (j=0; j<dc->nMaxScoreHolding; j++)	{
			dc->MaxScoreHolding[j] = TopNHolding[j];
		}
	}
	delete [] tabind;
	return true;
}

// str: content microdata record
// fill tables from a micro record

//TODO: add computation of cellkey
void TauArgus::FillTables(char *str)
{
	int i, j;
	char code[MAXCODEWIDTH];
	bool IsMissing;
	string tempcode;
	CVariable *var;
	CDataCell dc;
	string  tempPeepCode;
        vector<char *> VarCodes;
        bool readingFreeFormatResult=false; // Just to initialize PWOF 20170127

	if (!InFileIsFixedFormat) {
            readingFreeFormatResult = ReadVariablesFreeFormat(str, VarCodes);
        }
	//find the holding code and correct Holding Nr
	// if a new holding code is found.
	for (i = 0; i < m_ntab; i++){
		CTable *tab1 = &(m_tab[i]);
		if (tab1->ApplyHolding) {
			// find out what the current holding number is
			ASSERT(m_VarNrHolding >= 0);
			var = &(m_var[m_VarNrHolding]);
			if (InFileIsFixedFormat) {
				strncpy(code, (char *)&str[var->bPos], var->nPos);
				code[var->nPos] = 0;
			}
			else {
				if (readingFreeFormatResult) {
					strcpy(code, VarCodes[var->bPos]);
                        		var->NormaliseCode(code);
					code[var->nPos] = 0;
				}
			}

			if (LastHoldingCode != code){
				CurrentHoldingNr++;
				LastHoldingCode = code;
			}
		}
		break;
	}

	// set TableIndex and recode in VARIABLES as na
	for (i = 0; i < m_nvar; i++) {
		m_var[i].TableIndex = -1;
		m_var[i].Recode.DestCode = 0;
		m_var[i].ValueToggle = 0;
	}

	// compute for each involved variable the table index
	for (i = 0; i < m_ntab; i++) {
		CTable *tab = &(m_tab[i]);

		// explanation variables
		for (j = 0; j < tab->nDim; j++) {
			var = &(m_var[tab->ExplVarnr[j]]);
			if (var->TableIndex < 0) { // first time, so compute index
				if (InFileIsFixedFormat) {
					strncpy(code, (char *)&str[var->bPos], var->nPos);
					code[var->nPos] = 0;
				}
				else {
					if (readingFreeFormatResult) {
						strcpy(code, VarCodes[var->bPos]);
                                		var->NormaliseCode(code);
						code[var->nPos] = 0;
					}
				}
				if (var->IsHierarchical && var->nDigitSplit == 0) {
					var->TableIndex = var->FindHierarchicalCode(code);
				}
				else {
					var->TableIndex = BinSearchStringArray(var->sCode, code, var->nMissing, IsMissing);

				}
				ASSERT(var->TableIndex >= 0);
			}
		}
		// PeepVarnr

		//if Peep variables are set. Find Peep Code
		if (tab->ApplyPeeper) {
			var = &(m_var[tab->PeepVarnr]);
			if (InFileIsFixedFormat) {
				strncpy(code, (char *)&str[var->bPos], var->nPos);
				code[var->nPos] = 0;
			}
			else {
				if (readingFreeFormatResult) {
					strcpy(code, VarCodes[var->bPos]);
                        		var->NormaliseCode(code);
					code[var->nPos] = 0;
				}
			}
			tempPeepCode = code;
		}

		// ShadowVarnr
		if ((tab->ShadowVarnr > 0) && (tab->ShadowVarnr < m_nvar))	{
			var = &(m_var[tab->ShadowVarnr]);
			if (var->ValueToggle == 0) { // first time, so compute index
				if (InFileIsFixedFormat) {
					strncpy(code, (char *)&str[var->bPos], var->nPos);
					code[var->nPos] = 0;
				}
				else {
					if (readingFreeFormatResult) {
						strcpy(code, VarCodes[var->bPos]);
                                		var->NormaliseCode(code);
						code[var->nPos] = 0;
					}
				}
				var->Value = atof(code);
				var->ValueToggle = 1;
			}
		}
	/*	else	{
			var->Value = 1;
			var ->ValueToggle = 1;
		}*/

		// CostVarnr
		if (tab->CostVarnr >= 0) {
			var = &(m_var[tab->CostVarnr]);
			if (var->ValueToggle == 0) { // first time, so compute index
				if (InFileIsFixedFormat) {
					strncpy(code, (char *)&str[var->bPos], var->nPos);
					code[var->nPos] = 0;
				}
				else {
					if (readingFreeFormatResult) {
						strcpy(code, VarCodes[var->bPos]);
                                		var->NormaliseCode(code);
						code[var->nPos] = 0;
					}
				}
				var->Value = atof(code);
				var->ValueToggle = 1;
			}
		}

		// RespVarnr
		if ((tab->ResponseVarnr >= 0) && (tab->ResponseVarnr <m_nvar))	{
			var = &(m_var[tab->ResponseVarnr]);
			if (var->ValueToggle == 0) { // first time, so compute value
				if (InFileIsFixedFormat) {
					strncpy(code, (char *)&str[var->bPos], var->nPos);
					code[var->nPos] = 0;
				}
				else {
					if (readingFreeFormatResult) {
						strcpy(code, VarCodes[var->bPos]);
                                		var->NormaliseCode(code);
						code[var->nPos] = 0;
					}
				}
				var->Value = atof(code);
				var->ValueToggle = 1;
			}
		}
                
                // CellKeyVarnr
                if ((tab->CellKeyVarnr >= 0) && (tab->CellKeyVarnr < m_nvar)) {
                    var = &(m_var[tab->CellKeyVarnr]);
                    if (var->ValueToggle == 0) { // first time, so compute value
                        if (InFileIsFixedFormat) {
                            strncpy(code, (char *)&str[var->bPos], var->nPos);
                            code[var->nPos] = 0;
                        }
                        else {
                            if (readingFreeFormatResult){
                                strcpy(code, VarCodes[var->bPos]);
                                var->NormaliseCode(code);
                                code[var->nPos] = 0;
                            }
                        }
                        var->Value = atof(code);
                        var->ValueToggle = 1;
                    }
                }
	}

	// since there is only one weight var
	// weight var
	if (m_VarNrWeight >= 0) {
		var = &(m_var[m_VarNrWeight]);
		if (var->ValueToggle == 0) { // first time, so compute value
			if (InFileIsFixedFormat) {
				var = &(m_var[m_VarNrWeight]);
				strncpy(code, (char *)&str[var->bPos], var->nPos);
				code[var->nPos] = 0;
			}
			else {
				if (readingFreeFormatResult) {
					strcpy(code, VarCodes[var->bPos]);
                                        var->NormaliseCode(code);
					code[var->nPos] = 0;
				}
			}
			var->Value = atof(code);
			var->ValueToggle = 1;
		}
	}

	// now tabulate all tables from list
	for (i = 0; i < m_ntab; i++) {
            CTable *table = &m_tab[i];
		// set peeps to peep 1 and peep 2

		if (table->ApplyPeeper) {

			if (tempPeepCode == m_var[table->PeepVarnr].PeepCode1 ||
			  tempPeepCode == m_var[table->PeepVarnr].PeepCode2) {
			  dc.SetPeepCell(m_var[table->ShadowVarnr].Value);
			  dc.SetPeepHolding(m_var[table->ShadowVarnr].Value);
			  if (tempPeepCode == m_var[table->PeepVarnr].PeepCode1)	{
					dc.SetPeepSortCell(PEEP1);
					dc.SetPeepSortHolding(PEEP1);
			  }
			  else	{
					dc.SetPeepSortCell(PEEP2);
					dc.SetPeepSortHolding(PEEP2);
			  }
		  }
		  else {
			  dc.SetPeepCell(0);
			  dc.SetPeepHolding(0);
			  dc.SetPeepSortCell(NOPEEP);
			  dc.SetPeepSortHolding(NOPEEP);
		  }

	  }

		dc.SetWeight(m_var[m_VarNrWeight].Value);
                if ((table->ResponseVarnr >= 0) && (table->ResponseVarnr < m_nvar))	{
			dc.SetResp(m_var[table->ResponseVarnr].Value);
                        dc.SetNWResp(m_var[table->ResponseVarnr].Value);
		}
		else	{	//freq table
				dc.SetResp(1);
                                dc.SetNWResp(1);
		}
		if ((table->ShadowVarnr >= 0) && (table->ShadowVarnr < m_nvar))	{
 			dc.SetShadow(m_var[table->ShadowVarnr].Value);
		}
                
                if ((table->CellKeyVarnr >= 0) && (table->CellKeyVarnr < m_nvar)){
 			// Standard cellkey: add all recordkeys
                        dc.SetCellKey(m_var[table->CellKeyVarnr].Value);
                        
                        // Special cellkey: only add recordkeys of |contributions| > 0 (in practice: > 1E-8)
                        if ((table->ResponseVarnr >= 0) && (table->ResponseVarnr < m_nvar)){
                            if (fabs(m_var[table->ResponseVarnr].Value) > 1E-8){
                                dc.SetCellKeyNoZeros(m_var[table->CellKeyVarnr].Value);
                            }
                        }
		}
                
		if (table->CostVarnr >= 0) {
 		  dc.SetCost(m_var[table->CostVarnr].Value);
		}
		else {
 		  dc.SetCost(0); // later comes freq or constant 1 etc
		}
		dc.SetFreq(1);
		AddTableCells(m_tab[i], dc, 0, 0);
	}
}


void TauArgus::AddTableCells(CTable &t, CDataCell AddCell, int niv, long cellindex)
{
	// In this function the table is actually created. The totals are generated
	if (niv == t.nDim) {
		AddTableCell(t, AddCell, cellindex);
		return;
	}

	CVariable *var = &(m_var[t.ExplVarnr[niv]]); // for easier reference and performance

	cellindex *= var->GetnCode();
	AddTableCells(t, AddCell, niv + 1, cellindex + var->TableIndex); // for code
	AddTableCells(t, AddCell, niv + 1, cellindex);                   // for total

	// do hierarchical codelists

	if (var->IsHierarchical) {
		if (var->nDigitSplit > 0) {
			int n = 0;
			for (int i = 0; i < var->nDigitSplit - 1; i++) {
				n += var->DigitSplit[i];
				string s = var->sCode[var->TableIndex].substr(0, n);
				bool IsMissing;
				int hIndex = BinSearchStringArray(var->sCode, s, var->nMissing, IsMissing);
				ASSERT(hIndex >= 0 && hIndex < var->GetnCode());
				AddTableCells(t, AddCell, niv + 1, cellindex + hIndex); // for subtotals in codelist
			}
		}
		else {
			int i = var->TableIndex;
			int level = var->GethCode()[i].Level;
			for (i = i - 1; i > 0 && level > 1; i--) {
				if (var->GethCode()[i].Level == level - 1) {
					level--;
    				AddTableCells(t, AddCell, niv + 1, cellindex + i); // for subtotals in codelist
				}
			}
		}
	}
}

// In this function the cell that is created
//is added to the cells that already exists in the table
void TauArgus::AddTableCell(CTable &t, CDataCell AddCell, long cellindex)
{
	int i, j;
	double weight = 1;
	double x ;
	ASSERT(cellindex >= 0 && cellindex < t.nCell);
	CDataCell *dc = t.GetCell(cellindex);
	// Does a new cell have to be created
	if (!dc->IsFilled) {
		CDataCell *dcempty = new CDataCell(t.NumberofMaxScoreCell,t.NumberofMaxScoreHolding,t.ApplyHolding, t.ApplyWeight);
		t.CellPtr[cellindex] = dcempty;
		dc = t.GetCell(cellindex);
	}

	// copy weight
	if (t.ApplyWeight) {
		weight = m_var[m_VarNrWeight].Value;
	}

        // Needed for CKMType = "D", i.e., if KeepMinScore = true
        if (t.KeepMinScore){
            x = fabs(m_var[t.ShadowVarnr].Value);
            if (x < dc->MinScoreCell){
                dc->MinScoreCell = x;
                if (t.ApplyWeight) dc->MinScoreWeightCell = weight;
            }
        }

	// adjust the max score array to make way for a new greater score
	if (t.NumberofMaxScoreCell > 0) {
		x = fabs(m_var[t.ShadowVarnr].Value);
		for (i = 0; i < t.NumberofMaxScoreCell; i++) {
			if (x > dc->MaxScoreCell[i]) {
				// shift rest
				for (j = t.NumberofMaxScoreCell - 2; j >= i; j--) {
					dc->MaxScoreCell[j + 1] = dc->MaxScoreCell[j];
					if (t.ApplyWeight)	{
						dc->MaxScoreWeightCell[j + 1] = dc->MaxScoreWeightCell[j];
					}
				}
				// set at new place
				dc->MaxScoreCell[i] = x;
				if (t.ApplyWeight) {
					dc->MaxScoreWeightCell[i] = weight;
				}
				break;
			}
		}
        }

	// apply weight
	if (t.ApplyWeight) {
		AddCell.SetResp(AddCell.GetResp() * weight);
		AddCell.SetShadow(AddCell.GetShadow() * weight);
		if (t.CostVarnr >= 0) {
			///
			AddCell.SetCost(AddCell.GetCost(1) * weight);
		}
	}
	// if the table has holdins the Topn N Holdings have to be created
	if (t.ApplyHolding)	{
		if (t.NumberofMaxScoreHolding > 0)	{
			if (dc->GetHoldingNr() != CurrentHoldingNr)
			{
				x = fabs(dc->GetTempShadow());
				if (x> 0)
				{
					// double dtemp = x;
				}
				for (i = 0; i < t.NumberofMaxScoreHolding; i++) {
					if (x > dc->MaxScoreHolding[i]) {
				// shift rest
						for (j = t.NumberofMaxScoreHolding - 2; j >= i; j--) {
							dc->MaxScoreHolding[j + 1] = dc->MaxScoreHolding[j];
							dc->HoldingnrPerMaxScore[j + 1] = dc->HoldingnrPerMaxScore[j];
						}
						// set at new place
						dc->MaxScoreHolding[i] = x;
						dc->HoldingnrPerMaxScore[i]=dc->GetHoldingNr();
						break;
					}
				}

			}
		}
	}
	// Do the cell addition
	*dc += AddCell;
}

// when a table is recoded. The cells are added. to create the recoded table
void TauArgus::AddTableToTableCell(CTable &tabfrom, CTable &tabto, long ifrom, long ito)
{
	ASSERT(ifrom >= 0 && ifrom < tabfrom.nCell);
	CDataCell *dc1 = tabfrom.GetCell(ifrom);
	ASSERT(ito >= 0 && ito < tabto.nCell);
	CDataCell *dc2 = tabto.GetCell(ito);
	if (!dc2->IsFilled)	{
		CDataCell *dcempty = new CDataCell(tabto.NumberofMaxScoreCell,tabto.NumberofMaxScoreHolding, tabto.ApplyHolding, tabto.ApplyWeight);
		tabto.CellPtr[ito] = dcempty;
		dc2 = tabto.GetCell(ito);
	}
	// add and store
	*dc2 += *dc1;

}

// Frequencys and Topn are checked when holdings exist
void TauArgus::MergeLastTempShadow()
{
	for (int i = 0; i < m_ntab; i++)
	{
		CTable *tab = &(m_tab[i]);
		if (tab ->ApplyHolding)
		{
			for (int j = 0; j < tab->nCell;j++)
			{
				CDataCell *dc = tab->GetCell(j);
				if (dc->GetHoldingNr() != -1){
					if (tab->NumberofMaxScoreHolding > 0) {
						double x = fabs(dc->GetTempShadow());
						for (int k = 0; k < tab->NumberofMaxScoreHolding; k++) {
							if (x > dc->MaxScoreHolding[k]) {
							// shift rest
								for (int l = tab->NumberofMaxScoreHolding - 2; l >= k; l--) {
									dc->MaxScoreHolding[l + 1] = dc->MaxScoreHolding[l];
									dc->HoldingnrPerMaxScore[l+1] =dc->HoldingnrPerMaxScore[l];
								}
								dc->MaxScoreHolding[k] = x;
								dc->HoldingnrPerMaxScore[k]=dc->GetHoldingNr();
								break;
							}
						}
					}
					// add Freq once more
					dc->SetFreqHolding(dc->GetFreqHolding() + 1);
//					if ((dc->GetPeepSortHolding() == PEEP1) || (dc->GetPeepSortHolding() == PEEP2)) { AHNL 5.1.2004: Fout Moet zijn TempSort
					if ((dc->GetTempPeepSortCell() == PEEP1) || (dc->GetTempPeepSortCell() == PEEP2)) {

						if (dc->GetPeepHolding() < dc->GetTempShadow()) {
							dc->SetPeepHolding(dc->GetTempShadow());
						}
						if (dc->GetPeepSortHolding() == NOPEEP || dc->GetPeepSortHolding() == EMPTY) {
						   dc->SetPeepSortHolding (dc->GetTempPeepSortCell()); // AHNL 5.1.2004 toegevoegd
						}

					}
					if (tab->IsFrequencyTable)	{
						dc->SetResp(dc->GetFreqHolding());
					}
				}
			}
		}
	}
}

// All holding Tables need to get initialized.
void TauArgus::InitializeHoldingTables()
{
	for (int i = 0; i < m_ntab; i++){
		CTable *tab = & (m_tab[i]);
		if (tab->ApplyHolding){
			tab->InitializeHoldingNrs();
		}
	}

}

////////////////////////////////////////////////////////////////////////////
/// PARSE RECODE LIST
///  0 : - 90
///  1 : 90 - 500
///  2 : 500 -
///  3 : 11,13, 512, 530-570, 930-970
///  phase: CHECK       check only syntax
///         DESTCODE    compute and sort dest code
///         SRCCODE     compute link between src and dest code

bool TauArgus::ParseRecodeString(long VarIndex, LPCTSTR RecodeString, long FAR* ErrorType, long FAR* ErrorLine, long FAR* ErrorPos, int Phase)
{
	int PosInString = 0, LineNumber = 1;

	// first detect lines in RecodeString
	while (1) {
		int oke = ParseRecodeStringLine(VarIndex, &RecodeString[PosInString], ErrorType,  ErrorPos, Phase);
		if (!oke) {
			*ErrorLine = LineNumber;
			return false;
		}
		LineNumber++;
		const char *p = strstr(&RecodeString[PosInString], SEPARATOR);
		if (p == 0) break;
		PosInString = p - RecodeString + strlen(SEPARATOR);
	}

	return true;
}

// Parse a line of a recode string (until str[i] == 0 || str[i] == '\n' || str[i] == '\r')
bool TauArgus::ParseRecodeStringLine(long VarIndex, LPCTSTR str, long FAR* ErrorType, long FAR* ErrorPos, int Phase)
{
	int i = 0, len = strlen(str), res, fromto, position;
	int nPos = m_var[VarIndex].nPos;

	// the three codes
	char DestCode[MAXCODEWIDTH + 1];
	char SrcCode1[MAXCODEWIDTH + 1];
	char SrcCode2[MAXCODEWIDTH + 1];

	for (i = 0; i < len; i++) {
		if (str[i] > 32 || str[i] < 0) break;
		if (str[i] < 32) break;
	}

	if (i == len || str[i] < 32) {  // all characters useless, oke
		return true;
	}

	// now at least one character > 32
	// parse something like: 1 : 1 - 32 , 35 - 78, 99

	// first the destination code
	res = ReadWord(str, DestCode, NULL, ':', fromto, position);

	if (res < 0 || str[res] != ':') {
		*ErrorType = E_HARD;
		*ErrorPos = position + i + 1;
		return false;
	}
	i += res;

	//if (strlen(DestCode) > (size_t) nPos) {
	//  *ErrorType = E_LENGTHWRONG;
	//  *ErrorPos = position + i + 1;
	//  return false;
	//}


	// now the source code(s)
	do {
		i++;
		res = ReadWord(&str[i], SrcCode1, SrcCode2, ',', fromto, position);
		if (res < 0) {
			*ErrorType = E_HARD;
			*ErrorPos = position + i + 1;
			return false;
		}

		// length oke?
		if (strlen(SrcCode1) > (size_t) nPos || strlen(SrcCode2) > (size_t) nPos) {
			*ErrorType = E_LENGTHWRONG;
			*ErrorPos = position + i + 1;
			return false;
		}

		AddSpacesBefore(SrcCode1, nPos);
		AddSpacesBefore(SrcCode2, nPos);

		// ScrCode2 smaller then SrcCode1?
		if (fromto == FROMTO_RANGE) {
			if (strcmp(SrcCode1, SrcCode2) > 0) {
				*ErrorType = E_RANGEWRONG;
				*ErrorPos = position + i + 1;
				return false;
			}
		}

		switch (Phase) {
			case DESTCODE:
				AddRecode(VarIndex, DestCode);
				break;
			case SRCCODE:
			{
				int ErrCode = SetCode2Recode(VarIndex, DestCode, SrcCode1, SrcCode2, fromto);
				if (ErrCode == R_MISSING2VALID) {   // a mortal sin
					*ErrorType = ErrCode;
					*ErrorPos = position + i + 1;
					return false;
				}
			}
			break;
		}

		i += res;

	} while (strchr("\n\r", str[i]) == 0 && str[i] != 0);

	return true;
}

int TauArgus::ReadWord(LPCTSTR str, char* CodeFrom, char *CodeTo, char EndCode, int& fromto, int& pos)
{
	int i = 0, j, pass = 0;
	char *p;

	fromto = FROMTO_SOLO;
	pos = 0;
	CodeFrom[0] = 0;
	if (CodeTo) {
		CodeTo[0] = 0;
	}

	do {
		switch (++pass) {
			case 1:
				p = CodeFrom;
				break;
			case 2:
				if (fromto == FROMTO_SOLO) {  // in the case: 1 2, one should use a hyphen between the codes
					pos = i;
					return -1;
				}
				p = CodeTo;
				break;
			default:
				pos = i;
				return -1;
		}

		j = 0;
		while (str[i] == ' ') i++;  // skip spaces before word or hyphen

		// a hyphen before?
		if (str[i] == '-') {
			if (fromto == FROMTO_SOLO) {  // first hyphen?
				switch (pass) {
					case 1:
						fromto = FROMTO_TO;
						break;
					default:
						pos = i;
						return -1;
				}
			}
			else {
				pos = i;
				return -1;  // more than one hyphen
			}
			i++;
			while (str[i] == ' ') i++;  // skip spaces between - and word
		}

		if (fromto == FROMTO_FROM && pass == 2) {
			fromto = FROMTO_RANGE;
		}

		// do the code
		if (str[i] == '"') {  // first character double quote?
			i++;
			while (str[i] != '\0' && strchr("\n\r\"", str[i]) == 0) {
				if (j < MAXCODEWIDTH) {
				p[j++] = str[i++];
			}
			else {
				pos = i;
				return -1;
			}
      }
      if (str[i] != '"') { // end double quote not found
			pos = i;
			return -1;
      }
      i++;
    // not a starting double quote
   }
	else {
      while (str[i] != '\0' && str[i] != EndCode && strchr(" -\n\r", str[i])== 0) {
        if (j < MAXCODEWIDTH) {
          p[j++] = str[i++];
        }
		  else {
          pos = i;
          return -1;
        }
      }
    }
    p[j] = 0;
    if (j == 0) { // empty
      pos = i;
      return -1;
    }

    while (str[i] == ' ') i++; // skip spaces after word

    // an ending hyphen?
    if (str[i] == '-') {
      if (fromto == FROMTO_SOLO) {
        switch (pass) {
        case 1:
          fromto = FROMTO_FROM;
          break;
        default:
          pos = i;
          return -1;
        }
        i++;
      } else {
        pos = i;
        return -1;  // more than one -
      }
    }

    while (str[i] == ' ') i++; // skip spaces after word

  } while (str[i] != EndCode && str[i] != '\0' && str[i] != '\n' && str[i] != '\r');

  return i; // oke, return current position
}

// codes fromto     :                SrcCode1  SrcCode2
//  10. FROMTO_TO          -23              Y        NA
//  11. FROMTO_SOLO         34              Y        NA
//  12. FROMTO_FROM         23-             Y        NA
//  13. FROMTO_RANGE        23-25           Y         Y
// SrcCode1 and SrcCode2 have a width equal to m_var[VarIndex].nPos
// returns 0 if oke, else errorcode

int TauArgus::SetCode2Recode(int VarIndex, char *DestCode, char *SrcCode1, char *SrcCode2, int fromto)
{
	CVariable *v = &(m_var[VarIndex]);
	int c, c1 = 0, c2 = 0;
	int n_codes = v->sCode.size() - v->nMissing;
	int DestIndex, Exact;
	bool DestMissing, Src1Missing, Src2Missing;

	// compute index SrcCode1
	Exact = true;
	if (c1 = BinSearchStringArray(v->sCode, SrcCode1, v->nMissing, Src1Missing), c1 < 0) { // not found?
		Exact = false;

		if (v->sCode[n_codes - 1] < SrcCode1) {  // bigger then last code
			c1 = n_codes;
		}
		else {
			c1 = 0;
			while (m_var[VarIndex].sCode[c1] < SrcCode1) c1++;
		}
	}

	Src2Missing = false;
	switch (fromto) {
		case FROMTO_TO:
			c2 = c1;
			if (!Exact) c2--;
			c1 = 0;
			break;
		case FROMTO_SOLO:
			if (!Exact) {
				m_nNoSense++;
				return R_CODENOTINLIST;
			}
			c2 = c1;
			break;
		case FROMTO_FROM:
			c2 = n_codes - 1;
			break;
		case FROMTO_RANGE:
			Exact = true;
			if (c2 = BinSearchStringArray(v->sCode, SrcCode2,
													v->nMissing, Src2Missing), c2 < 0) { // not found?
				Exact = false;
				if (v->sCode[n_codes - 1] < SrcCode2) {  // bigger then last code, oke
					c2 = n_codes;
				}
				else {
					c2 = 0;
					while (m_var[VarIndex].sCode[c2] < SrcCode2) c2++;
				}
			}
			if (!Exact) c2--;
			break;
		default:
			ASSERT(1 == 2);
			return PROGRAMERROR;

	}

	if (c2 < c1) {
		m_nNoSense++;
		return R_NOSENSE;
	}

	// c1 and c2 now are correct, I assume
	ASSERT(c1 >= 0 && c1 < v->sCode.size() && c2 >= c1 && c2 < v->sCode.size());

	// search destcode in list
	ASSERT(v->nMissing > 0 && v->nMissing < 3);

	AddSpacesBefore(DestCode, v->Recode.CodeWidth);
	DestIndex = BinSearchStringArray(v->Recode.sCode, DestCode, v->Recode.nMissing, DestMissing);

	// never make from a missing value a valid value!
	if (!DestMissing && (Src1Missing || Src2Missing) ) {
		return R_MISSING2VALID;
	}

	ASSERT(DestIndex >= 0 && DestIndex < v->Recode.sCode.size() );
	if (DestIndex < 0 || DestIndex >= ((int) v->Recode.sCode.size()) ) {
		return PROGRAMERROR;
	}

	// put dest code
	for (c = c1; c <= c2; c++) {
		int d = v->Recode.DestCode[c];
		if (d >= 0) {
			m_nOverlap++;
		}
		if (d != -2) {
			v->Recode.DestCode[c] = DestIndex;
		}
	}
	return 0;
}

// compute recoded tables . sibling function of fill tables, for recoded tables
bool TauArgus::ComputeRecodeTables()
{
	int i, d, nRecodes = 0;

	if  (m_nvar == 0  || m_ntab == 0  || !m_CompletedCodeList)  {
		return false;
	}

	// compute number of variables with a recode
	for (i = 0; i < m_nvar; i++) {
		if (m_var[i].HasRecode) nRecodes++;
	}

	if (nRecodes == 0) { // nothing to do, why do you call me?
		return false;
	}

	for (i = 0; i < m_ntab; i++) {
		int nDim = m_tab[i].nDim;
		for (d = 0; d < nDim; d++) {
			// a base table with at least one recode?
			if (m_var[m_tab[i].ExplVarnr[d]].HasRecode) {
				break;
			}
		}
		if (d == nDim) continue; // table has no recodes

		// prepare recoded table, first free previous one
		if (m_tab[i + m_ntab].nCell != 0) {
			m_tab[i + m_ntab].CleanUp();
		}

		m_tab[m_ntab + i] = m_tab[i]; // copy table

		// correct SizeDim in recoded table
		for (d = 0; d < nDim; d++) {
			if (m_var[m_tab[i].ExplVarnr[d]].HasRecode) {
				m_tab[m_ntab + i].SizeDim[d] = m_var[m_tab[i].ExplVarnr[d] ].Recode.nCode;
			}
		}

		// compute recoded table
		ComputeRecodeTable(m_tab[i], m_tab[m_ntab + i]);
		m_tab[i].HasRecode = true;
		m_tab[m_ntab + i].HasRecode = false;
		ComputeCellStatuses(m_tab[m_ntab + i]);
		SetProtectionLevels(m_tab[m_ntab + i]);
                ComputeCellKeys(m_tab[m_ntab + i]);

#ifdef _DEBUGG
		{ int i;
			for (i = 0; i < m_ntab; i++) {
				string fname;
				fname.Format("tabrecode%02d.txt", i);
				ShowTable((LPCTSTR) fname, m_tab[i + m_ntab]);
			}
		}
#endif // _DEBUG

  }

  return true;
}

// for recodes, only in case of a new code: the code is added
// returns index
int TauArgus::AddRecode(int varnr, const char *newcode)
{
	int i, n;

	n = m_var[varnr].Recode.sCode.size();
	// not already in list?
	for (i = 0; i < n; i++) {                               // optimizable with binary search (number > ??)
		if (m_var[varnr].Recode.sCode[i] == newcode) break;   // found!
	}
	if (i == n) {  // not found
		m_var[varnr].Recode.sCode.push_back(newcode);
	}

	return i;
}

int TauArgus::MakeRecodelistEqualWidth(int VarIndex, LPCTSTR Missing1, LPCTSTR Missing2)
{
	int i, length, ncode = m_var[VarIndex].Recode.sCode.size(), maxwidth = 0;

	// if missings empty take missings of variable[VarIndex]
	if (Missing1[0] == 0 && Missing2[0] == 0) {
		length = m_var[VarIndex].Missing1.length();
		if (length > maxwidth) maxwidth = length;
		length = m_var[VarIndex].Missing2.length();
		if (length > maxwidth) maxwidth = length;
	}

	for (i = 1; i < ncode; i++) {
		if (length = m_var[VarIndex].Recode.sCode[i].length(), length > maxwidth) {
			maxwidth = length;
		}
	}
	if (strlen(Missing1) > (size_t) maxwidth) maxwidth = strlen(Missing1);
	if (strlen(Missing2) > (size_t) maxwidth) maxwidth = strlen(Missing2);
	m_var[VarIndex].Recode.CodeWidth = maxwidth;

	// spaces before shorter ones
	for (i = 1; i < ncode; i++) {
		AddSpacesBefore(m_var[VarIndex].Recode.sCode[i], maxwidth);
	}

	return maxwidth;
}
// from source table create recoded destination table
bool TauArgus::ComputeRecodeTable(CTable& srctab, CTable& dsttab)
{
	dsttab.nCell = 1;
	// compute number of cells
	for (int d = 0; d < dsttab.nDim; d++) {
		dsttab.nCell *= dsttab.SizeDim[d];
	}

	if (!dsttab.PrepareTable() ) {
		return false;
	}

	ComputeRecodeTableCells(srctab, dsttab, 0, 0, 0);

	return true;
}

// sibling of AddTableCells
void TauArgus::ComputeRecodeTableCells(
	     CTable& srctab, CTable& dsttab,
		   int niv, int iCellSrc, int iCellDst)
{
	int i, desti, nDim = srctab.nDim;

	if (niv == nDim) {
		ASSERT(iCellSrc >= 0 && iCellSrc < srctab.nCell);
		ASSERT(iCellDst >= 0 && iCellDst < dsttab.nCell);
		AddTableToTableCell(srctab, dsttab, iCellSrc, iCellDst);
		return;
	}

	int iVar = srctab.ExplVarnr[niv];
	int n = srctab.SizeDim[niv];

	bool HasRecode = m_var[iVar].HasRecode;

	// for total
	ComputeRecodeTableCells(srctab, dsttab, niv + 1, iCellSrc * srctab.SizeDim[niv],
							 iCellDst * dsttab.SizeDim[niv]);


	for (i = 1; i < n; i++) {
		if (HasRecode) {
			desti = m_var[iVar].Recode.DestCode[i];
			if (desti < 0) { // (sub)total, skip hierarchical descendants
				continue;
			}
			ASSERT(desti >= 0 && desti < m_var[iVar].Recode.nCode);
		}
		else {
			desti = i;
		}
		ComputeRecodeTableCells(srctab, dsttab, niv + 1,
                                        iCellSrc * srctab.SizeDim[niv] + i,
                                        iCellDst * dsttab.SizeDim[niv] + desti);
	}
}

// Clean memory allocated for tables
void TauArgus::CleanTables()
{
	if (m_ntab > 0) {
		delete [] m_tab;
		m_ntab = 0;
	}
}

// statuses are calculated for cells
void TauArgus::ComputeCellStatuses(CTable &tab)
{
	long *DimNr = new long [tab.nDim];

	for (int c = 0; c < tab.nCell; c++) {
		CDataCell *dc = tab.GetCell(c);
		tab.GetIndicesFromCellNr(c,DimNr);
		if (tab.SetMissingAsSafe)	{
			for (int j = 0; j<tab.nDim; j++)	{
				CVariable *var = &(m_var[tab.ExplVarnr[j]]);
				if ((var->GetnMissing() != 0)) {
					if (DimNr[j] > (var->GetnCode()- var->GetnMissing() - 1)) {
	// Replaced 06-10-2012 to get this right for recoded variables
	//			if ((var->nMissing != 0))	{
	//				if (DimNr[j] > (var->nCode - var->nMissing-1))	{
						if (dc->GetFreq() > 0)	{
							dc->SetStatus(CS_SAFE);
							break;
						}
					}

				}
			}
		}
		int St = dc->GetStatus();
		//Anco 2-2-2005 Toegevoegd als status = 0
		if (St == CS_EMPTY || St == 0) {
			dc->SetStatus(tab.ComputeCellSafeCode(*dc) );
		}
	}
	delete[] DimNr;
}

// Protection Levels for cells are calculated. first Cell Status
// has to be calculated
void TauArgus::SetProtectionLevels(CTable &tab)
{
	for (int c = 0; c < tab.nCell; c++) {
		CDataCell *dc = tab.GetCell(c);
		tab.SetProtectionLevelCell(*dc);
	}
}

void TauArgus::ComputeCellKeys(CTable &tab)
{
    for (int c=0; c<tab.nCell; c++){
        CDataCell *dc = tab.GetCell(c);
        tab.ComputeCellKeyCell(*dc);
    }
}

// set that the table has been recoded
void TauArgus::SetTableHasRecode()
{
	for (int t = 0; t < m_ntab; t++) {
		int v;
		for (v = 0; v < m_tab[t].nDim; v++) {
			if (m_var[m_tab[t].ExplVarnr[v]].HasRecode) {
				break;
			}
		}
		m_tab[t].HasRecode = (v != m_tab[t].nDim);
	}
}

// write table in comma seperated format
void TauArgus::WriteCSVTable(FILE *fd, CTable *tab, bool EmbedQuotes, long *DimSequence, long *Dims, int niv, char ValueSep, long RespType)
{
	// write Cell
	if (niv == tab->nDim) {
		WriteCSVCell(fd, tab, Dims, false, 0, RespType);
		if (Dims[DimSequence[niv-1]] < m_var[tab->ExplVarnr[DimSequence[niv-1]]].GetnCode()-1) fprintf(fd, "%c", ValueSep);
		return;
	}

	if (tab->nDim == 1) {
		WriteCSVColumnLabels(fd, tab, 0, ValueSep, EmbedQuotes);
		fprintf(fd, "%c", ValueSep);
	}

	int n = m_var[tab->ExplVarnr[DimSequence[niv]]].GetnCode();
	for (int i = 0; i < n; i++) {
		if (niv == tab->nDim - 2) { // row variable?
			if (i == 0) {
				if (tab->nDim > 2) { // one or more layers in table?
					for (int j = 0; j < niv; j++) {
  						fprintf(fd, "Var %ld,", DimSequence[j] + 1);
  						WriteCSVLabel(fd, tab, DimSequence[j], Dims[DimSequence[j]], EmbedQuotes);  // show all previous layer label(s)
  						if (j < (niv - 1)) fprintf(fd, ", ");
					}
					fprintf(fd, "\n");
				}
				WriteCSVColumnLabels(fd, tab, DimSequence[tab->nDim - 1], ValueSep, EmbedQuotes);
			}
			WriteCSVLabel(fd, tab, DimSequence[niv], i, EmbedQuotes);  // show row label
			fprintf(fd, "%c", ValueSep);
		}

		Dims[DimSequence[niv]] = i;
		WriteCSVTable(fd, tab, EmbedQuotes, DimSequence, Dims, niv + 1, ValueSep, RespType);
		if (niv == tab->nDim - 2) {
			fprintf(fd, "\n");
		}
	}

	// empty row after each layer
	if (niv == tab->nDim - 2) {
		fprintf(fd, "\n");
	}
}

// show column labels
void TauArgus::WriteCSVColumnLabels(FILE *fd, CTable *tab, long dim, char ValueSep, bool EmbedQuotes)
{
	int i, n = m_var[tab->ExplVarnr[dim]].GetnCode();

	fprintf(fd, "%c", ValueSep); // first label in table always empty
	for (i = 0; i < n; i++) {
		WriteCSVLabel(fd, tab, dim, i, EmbedQuotes);
		if (i < n - 1) fprintf(fd, "%c", ValueSep);
	}
	fprintf(fd, "\n");
}

void TauArgus::WriteCSVLabel(FILE *fd, CTable *tab, long dim, int code, bool EmbedQuotes)
{
	string s;
	ASSERT(dim >= 0 && dim < tab->nDim);
	ASSERT(code >= 0 && code < m_var[tab->ExplVarnr[dim]].GetnCode() );

	if (code == 0) {s = m_var[tab->ExplVarnr[dim]].TotalCode;
	}
	else	{
		s = m_var[tab->ExplVarnr[dim]].GetCode(code);
	}
	if (s.empty() ) s = m_ValueTotal;
	ReplaceStringInPlace(s, '"', '\'');
        if (EmbedQuotes)
            fprintf(fd, "\"%s\"", s.c_str());
        else
            fprintf(fd, "%s", s.c_str());
}

// write cells
void TauArgus::WriteCSVCell(FILE *fd, CTable *tab, long *Dim, bool ShowUnsafe, int SBSCode, long RespType)
{
	int nDec = m_var[tab->ResponseVarnr].nDec;

	CDataCell *dc = tab->GetCell(Dim);
	switch (dc->GetStatus() ) {
		case CS_SAFE:
		case CS_SAFE_MANUAL:
		case CS_EMPTY_NONSTRUCTURAL:
		case CS_PROTECT_MANUAL:
			switch (RespType ){
			case 0: fprintf(fd, "%.*f", nDec, dc->GetResp()); break;
			case 1: fprintf(fd, "%.*f", nDec, dc->GetRoundedResponse());	break;
			case 2: fprintf(fd, "%.*f", nDec, dc->GetCTAValue ());break;
                        case 3: fprintf(fd, "%.*f", nDec, dc->GetCKMValue ());break;
			}
			break;
		case CS_UNSAFE_FREQ:
		case CS_UNSAFE_PEEP:
		case CS_UNSAFE_ZERO:
		case CS_UNSAFE_RULE:
		case CS_UNSAFE_SINGLETON:
		case CS_UNSAFE_MANUAL:
		case CS_SECONDARY_UNSAFE:
		case CS_SECONDARY_UNSAFE_MANUAL:
			switch (RespType ){
			case 0: if (ShowUnsafe) {
				       fprintf(fd, "%.*f", nDec, dc->GetResp());}
				    else {
					   fprintf(fd, "x");
					}
					break;
			case 1: fprintf(fd, "%.*f", nDec, dc->GetRoundedResponse());	break;
			case 2: fprintf(fd, "%.*f", nDec, dc->GetCTAValue ());break;
                        case 3: fprintf(fd, "%.*f", nDec, dc->GetCKMValue ());break;
			}

			break;
		case CS_EMPTY:
			fprintf(fd, "-");
			break;
	}
	if ( ShowUnsafe && ( SBSCode == 0) ) {
		switch (RespType){
			case 0:	fprintf(fd, ",%ld", dc->GetStatus()); 
				    break;
			case 1:// fprintf(fd, ",%.*f", nDec, dc->GetResp()); break;
			case 2: 
     				if (dc->GetStatus() == CS_EMPTY) {fprintf(fd, ",-");}
	    			else                             {fprintf(fd, ",%.*f", nDec, dc->GetResp());}
		    		if (RespType == 2){fprintf(fd, ",%ld", dc->GetStatus());}
			    	break;
		}
//		fprintf(fd, ",%d", dc->GetStatus());
	}

}

// Write the First Line in a CSV file
void TauArgus::WriteFirstLine(FILE *fd, LPCTSTR FirstLine)
{
	int LenFirstLine;

	if (LenFirstLine = strlen(FirstLine), LenFirstLine >= 1)	{
      fprintf(fd,"%s",FirstLine);
      fprintf(fd, "\n");
	}
}


// write SBSStaart
void TauArgus::WriteSBSStaart(FILE *fd, CTable *tab, long *Dim, char ValueSep, long SBSCode)
{
	CDataCell *dc = tab->GetCell(Dim);
	double X=0, X1=0, X2=0, XS=0; // Just to initialize PWOF 20170127
	//bool DomRule = tab->DominanceRule;
	bool PQRule = tab->PQRule;
	int f1 = dc->GetFreq();
	int f2 = dc->GetFreqHolding();
	if (f2 > 0) { f1 = f2; }

	fprintf(fd, ",%d%c", f1, ValueSep);
//		fprintf(fd, "%c", ValueSep);
	bool UnsafeRule = false;
	switch (dc->GetStatus() ) {
		case CS_SAFE:
		case CS_SAFE_MANUAL:
		case CS_EMPTY_NONSTRUCTURAL:
		case CS_PROTECT_MANUAL:
			fprintf(fd, "V");
			break;
		case CS_UNSAFE_FREQ:
			fprintf(fd, "A");
			break;
		case CS_UNSAFE_PEEP:
		case CS_UNSAFE_ZERO:
    	case CS_UNSAFE_SINGLETON:
		case CS_UNSAFE_MANUAL:
			fprintf(fd, "B");
			UnsafeRule = true;
			break;
		case CS_UNSAFE_RULE:
			if (PQRule) {
				fprintf(fd, "F"); }
			else
			if ((SBSCode == 3) || (SBSCode == 5)) {
				fprintf(fd, "C"); }
			else {
				fprintf(fd, "B"); }
			UnsafeRule = true;
			break;
		case CS_SECONDARY_UNSAFE:
		case CS_SECONDARY_UNSAFE_MANUAL:
			fprintf(fd, "D");
			break;
		case CS_EMPTY:
			fprintf(fd, "-");
			break;
	} //switch

	if (UnsafeRule) {
	X = 100;
		switch (SBSCode) {
			case 0: X1 = 0; break; // geen percentage mogelijk
			case 1: X1 = 0; break;
			case 2: X1 = dc->MaxScoreCell[0]; break;
			case 3:
				if (PQRule) {
					X1 = dc->MaxScoreCell[0];
					X2 = dc->MaxScoreCell[1];}
				else {
					X1 = dc->MaxScoreCell[0] + dc->MaxScoreCell[1];}
				break;
			case 4: X1 = dc->MaxScoreHolding[0]; break;
			case 5:
				if (PQRule) {
					X1 = dc->MaxScoreHolding[0];
					X2 = dc->MaxScoreHolding[1];}
				else {
					X1 = dc->MaxScoreHolding[0] + dc->MaxScoreHolding[1];}
				break;
		}
		XS = dc->GetShadow();
		if (XS == 0) { X = 0; }
		else { X = 100.0 * X1 / XS;}
		fprintf(fd, "%c%.1f", ValueSep, X);
		if (PQRule){
			if (XS == 0) { X = 0; }
			else { X = 100.0 * X2 / XS;}
			fprintf(fd, "%c%.1f", ValueSep, X);
		}
	}
}

// write cell
void TauArgus::WriteCellRecord(FILE *fd, CTable *tab, long *Dims, int niv, char ValueSep, long SBSCode, bool bSBSLevel,
                                    bool SuppressEmpty, bool ShowUnsafe, bool EmbedQuotes, long RespType)
{
	// write Cell
	if (niv == tab->nDim) {
		if (SuppressEmpty) {
			if (tab->GetCell(Dims)->GetFreq() == 0) return;
		}
		WriteCellDimCell(fd, tab, Dims, ValueSep, SBSCode, bSBSLevel, ShowUnsafe, EmbedQuotes, RespType);
		return;
	}

	int n = m_var[tab->ExplVarnr[niv]].GetnCode();
	for (int i = 0; i < n; i++) {
		Dims[niv] = i;
		WriteCellRecord(fd, tab, Dims, niv + 1, ValueSep, SBSCode, bSBSLevel, SuppressEmpty, ShowUnsafe, EmbedQuotes, RespType);
	}
}

void TauArgus::WriteCellDimCell(FILE *fd, CTable *tab, long *Dims, char ValueSep, long SBSCode, bool SBSLevel,
                                    bool ShowUnsafe, bool EmbedQuotes, long RespType)
{
	int l, n = tab->nDim;
	for (int i = 0; i < n; i++) {
		WriteCSVLabel(fd, tab, i, Dims[i], EmbedQuotes);
		fprintf(fd, "%c", ValueSep);
		if (SBSLevel) {
			if (Dims[i] == 0 || !m_var[tab->ExplVarnr[i]].IsHierarchical) {
			l = 0;
			}
			else{
        	l =  m_var[tab->ExplVarnr[i]].hCode[Dims[i]].Level; //.GetLevel(Dims[i]);
			}
			fprintf(fd, "%d%c", l, ValueSep);
		}
	}
	// Anco Not too sure about this
	WriteCSVCell(fd, tab, Dims, ShowUnsafe, SBSCode, RespType);
    if (SBSCode > 0) {
		WriteSBSStaart(fd, tab, Dims, ValueSep, SBSCode);
	}

	fprintf(fd, "\n");
}

CTable * TauArgus::GetTable(int t)
{
  ASSERT( t >= 0 && t < m_ntab);
  if (m_tab[t].HasRecode) return &(m_tab[t + m_ntab]);
	else	return &(m_tab[t]);
}



///////////////////////////////////////////////////////////////////////////
/////////// start debug functions
///////////////////////////////////////////////////////////////////////////


// only for debug
void TauArgus::ShowCodeLists()  // in output pane
{ 
  char s[100];
  for (int i = 0; i < m_nvar; i++) {
    CVariable *v = &(m_var[i]);
		sprintf(s, "%3ld %3ld %3ld %3d %8.0f %8.0f", v->bPos + 1, v->nPos, v->nDec, v->GetnCode(), v->MaxValue, v->MinValue);
    TRACE("Var %2d. %s\n", i + 1, s);
		for (int j = 0; j < v->GetnCode(); j++) {
      TRACE("  %4d. [%s]\n", j + 1, v->GetCode(j).c_str() );
    }
	}
}

// only for debug
bool TauArgus::ShowTable(const char *fname, CTable& tab)
{ FILE *fd;
 
  fd = fopen(fname, "w");
	if (fd == 0) return false;

  switch (tab.nDim) {
	case 1:
		{ int i;
		  for (i = 0; i < tab.SizeDim[0]; i++) {
				CDataCell *dc = tab.GetCell(i);
				if (dc->GetResp() == 0) {
				  fprintf(fd, "%3d. %12s \n", i + 1, "-");
				} else {
				  fprintf(fd, "%3d. %12.2f \n", i + 1, dc->GetResp());
				}
				if (dc->GetFreq() == 0) {
					fprintf(fd, "%5s \n", "-");
				} else {
					fprintf(fd, "%5ld \n", dc->GetFreq());
				}
			}
		}
		break;
	case 2:
    ShowTableLayer(fd, 0, 1, 0, tab);
		break;
	case 3:
		{ int i;
			for (i = 0; i < tab.SizeDim[0]; i++) {
				fprintf(fd, "\n*** Layer %d Code [%s] ***\n\n", i + 1, m_var[tab.ExplVarnr[0]].GetCode(i).c_str() );
        ShowTableLayer(fd, 1, 2, i * tab.SizeDim[1] * tab.SizeDim[2], tab);
      }
		}
		break;
	case 4:
		{ int i, j;
			for (i = 0; i < tab.SizeDim[0]; i++) {
				fprintf(fd, "\n*** Layer %d Code %s ***\n", i + 1, m_var[tab.ExplVarnr[0]].GetCode(i).c_str() );
				for (j = 0; j < tab.SizeDim[1]; j++) {
				  fprintf(fd, "   *** Layer %d *** Code %s\n\n", j + 1, m_var[tab.ExplVarnr[1]].GetCode(j).c_str());
          ShowTableLayer(fd, 2, 3, (i * tab.SizeDim[1] + j) * tab.SizeDim[2] * tab.SizeDim[3], tab);
				}
      }
		}
		break;
	}
	fclose(fd);
	return true;
}

// only for debug
int TauArgus::ShowTableLayerCell(char *str, double val, int ndec)
{ int c;

	if (val == 0) c = sprintf(str, "%12s ", "-");
	else          c = sprintf(str, "%12.*f ", ndec, val);
  return c;
}

int TauArgus::ShowTableLayerCell(char *str, long val)
{ int c;

	if (val == 0) c = sprintf(str, "%6s ", "-");
	else    	    c = sprintf(str, "%6ld ", val);
  return c;
}


// only for debug
void TauArgus::ShowTableLayer(FILE *fd, int var1, int var2, int cellnr, CTable& tab)
{ int r, k, c, colwidth = 0;
  char str[200];

  // pointer to codes for row and column
  vector<string>  *sCol = m_var[tab.ExplVarnr[var2]].GetCodeList();
  vector<string>  *sRow = m_var[tab.ExplVarnr[var1]].GetCodeList();

  { // compute width column
		k = c = 0;
		CDataCell *dc = tab.GetCell(c + k);
		colwidth += ShowTableLayerCell(str, dc->GetResp(), m_var[tab.ResponseVarnr].nDec);
#ifndef COMPACT
		colwidth += ShowTableLayerCell(str, dc->GetFreq());
		colwidth += ShowTableLayerCell(str, dc->GetShadow(), m_var[tab.ShadowVarnr].nDec);
		if (tab.CostVarnr < 0) {
  		colwidth += ShowTableLayerCell(str, dc->GetCost(), 0);
		} else {
			colwidth += ShowTableLayerCell(str, dc->GetCost(tab.Lambda), m_var[tab.CostVarnr].nDec);
		}

		for (int i = 0; i < dc->nMaxScore; i++) {
			colwidth += ShowTableLayerCell(str, dc->MaxScore[i], m_var[tab.ShadowVarnr].nDec);
		}
#endif
		colwidth += 3;
  }

  // show code column variable
  fprintf(fd, "%14s", m_ValueTotal.c_str());
  for (k = 0; k < tab.SizeDim[var2]; k++) {
	  fprintf(fd, "%-*s", colwidth, (*sCol)[k].c_str());
	}
  fprintf(fd, "\n\n");

  for (r = 0; r < tab.SizeDim[var1]; r++) {
		c = cellnr + r * tab.SizeDim[var2];
		if (r == 0) {
		  fprintf(fd, "%s  => ", m_ValueTotal.c_str());
		} else {
		  fprintf(fd, "%-6s => ", (*sRow)[r].c_str() );
		}
		for (k = 0; k < tab.SizeDim[var2]; k++) {
			ASSERT(c + k >= 0 && c + k < tab.nCell);
			CDataCell *dc;
			dc = tab.GetCell(c + k);
			ShowTableLayerCell(str, dc->GetResp(), m_var[tab.ResponseVarnr].nDec); fprintf(fd, "%s", str);
#ifndef COMPACT
			ShowTableLayerCell(str, dc->GetFreq()); fprintf(fd, "%s", str);
			ShowTableLayerCell(str, dc->GetShadow(), m_var[tab.ShadowVarnr].nDec); fprintf(fd, "%s", str);
			if (tab.CostVarnr < 0) {
  			ShowTableLayerCell(str, dc->GetCost(tab.Lambda), 0); fprintf(fd, "%s", str);
			} else {
				ShowTableLayerCell(str, dc->GetCost(tab.Lambda), m_var[tab.CostVarnr].nDec); fprintf(fd, "%s", str);
			}
			for (int i = 0; i < dc->nMaxScore; i++) {
				ShowTableLayerCell(str, dc->MaxScore[i], m_var[tab.ShadowVarnr].nDec); fprintf(fd, "%s", str);
			}
#endif
			switch (dc->GetStatus() ) {
			case CS_SAFE:
        fprintf(fd, " | ");
				break;
			case CS_EMPTY_NONSTRUCTURAL:
				fprintf(fd, " | ");
				break;
			case CS_EMPTY:
        fprintf(fd, " | ");
				break;
			case CS_UNSAFE_FREQ:
				fprintf(fd, "*| ");
				break;
		  case CS_UNSAFE_RULE:
				fprintf(fd, "*| ");
				break;
		   case CS_UNSAFE_PEEP:
				fprintf(fd, "*| ");
				break;
			case CS_UNSAFE_ZERO:
				fprintf(fd, "*| ");
				break;
		  case CS_UNSAFE_SINGLETON:
				fprintf(fd, "*| ");
				break;
			case CS_PROTECT_MANUAL:
        fprintf(fd, "P| ");
				break;
			case CS_SECONDARY_UNSAFE:
        fprintf(fd, "+| ");
				break;
			default:
        fprintf(fd, "?| ");
				break;
      }
    }
		fprintf(fd, "\n");
	}
}
//////////////////////////////////////////
// end debug function///////////////////////
//////////////////////////////////////////

// checks if the table is additive
bool TauArgus::IsTable (CTable *tab)
{
        long DimNr[MAXDIM];
	bool IsGoodTable = true;

	for (long d = 0; d < tab->nDim; d++) {
		//WriteRange(fd, tab, var, d, DimNr, 0, WithBogus, tdp);
		//long DimNr[MAXDIM];
		TestTable(tab, d, DimNr, 0, &(IsGoodTable));
		if (!IsGoodTable) {
				return false;
		}
	}
	return true;
}

// to find subtotals from basal cells
void TauArgus::AdjustTable(CTable *tab)
{
	for (long d = 0; d < tab->nDim; d++) {
		long DimNr[MAXDIM];
		AdjustNonBasalCells(tab,d,DimNr,0);
	}
        //Why is this needed? Does nothing???
        //for (int i = 0; i < tab->nCell; i++) {
	//	CDataCell *dctemp = tab->GetCell(i);
	//}
}

// Is good table is false if the table is not additive
void TauArgus::TestTable(CTable *tab, long TargetDim, long *DimNr, long niv, bool *IsGoodTable)
{
	vector<unsigned int> Children;
	CDataCell *dc;
	double sum,test;

	if (niv == tab->nDim) {
		CVariable *v = &(m_var[tab->ExplVarnr[TargetDim]]);
		int nCode = v->GetnCode();
		// get hierarchical totals and compare them with
		// sum of basal cells
		if (v->IsHierarchical) {
			int i, j, k;

			for (i = 0, k = 0; i < nCode; i++) {
				sum =0; test = 0;
				int n = GetChildren(*v,i,Children);

				if (n > 0) {
					DimNr[TargetDim] = k;
					sum = 0;
					dc = tab->GetCell(DimNr);
					test = dc->GetResp();
					for (j = 0; j < n; j++) {
						long RealCode = Children[j];
  						DimNr[TargetDim] = RealCode;
						dc = tab-> GetCell(DimNr);
						sum = sum + dc->GetResp();
					}

					if (!DBL_EQ(sum,test))	{
					    *IsGoodTable = false;
					}
				}
				k++;
			}

		}
		else {  // not hierarchical
			DimNr[TargetDim] = 0;
			sum = 0;
			dc= tab->GetCell(DimNr);
			test = dc->GetResp();
			for (int i = 1; i < nCode; i++) {
				DimNr[TargetDim] = i;
				dc = tab->GetCell(DimNr);
				sum = sum + dc->GetResp();
			}
			if (!DBL_EQ(sum,test))	{
				*IsGoodTable = false;
			}
		}

	}
	else {
		if (niv != TargetDim) {
			int i, j;
			CVariable *v = &(m_var[tab->ExplVarnr[niv]]);
			int nCode = v->GetnCode();
  			for (i = 0, j = 0; i < nCode; i++) {
				if (!v->IsHierarchical || !v->GethCode()[i].IsBogus) {
					DimNr[niv] = j++;
					TestTable(tab, TargetDim, DimNr, niv + 1, IsGoodTable);
				}
			}
		}
		else {
			TestTable(tab, TargetDim, DimNr, niv + 1, IsGoodTable);

		}
	}
}

// Get Children for a code
int TauArgus::GetChildren(CVariable &var, int CodeIndex, vector<unsigned int> &Children)
{
	int i, n = 0, LevelParent, LevelDesc, nCode;
	CCode *hCode = var.GethCode();
	ASSERT(var.IsHierarchical);

	Children.clear();
	nCode = var.GetnCode();
	// here I better do something
	if (hCode[CodeIndex].IsParent) {
		LevelParent = hCode[CodeIndex].Level;
		for (i = CodeIndex + 1; i<nCode; i++) {
			LevelDesc = hCode[i].Level;
			if (LevelDesc <= LevelParent) break;
			if (LevelDesc == LevelParent + 1) { // that's a child
				Children.push_back(i);
				n++;
			}
		}
	}
	// return number of children
	return n;
}

// similar to Is Table except here the totals are created instead of verified
void TauArgus::AdjustNonBasalCells(CTable *tab, long TargetDim, long *DimNr, long niv)
{
	vector<unsigned int> Children;
	CDataCell *dc;
	CDataCell *dctemp;//, *dcramya;
        CDataCell *addcell;
	long tempDimNr;
	double sum;//,test;


	if (niv == tab->nDim) {
		CVariable *v = &(m_var[tab->ExplVarnr[TargetDim]]);
		int nCode = v->GetnCode();
		if (v->IsHierarchical) {
			int i, j, k;

			for (i = 0, k = 0; i < nCode; i++) {
//				sum =0; test = 0;
				int n = GetChildren(*v,i,Children);

				if (n > 0) {
          // count number of bogus codes before code i

					DimNr[TargetDim] = k;
					sum = 0;

					tempDimNr = tab->GetCellNrFromIndices(DimNr);
					dctemp = tab->GetCell(DimNr);
					//test = dc->GetResp();
					//fprintf(fd, "0.0 %d : %d (-1) ", n + 1, GetCellNrFromIndices(tab->nDim, DimNr, tdp) );
					sum = 0;
					addcell = new CDataCell(tab->NumberofMaxScoreCell,
						tab->NumberofMaxScoreHolding, tab->ApplyHolding, tab->ApplyWeight );
					addcell->SetStatus(0);
					for (j = 0; j < n; j++) {
						long RealCode = Children[j];
  						DimNr[TargetDim] = RealCode;
    					//fprintf(fd, "%d (1) ", GetCellNrFromIndices(tab->nDim, DimNr, tdp));
						dc = tab-> GetCell(DimNr);
						sum = sum + dc->GetResp();
						*addcell += *dc;
					}
					if (!dctemp->Compare(*addcell))	{

					//dctemp = addcell; //maybe I need a way to set equality
					//delete[] dctemp;
						if (!dctemp->Compare(*(tab->GetCell(tab->nCell))))	{
							delete dctemp;
						}
						tab->CellPtr[tempDimNr] = addcell;
					}
					else {
						delete addcell;
					}
				}
				k++;
			}
			//now do the check

		}
		else {  // not hierarchical
			DimNr[TargetDim] = 0;
			sum = 0;
			tempDimNr = tab->GetCellNrFromIndices(DimNr);
			dctemp = tab->GetCell(DimNr);
			//test = dctemp->GetResp(); // Is not used ??? PWOF 20170127
			//fprintf(fd, "0.0 %d : %d (-1) ", nCode, GetCellNrFromIndices(tab->nDim, DimNr, tdp) );
			addcell = new CDataCell(tab->NumberofMaxScoreCell, tab->NumberofMaxScoreCell,
				tab->ApplyHolding, tab->ApplyWeight);
			addcell->SetStatus(0);
			for (int i = 1; i < nCode; i++) {
				DimNr[TargetDim] = i;
				//fprintf(fd, "%d (1) ", GetCellNrFromIndices(tab->nDim, DimNr, tdp) );
				dc = tab->GetCell(DimNr);
				sum = sum + dc->GetResp();
				*addcell += *dc;
			}

			//if (test != sum) {
			//	*IsGoodTable = false;
			//}
			if (!dctemp->Compare(*addcell))	{
			//if (dctemp != addcell)	{
				//dctemp = addcell; //maybe I need a way to set equality
				//delete [] dctemp;
				if (!dctemp->Compare(*(tab->GetCell(tab->nCell))))	{
					delete dctemp;
				}
				tab->CellPtr[tempDimNr] = addcell;
			}
			else {

				delete addcell;
			}
                        // Does not do anything ??? PWOF 20170127
			//dcramya = tab->GetCell(tempDimNr);
		}

	}
	else {
		if (niv != TargetDim) {
			int i, j;
			CVariable *v = &(m_var[tab->ExplVarnr[niv]]);
			int nCode = v->GetnCode();
  			for (i = 0, j = 0; i < nCode; i++) {
				if (!v->IsHierarchical || !v->GethCode()[i].IsBogus) {
					//) {
					DimNr[niv] = j++;
					//WriteRange(fd, tab, var, TargetDim, DimNr, niv + 1, WithBogus, tdp);
					AdjustNonBasalCells(tab, TargetDim, DimNr, niv + 1);
				}
			}
		}
		else {
			AdjustNonBasalCells(tab, TargetDim, DimNr, niv + 1);

		}
	}
}

// read variables from a free formated file
bool TauArgus::ReadVariablesFreeFormat(char *Str, vector<char *> &VarCodes)
{
	VarCodes.resize(m_maxBPos + 1);
	if (InFileSeperator == " " || InFileSeperator.length() != 1)
		return false;
        
	char separator = InFileSeperator[0];
	char *startpos = Str;
	char *endpos;
	for (int varIndex = 0; varIndex <= m_maxBPos; varIndex++) {
                //CVariable *var = &(m_var[varIndex]); // Not used ???? PWOF 20170127
		endpos = strchr(startpos, separator);
		if (endpos == NULL) {
			if (varIndex != m_maxBPos)
				return false; // not enough variables
		} 
		VarCodes[varIndex] = startpos;
		if (endpos != NULL) {
			*endpos = '\0';
			startpos = endpos + 1;
		}
	}
	return true;
}

// returns depth of hierarchical tree.
long TauArgus::MaxDiepteVanSpanVariablen(CTable *tab)
{
	long maximum = 0;
	for (long i = 0; i < tab->nDim; i++)	{
		CVariable *var = &(m_var[tab->ExplVarnr[i]]);
		long maxvardiepte = var->GetDepthOfHerarchicalBoom(false); // false = original coding, no recoding
		maximum = max(maximum, maxvardiepte);
	}
	return maximum;
}

// Test the subcode list (Networking. This is just a test program and can be converted to
// a real interface.
bool TauArgus::TestSubCodeList()
{
	for (long i = 0; i < m_ntab; i++) {
		CTable *tab = &(m_tab[i]);
		for (long j = 0; j < tab->nDim; j++) {
			CVariable *var = &(m_var[tab->ExplVarnr[j]]);
			if (!var->PrepareSubCodeList()) {
				return false;
			}
			//create subcodelist for table
			if (!var->FillSubCodeList()) {
				return false;
			}
			// just to see sub codes are filled
			/* Does not do anything. Will crash if sub codes are not filled?  PWOF 20170127
                         for (long l = 0; l<var->NumSubCodes; l++) { 
                        	for (long k = 0; k < var->m_SubCodes[l].NumberOfSubCodes(); k++) {
					long test = var->m_SubCodes[l].GetSubCodeIndex(k);
				}
			}*/
		}
	}
	return true;
}

// These functions are used for the networking problem.
//this is used for findind all subtables of a given table.
// i.e. to remove all hierarchical structures in it.
// This functions below have not yet been tested and can contain some bugs.
// find number of subtables
long TauArgus::NumberOfSubTables(long IndexTable)
{
	long Num=1;

	CTable *tab = &(m_tab[IndexTable]);
	for (long i = 0; i < tab->nDim; i++) {
		CVariable *var = &(m_var[tab->ExplVarnr[i]]);
		Num *= var->NumSubCodes;
	}
	return Num;
}

// Given a table and a subtable, Find out which Tuple the subtable is made of
// i.e. which subcode idices of the explanatory variable the subtale is made of of
bool TauArgus::SubTableTupleForSubTable(long TableIndex, long SubTableIndex,
													 long *SubCodeIndices)
{
	long c = SubTableIndex;

	CTable *tab = &(m_tab[TableIndex]);
	for (long i = tab->nDim-1; i >= 0; i--)	{
		CVariable *var = &(m_var[tab->ExplVarnr[i]]);
		SubCodeIndices[i] = c%var->NumSubCodes;
		c	-= SubCodeIndices[i];
		c /= var->NumSubCodes;
	}
	return (c != 0);
}

// given a table and a cell (in araay of dimensions) find all subtables where this cell occurs
// thus internally and in the marginals
long TauArgus::WriteCellInTempFile(long UnsafeCellNum, long TableIndex, long CellNum, FILE *fdtemp, double MaxScale) //long *CellDimension)
{
	long i,j,k,l;
//	long SubTableIndex = -1; // Not used PWOF 20170127
	long nCellSubTable;
	CTable *tab = &(m_tab[TableIndex]);
//	bool found = false; // Not used PWOF 20170127
	long SubTableCellNum;
	long teller = UnsafeCellNum;
	CVariable *var;
	CDataCell *dc;
	long nDec = 8;
	double UPL, LPL;
	CVariable *var1 = &(m_var[tab->ExplVarnr[tab->nDim-1]]);
	long NumberOfColumns = var1->m_SubCodes[0].NumberOfSubCodes();
	long *SubTableTuple = new long [tab->nDim];
	long *TableCellIndex = new long [tab->nDim];
	long *MarginalCellIndex = new long [tab->nDim];
	long * SubTableCellIndex = new long [tab->nDim];
	long marginal;
	long nsubtable = NumberOfSubTables(TableIndex);
	for (i = 0; i<nsubtable; i++)	{
		//Find a SubTableTuple
		SubTableTupleForSubTable(TableIndex,i,SubTableTuple);
		nCellSubTable = FindNumberOfElementsInSubTable(SubTableTuple, TableIndex);
		marginal =0;
		for (j = 0; j <nCellSubTable; j++)	{
			// Find out for each cell the cell index

			FindCellIndexForSubTable(TableCellIndex,TableIndex,SubTableTuple,j,SubTableCellIndex);
			SubTableCellNum = tab->GetCellNrFromIndices(TableCellIndex);
			marginal++;
			if (SubTableCellNum == CellNum)	{
				//then print the stuff and go on to see if it
				// occurs in other tables
				dc = tab->GetCell(CellNum);
				fprintf(fdtemp, "%ld    %ld    " , teller, i);
				fprintf(fdtemp, "%ld    %ld    ", SubTableCellIndex[0], SubTableCellIndex[1]);
				UPL = dc->GetUpperProtectionLevel();
				LPL = dc->GetLowerProtectionLevel();
				if (MaxScale > 0){
				 if ( LPL > MaxScale * dc->GetResp()) LPL = MaxScale * dc->GetResp();
				 if ( UPL > MaxScale * dc->GetResp()) UPL = MaxScale * dc->GetResp();
				}
//				fprintf (fdtemp, "%.*f    %.*f\n", nDec, dc->GetLowerProtectionLevel(),nDec, dc->GetUpperProtectionLevel());
				fprintf (fdtemp, "%.*f    %.*f\n", (int)nDec, LPL, (int)nDec, UPL);
				teller++;
				break;
			}
			// (marginalColumn)
			if	(marginal == NumberOfColumns)	{
				for (k=0; k<tab->nDim; k++)	{
					var = &(m_var[tab->ExplVarnr[k]]);
					if (k == tab->nDim-1)	{
						MarginalCellIndex[k] = NumberOfColumns;
						TableCellIndex[k] = var->m_SubCodes[SubTableTuple[k]].GetParentIndex();
					}
					else {
						MarginalCellIndex[k] = SubTableCellIndex[k];
						TableCellIndex[k] = var->m_SubCodes[SubTableTuple[k]].GetSubCodeIndex(SubTableCellIndex[k]);
					}
				}
				SubTableCellNum = tab->GetCellNrFromIndices(TableCellIndex);
				marginal = 0;
				if (SubTableCellNum == CellNum)	{
					dc = tab->GetCell(CellNum);
					fprintf(fdtemp, "%ld    %ld    " , teller, i);
					fprintf(fdtemp, "%ld    %ld    ", MarginalCellIndex[0], MarginalCellIndex[1]);
					UPL = dc->GetUpperProtectionLevel();
					LPL = dc->GetLowerProtectionLevel();
					if ( MaxScale > 0) {
					 if ( LPL > MaxScale * dc->GetResp()) LPL = MaxScale * dc->GetResp();
					 if ( UPL > MaxScale * dc->GetResp()) UPL = MaxScale * dc->GetResp();
					}
					//fprintf (fdtemp, "%.*f    %.*f\n", nDec, dc->GetLowerProtectionLevel(),nDec, dc->GetUpperProtectionLevel());
					fprintf (fdtemp, "%.*f    %.*f\n", (int)nDec, LPL, (int)nDec, UPL);
					teller++;
					break;
				}

			}
		}

		// Now the last Rows
		for (k =0; k <NumberOfColumns; k++)	{
			for (l =0; l <tab->nDim; l++)	{
				var = &(m_var[tab->ExplVarnr[l]]);
				if (l == tab->nDim-1)	 {
					MarginalCellIndex[l] = k;
					TableCellIndex[l] = var->m_SubCodes[SubTableTuple[l]].GetSubCodeIndex(k);
				}
				else	{
					MarginalCellIndex[l] = var->m_SubCodes[SubTableTuple[l]].NumberOfSubCodes();
					TableCellIndex[l] = var->m_SubCodes[SubTableTuple[l]].GetParentIndex();
				}
			}
			SubTableCellNum = tab->GetCellNrFromIndices(TableCellIndex);
			marginal = 0;
			if (SubTableCellNum == CellNum)	{
				//then print the stuff and go on to see if it
				// occurs in other tables
					dc = tab->GetCell(CellNum);
					fprintf(fdtemp, "%ld    %ld    " , teller, i);
					fprintf(fdtemp, "%ld    %ld    ", MarginalCellIndex[0], MarginalCellIndex[1]);
					UPL = dc->GetUpperProtectionLevel();
					LPL = dc->GetLowerProtectionLevel();
					if (MaxScale > 0 ) {
					 if ( LPL > MaxScale * dc->GetResp()) LPL = MaxScale * dc->GetResp();
					 if ( UPL > MaxScale * dc->GetResp()) UPL = MaxScale * dc->GetResp();
					}
//					fprintf (fdtemp, "%.*f    %.*f\n", nDec, dc->GetLowerProtectionLevel(),nDec, dc->GetUpperProtectionLevel());
					fprintf (fdtemp, "%.*f    %.*f\n", (int)nDec, LPL, (int)nDec, UPL);
	  				teller++;
				//break;
			}
		}
		// Now the last Cell
		for (l = 0; l <tab->nDim; l++)	{
			var = &(m_var[tab->ExplVarnr[l]]);
			MarginalCellIndex[l] = var->m_SubCodes[SubTableTuple[l]].NumberOfSubCodes();
			TableCellIndex[l] = var->m_SubCodes[SubTableTuple[l]].GetParentIndex();
		}
		SubTableCellNum = tab->GetCellNrFromIndices(TableCellIndex);
		marginal = 0;
		if (SubTableCellNum == CellNum)	{
			//then print the stuff and go on to see if it
			// occurs in other tables
			dc = tab->GetCell(CellNum);
			fprintf(fdtemp, "%ld    %ld    " , teller, i);
			fprintf(fdtemp, "%ld    %ld    ", MarginalCellIndex[0], MarginalCellIndex[1]);
			UPL = dc->GetUpperProtectionLevel();
			LPL = dc->GetLowerProtectionLevel();
			if (MaxScale > 0) {
			 if ( LPL > MaxScale * dc->GetResp()) LPL = MaxScale * dc->GetResp();
			 if ( UPL > MaxScale * dc->GetResp()) UPL = MaxScale * dc->GetResp();
			}
//			fprintf (fdtemp, "%.*f    %.*f\n", nDec, dc->GetLowerProtectionLevel(),nDec, dc->GetUpperProtectionLevel());
			fprintf (fdtemp, "%.*f    %.*f\n", (int)nDec, LPL, (int)nDec, UPL);

			teller++;
			//break;
		}

	}
	delete [] MarginalCellIndex;
	delete [] SubTableTuple;
	delete [] TableCellIndex;
	delete [] SubTableCellIndex;
	return teller;
}

// Compare two arrays and return if they are equal
bool TauArgus::ArrayCompare (long *arr1, long *arr2, long nDim)
{
	bool equal = true;
	for (long i = 0; i<nDim; i++)	{
		if (arr1[i] != arr2[i])	{
			equal = false;
			break;
		}
	}
	return equal;
}

// Given a subtable in tuples for example (0,1, .. 0). Is the zeroth subcodelist of m_tab{tableIndex],
// 1st subcodelist of 2nd Expl Variable in m_tab[Index] etc.
// return the number of cells
long TauArgus::FindNumberOfElementsInSubTable(long *SubTableTuple, long TableIndex)
{
	long Num = 1;
	CTable *tab =  &(m_tab[TableIndex]);

	for (long i = 0; i<tab->nDim; i++)	{
		CVariable *var = &(m_var[tab->ExplVarnr[i]]);
		Num *= var->m_SubCodes[SubTableTuple[i]].NumberOfSubCodes();
	}

	return Num;
}

// Now given a subtable tuple and a table, and a cell in the subtable, find the
// Index of this cell
bool TauArgus::FindCellIndexForSubTable(long *TableCellIndex,
														 long TableIndex, long *SubTableTuple,
														 long CellIndexInSubTable, long *SubTableCellIndex )
{
	long i;
	long c = CellIndexInSubTable;
	//long * SubTableTupleIndex;

	CTable *tab = &(m_tab[TableIndex]);
	//SubTableTupleIndex = new long [tab->nDim];

	for (i = tab->nDim-1; i >= 0; i--)	{
	/*	var = &(m_var[tab->ExplVarnr[i]]);
		SubCodeIndices[i] = c%var->NumSubCodes;
		c	-= SubCodeIndices[i];
		c /= var->NumSubCodes;
		*/
		CVariable *var = &(m_var[tab->ExplVarnr[i]]);
		SubTableCellIndex[i] = c%var->m_SubCodes[SubTableTuple[i]].NumberOfSubCodes();
		c -= SubTableCellIndex[i];
		c /= var->m_SubCodes[SubTableTuple[i]].NumberOfSubCodes();
	}
	// got the subtable cell index get dim nr in table
	for (i = 0; i<tab->nDim; i++)	{
		CVariable *var = &(m_var[tab->ExplVarnr[i]]);
		TableCellIndex[i] = var->m_SubCodes[SubTableTuple[i]].GetSubCodeIndex(SubTableCellIndex[i]);
	}


	//delete [] SubTableTupleIndex;
	return (c != 0);
}

// given a table and a cell (in araay of dimensions) find a subtable where this cell occurs
long TauArgus::SubTableForCellDimension(long TableIndex, long *CellDimension, long * SubTableTupleIndex)
{
	long i,j;
	long SubTableIndex = -1;
	CTable *tab = &(m_tab[TableIndex]);
	bool found = false;

	long *SubTableTuple = new long [tab->nDim];
	long *SubCodeList = new long [tab->nDim];
	long nsubtable = NumberOfSubTables(TableIndex);
	for (i = 0; i<nsubtable; i++)	{
		//Find a SubTableTuple
		SubTableTupleForSubTable(TableIndex,i,SubTableTuple);
		long nCellSubTable = FindNumberOfElementsInSubTable(SubTableTuple, TableIndex);
		for (j = 0; j <nCellSubTable; j++)	{
			// Find out for each cell the cell index
			FindCellIndexForSubTable(SubCodeList,TableIndex,SubTableTuple,j,SubTableTupleIndex);
			found = ArrayCompare(SubCodeList,CellDimension,tab->nDim);
			if (found)	{
				break;
			}
		}
		if (found)	{
			SubTableIndex = i;
			break;
		}
	}
	delete [] SubTableTuple;
	delete [] SubCodeList;
	return SubTableIndex;
}
// Given a cell in a table find out which subtable this cell ocuurs in
long TauArgus::FindSubTableForCell(long CellIndex, long TableIndex, long *SubTableTupleIndex)
{
	CTable *tab = &(m_tab[TableIndex]);
	long *dimnr = new long[tab->nDim];
	tab->GetIndicesFromCellNr(CellIndex,dimnr);
	long SubTableIndex = SubTableForCellDimension(TableIndex,dimnr,SubTableTupleIndex);
	delete [] dimnr;
	return SubTableIndex;
}

// This is is not generalized but can be with some minor changes
// To generalize this all the help functions are there above
bool TauArgus::WriteAllSubTablesInAMPL(FILE *fd, long tabind)
{
	long i, j, k, cellnr;
	long marginal = 0;

	long nDecResp = 8;
	CTable *tab = &(m_tab[tabind]);
	CVariable *var1 = &(m_var[tab->ExplVarnr[tab->nDim-1]]); // last variable at this point its 2
																// dim
	long NumberOfColumns = var1->m_SubCodes[0].NumberOfSubCodes();

	long *SubTableTuple = new long[tab->nDim];
	long *SubTableCellIndex = new long[tab->nDim];
	long *TableCellIndex = new long[tab->nDim];
//	dimnr = new long [tab->nDim];
	long *MarginalCellIndex = new long [tab->nDim];
	long NumSubTables = NumberOfSubTables(tabind);

	fprintf (fd, "%s\n", "#table values and weights 9indexed by (table,row,column)), marginals included");
	fprintf (fd, "%s\n", "param : a         weight   :=");
	for (i =0; i<NumSubTables; i++)	{
		SubTableTupleForSubTable(tabind,i,SubTableTuple);
		long NumSubTableCells = FindNumberOfElementsInSubTable(SubTableTuple,tabind);
		marginal = 0; //long sum = 0; Not used PWOF 20170127
		for (j= 0; j<NumSubTableCells; j++)	{
			// just print cell
			FindCellIndexForSubTable(TableCellIndex,tabind,SubTableTuple,j, SubTableCellIndex);
			cellnr = tab->GetCellNrFromIndices(TableCellIndex);
			CDataCell *dc = tab->GetCell(cellnr);
			fprintf (fd, "%ld    %ld    %ld    ",  i, SubTableCellIndex[0], SubTableCellIndex[1]);
			fprintf (fd, "%.*f    %.*f\n", (int)nDecResp, dc->GetResp(), (int)nDecResp, dc->GetCost(tab->Lambda));

			marginal++;
			// Last Column (marginal column)
			if (marginal == NumberOfColumns)	{

				for (k= 0; k<tab->nDim; k++)	{
					CVariable *var = &(m_var[tab->ExplVarnr[k]]);
					if (k == tab->nDim-1)	{
						MarginalCellIndex[k] = NumberOfColumns;
						TableCellIndex[k] = var->m_SubCodes[SubTableTuple[k]].GetParentIndex();
					}
					else	{
						MarginalCellIndex[k] = SubTableCellIndex[k];
						TableCellIndex[k] = var->m_SubCodes[SubTableTuple[k]].GetSubCodeIndex(SubTableCellIndex[k]);
					}
				}

				// This has to ber generalized
				// Now Print the marginal column cell
				cellnr = tab->GetCellNrFromIndices(TableCellIndex);
				dc = tab->GetCell(cellnr);
				// print table number etc
				fprintf (fd, "%ld    %ld    %ld    " , i, MarginalCellIndex[0], MarginalCellIndex[1]);
				fprintf(fd,"%.*f    %.*f\n",	(int)nDecResp, dc->GetResp(), (int)nDecResp, dc->GetCost(tab->Lambda));

				marginal = 0;
			}
		}

		// Now the last Rows which is the marginal row
		for (k = 0; k<NumberOfColumns; k++)	{
			for (long l= 0; l<tab->nDim; l++)	{
				CVariable *var = &(m_var[tab->ExplVarnr[l]]);
				if (l == tab->nDim-1)	{
					MarginalCellIndex[l]  = k;
					TableCellIndex[l] = var->m_SubCodes[SubTableTuple[l]].GetSubCodeIndex(k);
				}
				else	{
					//MarginalCellIndex[l] = var->m_SubCodes[SubTableTuple[l]]
					MarginalCellIndex[l] = var->m_SubCodes[SubTableTuple[l]].NumberOfSubCodes();	// No of Rows;
					TableCellIndex[l] = var->m_SubCodes[SubTableTuple[l]].GetParentIndex();
				}
			}
			cellnr = tab->GetCellNrFromIndices(TableCellIndex);
			CDataCell *dc = tab->GetCell(cellnr);
			// print table number etc
			fprintf (fd, "%ld    %ld    %ld    ", i, MarginalCellIndex[0], MarginalCellIndex[1]);

			fprintf(fd,"%.*f    %.*f\n",	(int)nDecResp, dc->GetResp(), (int)nDecResp, dc->GetCost(tab->Lambda));
		}
		//Now the last lot of the sub table i.e. the totaal generaal of the sub table
		for (long l= 0; l<tab->nDim; l++)	{
			CVariable *var = &(m_var[tab->ExplVarnr[l]]);
			MarginalCellIndex[l] = var->m_SubCodes[SubTableTuple[l]].NumberOfSubCodes();	// No of Rows +1
			TableCellIndex[l] = var->m_SubCodes[SubTableTuple[l]].GetParentIndex();
		}
		cellnr = tab->GetCellNrFromIndices(TableCellIndex);
		CDataCell *dc = tab->GetCell(cellnr);
		// print table number etc
		fprintf (fd, "%ld    %ld    %ld    ", i, MarginalCellIndex[0], MarginalCellIndex[1]);
		fprintf(fd,"%.*f    %.*f\n", (int)nDecResp, dc->GetResp(), (int)nDecResp, dc->GetCost(tab->Lambda));


	}
	fprintf (fd, "%s\n", ";");
	delete [] MarginalCellIndex;
	delete [] TableCellIndex;
	delete [] SubTableTuple;
	delete [] SubTableCellIndex;
	return true;
}

// This is is not generalized but can be with some minor changes
bool TauArgus::WriteTableSequenceHierarchyInAMPL(FILE *fd, long tabind,
																	 long varind)
{
	long j, k;
	long RowNum;

	CTable *tab = &(m_tab[tabind]);

	// since the first explanatory variable is hierarchical
	CVariable *var = &(m_var[tab->ExplVarnr[varind]]);
	if (!var->IsHierarchical) {
		return false;
	}

	long *SubTableTuple = new long [tab->nDim];
	long nsubtab = NumberOfSubTables(tabind);
	bool foundRowNum = false;
	bool foundParent = false;
	long param = 0;

	//Print the row number the table and the table where it is split into
	fprintf(fd, "%s\n", "#(T-1) lines with hierarchical info: row rh,of table th decomposed in table tdh");
	fprintf(fd, "%s\n", " param:     rh     th     tdh :=");
	for (long i = 1; i < var->nCode; i++)	{
		string scode = var->sCode[i];
		long CodeIndex = i;
		// Is a parent
		if (var->FindNumberOfChildren(CodeIndex) > 0)	{
			// find out which table this parent is in
			long SubTableIndex;
			for (j = 0; j < nsubtab; j++)	{
				SubTableTupleForSubTable(tabind,j,SubTableTuple);
				long SubCodeIndex = SubTableTuple[varind];
				CSubCodeList* osubcodelist = &(var->m_SubCodes[SubCodeIndex]);
				//Which row the parent occurs in the table
				RowNum = osubcodelist->IsInSubCodes(scode);
				if (RowNum >= 0)	{
					SubTableIndex = j;
					foundRowNum = true;
					break;
				}
			}
			if (foundRowNum)	{
				// Find out which table the parent aboves children is in.
				// that is the codelist where parent index is the same as the index
				// above
				long ChildSubTableIndex;
				for (k = 0; k < nsubtab; k++)	{
					SubTableTupleForSubTable(tabind,k,SubTableTuple);
					long SubCodeIndex = SubTableTuple[varind];
					CSubCodeList* osubcodelist = &(var->m_SubCodes[SubCodeIndex]);
					if (scode == osubcodelist->GetParentCode())	{
						foundParent = true;
						ChildSubTableIndex = k;
						break;
					}
				}
				if (foundParent)	{
					//Print the stuff
					fprintf (fd, "%ld    %ld    %ld   %ld\n", param,RowNum, SubTableIndex, ChildSubTableIndex);
					param ++;
					foundParent = false;
					foundRowNum = false;

				}
				if ((k == nsubtab) || (j == nsubtab))	{
					delete [] SubTableTuple;
					return false;
				}
			}
		}
	}
	fprintf (fd,"%s\n", ";");
	delete [] SubTableTuple;
	return true;
}

// Write Hierarchical Table in AMPL format
// This can only be used if the first dimension of the table
// is hierarchical and the second dimension is not.
// The table should be 2 dimensional.
bool TauArgus::WriteHierTableInAMPL(FILE *fd, long tabind, const string &TempDir, double MaxScale)
{
	FILE *fdtempw = fopen((TempDir +"/TMPAMPLU").c_str(), "w");
	if (fd == 0)	{
		return false;
	}
	CTable *tab = &(m_tab[tabind]);
	long *DimNr = new long [tab->nDim];
	long *RowColIndex = new long [tab->nDim];
	long i;
	//long nDec = 8;
	fprintf(fd,"%s\n", "# AMPL generated By TauArgus");
	fprintf(fd,"%s\n", "#");
	// The number of sub tables
	fprintf(fd, "%s\n", "#T = number of 2D tables");
	long nsubtab = NumberOfSubTables(tabind);
	fprintf (fd, "%s%ld%s\n", "param T := ", nsubtab, ";");

	// Print Number of Rows
	fprintf (fd, "%s\n", "#M = number of rows of each table (without marginal row)");
	fprintf (fd,"%s\n", "param M := ");
	CVariable *var0 = &(m_var[tab->ExplVarnr[0]]);
	CVariable *var1 = &(m_var[tab->ExplVarnr[1]]);
	//for (i = 0; i < var0->NumSubCodes; i++)	{
        for (unsigned long j = 0; j < var0->NumSubCodes; j++)	{
		fprintf(fd,"%ld                  %ld\n", j, var0->m_SubCodes[j].NumberOfSubCodes());
	}
	fprintf(fd,"%s\n", ";");

	// Print Number Of Columns
	fprintf (fd, "%s\n", "#N = number of columns common to each table (without marginal row)");
	fprintf (fd,"%s%ld%s\n", "param N := ", var1->m_SubCodes[0].NumberOfSubCodes(), ";");

	// Print Hierarchical info
	// since the first variable is hierarchical
	WriteTableSequenceHierarchyInAMPL(fd,tabind, 0);

	// Print Number of Unsafe Cells
	long nUnsafe = 0;
	for (i = 0; i < tab->nCell; i++) {
		CDataCell *dc = tab->GetCell(i);
		long status = dc->GetStatus();
		if ((status == CS_UNSAFE_RULE) || (status == CS_UNSAFE_PEEP) || (status == CS_UNSAFE_FREQ) ||
			(status == CS_UNSAFE_ZERO) || (status == CS_UNSAFE_SINGLETON) || (status == CS_UNSAFE_SINGLETON_MANUAL) ||
			(status == CS_UNSAFE_MANUAL) || (status == CS_PROTECT_MANUAL))	{
			nUnsafe = WriteCellInTempFile(nUnsafe,tabind, i, fdtempw,MaxScale);

		}
	}
	fclose(fdtempw);
	fprintf (fd, "%s\n", "#P = number of primary supression cells of each table;");
	fprintf (fd,"%s %ld %s\n", "param P := ", nUnsafe, ";");
	//and where the unsafe cells occur with LPL and UPL.
	fprintf (fd, "%s\n", "# for each primary : table, row, column, lower prot, and upper prot.");
	fprintf  (fd,"%s\n", "param : p_t    p_r    p_c    lpl    upl  :=");
	// now open the temp file
	FILE *fdtempr = fopen((TempDir +"/TMPAMPLU").c_str(), "r");
	if (fdtempr == 0)	{
		return false;
	}
	fseek( fdtempr, 0L, SEEK_SET );
	for (i=0; i<nUnsafe; i++)	{
		char str[MAXRECORDLENGTH];
		//int res = ReadMicroRecord(fdtempr, str);
                ReadMicroRecord(fdtempr, str);
		fprintf(fd,"%s\n",str);
	}
	fprintf(fd,"%s\n", ";");
	/*
	fprintf (fd, "%s\n", "#P = number of primary supression cells of each table;");
	fprintf (fd,"%s %d %s\n", "param P := ", nUnsafe, ";");


	//and where the unsafe cells occur with LPL and UPL.
	fprintf (fd, "%s\n", "# for each primary : table, row, column, lower prot, and upper prot.");
	fprintf  (fd,"%s\n", "param : p_t    p_r    p_c    lpl    upl  :=");
	nUnsafe = 0;
	for (i=0; i <tab->nCell; i++)	{
		dc = tab->GetCell(i);
		status = dc->GetStatus();
		if ((status == CS_UNSAFE_RULE) || (status == CS_UNSAFE_PEEP) || (status == CS_UNSAFE_FREQ) ||
			(status == CS_UNSAFE_ZERO) || (status == CS_UNSAFE_SINGLETON) || (status == CS_UNSAFE_SINGLETON_MANUAL) ||
			(status == CS_UNSAFE_MANUAL) || (status == CS_PROTECT_MANUAL))	{

			fprintf (fd," %d", nUnsafe);  //param
			nUnsafe ++;
		//	SubTableNum = FindSubTableForCell(i,tabind, RowColIndex);
			fprintf (fd,"    %d", SubTableNum);
			for (j = 0; j < tab ->nDim; j++)	{
				fprintf (fd,"   %d",  RowColIndex[j]);
			}
			fprintf (fd, "   %.*f        %.*f\n",nDec, dc->GetLowerProtectionLevel(),nDec, dc->GetUpperProtectionLevel());
		}
	}
	fprintf(fd,"%s\n", ";");
*/
	// Now print all subtables
	WriteAllSubTablesInAMPL(fd, tabind);

	delete [] DimNr;
	delete [] RowColIndex;
	return true;
}

bool TauArgus::testampl(long ind)
{
	string sTempDir = "E:/Temp";

	CTable *tab = &(m_tab[ind]);
	for (long j = 0; j < tab->nDim; j++) {
		CVariable *var = &(m_var[tab->ExplVarnr[j]]);
		if (!var->PrepareSubCodeList()) {
			return false;
		}
		//create subcodelist for table
		if (!var->FillSubCodeList()) {
			return false;
		}
		// just to see sub codes are filled
                /* Does not really do anything PWOF 20170127
		for (long l = 0; l < var->NumSubCodes; l++) {
			for (long k = 0; k < var->m_SubCodes[l].NumberOfSubCodes(); k++) {
				long test = var->m_SubCodes[l].GetSubCodeIndex(k);
			}
		}*/
	}

	FILE *fd = fopen ("E:/Temp/Hierampl.txt", "w");

	WriteHierTableInAMPL(fd, ind, sTempDir, 0.0);
	fclose(fd);
	return true;
}

/**
 * Determines the noise to be added in a frequency count table, according to the 
 * cell key method with probabilities in the p-table
 * @param TabNo         table in tableset to be protected
 * @param PTableFile    name of file containing information on p-table
 * @return              maximum amount of noise (absolute value)
 */

int TauArgus::SetCellKeyValuesFreq(long TabNo, const char* PTableFile, int *MinDiff, int *MaxDiff){
    CDataCell *dc;
    int RowNr, Diff;
    double minDiffWeighted=1e10, maxDiffWeighted=-1e10;
    PTable ptable;
    PTableRow row;
    PTableRow::iterator pos;
    
    if (TabNo < 0 || TabNo >= m_ntab) return -1;
    if (m_tab[TabNo].HasRecode) TabNo += m_ntab;

    if (!ptable.ReadFromFreqFile(PTableFile)) return -9;
    
    for (long i=0; i < m_tab[TabNo].nCell; i++){
        dc = m_tab[TabNo].GetCell(i);
        if (dc->GetStatus() != CS_EMPTY){
            if (dc->GetStatus() != CS_PROTECT_MANUAL){
                RowNr = (dc->GetNWResp() >= ptable.GetmaxNi()) ? ptable.GetmaxNi() : (int) dc->GetNWResp();
                row = ptable.GetData()[RowNr];
                Diff = 0;
                for (pos=row.begin();pos!=row.end();++pos){
                    if (dc->GetCellKey() < pos->second){
                        Diff = pos->first - RowNr;
                        break;
                    }
                }
                dc->SetCKMValue((double) (dc->GetNWResp() + Diff));
            }
            else{
                Diff = 0;
                dc->SetCKMValue((double) (dc->GetNWResp()));
            }
            if (m_tab[TabNo].ApplyWeight) {
                dc->SetCKMValue(dc->GetCKMValue()*dc->GetWeight()/dc->GetFreq());
                minDiffWeighted = std::min(minDiffWeighted,Diff*dc->GetWeight()/dc->GetFreq());
                maxDiffWeighted = std::max(maxDiffWeighted,Diff*dc->GetWeight()/dc->GetFreq());
            }
        }
        else dc->SetCKMValue(0); // Empty cell
    }
    
    if (!m_tab[TabNo].ApplyWeight){
        MinDiff[0] = ptable.GetminDiff();
        MaxDiff[0] = ptable.GetmaxDiff();
    }
    else{
        MinDiff[0] = (int) std::round(minDiffWeighted);
        MaxDiff[0] = (int) std::round(maxDiffWeighted);
    }
    return 1;
}

int TauArgus::SetCellKeyValuesCont(long TabNo, const char* PTableFileCont, const char* PTableFileSep, const char* CKMType, 
                                    int topK, bool IncludeZeros, bool Parity, bool Separation, double m1sqr, const char* Scaling, 
                                    double s0, double s1, double z_f, double q, double* epsilon, double muC){
    CDataCell *dc;
    int nDec;
    double z_s, E, muCused, di;
    double xj, Vj, cellKey, x, xdelta, X1help;
    double m_one = 0;
    std::map<int, PTableDRow> ptableL, ptableS;
    std::map<int, PTableDRow>::reverse_iterator ptablepos;
    PTableCont ptableLarge;
    PTableCont ptableSmall;
    PTableDRow row;
    PTableDRow::iterator pos;

    if (TabNo < 0 || TabNo >= m_ntab) return -1;
    if (m_tab[TabNo].HasRecode) TabNo += m_ntab;

    if (!ptableLarge.ReadFromFile(PTableFileCont)) return -90; // Need ptable for continuous CKM
    
    E = epsilon[0]*epsilon[0]; // Should be 1.0
    if (topK > 1){ // Need E for proportional flex function and threshold g1 in case of Separation and topK>=2
        for (int i=1; i<topK; i++){
            E += epsilon[i]*epsilon[i];
        }
    }
    
    if (Separation) { // If SEPARATION=N then z_s=0 
        if (!ptableSmall.ReadFromFile(PTableFileSep)) return -91; // If SEPARATION=Y then need additional ptable
        //Calculate m1sqr from ptable
        ptablepos = ptableSmall.GetData("all").rbegin();
        di = (double) ptablepos->first;
        for (pos=ptablepos->second.begin();pos!=ptablepos->second.end();++pos){
            m_one += (pos->first - di)*(pos->first - di)*pos->second[0];  // (j-i)^2 * pij
        }
        m_one = sqrt(m_one);
        z_s = m_one/(s1*sqrt(E));

        ptableS = ptableSmall.GetData("all"); // Will need additional ptable for small values
        if (ptableS.size()==0) return 92; // No small table with "all"
    }
    else{
        z_s = 0;
        m_one = 1;
    }
    
    nDec = m_var[m_tab[TabNo].CellKeyVarnr].nDec; // Number of decimals in recordkey

    //printf("E=%g m1=%g z_s=%g sigma0=%g sigma1=%g\n",E,m_one,z_s,s0,s1);
            
    // Loop through all cells of the table
    for (long i=0; i < m_tab[TabNo].nCell; i++){
        dc = m_tab[TabNo].GetCell(i);
        //printf("IncludeZeros=%d\n",(IncludeZeros ? 1 : 0));
        cellKey = IncludeZeros ? dc->GetCellKey() : dc->GetCellKeyNoZeros();
        
        if (dc->GetStatus() != CS_EMPTY){ // not empty
            if (dc->GetStatus() != CS_PROTECT_MANUAL){ // not protected
                if (Parity){
                    ptableL = (dc->GetFreq() % 2 == 0) ? ptableLarge.GetData("even") : ptableLarge.GetData("odd");
                    if (ptableL.size()==0) return -93; // No "even" or "odd" found
                }
                else{
                    ptableL = ptableLarge.GetData("all");
                    if (ptableL.size()==0) return -94; // No "all" found
                }
                // j = 1
                // First implementation: SAMEKEY = NO, i.e., also perturb cell key for j=1
                cellKey = ShiftFirstDigit(cellKey,nDec);

                // j = 1
                xj = GetXj(CKMType, 1, *dc, m_tab[TabNo].ApplyWeight);
                // Use muC > 0 only if unsafe cell and xj large enough
                // otherwise set muC = 0
                //printf("muC=%g becomes",muC);
                if (((dc->GetStatus() < CS_UNSAFE_RULE) || (dc->GetStatus() > CS_UNSAFE_MANUAL)) || (fabs(xj) < z_s)){ muCused = 0; }
                else {muCused = muC;}
                //printf("%g\n",muCused);
                x = dc->GetResp();
                xdelta = (fabs(xj) >= z_s) ? xj*flexfunction(fabs(xj),z_s,s0,s1,z_f,q) : 1.0;
                if (fabs(x) < fabs(xdelta)){
                    xdelta = x;
                    xj = x/s1;
                    if (fabs(xj) < z_s) xdelta = 1.0;
                }
                Vj = LookUpVinptable( (fabs(xj) > z_s)? ptableL : ptableS, fabs(x/xdelta), cellKey);

                X1help = mysign(x)*fabs(xdelta)*mysign(Vj)*(muCused + fabs(Vj));
                //printf("j=%d x=%9lf ck=%g xj=%g xdelta=%g |x/xdelta|=%g Vj=%g X1help=%g ",1,x,cellKey,xj,xdelta,fabs(x/xdelta),Vj,X1help);                
                
                if (Vj >= 0){ x = x + X1help; }
                else{
                    if (x<0){ x = x + min(X1help, -x); }
                    else{ x = x + max(X1help, -x); }
                }
                //printf("new x=%9lf\n",x);
                
                
                // j = 2, ..., topK
                // What if topK > number of contributions to the cell???
                for (int j=2; j<=topK; j++){ // Only in case topK >=2
                    xj = GetXj(CKMType, j, *dc, m_tab[TabNo].ApplyWeight);
                    if (fabs(xj) >= z_s){
                        xdelta = xj*epsilon[j-1]*flexfunction(fabs(xj),z_s,s0,s1,z_f,q);
                        if (fabs(x) < fabs(xdelta)){
                            xdelta = x;
                            xj = x/(epsilon[j-1]*s1);
                        }
                        if (fabs(xj) >= z_s){ // xj may have changed so need to check again
                            cellKey = ShiftFirstDigit(cellKey,nDec); // always do this for j>=2
                            Vj = LookUpVinptable(ptableL, fabs(x/xdelta), cellKey);
                            //printf("j=%d x=%9lf ck=%g xj=%g xdelta=%g |x/xdelta|=%g Vj=%g ",j,x,cellKey,xj,xdelta,fabs(x/xdelta),Vj);
                            x = x + mysign(x)*fabs(xdelta)*Vj; // never use muC for j>=2
                            //printf("new x=%9lf\n",x);
                        }// else add zero, i.e. do nothing
                    }// else add zero, i.e. do nothing
                    
                }
                dc->SetCKMValue(x);
            }
            else{ // if protected do nothing, i.e., original response value
                dc->SetCKMValue(dc->GetResp());
            }
        }
        else{ // if empty cell stays empty, i.e., value 0
            dc->SetCKMValue(0); 
        }
    }    
    
    return 1;
}

// nDec = number of decimals for the shift
// Assumes 0 <= key < 1
double TauArgus::ShiftFirstDigit(double key, int nDec){
    char buffer[64];
    snprintf(buffer,64,"%*.*f", nDec+2, nDec, key);
    std::string cstr(buffer);
    cstr = "0." + cstr.substr(3)+cstr.substr(2,1);
    return atof(cstr.c_str());
}

// x: value to evaluate the flex function
// g1: lower bound to define "small" = m_1/(sigma_1 * E)
// s0, s1: sigma0 and sigma1, scaled for j-th largest observation
// xstar: threshold to define "large" 
double TauArgus::flexfunction(double z, double z_s, double s0, double s1, double z_f, double q){
    double result;
    if (z >= z_f){
        result = s0 * (1.0 + ((s1*z - s0*z_f)/(s0*z_f))*pow(((2*z_f)/(z+z_f)),q));
    }
    else{
        if (z > z_s){
            result = s1;
        }
        else{
            result = 0.0;
        }
    }
    return result;
}

// Find perturbation value V for value z
// Interpolate between z_lower < z < z_upper if needed
double TauArgus::LookUpVinptable(std::map<int,PTableDRow> ptable, double z, double RKey){
    double result = 0, Diff0 = 0, Diff1 = 0;
    double a0, a1, lambda;
    std::map<int,PTableDRow>::iterator row1, row0;
    PTableDRow::iterator PTRow;
    
    // ptable is sorted in the standard way on the keys, 
    // so first element has smallest key, last element has largest key
    double Jmin = (double) ptable.begin()->first; // Should be 0
    double Jmax = (double) ptable.rbegin()->first; // rbegin() points to last item, end() points to past-the-end element
    
    if ((Jmin <= z) && (z < Jmax)){
        row1 = ptable.upper_bound(z);
        row0 = std::prev(row1);
        a0 = (double) row0->first;
        a1 = (double) row1->first;
        lambda = (z - a0)/(a1-a0); // By construction a0 <= z < a1
        
        for (PTRow=row0->second.begin();PTRow!=row0->second.end();++PTRow){
            if (RKey < PTRow->second[1]){
                Diff0 = (double) PTRow->first - (double) row0->first;
                break;
            }
        }
        if (fabs(a0-z)>1E-8){ // Only needed if a0 != z, in practice if (a0 - z) > 1E-8
            for (PTRow=row1->second.begin();PTRow!=row1->second.end();++PTRow){
                if (RKey < PTRow->second[1]){
                    Diff1 = (double) PTRow->first - (double) row1->first;
                    break;
                }
            }
        }
        result = (1.0-lambda)*Diff0 + lambda*Diff1;
    }
    
    if (z >= Jmax){ // take difference from distribution of largest key in ptable
        for (PTRow=ptable.rbegin()->second.begin(); PTRow!=ptable.rbegin()->second.end(); ++PTRow){
            if (RKey < PTRow->second[1]) {
                result = (double) PTRow->first - (double) ptable.rbegin()->first;
                break;
            }
        }
    }
    
    return result;
}

double TauArgus::GetXj(const char* CKMType, int j, CDataCell &dc, bool WeightApplied){
    ASSERT (dc.GetStatus() != CS_EMPTY);
    
    if (strcmp(CKMType,"T")==0){
        double wj = WeightApplied ? dc.MaxScoreWeightCell[j-1] : 1.0;
        return  wj*dc.MaxScoreCell[j-1]; // j = 1, ..., topK
    }
    if (strcmp(CKMType,"M")==0){
        return (WeightApplied ? dc.GetResp()/dc.GetWeight() : dc.GetResp()/dc.GetFreq());
    }
    if (strcmp(CKMType,"D")==0){
        return (dc.MaxScoreCell[0] - dc.MinScoreCell);
    }
    if (strcmp(CKMType,"V")==0){
        return dc.GetResp();
    }

    return -1E42; // Should not happen
}