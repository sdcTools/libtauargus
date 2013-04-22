#include "stdafx.h"
#include "TauArgus.h"

#include "Globals.h"
#include "float.h"
#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//extern int CurrentHoldingNr;

/////////////////////////////////////////////////////////////////////////////
// TauArgus

#ifdef _DEBUG
	static int DEBUGprintf(char *fmt, ...)
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
	inline static int DEBUGprintf(char *fmt, ...) { return 0; }
#endif

void TauArgus::SetProgressListener(IProgressListener* ProgressListener)
{
	m_ProgressListener = ProgressListener;
}

void TauArgus::FireUpdateProgress(int Perc)
{
	DEBUGprintf("m_ProgressListener->UpdateProgress(Perc)\n");
	if (m_ProgressListener != NULL) {
		DEBUGprintf("%p-%p\n", m_ProgressListener, m_ProgressListener->UpdateProgress);
		m_ProgressListener->UpdateProgress(Perc);
	}
}

// cells that are set as secondary unsafe to be undone
STDMETHODIMP TauArgus::UndoSecondarySuppress(long TableIndex, long SortSuppress,
																VARIANT_BOOL *pVal)
{
	int t = TableIndex - 1;
   CTable *tab;

   if (t < 0 || t >= m_ntab) {
 		*pVal = VARIANT_FALSE;
 		return S_OK;
 	}
	if ((SortSuppress < 1) || (SortSuppress > 3)) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

   tab = GetTable(t);
   tab->UndoSecondarySuppress(SortSuppress);

 	*pVal =  VARIANT_TRUE;


 	return S_OK;

}

// Set number of Variables
bool STDMETHODCALLTYPE TauArgus::SetNumberVar(long nVar)
{
	#ifdef _DEBUG
		DEBUGprintf("SetNumberVar(%ld)\n", nVar);
	#endif
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
bool STDMETHODCALLTYPE TauArgus::SetNumberTab(long nTab)
{
	#ifdef _DEBUG
		DEBUGprintf("SetNumberTab(%ld)\n", nTab);
	#endif
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
bool STDMETHODCALLTYPE TauArgus::ComputeTables(long *ErrorCode, long *TableIndex)
{
	DEBUGprintf("ComputeTables\n");
	
	// long MemSizeAll = 0, MemSizeTable;
	int i;
	FILE *fd;
	UCHAR str[MAXRECORDLENGTH];



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
	fd = fopen(m_fname, "r");
	if (fd == 0) {
		*ErrorCode = FILENOTFOUND;
		return false;
	}


	int recnr = 0;
	while (!feof(fd) ) {
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


#ifdef _DEBUGG
{
	int i;
	for (i = 0; i < m_ntab; i++) {
		CString fname;
		fname.Format("tab%02d.txt", i);
		ShowTable((LPCTSTR) fname, m_tab[i]);
	}
}
#endif // _DEBUG

	for (i = 0; i < m_nvar; i++) {
		if (m_var[i].IsHierarchical && m_var[i].nDigitSplit == 0) {
			// empty no longer needed arrays
			m_var[i].hLevel.RemoveAll();
			m_var[i].hLevelBasic.RemoveAll();
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

	// RecodeStrings are changed to CString. Check this out
	CString sRecodeString;
	sRecodeString = RecodeString;
	int i, v = VarIndex, oke, maxwidth = 0;
	*ErrorType = *ErrorLine = *ErrorPos = -1;
	CString temp, Missing1, Missing2;

	Missing1 = eMissing1;
	Missing2 = eMissing2;

	// for warnings
	m_nOverlap = 0;
	m_nUntouched = 0;
	m_nNoSense = 0;

	m_WarningRecode.Empty();

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
	oke = ParseRecodeString(v, sRecodeString, ErrorType, ErrorLine, ErrorPos, CHECK);
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
	if (Missing1.IsEmpty() && !Missing2.IsEmpty()) {
		Missing1 = Missing2;
	}

	// no missings for recode specified? Take the missing(s) of the source
	if (Missing1.IsEmpty() && Missing2.IsEmpty()) {
		Missing1 = m_var[v].Missing1;
		if (m_var[v].nMissing == 2) {
			Missing2, m_var[v].Missing2;
		}
		else {
			Missing2 = Missing1;
		}
	}

	m_var[v].Recode.sCode.RemoveAll();
	m_var[v].Recode.nCode = 0;
	m_var[v].Recode.CodeWidth = 0;
	m_var[v].Recode.nMissing = 0;

	m_var[v].Recode.sCode.Add(""); // for Total

	// another time, now compute list of dest codes
	ParseRecodeString(v, sRecodeString, ErrorType, ErrorLine, ErrorPos, DESTCODE);
	if (m_var[v].Recode.sCode.GetSize() < 2) {
		*ErrorType = E_EMPTYSPEC;
		*ErrorLine = 1;
		*ErrorPos = 1;
		return false;
	}
	// sort list of dest codes, still without missing values (coming soon)
	QuickSortStringArray(m_var[v].Recode.sCode, 0, m_var[v].Recode.sCode.GetSize() - 1);


	// now the number of codes is known, but not the not mentioned ones
	// m_var[v].Recode.nCode = m_var[v].Recode.sCode.GetSize();

	// again, now compute dest codes and link between dest and src
	oke = ParseRecodeString(v, sRecodeString, ErrorType, ErrorLine, ErrorPos, SRCCODE);
	if (!oke) {
		return false; // missing to valid codes, a terrible shame
	}

	// compute untouched codes, add them to the recode codelist
	m_nUntouched = 0;
	for (i = 0; i < m_var[v].nCode - m_var[v].nMissing; i++) {
		if (m_var[v].Recode.DestCode[i] == -1) { // not touched
			m_nUntouched++;
			AddRecode(v, (LPCTSTR) m_var[v].sCode[i]);
		}
	}

	// make all recode codes same width
	maxwidth = MakeRecodelistEqualWidth(v, (LPCTSTR) Missing1, (LPCTSTR) Missing2);

	// remove the missing codes,
	{
		if (!((nMissing == 0) && (m_var[v].nMissing ==0)))	{
			int n;
			bool IsMissing;
			CString mis = Missing1;
			AddSpacesBefore(mis, maxwidth);
			if (n = BinSearchStringArray(m_var[v].Recode.sCode, mis, 0, IsMissing), n >= 0) {
				m_var[v].Recode.sCode.RemoveAt(n);
			}
			mis = Missing2;
			AddSpacesBefore(mis, maxwidth);
			if (n = BinSearchStringArray(m_var[v].Recode.sCode, mis, 0, IsMissing), n >= 0) {
				m_var[v].Recode.sCode.RemoveAt(n);
			}
		}
	}

	// sort list of dest codes, still without missing values (coming soon)
	QuickSortStringArray(m_var[v].Recode.sCode, 0, m_var[v].Recode.sCode.GetSize() - 1);

	// ADD MISSING1 AND -2
	// both empty impossible, see start of function
	// swap missings if missing1 empty

	if (!((nMissing == 0) && (m_var[v].nMissing ==0)))	{

		if (Missing1.IsEmpty() && Missing2.IsEmpty()) {             // no missing specified?
			m_var[v].Recode.Missing1 = m_var[v].Missing1;  // take the missing of source variable
			m_var[v].Recode.Missing2 = m_var[v].Missing2;
		}
		else {
			if (Missing1.IsEmpty()) {  // at least one missing specified
				m_var[v].Recode.Missing1 = Missing2;
				m_var[v].Recode.Missing2 = Missing1;
			}
			else {
				m_var[v].Recode.Missing1 = Missing1;
				m_var[v].Recode.Missing2 = Missing2;
			}
		}

		// second empty?
	if (m_var[v].Recode.Missing2.IsEmpty()) {
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
	AddRecode(v, (LPCTSTR) m_var[v].Recode.Missing1);
	if (m_var[v].Recode.nMissing == 2) {
		 AddSpacesBefore(m_var[v].Recode.Missing2, maxwidth);
		 AddRecode(v, (LPCTSTR) m_var[v].Recode.Missing2);
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

	oke = ParseRecodeString(v, sRecodeString, ErrorType, ErrorLine, ErrorPos, SRCCODE);
	if (!oke) {
		return false; // missing to valid codes, a terrible shame
	}

	// yep, the number of codes is known and the codes are sorted (except one or two MISSINGs at the end of te list)
	m_var[v].Recode.nCode = m_var[v].Recode.sCode.GetSize();


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
				CString str = m_var[v].sCode[i];
				AddSpacesBefore(str, c->CodeWidth);
				index = BinSearchStringArray(c->sCode, str, c->nMissing, IsMissing);
				ASSERT(index >= 0 && index < c->nCode);
				c->DestCode[i] = index;
			}
		}
	}

	// WARNINGS in recode:

  // show untouched codes:
	if (m_nUntouched > 0) {
		CString temp;
		temp.Format("Number of untouched codes: %d\r\n", m_nUntouched);
		m_WarningRecode += temp;
	}

	// show warnings
	if (m_nOverlap > 0) {
		temp.Format("Number of overlapping codes: %d\r\n", m_nOverlap);
		m_WarningRecode += temp;
	}

	if (m_nNoSense > 0) {
		temp.Format("Number of \"no sense\" codes: %d\r\n", m_nNoSense);
		m_WarningRecode += temp;
	}

	if (m_WarningRecode.IsEmpty() ) {
		m_WarningRecode = "Recode OK";
	}


	*WarningString = m_WarningRecode;

	m_var[v].HasRecode = true;

	m_var[v].Recode.nCode = m_var[v].Recode.sCode.GetSize();

	m_var[v].Recode.sCode[0] = "";

	return true;
}

// Apply Recoding. You need this for recoding tables
void TauArgus::ApplyRecode()
{
	ComputeRecodeTables();
}

// Clean all allocated memory. Destructor does this
void STDMETHODCALLTYPE TauArgus::CleanAll()
{
	#ifdef _DEBUG
		DEBUGprintf("CleanAll()\n");
	#endif
	CleanUp();
}

// Used for setting Hierarchical Variables with digit Split
bool STDMETHODCALLTYPE TauArgus::SetHierarchicalDigits(long VarIndex, long nDigitPairs, long *nDigits)
{
	#ifdef _DEBUG
		DEBUGprintf("SetHierarchicalDigits(%ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld)\n", VarIndex, nDigitPairs, nDigits[0], nDigits[1], nDigits[2], nDigits[3], nDigits[4], nDigits[5], nDigits[6], nDigits[7], nDigits[8], nDigits[9]);
	#endif
			
	if (VarIndex < 0 || VarIndex >= m_nvar || !m_var[VarIndex].IsHierarchical) {
		return false;
	}

	if (m_var[VarIndex].hLevel.GetSize() != 0) {
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
bool STDMETHODCALLTYPE TauArgus::GetTableRow(long TableIndex, long *DimIndex, double *Cell,
													long *Status, long CountType)
{
	int tab = TableIndex, i, coldim = -1, nCodes;
	long DimNr[MAXDIM];
	long DoAudit, CountTypeLocal;
	double 	XRL, XRU, XPL, XPU, XC;

	// check parameters
	if (tab < 0 || tab >= m_ntab) {
		return false;
	}
    DoAudit = 0;
	CountTypeLocal = CountType;
	if (CountTypeLocal < 0 ) {
		DoAudit = 1;
        CountTypeLocal = -CountTypeLocal;
	}

	CTable *Table = GetTable(tab);

	// check DimIndices
	for (i = 0; i < Table->nDim; i++) {
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
STDMETHODIMP TauArgus::UnsafeVariable(long VarIndex,
														long *Count, long *UCArray,
														VARIANT_BOOL *pVal)
{
	int t, v = VarIndex - 1, var;
	long nUnsafe[MAXDIM];
	CTable *tab;

	if (v < 0 || v >= m_nvar) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	// compute count
	*Count = 0;
	for (t = 0; t < m_ntab; t++) {
		tab = GetTable(t);
		if (tab->nDim > *Count) {
			*Count = tab->nDim;
		}
	}

	memset(UCArray, 0, *Count * sizeof(long) );

	// compute nUnsafe for variable v, add to UCArray
	for (t = 0; t < m_ntab; t++) {
		tab = GetTable(t);
		for (var = 0; var < tab->nDim; var++) {
			if (tab->ExplVarnr[var] == v) break;  // hebbes
		}
		if (var < tab->nDim) {
			tab->GetUnsafeCells(v, nUnsafe);
			for (int i = 0; i < tab->nDim; i++) {
				UCArray[i] += nUnsafe[i];
			}
		}
	}

	TRACE("Var %d unsafe %d %d %d\n", VarIndex, UCArray[0], UCArray[1], UCArray[2]);



	*pVal = VARIANT_TRUE;
	return S_OK;
}

// In this function the input file is read and the code list is built
bool STDMETHODCALLTYPE TauArgus::ExploreFile(const char* FileName, long *ErrorCode, long *LineNumber, long *VarIndex)
{
	DEBUGprintf("ExploreFile\n");

	CString sFileName;
	sFileName = FileName;
	FILE *fd;
   UCHAR str[MAXRECORDLENGTH];
   int i, length, recnr = 0, Result;

   *ErrorCode = *LineNumber = 0;
   *VarIndex = -1;

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

	fd = fopen(sFileName, "r");
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

	// record length oke? hierachie oke?
	for (i = 0; i < m_nvar; i++) {
		if (InFileIsFixedFormat) {
			if (m_var[i].bPos + m_var[i].nPos > m_fixedlength) {
				*ErrorCode = RECORDTOOSHORT;
				goto error;
			}
		}

		if (m_var[i].IsHierarchical) {
			if (m_var[i].nDigitSplit == 0 && m_var[i].hLevel.GetSize() == 0) {
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
					*VarIndex = varindex;
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
		// add Missing1 and -2
		if (m_var[i].IsCategorical) {
			if (m_var[i].nMissing != 0)	{
				m_var[i].AddCode((LPCTSTR) m_var[i].Missing1, true);
				if (m_var[i].IsHierarchical && m_var[i].nDigitSplit == 0) {
					m_var[i].hLevel.Add(1);
					m_var[i].hLevelBasic.Add(true);
				}
				if (m_var[i].nMissing == 2) {
					m_var[i].AddCode((LPCTSTR) m_var[i].Missing2, true);
					if (m_var[i].IsHierarchical && m_var[i].nDigitSplit == 0) {
						m_var[i].hLevel.Add(1);
						m_var[i].hLevelBasic.Add(true);
					}
				}
			}
		}

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

		m_var[i].nCode = m_var[i].sCode.GetSize();
		// allocate and initialize memory
		if (m_var[i].IsHierarchical) {
			if (!m_var[i].SetHierarch() ) {
				*ErrorCode = WRONGHIERARCHY;
				*LineNumber = -1;
				return false;
			}
		}
	}

	strcpy(m_fname, sFileName);  // Save name for use in ComputeTables
	m_CompletedCodeList = true;
	// ShowCodeLists();  // in output pane

	return true;

error:
	fclose(fd);

	return false;
}

// get maximum unsafe Combination
STDMETHODIMP TauArgus::GetMaxnUc(long *pVal)
{
	int t, c, max = 0;
	long tempstatus;


	if (m_nvar == 0  ||  m_ntab  == 0  || !m_CompletedCodeList)  {
		*pVal = -1;
	}

	for (t = 0; t < m_ntab; t++) {
		CTable *tab;
		tab = GetTable(t);
		for (c = 0; c < tab->nCell; c++) {
			tempstatus = tab->GetCell(c)->GetStatus();
			if ((tempstatus == CS_UNSAFE_FREQ) || (tempstatus == CS_UNSAFE_PEEP) ||
				( tempstatus == CS_UNSAFE_RULE) || (tempstatus == CS_UNSAFE_SINGLETON) ||
				(tempstatus == CS_UNSAFE_ZERO)) max++;
		}
	}

	*pVal = max;

	return S_OK;
}

// Undo recode. Undo recodes for a variable. This is used when a table is
// created to be recoded
bool TauArgus::UndoRecode(long VarIndex)
{
	int v = VarIndex;

	/*if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
    return false;
	}*/

	if (m_nvar == 0  || m_ntab == 0  || !m_CompletedCodeList)  {
		return false;
	}

	// wrong VarIndex
	if (v < 0 || v >=
		m_nvar || !m_var[v].IsCategorical) {
		return false;
	}

	m_var[v].UndoRecode();

	// recomputes for all tables the flag HasRecode
	SetTableHasRecode();

	return true;
}


//Sets the status of a cell to a given status
bool TauArgus::SetTableCellStatus(long TableIndex, long *DimIndex, long CelStatus)
{

	// redo protection levels
	int tab = TableIndex, i;
   CDataCell *dc;

   int iOriginalStatus;

	// check parameters
   if (tab < 0 || tab >= m_ntab) {
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

	CTable *table = GetTable(tab);
	//dc = table ->GetCell(DimIndex);
	//iTemporaryStatus = table->ComputeCellSafeCode(*dc);

	// check DimIndices
	for (i = 0; i < table->nDim; i++) {
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
	int tab = TableIndex, i;
   CDataCell *dc;

	// check parameters
   if (tab < 0 || tab >= m_ntab) {
		return false;
	}
	//if (m_fname[0] == 0)      		return false;
   if (!m_CompletedCodeList) {
		return false;
	}

	CTable *table = GetTable(tab);
	//dc = table ->GetCell(DimIndex);
	//iTemporaryStatus = table->ComputeCellSafeCode(*dc);

	// check DimIndices
	for (i = 0; i < table->nDim; i++) {
		int nCodes = m_var[table->ExplVarnr[i]].GetnCode();
		ASSERT(DimIndex[i] >= 0 && DimIndex[i] < nCodes);
		if (DimIndex[i] < 0 || DimIndex[i] >= nCodes)  {
			return false;
		}
	}
	dc = table ->GetCell(DimIndex);
	dc->SetCost(Cost);
	return true;
}

// Get Status and Cost per dimensie
STDMETHODIMP TauArgus::GetStatusAndCostPerDim(long TableIndex, long *Status,
																double *Cost, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	int t = TableIndex - 1;

	if (t < 0 || t >= m_ntab) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	GetTable(t)->GetStatusAndCostPerDim(Status, Cost);
	*pVal = VARIANT_TRUE;
	return S_OK;
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
bool STDMETHODCALLTYPE TauArgus::GetVarNumberOfCodes(long VarIndex, long *NumberOfCodes,
															long *NumberOfActiveCodes)
{
	int v = VarIndex;

	if (v < 0 || v >= m_nvar) {
		return false;
	}

	*NumberOfCodes = m_var[v].GetnCode();
	*NumberOfActiveCodes = m_var[v].GetnCodeActive();

	#ifdef _DEBUG
		DEBUGprintf("GetVarNumberOfCodes(%ld, %ld, %ld)\n", VarIndex, *NumberOfCodes, *NumberOfActiveCodes);
	#endif
	return true;
}

// Do recode for all active codes
bool TauArgus::DoActiveRecode(long VarIndex)

{
	int v = VarIndex;

	// too early?
 /* if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
    return false;
  }*/

	if  (m_nvar == 0 || m_ntab == 0 || !m_CompletedCodeList)  {
		return false;
	}

  // wrong VarIndex, not hierarchical
	if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical || !m_var[v].IsHierarchical) {
		return false;
	}

	// nothing to do?
	if (m_var[v].hCode == 0 || m_var[v].GetnCodeInActive() == 0) {
		return false;
	}

	return m_var[v].SetHierarchicalRecode() == TRUE;
}

// Set Variable. All information to set in the variable object is given
bool STDMETHODCALLTYPE TauArgus::SetVariable(long VarIndex, long bPos,
												 long nPos, long nDec, long nMissing, const char* Missing1,
												 const char* Missing2, const char* TotalCode, bool IsPeeper,
												 const char* PeeperCode1, const char* PeeperCode2,
												 bool IsCategorical,
												 bool IsNumeric, bool IsWeight,
												 bool IsHierarchical,
												 bool IsHolding)
{
	#ifdef _DEBUG
		DEBUGprintf("SetVariable(%ld, %ld, %ld, %ld, %ld, %s, %s, %s, %d, %s, %s, %d, %d, %d, %d, %d)\n",
			VarIndex, bPos, nPos, nDec, nMissing, Missing1, Missing2, TotalCode, IsPeeper, PeeperCode1, PeeperCode2, IsCategorical, IsNumeric, IsWeight, IsHierarchical, IsHolding);
	#endif

	CString sMissing1;
	CString sMissing2;
	CString sTotalCode;
	CString tempPeeperCode1 = PeeperCode1;
	CString tempPeeperCode2 = PeeperCode2;
	sMissing1 = Missing1;
	sMissing2 = Missing2;
	sTotalCode = TotalCode;

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
	  IsHolding, IsPeeper) )		{
		return false;
	}
	if ((nMissing < 0) || (nMissing  > 2))	{
		return false;
	}
	if (!m_var[VarIndex].SetMissing(sMissing1, sMissing2, nMissing) )	{
		return false;
	}
	if (!m_var[VarIndex].SetTotalCode(sTotalCode) )	{
		return false;
	}
	if (!m_var[VarIndex].SetPeepCodes(tempPeeperCode1, tempPeeperCode2))	{
		return false;
	}

	return true;
}

// Sets all the information for the Table object this together with
// SetTableSafety does the trick.
bool STDMETHODCALLTYPE TauArgus::SetTable(long Index, long nDim, long *ExplanatoryVarList,
												bool IsFrequencyTable,
												long ResponseVar, long ShadowVar, long CostVar,
												double Lambda,
												double MaxScaledCost,
												long PeepVarnr,
												bool SetMissingAsSafe)
{
	#ifdef _DEBUG
		DEBUGprintf("SetTable(%ld, %ld, (", Index, nDim);
		for (int k=0; k<nDim; k++) 
			DEBUGprintf("%ld, ", ExplanatoryVarList[k]);
		DEBUGprintf("), %d, %ld, %ld, %ld, %f, %f, %ld, %d)\n",
				IsFrequencyTable, ResponseVar, ShadowVar, CostVar, Lambda, MaxScaledCost, PeepVarnr, SetMissingAsSafe);
	#endif

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
	m_tab[i].SetVariables(nDim, ExplanatoryVarList, ResponseVar, ShadowVar, CostVar, PeepVarnr);

	// add SizeDim to tab
	for (int d = 0; d < nDim; d++) {
		m_tab[i].SetDimSize(d, m_var[ExplanatoryVarList[d]].nCode);
	}

	return true;
}


STDMETHODIMP TauArgus::GetTableCellValue(long TableIndex, long CellIndex,
                                            double *CellResponse, VARIANT_BOOL *pVal)
{	int tab = TableIndex;


	if (!m_CompletedCodeList)  {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	CTable *table = GetTable(tab);

	CDataCell *dc;
	dc = table->GetCell(CellIndex);

	*CellResponse = dc->GetResp();

	*pVal =  VARIANT_TRUE;
	return S_OK;
}


// Returns the information in a cell.
bool STDMETHODCALLTYPE TauArgus::GetTableCell(long TableIndex, long *DimIndex,
													double *CellResponse, long *CellRoundedResp, double *CellCTAResp,
													double *CellShadow, double *CellCost,
													 long *CellFreq, long *CellStatus,
													 double *CellMaxScore,double *CellMAXScoreWeight,
													 long *HoldingFreq,
													 double *HoldingMaxScore, long *HoldingNrPerMaxScore,
													 double * PeepCell, double * PeepHolding, long * PeepSortCell, long * PeepSortHolding,
													 double *Lower, double *Upper,
													 double *RealizedLower,double * RealizedUpper)
{
	int tab = TableIndex, i;


	// check parameters
	if (tab < 0 || tab >= m_ntab) {
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

	CTable *table = GetTable(tab);

	// check DimIndices
	for (i = 0; i < table->nDim; i++) {
		int nCodes = m_var[table->ExplVarnr[i]].GetnCode();
		ASSERT(DimIndex[i] >= 0 && DimIndex[i] < nCodes);
		if (DimIndex[i] < 0 || DimIndex[i] >= nCodes) {
			return false;
		}
	}

	CDataCell *dc;
	dc = table->GetCell(DimIndex);

	*CellResponse = dc->GetResp();
	*CellRoundedResp = dc->GetRoundedResponse();
	*CellCTAResp = dc->GetCTAValue();
	*CellShadow  = dc->GetShadow();
	*CellCost    = dc->GetCost(table->Lambda);
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
	#ifdef _DEBUG
		DEBUGprintf("GetTableCell(%ld, (%ld, %ld), %6.2f, %ld, %6.2f, %6.2f, %6.2f, %ld, %ld, %6.2f, %6.2f, %ld, %6.2f, %ld, %6.2f, %6.2f, %ld, %ld, %6.2f, %6.2f, %6.2f, %6.2f)\n",
														TableIndex, DimIndex[0], DimIndex[1],
														*CellResponse, *CellRoundedResp, *CellCTAResp,
														*CellShadow, *CellCost,
														*CellFreq, *CellStatus,
														*CellMaxScore, *CellMAXScoreWeight,
														*HoldingFreq,
														*HoldingMaxScore, *HoldingNrPerMaxScore,
														*PeepCell, *PeepHolding, *PeepSortCell, *PeepSortHolding,
														*Lower, *Upper,
														*RealizedLower, *RealizedUpper);
	#endif
	return true;
}

// Set information necessary to build a tableobject. This functions works together with SetTable.
bool STDMETHODCALLTYPE TauArgus::SetTableSafety( long Index, bool DominanceRule,
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
														long ZeroSafetyRange,	long ManualSafetyPerc,
														long * CellAndHoldingFreqSafetyPerc)
{
	#ifdef _DEBUG
		DEBUGprintf("SetTableSafety( %ld, %d, (%ld, %ld, %ld, %ld), (%ld, %ld, %ld, %ld), %d, (%ld, %ld, %ld, %ld), (%ld, %ld, %ld, %ld), (%ld, %ld, %ld, %ld), (%ld, %ld), (%ld, %ld, %ld, %ld), (%ld, %ld), (%ld, %ld), %d, %d, %d, %d, %d, %d, %ld, %ld, %ld, (%ld, %ld))\n",
				Index, DominanceRule,
				DominanceNumber[0], DominanceNumber[1], DominanceNumber[2], DominanceNumber[3],
				DominancePerc[0], DominancePerc[1], DominancePerc[2], DominancePerc[3],
				PQRule,
				PriorPosteriorP[0], PriorPosteriorP[1], PriorPosteriorP[2], PriorPosteriorP[3], 
				PriorPosteriorQ[0], PriorPosteriorQ[1], PriorPosteriorQ[2], PriorPosteriorQ[3], 
				PriorPosteriorN[0], PriorPosteriorN[1], PriorPosteriorN[2], PriorPosteriorN[3],
				SafeMinRecAndHoldings[0], SafeMinRecAndHoldings[1],
				PeepPerc[0], PeepPerc[1], PeepPerc[2], PeepPerc[3],
				PeepSafetyRange[0], PeepSafetyRange[1], 
				PeepMinFreqCellAndHolding[0], PeepMinFreqCellAndHolding[1],
				ApplyPeep,
				ApplyWeight, ApplyWeightOnSafetyRule,
				ApplyHolding, ApplyZeroRule,
				EmptyCellAsNonStructural, NSEmptySafetyRange,
				ZeroSafetyRange, ManualSafetyPerc,
				CellAndHoldingFreqSafetyPerc[0], CellAndHoldingFreqSafetyPerc[1]);
	#endif
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
STDMETHODIMP TauArgus::PrepareHITAS(long TableIndex, BSTR NameParameterFile,
												  BSTR NameFilesFile, BSTR TauTemp, VARIANT_BOOL *pVal)
{
	CString sNameParameterFile;
	CString sNameFilesFile;
	sNameParameterFile = NameParameterFile;
	sNameFilesFile = NameFilesFile;
    m_hitas.TempPath = TauTemp; // Temp doorgeven vanuit de TAU ipv zlf bepalen.

	FILE *fdParameter, *fdFiles;
	long t = TableIndex - 1;

	if (t < 0 || t >= m_ntab)	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	if (m_tab[t].HasRecode) t += m_ntab;

	fdParameter = fopen(sNameParameterFile, "w");
	if (fdParameter == 0)	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	fdFiles = fopen(sNameFilesFile, "w");
	if (fdFiles == 0) {
		fclose(fdParameter);
		remove(sNameParameterFile);	{
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}

	if (!m_hitas.WriteParameterFile(fdParameter, m_tab[t]) ) goto error;
	if (!m_hitas.WriteFilesFile(fdFiles, m_tab[t], m_var) ) goto error;


	fclose(fdParameter);
	fclose(fdFiles);
	*pVal = VARIANT_TRUE;
	return S_OK;

error:
	fclose(fdParameter);
	fclose(fdFiles);
	remove(sNameParameterFile);
	remove(sNameFilesFile);
   *pVal = VARIANT_FALSE;
	return S_OK;

}

// Cells that are found to be secondary unsafe by Hitas is set as Unsafe in the table
STDMETHODIMP TauArgus::SetSecondaryHITAS(long TableIndex, long *nSetSecondary,
														 VARIANT_BOOL *pVal)
{
	FILE *fd;
	CString SecFileName = m_hitas.TempPath + m_hitas.NameSecFile;
	int t = TableIndex - 1;

	if (t < 0 || t >= m_ntab)	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	if (m_tab[t].HasRecode) t += m_ntab;

	fd = fopen((LPCTSTR) SecFileName, "r");
	if (fd == 0)	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	BOOL res = m_tab[t].SetSecondaryHITAS(fd, m_var, nSetSecondary);

	fclose(fd);

	#ifdef _DEBUGG
		ShowTable("c:\\temp\\hitasres.txt", m_tab[t]);
	#endif

	if (!res) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	else {
		*pVal = VARIANT_TRUE;
		return S_OK;
	}

}

// sets a Hierarchical codelist (i.e a codelist given through a file not
// through digit splits)
long STDMETHODCALLTYPE TauArgus::SetHierarchicalCodelist(long VarIndex, const char* FileName, const char* LevelString)
{
	#ifdef _DEBUG
		DEBUGprintf("SetHierarchicalCodelist(%ld, %s, %s)\n", VarIndex, FileName, LevelString);
	#endif
	CString sFileName;
	CString sLevelString;
	sFileName = FileName;
	sLevelString = LevelString;

	if (VarIndex < 0 || VarIndex >= m_nvar || !m_var[VarIndex].IsHierarchical) return HC_NOTHIERARCHICAL;
	if (m_var[VarIndex].nDigitSplit != 0) return HC_HASSPLITDIGITS;

	return m_var[VarIndex].SetCodeList(sFileName, sLevelString);
}

// Gets a code if given an index and a variable number
bool STDMETHODCALLTYPE TauArgus::GetVarCode(long VarIndex, long CodeIndex,
												long *CodeType, const char** CodeString,
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
	} else {
		if (m_var[v].IsCodeBasic(CodeIndex) ) *CodeType = CTY_BASIC;
		else                                  *CodeType = CTY_TOTAL;
    *Level = (CodeIndex == 0 ? 0 : 1);
  }

  // Gets a pointer to an internal buffer. It is still safe because the referenced object keeps living
  *CodeString = m_var[v].GetCode(CodeIndex);
  *IsMissing = (CodeIndex >= m_var[v].GetnCode() - m_var[v].GetnMissing());

	#ifdef _DEBUG
	DEBUGprintf("GetVarCode(%ld, %ld, %ld, %s, %ld, %ld)\n", VarIndex, CodeIndex, *CodeType, * CodeString, *IsMissing, *Level);
	#endif

	return true;
}


// return the codes for unsafe variables
STDMETHODIMP TauArgus::UnsafeVariableCodes(long VarIndex, long CodeIndex,
															long *IsMissing, long *Freq,
															BSTR *Code, long *Count,
															long *UCArray, VARIANT_BOOL *pVal)
{
	int t, v = VarIndex - 1, var;
	long nUnsafe[MAXDIM];
	CTable *tab;

	if (v < 0 || v >= m_nvar) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	int nCodes = m_var[v].GetnCode();
	if (CodeIndex < 0 || CodeIndex >= nCodes) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	// compute count
	*Count = 0;
	for (t = 0; t < m_ntab; t++) {
		tab = GetTable(t);
		if (tab->nDim > *Count) {
			*Count = tab->nDim;
		}
	}

	memset(UCArray, 0, *Count * sizeof(long) );

	// compute Freq
	for (t = 0; t < m_ntab; t++) {
		tab = GetTable(t);
		for (var = 0; var < tab->nDim; var++) {
			if (tab->ExplVarnr[var] == v) break;  // hebbes
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
  			tab = GetTable(t);
			for (var = 0; var < tab->nDim; var++) {
				if (tab->ExplVarnr[var] == v) break;  // hebbes
			}
			if (var < tab->nDim) {
				tab->GetUnsafeCells(v, CodeIndex, nUnsafe);
				for (int i = 0; i < tab->nDim; i++) {
					UCArray[i] += nUnsafe[i];
				}
			}
		}
	}

	CString s;
	s = m_var[v].GetCode(CodeIndex);
	*Code = s.AllocSysString();
	*IsMissing = (CodeIndex >= nCodes - m_var[v].GetnMissing() );

	// TRACE("Var %d Code [%s] Freq = %d, unsafe %d %d %d\n", VarIndex, (LPCTSTR) m_var[v].GetCode(CodeIndex), *Freq, UCArray[0], UCArray[1], UCArray[2]);

	*pVal =  VARIANT_TRUE;
	return S_OK;
}

// return properties given a Variable and Code Index
bool TauArgus::GetVarCodeProperties(long VarIndex, long CodeIndex,
															 long *IsParent, long *IsActive,
															 long *IsMissing, long *Level,
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
	CStringArray *psCode = &(m_var[v].sCode);

	*IsParent = phCode[c].IsParent;
	*IsActive = phCode[c].Active;
	*IsMissing = (c >= m_var[v].nCode - m_var[v].nMissing);
	*Level = phCode[c].Level;
	*nChildren = phCode[c].nChildren;
	*Code = psCode->GetAt(c);

	return true;
}

// Write Table in GHmiter file. This is used in secondary supressions
STDMETHODIMP TauArgus::WriteGHMITERSteuer(BSTR FileName, BSTR EndString1,
														  BSTR EndString2, long TableIndex,
														  long *pVal)
{
	// check table index
	CString sEndString1;
	CString sEndString2;
	CString sFileName;
	sEndString1 = EndString1;
	sEndString2 = EndString2;
	sFileName = FileName;
	int t = TableIndex - 1;
	if (t < 0 || t >= m_ntab) {
		*pVal = GHM_TABLEINDEXWRONG;
		return S_OK;
	}

	CTable *tab = GetTable(t);

	CString s;
	int result;

	s.Format("Table %d", TableIndex);
	result = m_ghmiter.ControlDataTable(sFileName, (LPCTSTR)s, sEndString1, sEndString2, tab->nDim, tab->ExplVarnr, m_var);

	//result = m_ghmiter.ControlDataTable(FileName, (LPCTSTR) s, sEndString1, sEndString2, tab->nDim, tab->ExplVarnr, m_var);
	if (result == 0) {
		*pVal = GHM_STEUERINCORRECT;
		return S_OK;
	}

	*pVal = 1;
	return S_OK;
}

// Write data Cells for GHMiter.
STDMETHODIMP TauArgus::WriteGHMITERDataCell(BSTR FileName, long TableIndex,
															  VARIANT_BOOL IsSingleton,
															 long *pVal)
{
	CString sFileName;
	sFileName = FileName;
	bool isSingle;
	if (IsSingleton) {
		isSingle = true;
	}
	else {
		isSingle = false;
	}

	// check table index
	int t = TableIndex - 1;
	if (t < 0 || t >= m_ntab) {
		*pVal = GHM_TABLEINDEXWRONG;
		return S_OK;
	}

	CTable *tab = GetTable(t);

	int result = m_ghmiter.CellsTable(sFileName, tab, m_var, isSingle);
	if (result == 0) {
		*pVal = GHM_EINGABEINCORRECT;
		return S_OK;
	}
	*pVal = 1;
	return S_OK;
}

// Cells that are given as secondary Unsafe by GHmiter
// to be set in the table
STDMETHODIMP TauArgus::SetSecondaryGHMITER(BSTR FileName, long TableIndex,
															long *nSetSecondary,VARIANT_BOOL IsSingleton,
															long *pVal)
{
	// check table index
	CString sFileName;
	sFileName = FileName;
	bool isSingle;
	if (IsSingleton) {
		isSingle = true;
	}
	else {
		isSingle = false;
	}

   int t = TableIndex - 1;
	if (t < 0 || t >= m_ntab) {
		*pVal= GHM_TABLEINDEXWRONG;
		return S_OK;
	}

	CTable *tab = GetTable(t);

	int ErrorCode;
	int result = m_ghmiter.SetSecondaryUnsafe(sFileName, tab, nSetSecondary, &ErrorCode, isSingle);

	// if secondary supress does not wiork for some reason
	if (result == 0) {// Not too sure about this
		if (IsSingleton) {
			tab->UndoSecondarySuppress(WITH_SINGLETON);
		}
		else {
			tab->UndoSecondarySuppress(WITHOUT_SINGLETON);
		}
		*pVal = ErrorCode;
		return S_OK;
	}

   *pVal = 1;
	return S_OK;
}

// Get information per status
STDMETHODIMP TauArgus::GetCellStatusStatistics(long TableIndex,
																  long *StatusFreq,
																  long *StatusCellFreq,
																  long *StatusHoldingFreq,
																  double *StatusCellResponse,
																  double *StatusCellCost,
																  VARIANT_BOOL *pVal)
{
	int t = TableIndex - 1;

	if (t < 0 || t >= m_ntab) {
		*pVal = VARIANT_FALSE;
	  	return S_OK;
	}

	CTable *tab = GetTable(t);
	tab->GetStatusStatistics(StatusFreq,StatusCellFreq,StatusHoldingFreq, StatusCellResponse, StatusCellCost);

	*pVal = VARIANT_TRUE;
	return S_OK;
}


// This function is not usedb at the moment
STDMETHODIMP TauArgus::PrepareCellDistance(long TableIndex, VARIANT_BOOL *pVal)
{
	int t = TableIndex - 1;

	if (t < 0 || t >= m_ntab)	{
		*pVal =  VARIANT_FALSE;
		return S_OK;
	}
  GetTable(t)->PrepareComputeDistance();
	*pVal = VARIANT_TRUE;
	return S_OK;
}

// This function is not used either at the moment
STDMETHODIMP TauArgus::GetCellDistance(long TableIndex, long *Dims,
													  long *Distance, VARIANT_BOOL *pVal)
{
	int t = TableIndex - 1;
   if (t < 0 || t >= m_ntab) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
   CTable *tab = GetTable(t);
	tab->GetCellDistance(tab->GetCellNrFromIndices(Dims), Distance);
	*pVal =  VARIANT_TRUE;
	return S_OK;
}

// Write a table as Comma seperated file
STDMETHODIMP TauArgus::WriteCSV(long TableIndex, BSTR FileName,
											 long *DimSequence, long RespType, VARIANT_BOOL *pVal)
{
	CString sFileName;
	sFileName = FileName;
	int t = TableIndex - 1, d, e;
	CTable *tab;
	FILE *fd;
	long Dims[MAXDIM];
//	bool bRoundedResp;
//	if (ReplaceRespWithRounded)	{
//		bRoundedResp = true;
//	}
//	else	{
//		bRoundedResp = false;
//	}

	if (t < 0 || t >= m_ntab)	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	// check DimSequence
	// first decrease dim index
	for (d = 0; d < m_tab[t].nDim; d++) {
		DimSequence[d]--;
	}
	// correct and unique?
	for (d = 0; d < m_tab[t].nDim; d++) {
		if (DimSequence[d] < 0 || DimSequence[d] >= m_tab[t].nDim){
			*pVal = VARIANT_FALSE;
			return S_OK;
	  }
		for (e = d + 1; e < m_tab[t].nDim; e++) {
			if (DimSequence[d] == DimSequence[e])	{
				*pVal = VARIANT_FALSE;
				return S_OK;
			}
		}
	}

	// open result file
	fd = fopen(sFileName, "w");
	if (fd == 0)	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	// get pointer to (recoded) table
	tab = GetTable(t);
	WriteCSVTable(fd, tab, DimSequence, Dims, 0, m_ValueSeparator, RespType);

	fclose(fd);

	*pVal = VARIANT_TRUE;
	return S_OK;
}

// Write Table in JJ Format
STDMETHODIMP TauArgus::WriteJJFormat(long TableIndex, BSTR FileName,
													double LowerBound, double UpperBound,
													VARIANT_BOOL WithBogus, VARIANT_BOOL AsPerc,
													VARIANT_BOOL ForRounding,VARIANT_BOOL *pVal)
{

	CString sFileName;
	sFileName = FileName;
	CString sFileNameFreq;
	sFileNameFreq = sFileName + ".frq";
	FILE *fd, *fdFrq;
	double MaxCost, MaxResp, x;
	int i;
	CDataCell *dc;

	int t = TableIndex - 1;
	if (t < 0 || t >= m_ntab)	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	CTable *tab = GetTable(t);

	fd = fopen(sFileName, "w");
	if (fd == 0)	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	fdFrq = fopen(sFileNameFreq, "w");
	if (fdFrq == 0)	{
		*pVal = VARIANT_FALSE;
		return S_OK;
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
	*pVal = VARIANT_TRUE;
	return S_OK;
error:
	fclose(fd);
	fclose (fdFrq);
	*pVal = VARIANT_FALSE;
	return S_OK;
}
// Cells that are give as secondary unsafe by JJ to be set in the table
STDMETHODIMP TauArgus::SetSecondaryJJFORMAT(long TableIndex, BSTR FileName, VARIANT_BOOL WithBogus, long *nSetSecondary, long *pVal)
{
	CString sFileName;
	sFileName = FileName;
	int t = TableIndex - 1;
	long ErrorCode;

	if (t < 0 || t >= m_ntab) {
		*pVal = JJF_TABLEINDEXWRONG;
	}

	CTable *tab = GetTable(t);

	int result = m_jjformat.SetSecondaryUnsafe(sFileName, tab, m_var, nSetSecondary, &ErrorCode, WithBogus);
	if (!result)	{
		*pVal = ErrorCode;
	}
	return S_OK;
}

// Get Size of a tabel. This function has to be changed
STDMETHODIMP TauArgus::GetTotalTabelSize(long TableIndex,
														   long* nCell,long * SizeDataCell)
{
	int lenarrcell,lenarrhold;
	int applyhold, applyweight;
	long ind = TableIndex -1;
	*nCell = m_tab[ind].nCell;
	lenarrcell = m_tab[ind].NumberofMaxScoreCell;
	lenarrhold = m_tab[ind].NumberofMaxScoreHolding;
	applyhold = m_tab[ind].ApplyHolding;
	applyweight = m_tab[ind].ApplyWeight;
	*SizeDataCell = sizeof(CDataCell(lenarrcell,lenarrhold,applyhold,applyweight));


	return S_OK;
}

// Write Cell in file
STDMETHODIMP TauArgus::WriteCellRecords(long TableIndex, BSTR FileName,
									        long SBS,
									        VARIANT_BOOL SBSLevel,
											VARIANT_BOOL SuppressEmpty, BSTR FirstLine,
											VARIANT_BOOL ShowUnsafe,
										    long RespType,
											VARIANT_BOOL *pVal)
{
	bool bShowUnsafe;
	if (ShowUnsafe) {
		bShowUnsafe = true;
	}
	else {
		bShowUnsafe = false;
	}
	bool bSBSLevel;
	if (SBSLevel) { bSBSLevel = true;}
	else          { bSBSLevel = false;}

//	bool bRoundedResp;
//	if (ReplaceRespWithRounded)	{
//		bRoundedResp = true;
//	}
//	else	{
//		bRoundedResp = false;
//	}


	CString sFileName;
	CString sFirstLine;
	sFileName = FileName;
	sFirstLine = FirstLine;
	int t = TableIndex - 1, SBSCode;
	CTable *tab;
	FILE *fd;
	long Dims[MAXDIM];

	if (t < 0 || t >= m_ntab) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	fd = fopen(sFileName, "w");
	if (fd == 0) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	tab = GetTable(t);
    WriteFirstLine(fd, sFirstLine);
// SBSCode: 0 doet niet mee
//          1 geen topn-info aanwezig
//			2 een topn individueel
//			3 meer dan een topn individueel
//			4 een topn Holding
//			5 meer dan een topn Holding

	SBSCode = 0;
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
	WriteCellRecord(fd, tab, Dims, 0, m_ValueSeparator, SBSCode, bSBSLevel, SuppressEmpty, bShowUnsafe, RespType);

	fclose(fd);

	*pVal = VARIANT_TRUE;
	return S_OK;
}

// Code list to be created. This is a sibling of the explore file. This is needed
// for crerating a codelist when a table is directly given instead of being created from
// micro data.
STDMETHODIMP TauArgus::SetInCodeList(long NumberofVar, long *VarIndex,
													VARIANT *sCode, long *ErrorCode,
													long *ErrorInVarIndex, VARIANT_BOOL *pVal)
{
	long i;
	long lvarindex;
	BSTR bstrtemp;
	CString tempcode;
	CVariable *var;
	SAFEARRAY *sa = sCode->parray;
	HRESULT hresult;
 // check if number of variables = length of array codes


	*ErrorInVarIndex = 0;
	if (m_nvar == 0) {
		*ErrorCode = NOVARIABLES;
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	for (i = 0; i < m_nvar; i++) {
		if (m_var[i].IsHierarchical) {
			if (m_var[i].nDigitSplit == 0 && m_var[i].hLevel.GetSize() == 0) {
				*ErrorCode = WRONGHIERARCHY;
				*ErrorInVarIndex = i;
				*pVal = VARIANT_FALSE;
				return S_OK;
			}
		}
	}

	// Now start reading the codes in Not if the code is in a hierarchical
	// codelist without digit split don't bother.

	for (i=1; i<=NumberofVar; i++)  {
		lvarindex = VarIndex[i-1];
		var = &(m_var[lvarindex-1]);
		// To get a string from an array of strings. Use SafeArray
		hresult = SafeArrayGetElement(sa, &i, &bstrtemp);
		tempcode = bstrtemp;
		if (tempcode != "" ) {
		if (var->IsCategorical)  {
			if ((var ->IsHierarchical) && (var ->nDigitSplit == 0))  {
				if (var ->FindAllHierarchicalCode(tempcode) < 0)  {
					*ErrorCode = CODENOTINCODELIST;
					*ErrorInVarIndex = i;
					 // add some info here
					*pVal = VARIANT_FALSE;
					return S_OK;

				}
			}
			else {
				if (!var->AddCode(tempcode, false) ) {   // adds if new, else does nothing
					*ErrorCode = PROGRAMERROR;
					*pVal = VARIANT_FALSE;  // add program error
					return S_OK;
				}
			}
		}
		}
	}

	*pVal = VARIANT_TRUE;
	return S_OK;
}

// Once all code list a finished. Set Hierarchies and totals. This happens automatically in
// the end of explore file
STDMETHODIMP TauArgus::SetTotalsInCodeList(long NumberofVariables, long *VarIndex,
															long *ErrorCode, long *ErrorInVarIndex,
															VARIANT_BOOL *pVal)
{
	long i, n;
	long lvarindex;
	CVariable *var;

	for (i=0; i<NumberofVariables; i++)  {
		lvarindex = VarIndex[i];
		var = &(m_var[lvarindex-1]);

		if (var->IsCategorical)  {

			if (var->nMissing > 0)	{
				var->AddCode((LPCTSTR) var->Missing1, true);
				if (var->IsHierarchical && var->nDigitSplit == 0) {
					var->hLevel.Add(1);
					var->hLevelBasic.Add(true);
				}

				if (var->nMissing == 2)  {
					var->AddCode((LPCTSTR) var->Missing2, true);
					if  (var->IsHierarchical && var->nDigitSplit == 0)   {
						var->hLevel.Add(1);
						var->hLevelBasic.Add(true);
					}
				}
			}
		}

			if ((var->nDigitSplit > 0)) {
				if (!(var->ComputeHierarchicalCodes()) ) {
					*ErrorCode = WRONGHIERARCHY;
					*pVal = VARIANT_FALSE;
					return S_OK;
				}
			}

			// Add Totals to the Code
			if (!var->IsHierarchical || var->nDigitSplit > 0) {
				var->AddCode("", false);
			}


//			m_var[i].nCode = m_var[i].sCode.GetSize();
			n = m_var[lvarindex-1].sCode.GetSize();
			m_var[lvarindex-1].nCode = n;
			if (var->IsHierarchical)  {
				if (!var->SetHierarch()) {
					*ErrorCode = WRONGHIERARCHY;
					*pVal = VARIANT_FALSE;
					return S_OK;
				}
			}


	}

	m_CompletedCodeList = true;

	*pVal =  VARIANT_TRUE;
	return S_OK;
}

// Tells you that you a given a table directly and not to
//create one from micro data
STDMETHODIMP TauArgus::ThroughTable()
{
	m_UsingMicroData = false;

	return S_OK;
}

// Set data in table. Since table can be given in many ways.
// See SetTableSafetyInfo, not all input is valid input.
STDMETHODIMP TauArgus::SetInTable(long Index, VARIANT *sCode,
												double Shadow, double Cost,
												double Resp, long Freq,
												double *MaxScoreCell, double *MaxScoreHolding,
												long Status, double LowerProtectionLevel, double UpperProtectionLevel,
												long *ErrorCode, long *ErrVNum,
												VARIANT_BOOL *pVal)
{
	int  ind, dim;
	ind = Index -1;
	long error, ErrorVarNo;

	*ErrVNum = 0;
	SAFEARRAY *sa = sCode->parray;
	long l;
	HRESULT hresult;
	BSTR bstrtemp;
	double LPL, UPL;

	// check if table is defined
	if (m_tab[ind].nDim == 0) {
		*ErrorCode = TABLENOTSET;
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	// Memory allocated for table. If not do it
	if (!m_tab[ind].Prepared)  {
		if (!m_tab[ind].PrepareTable() ) {
			*ErrorCode = NOTABLEMEMORY;
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}

	dim = m_tab[ind].nDim;
	CString *sCodes = new CString[dim];


	//  Take out the codes and set in a CString array
	for (l=1; l<=dim; l++) {
		hresult = SafeArrayGetElement(sa, &l, &bstrtemp);
		sCodes[l-1]=bstrtemp;
	}

	LPL = LowerProtectionLevel;
	UPL = UpperProtectionLevel;

	// fill it in the table cell
	if (!FillInTable(ind, sCodes, Cost,
								Resp, Shadow, Freq, MaxScoreCell, MaxScoreHolding,
								LPL, UPL, Status, error, ErrorVarNo))  {
		*ErrorCode = error;
		*ErrVNum = ErrorVarNo;
		*pVal = VARIANT_FALSE;
		delete [] sCodes;
		return S_OK;
	}


	delete [] sCodes;
	*pVal = VARIANT_TRUE;

	return S_OK;
}

// To state all the cells have been read and the table has to be built.
// In some case the marginals (or sub totals are given ) in other cases they have to be
// calculated
STDMETHODIMP TauArgus::CompletedTable(long Index, long *ErrorCode,
										 BSTR FileName,
										 VARIANT_BOOL CalculateTotals,
										 VARIANT_BOOL SetCalculatedTotalsAsSafe,
										 VARIANT_BOOL ForCoverTable,
										 VARIANT_BOOL *pVal)
{
//	CString sFileName;
//	sFileName = FileName;
    long ind;
	ind = Index -1;
	int i;
	CDataCell *dc ;
	long tel;
	long maxdiepte;
	bool IsAdditive;

//	FILE *fddebug;

//	CDataCell *dcempty = new CDataCell(m_tab[ind].NumberofMaxScoreCell,m_tab[ind].NumberofMaxScoreHolding,m_tab[ind].ApplyHolding);

//	fddebug = fopen("f:TMPTest", "w");
//	fprintf(fddebug,"%s\n", "CompletedTable");
//	fclose(fddebug);

	// if status for each cel is given
	IsAdditive = true;
	i = 1;
	if (m_HasStatus)  {

//	fddebug = fopen("f:TMPText", "a");
//	fprintf(fddebug,"%s\n", "Has status so filled");
//	fclose(fddebug);

//  WriteJJFormat(Index, FileName, -1000, 1000, true, false, false);
//													double LowerBound, double UpperBound,
//													VARIANT_BOOL WithBogus, VARIANT_BOOL AsPerc,
//													VARIANT_BOOL ForRounding,

		for (i=0; i <m_tab[ind].nCell; i++)  {
			// if not safe or unsafe set empty; all not entered cells
			dc = m_tab[ind].GetCell(i);
			if (dc->GetStatus() == 0)  {
			//	m_tab[ind].CellPtr.SetAt(i,dcempty);
			//	dc = m_tab[ind].GetCell(i);
				dc->SetStatus(CS_EMPTY);
				dc->IsFilled = false; //Why false,AncoJuly 2012
			}
		}
		// Sub totals are given. Check if table is additive
		// CoverTabel always has a status
		if (!ForCoverTable){
			if (!CalculateTotals)	{
	    		if (!IsTable(&(m_tab[ind]))) {
		    		*ErrorCode = TABLENOTADDITIVE;
     			    WriteJJFormat(Index, FileName, -1000, 1000, VARIANT_FALSE, VARIANT_FALSE, VARIANT_FALSE, pVal);
					IsAdditive = false;
//			    	*pVal = VARIANT_FALSE;
//					return S_OK;
				}
			}

			// other wise make sub totals
			else
			{
				tel = 0;
				maxdiepte = MaxDiepteVanSpanVariablen(&(m_tab[ind]));

				while (!IsTable(&(m_tab[ind])))	{
					AdjustTable(&(m_tab[ind]));
					// To preven an unending loop
					tel++;
					if (tel > maxdiepte + 6 )	{

						break;
					}
				}


				if (tel > maxdiepte + 6)	{
					*ErrorCode = CANNOTMAKETOTALS;
					IsAdditive = false;
//					*pVal = VARIANT_FALSE;
//					return S_OK;
				}
			}
		}
		// Calculated totals to be set as safe
		if (SetCalculatedTotalsAsSafe)	{
			for (i=0; i <m_tab[ind].nCell; i++)  {
			// if not safe or unsafe set empty
				dc = m_tab[ind].GetCell(i);
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
				*pVal = VARIANT_FALSE;
				return S_OK;
			}
			else
			{
				for (i=0; i <m_tab[ind].nCell; i++)  {
			// if not safe or unsafe set empty
					dc = m_tab[ind].GetCell(i);
					if (dc->GetStatus() == 0)  {
						//	m_tab[ind].CellPtr.SetAt(i,dcempty);
						//	dc = m_tab[ind].GetCell(i);
						dc->SetStatus(m_tab[ind].ComputeCellSafeCode(*dc));
						m_tab[ind].SetProtectionLevelCell(*dc);
						dc->IsFilled = false;  //Why false,AncoJuly 2012
					}
				}
			}
		}

		/// check if is table
		if (IsAdditive){
			*pVal = VARIANT_TRUE;
			return S_OK;
        }
		else
		{
			*pVal = VARIANT_FALSE;
			return S_OK;
		}

	} // End loop status is given

//	fddebug = fopen("f:TMPText", "a");
//	fprintf(fddebug,"%s\n", "End loop status is given");
//	fclose(fddebug);

	if ((m_HasFreq) || (m_HasMaxScore))  {
		//ComputeCellStatuses(m_tab[ind]);


//	fddebug = fopen("f:TMPText", "a");
//	fprintf(fddebug,"%s\n", "(m_HasFreq) || (m_HasMaxScore)");
//	fclose(fddebug);

		//Once more check all statuses are filled
		for (i=0; i <m_tab[ind].nCell; i++)  {
			// if not safe or unsafe set empty
			dc = m_tab[ind].GetCell(i);
			if (dc->GetStatus() == 0)  {
				dc->SetStatus(CS_EMPTY);
			}
		}
		if (!CalculateTotals)	{
			if (!IsTable(&(m_tab[ind]))) {
				// maybe error code shouls show that table is not ok.
				*ErrorCode = TABLENOTADDITIVE;
   			    WriteJJFormat(Index, FileName, -1000, 1000, VARIANT_FALSE, VARIANT_FALSE, VARIANT_FALSE, pVal);
                IsAdditive = false;
//			*pVal = VARIANT_FALSE;
//				return S_OK;
			}
		}
		else
		{
			tel = 0;
			maxdiepte = MaxDiepteVanSpanVariablen(&(m_tab[ind]));
			while (!IsTable(&(m_tab[ind])))	{
				AdjustTable(&(m_tab[ind]));
				// To prevent an unending loop
				tel++;
				if (tel > maxdiepte+6 )	{
					break;
				}
			}


			if (tel > maxdiepte +6)	{
				*ErrorCode = CANNOTMAKETOTALS;
				IsAdditive = false;
//				*pVal = VARIANT_FALSE;
//				return S_OK;
			}
		}
		//for each cell create safe code
		ComputeCellStatuses(m_tab[ind]);
		SetProtectionLevels(m_tab[ind]);
	} //end loop HasFreq or HasMaxScore

//	fddebug = fopen("f:TMPText", "a");
//	fprintf(fddebug,"%s\n", "end loop HasFreq or HasMaxScore");
//	fclose(fddebug);


	// finale check: alles met status = 0  wordt empty als leeg or safe als niet leeg
    // Bijvoorbeeld als er niets bekend is over de status
	for (i=0; i <m_tab[ind].nCell; i++)  {
		// if not safe or unsafe set empty
		dc = m_tab[ind].GetCell(i);
		if (dc->GetStatus() == 0)  {
		//	m_tab[ind].CellPtr.SetAt(i,dcempty);
		//	dc = m_tab[ind].GetCell(i);
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

		if (IsAdditive){
			*pVal = VARIANT_TRUE;
			return S_OK;
        }
		else
		{
			*pVal = VARIANT_FALSE;
			return S_OK;
		}

}

// variable set. This is incase table is given
//directly and not built from micro data
STDMETHODIMP TauArgus::SetVariableForTable(long Index, long nMissing, BSTR Missing1,
															BSTR Missing2, long nDec, VARIANT_BOOL IsPeeper,
															BSTR PeeperCode,
															VARIANT_BOOL IsHierarchical,
															VARIANT_BOOL IsNumeriek, long nPos,
															VARIANT_BOOL *pVal)
{
	CString sMissing1;
	CString sMissing2;
	sMissing1 = Missing1;
	sMissing2 = Missing2;
	int i = Index - 1;

	if (i < 0 || i >= m_nvar)	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	if ((nMissing < 0) || (nMissing  > 2))	{
	  *pVal = VARIANT_FALSE;
		return S_OK;
	}
	if (!m_var[i].SetMissing(sMissing1, sMissing2, nMissing) ) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

   m_var[i].nPos = nPos;
	//variable ia always categorical, weight and holdings don't come into play
	if (IsNumeriek) {
		if (!m_var[i].SetDecPosition(nDec))	{
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
		if (!m_var[i].SetType(false, IsNumeriek, false, IsHierarchical, false, false) )	{
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}
	else  {
		if (!m_var[i].SetType(true, false, false, IsHierarchical, false, false) )	{
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}

	*pVal = VARIANT_TRUE;
	return S_OK;
}

// set safety information. this function is used along with set table.
// this is the sibling of settablesafety. for instance holding and weight cannot be made
STDMETHODIMP TauArgus::SetTableSafetyInfo(long TabIndex,
															VARIANT_BOOL HasMaxScore,
															VARIANT_BOOL DominanceRule,
															long * DominanceNumber, long * DominancePerc,
															VARIANT_BOOL PQRule,
															long * PriorPosteriorP, long * PriorPosteriorQ, long * PriorPosteriorN,
															VARIANT_BOOL HasFreq,
															long	CellFreqSafetyPerc, long SafeMinRec,
															VARIANT_BOOL HasStatus, long ManualSafetyPerc,
															VARIANT_BOOL ApplyZeroRule, long ZeroSafetyRange,
															VARIANT_BOOL EmptyCellAsNonStructural, long NSEmptySafetyRange,
															long *ErrorCode, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	int i = TabIndex - 1;
	// Check this out
	if (!(HasMaxScore == 0))  {
		m_HasMaxScore = true;
	}

	if (!(HasStatus ==0))  {
		m_HasStatus = true;
	}

	if (!(HasFreq == 0))  {
		m_HasFreq = true;
	}

	// check TableIndex
	if (m_nvar == 0)  {
		*ErrorCode = VARIABLENOTSET;
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	if (i < 0 || i >= m_ntab) {
		*ErrorCode = TABLENOTSET;
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	if (HasFreq)  {
		if ((CellFreqSafetyPerc < 0 ) || (CellFreqSafetyPerc > 100))  {
			*ErrorCode = FREQPERCOUTOFRANGE;
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
		//!!!!!!!!!!!!!!!!!!
		if (!m_tab[i].SetSafeMinRecAndHold(SafeMinRec, 0) ) {
			*ErrorCode = PROGRAMERROR;
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}

	if (HasStatus)  {
		if ((ManualSafetyPerc < 0) || (ManualSafetyPerc > 100)) {
			*ErrorCode = STATUSPERCOUTOFRANGE;
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}


	// Set safety Rule
	if (DominanceRule)	{
		if (!m_tab[i].SetDominance(DominanceNumber,DominancePerc )) {
			*ErrorCode = PROGRAMERROR;
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}
	if (PQRule)	{
		if (!m_tab[i].SetPQRule(PriorPosteriorP,PriorPosteriorQ,PriorPosteriorN) ) {
			*ErrorCode = PROGRAMERROR;
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}

	if (EmptyCellAsNonStructural)	{
		if (NSEmptySafetyRange < 0 || NSEmptySafetyRange >= 100)	{
			*pVal = VARIANT_FALSE;
			return S_OK;
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

	*pVal = VARIANT_TRUE;
	return S_OK;
}

//CTA-value
STDMETHODIMP TauArgus::SetCTAValues(long TabNr, long CelNr,
                                         double OrgVal, double CTAVal,
                                         long *Sec , VARIANT_BOOL *pVal)
{CDataCell *dc;
	long TabIndex = TabNr -1;
	long CelIndex = CelNr -1;
	long status;
	if ((TabIndex <0 ) || (TabIndex >= m_ntab)) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	if (m_tab[TabIndex].HasRecode) TabIndex += m_ntab;

	if ((CelIndex < 0) || (CelIndex > m_tab[TabIndex].GetSizeTable())) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	dc = m_tab[TabIndex].GetCell(CelIndex);
    status = dc->GetStatus();
	dc->SetCTAValue (CTAVal);
	if (OrgVal != CTAVal){
	 if (status == CS_SAFE) {
		 dc->SetStatus (CS_SECONDARY_UNSAFE); *Sec= *Sec + 1;}
	 if ((status == CS_SAFE_MANUAL)){
			dc->SetStatus (CS_SECONDARY_UNSAFE_MANUAL); *Sec= *Sec + 1;}
	}

	*pVal = VARIANT_TRUE;
	return S_OK;
}

// Set Lower and upper protection level. This is the one calculated by Anneke
STDMETHODIMP TauArgus::SetRealizedLowerAndUpper(long TabNr, long CelNr,
																	double RealizedUpper, double RealizedLower,
																	VARIANT_BOOL *pVal)
{
	CDataCell *dc;
	//	CTable *tab;
	long TabIndex = TabNr -1;
	long CelIndex = CelNr -1;
	if ((TabIndex <0 ) || (TabIndex >= m_ntab)) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	if (m_tab[TabIndex].HasRecode) TabIndex += m_ntab;

	//	tab = m_tab[TabIndex];

	if ((CelIndex < 0) || (CelIndex > m_tab[TabIndex].GetSizeTable())) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	dc = m_tab[TabIndex].GetCell(CelIndex);
	if ((dc->GetStatus() == CS_UNSAFE_FREQ) || (dc->GetStatus() == CS_UNSAFE_PEEP) ||
		(dc->GetStatus() == CS_UNSAFE_RULE) || (dc->GetStatus() == CS_UNSAFE_SINGLETON) ||
		(dc->GetStatus() == CS_UNSAFE_ZERO) ||
		(dc->GetStatus() == CS_SECONDARY_UNSAFE) ||
		(dc->GetStatus() == CS_UNSAFE_MANUAL)  || (dc->GetStatus() == CS_SECONDARY_UNSAFE_MANUAL)) {
		dc->SetRealizedLowerValue(RealizedLower);
		dc->SetRealizedUpperValue(RealizedUpper);
		*pVal = VARIANT_TRUE;
		return S_OK;
	}
	else  {

		*pVal = VARIANT_FALSE;
		return S_OK;
	}

}

// Check if Realized Upper and Lower bound are a better
// approximation than the Protection Level.

STDMETHODIMP TauArgus::CheckRealizedLowerAndUpperValues(long TabNr, long *pVal)
{

	long TabIndex = TabNr-1;

	double UPL;
	double LPL;
	//double Sliding;
	//double Capacity;
	long NumFaults =0;
	long i;

	CDataCell *dc;

	for (i=0; i< m_tab[TabIndex].nCell; i++)  {

		// get UPL and LPL
		dc = m_tab[TabIndex].GetCell(i);
		LPL = dc->GetLowerProtectionLevel();
		UPL = dc->GetUpperProtectionLevel();
		if ((UPL > dc->GetRealizedUpperValue()) || (LPL < dc->GetRealizedLowerValue()))  {
			NumFaults ++;
		}
	}

	*pVal = NumFaults;
	return S_OK;
}

// given an array of codes, calculate the corresponding indexes or cell number
STDMETHODIMP TauArgus::ComputeCodesToIndices(long TableIndex, VARIANT *sCode, long *dimIndex, VARIANT_BOOL *pVal)
{
	long ind = TableIndex-1;
	SAFEARRAY *sa = sCode->parray;
	long l;
	HRESULT hresult;
	BSTR bstrtemp;
	CTable *tab;
	long dim,j;
	CVariable *var;
	CString tempcode;
	bool IsMissing;
	// TODO: Add your implementation code here
	tab = &(m_tab[ind]);
	dim = tab->nDim;
	CString *sCodes = new CString[dim];

		//  Take out the codes and set in a CString array
	for (l=1; l<=dim; l++) {
		hresult = SafeArrayGetElement(sa, &l, &bstrtemp);
		sCodes[l-1]=bstrtemp;
	}

	for (j=0; j< tab->nDim; j++)  {
		var = &(m_var[tab->ExplVarnr[j]]);
		tempcode = sCodes[j];
		if (tempcode == "") {
			var->TableIndex = 0;
		}
		else {
	  		if (var->IsHierarchical && var->nDigitSplit == 0) {
				var->TableIndex = var->FindAllHierarchicalCode(tempcode);
			}
			else {
				var->TableIndex = BinSearchStringArray(var->sCode, tempcode, var->nMissing,
					IsMissing);
			}
      }
		if (var->TableIndex < 0) {
			delete [] sCodes;
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
		dimIndex[j] = var->TableIndex;
	}
	delete [] sCodes;

	*pVal = VARIANT_TRUE;
	return S_OK;
}

//Give input file information. This important when you are reading a file with free format
void STDMETHODCALLTYPE TauArgus::SetInFileInfo(bool IsFixedFormat, const char* Separator)
{
	InFileIsFixedFormat = IsFixedFormat;
	InFileSeperator = Separator;
}

// Write Table in AMPL format. This is used in networking
STDMETHODIMP TauArgus::WriteTableInAMPLFormat(BSTR AMPLFileName, long TableIndex, VARIANT_BOOL *pVal)
{
	FILE *fdampl;
	CString amplpath;
	amplpath = AMPLFileName;
	fdampl = fopen(amplpath,"w");
	CVariable *var;
	long i;
	CAMPL oampl;
	CTable *tab;
	tab = GetTable(TableIndex-1);
	//tab = &(m_tab[TableIndex - 1]);
	if(tab->nDim != 2) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	for (i=0; i<2; i++) {
		 var = &(m_var[tab->ExplVarnr[i]]);
		 if (var ->IsHierarchical) {
			 *pVal = VARIANT_FALSE;
			 return S_OK;
		 }
	}


	oampl.WriteTableInAMPL(tab,m_var,fdampl);
	fclose(fdampl);
	*pVal = VARIANT_TRUE;
	return S_OK;
}


STDMETHODIMP TauArgus::SetProtectionLevelsForFrequencyTable(long TableIndex, long Base,
																					 long K, VARIANT_BOOL *pVal)
{
	long i = TableIndex - 1;
	long c;
	CDataCell *dc;
	if ((i<0) || (i >= m_ntab))	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	if (!m_tab[i].IsFrequencyTable)	{
		*pVal = VARIANT_FALSE;
		return S_OK;

	}

	for (c = 0; c < m_tab[i].nCell; c++) {
		dc = m_tab[i].GetCell(c);
		if (!m_tab[i].SetProtectionLevelCellFrequency(*dc, Base, K))	{
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}

	*pVal = VARIANT_TRUE;
	return S_OK;


}

STDMETHODIMP TauArgus::SetProtectionLevelsForResponseTable(long TableIndex, long *DimIndex,
															   double LowerBound,double UpperBound,
															   VARIANT_BOOL *pVal)
{
	int tab = TableIndex - 1, i;
   CDataCell *dc;

	// check parameters
   if (tab < 0 || tab >= m_ntab) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	//if (m_fname[0] == 0)      		return false;
   if (!m_CompletedCodeList) {
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	CTable *table = GetTable(tab);
	//dc = table ->GetCell(DimIndex);
	//iTemporaryStatus = table->ComputeCellSafeCode(*dc);

	// check DimIndices
	for (i = 0; i < table->nDim; i++) {
		int nCodes = m_var[table->ExplVarnr[i]].GetnCode();
		ASSERT(DimIndex[i] >= 0 && DimIndex[i] < nCodes);
		if (DimIndex[i] < 0 || DimIndex[i] >= nCodes)  {
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}
	dc = table ->GetCell(DimIndex);
//	dc->SetCost(Cost);
//	*pVal = VARIANT_TRUE;
//	return S_OK;
//}

	dc->SetLowerProtectionLevel (LowerBound);
	dc->SetUpperProtectionLevel (UpperBound);
//	if (dc->GetResp() < LowerBound) {
//		if (!m_tab[i].SetProtectionLevelCellFrequency(*dc, Base, K))	{
//		*pVal = VARIANT_FALSE;
//		return S_OK;
//	}

	*pVal = VARIANT_TRUE;
	return S_OK;


}


STDMETHODIMP TauArgus::MaximumProtectionLevel(long TableIndex, double *Maximum)
{

	double maxprot = 0;

	long c,i, St;
	i = TableIndex -1;
	CDataCell *dc;

	for (c=0; c<m_tab[i].nCell; c++)	{
		dc = m_tab[i].GetCell(c);
		St = dc->GetStatus();
		// 10 juni 2005 toegevoegd: alleen voor onveilige cellen AHNL
		if ( St >= CS_UNSAFE_RULE && St <= CS_UNSAFE_MANUAL) {
			maxprot = __max(maxprot,dc->GetLowerProtectionLevel() + dc ->GetUpperProtectionLevel());
		}

	}

	*Maximum = maxprot;
	return S_OK;
}

//GetMinimumCellValue
double STDMETHODCALLTYPE TauArgus::GetMinimumCellValue(long TableIndex, double *Maximum)
{

	double mincell = 10E+60, maxcell = -10E+60;

	long c, St;
	CDataCell *dc;

	for (c=0; c<m_tab[TableIndex].nCell; c++)	{
		dc = m_tab[TableIndex].GetCell(c);
		St = dc->GetStatus();
		// 10 juni 2005 toegevoegd: alleen voor onveilige cellen AHNL
		if ( St < CS_EMPTY) {
			mincell = __min(mincell, dc->GetResp());
			maxcell = __max(maxcell, dc->GetResp());
		}

	}

	*Maximum = maxcell;
	return mincell;
}

STDMETHODIMP TauArgus::SetRoundedResponse(BSTR RoundedFile, long TableIndex, VARIANT_BOOL *pVal)
{
	FILE *frounded;
	CString RoundedFileName;
	RoundedFileName = RoundedFile;
	long CellNum;
	CDataCell *dc;
	double RoundedResp; //was float
	double d;
	char s[30];
	long itel = 0;

	CTable *tab;
	long i = TableIndex -1;
	if ((i < 0) || (i >= m_ntab))	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	tab = GetTable(i);

	frounded = fopen(RoundedFileName, "r");
	if (frounded == NULL)	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	// Go to the beginning
	fseek( frounded, 0L, SEEK_SET );
	while (!feof(frounded) ) {
		if (itel == tab->nCell)	{ //was nCell - 1
			break;
		}
		fscanf( frounded, "%ld", &CellNum );
		fscanf( frounded, "%s", s);
		fscanf ( frounded, "%f", &d); // original value
		fscanf (frounded,"%s", s); // to
		//fscanf (frounded, "%f", &RoundedResp);
		fscanf (frounded, "%s", s);
		RoundedResp = atof(s);
		//fscanf( frounded, "%s", s);

		if ((CellNum < 0) || CellNum >= tab->nCell)	{
			fclose(frounded);
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
		dc = tab->GetCell(CellNum);
		if ((RoundedResp != 0) && ((dc->GetStatus() == CS_EMPTY) ||
				(dc->GetStatus() == CS_EMPTY_NONSTRUCTURAL)))	{

			CDataCell *newcell = new CDataCell(tab->NumberofMaxScoreCell,tab->NumberofMaxScoreHolding,tab->ApplyHolding, tab->ApplyWeight);
			tab->CellPtr.SetAt(CellNum,newcell);
			dc = tab->GetCell(CellNum);
			dc->SetStatus(CS_SAFE);

		}

		dc->SetRoundedResponse((long) RoundedResp);

		itel++;
	}

	fclose(frounded);
	*pVal = VARIANT_TRUE;
	return S_OK;

}


STDMETHODIMP TauArgus::WriteHierarchicalTableInAMPLFormat(BSTR AMPLFilename, BSTR TempDir,
															 long Tableindex, double MaxScale, long *ErrorCode,  VARIANT_BOOL *pVal)
{
	long tabind = Tableindex-1;

	long j;
	CTable *tab;
	CVariable * var;
	CString OutFile;
	CString sTempDir;
	CString sFileName;
	OutFile = AMPLFilename;
	sTempDir = TempDir;
	sFileName = AMPLFilename;
	FILE *fd;
	if ((tabind <0) || (tabind >= m_ntab))	{
		*ErrorCode = NOTABLE;
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	tab = GetTable(tabind);
	//tab = &(m_tab[tabind]);

	if (tab->nDim >2)	{
		*ErrorCode = TABLEWITHMORETHAN2VARIABLES;
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	// Check if 1st variable is Hierarchical and the last is not;
	var = &(m_var[tab->ExplVarnr[0]]);
	if (!var->IsHierarchical)	{
		*ErrorCode = FIRSTVARNOTHIERARCHICAL;
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	var = &(m_var[tab->ExplVarnr[1]]);
	if  (var->IsHierarchical)	{
		*ErrorCode =SECONDVARNOTFLAT;
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	for (j = 0; j<tab->nDim; j++)	{
		var = &(m_var[tab->ExplVarnr[j]]);
		if (!var->PrepareSubCodeList())	{
			*ErrorCode = NOMEMORYFORSUBCODES;
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
		//create subcodelist for table
		if (!var->FillSubCodeList())	{
			*ErrorCode = COULDNOTCREATESUBCODES;
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}
	fd = fopen(OutFile,"w");
	if (fd == 0)	{
		*ErrorCode = COULDNOTOPENFILE;
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	if (!WriteHierTableInAMPL(fd,tabind,sTempDir, MaxScale))	{
		*ErrorCode = COULDNOTCREATEAMPL;
		fclose(fd);
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	fclose(fd);
	*pVal = VARIANT_TRUE;
	return S_OK;
}


STDMETHODIMP TauArgus::SetSecondaryFromHierarchicalAMPL(BSTR FileName,
																			  long TableIndex, long *ErrorCode,
																			  VARIANT_BOOL *pVal)
{
	FILE *fd;
	CTable *tab;
	long *SubTableCellIndexTuple;
	long *SubTableTuple;
	long *TableCellIndex;
	CDataCell *dc;
	long Cellnum;
	long tabind = TableIndex -1;
	CString sFileName = FileName;
	long subtablenum, nSuppr;
	char s[30];
	long i,j;
	CVariable *var;

	if ((tabind < 0) || (tabind >= m_ntab))	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	fd = fopen(sFileName, "r");
//	fd = fopen("E:/Temp/NetSolHH.tmp","r");
	if (fd == 0)	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	tab = GetTable(tabind);
	//tab = &(m_tab[tabind]);
	SubTableCellIndexTuple = new long [tab->nDim];
	SubTableTuple = new long [tab->nDim];
	TableCellIndex = new long [tab->nDim];

	fseek( fd, 0L, SEEK_SET );
	fscanf(fd, "%ld", &nSuppr);
	//while (!feof(fd) ) {
	for (j= 0; j <nSuppr; j++)	{
		fscanf(fd, "%ld", &subtablenum );
		fscanf( fd, "%c", s);
		for (i= 0; i< tab->nDim; i++)	{
			fscanf ( fd, "%ld", &SubTableCellIndexTuple[i]);
			if (i == 0)	{
				fscanf( fd, "%c", s);
			}
		}
		SubTableTupleForSubTable(tabind,subtablenum,SubTableTuple);
		for (i= 0; i<tab->nDim; i++)	{
			var = &(m_var[tab->ExplVarnr[i]]);
			if (SubTableCellIndexTuple[i] < var->m_SubCodes[SubTableTuple[i]].NumberOfSubCodes() )	{
				TableCellIndex[i] = var->m_SubCodes[SubTableTuple[i]].GetSubCodeIndex(SubTableCellIndexTuple[i]);
			}
			else	{
				TableCellIndex[i] = var->m_SubCodes[SubTableTuple[i]].GetParentIndex();
			}
		}

		Cellnum = tab->GetCellNrFromIndices(TableCellIndex);

		dc = tab->GetCell(Cellnum);
		if ((dc->GetStatus() != CS_EMPTY) && (dc->GetStatus() != CS_EMPTY_NONSTRUCTURAL))	{
			dc->SetStatus(CS_SECONDARY_UNSAFE);
		}
		else	{
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}
	//}
	delete [] SubTableCellIndexTuple;
	delete [] SubTableTuple;
	delete [] TableCellIndex;
	*pVal = VARIANT_TRUE;
	return S_OK;
}

bool STDMETHODCALLTYPE TauArgus::SetAllEmptyNonStructural(long TableIndex)
{
	long i;
	CTable *tab;
	CDataCell *dc;
	CDataCell *dcempty;
	if ((TableIndex <0) || (TableIndex >= m_ntab))	{
		return false;
	}

	tab = GetTable(TableIndex); //&(m_tab[index]);
	for (i=0; i<tab->nCell; i++)	{
		dc = tab->GetCell(i);
		if (dc->GetStatus() == CS_EMPTY)	{
			dcempty = new CDataCell(tab->NumberofMaxScoreCell,tab->NumberofMaxScoreHolding,tab->ApplyHolding, tab->ApplyWeight);
			tab->CellPtr.SetAt(i,dcempty);
			dc = tab->GetCell(i);
			dc->SetStatus(CS_EMPTY_NONSTRUCTURAL);
//			tab->ComputeCellSafeCode (dc);
		}
	}
	return true;
}

bool STDMETHODCALLTYPE  TauArgus::SetSingleEmptyAsNonStructural(long TableIndex, long *DimIndex)
{
	/*long index = TableIndex;
	long i;
	CTable *tab;
	CDataCell *dc;
	CDataCell *dcempty;
	if ((index <0) || (index >= m_ntab))	{
		return false;
	}

	tab = &(m_tab[index]);
	for (i = 0; i < tab->nDim; i++) {
		int nCodes = m_var[tab->ExplVarnr[i]].GetnCode();
		ASSERT(DimIndex[i] >= 0 && DimIndex[i] < nCodes);
		if (DimIndex[i] < 0 || DimIndex[i] >= nCodes) {
			return false ;
		}
	}

	dc = tab->GetCell(DimIndex);
	if (dc->GetStatus() == CS_EMPTY)	{
		dcempty = new CDataCell(tab->NumberofMaxScoreCell,tab->NumberofMaxScoreHolding,tab->ApplyHolding, tab->ApplyWeight);
		tab->CellPtr.SetAt(tab->GetCellNrFromIndices(DimIndex),dcempty);
		dc = tab->GetCell(tab->	GetCellNrFromIndices(DimIndex));
		dc->SetStatus(CS_EMPTY_NONSTRUCTURAL);
	}
	else	{
		return false;
	}
	return true;
*/
	return true;
}


STDMETHODIMP TauArgus::SetSingleNonStructuralAsEmpty(long TableIndex, long *DimIndex, VARIANT_BOOL *pVal)
{
	/*
	long index = TableIndex -1;
	long i;
	CTable *tab;
	CDataCell *dc;
	if ((index <0) || (index >= m_ntab))	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	tab = &(m_tab[index]);
	for (i = 0; i < tab->nDim; i++) {
		int nCodes = m_var[tab->ExplVarnr[i]].GetnCode();
		ASSERT(DimIndex[i] >= 0 && DimIndex[i] < nCodes);
		if (DimIndex[i] < 0 || DimIndex[i] >= nCodes) {
			*pVal = VARIANT_FALSE;
			return S_OK;
		}
	}

	dc = tab->GetCell(DimIndex);
	if (dc->GetStatus() == CS_EMPTY_NONSTRUCTURAL)	{
		//dcempty = new CDataCell(tab->NumberofMaxScoreCell,tab->NumberofMaxScoreHolding,tab->ApplyHolding, tab->ApplyWeight);
		dc = tab->GetCell(tab->	GetCellNrFromIndices(DimIndex));
		delete[]  dc;
		tab->CellPtr.SetAt(tab-> GetCellNrFromIndices(DimIndex),NULL);
	}
	else	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}
	*pVal = VARIANT_TRUE;
	return S_OK;
*/
	return S_OK;
}



STDMETHODIMP TauArgus::SetAllNonStructuralAsEmpty(long TableIndex, VARIANT_BOOL *pVal)
{

	long index = TableIndex -1;
	long i;
	CTable *tab;
	CDataCell *dc;
	if ((index <0) || (index >= m_ntab))	{
		*pVal = VARIANT_FALSE;
		return S_OK;
	}

	tab = GetTable(index); // &(m_tab[index]);
	for (i=0; tab->nCell; i++)	{
		dc = tab->GetCell(i);
		if (dc->GetStatus() == CS_EMPTY_NONSTRUCTURAL)	{
			//dcempty = new CDataCell(tab->NumberofMaxScoreCell,tab->NumberofMaxScoreHolding,tab->ApplyHolding, tab->ApplyWeight);
			dc = tab->GetCell(i);
			delete [] dc;
			tab->CellPtr.SetAt(i,NULL);
		}
	}
	*pVal = VARIANT_TRUE;
	//return S_OK;

	return S_OK;

	return S_OK;
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

}

// return -1: error
// returns 0: EOF
// return 1: oke
// a line in the micro record is read and set in the string
int TauArgus::ReadMicroRecord(FILE *fd, UCHAR *str)
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
int TauArgus::DoMicroRecord(UCHAR *str, int *varindex)
{
	int i, bp, ap;
	char code[MAXCODEWIDTH];
	CString tempcode;
	CVariable *var;

	for (i = 0; i < m_nvar; i++) {
		*varindex = i;
		var = &(m_var[i]);
		if (var->IsCategorical || var->IsNumeric) {
			if (InFileIsFixedFormat) {
				bp = var->bPos;         // startposition
				ap = var->nPos;         // number of positions
				strncpy(code, (const char *)&str[bp], ap); // get code from record
				code[ap] = 0;
			}
			else {
				ap = var->nPos;         // number of positions
				if (ReadVariableFreeFormat(str,i,&(tempcode))) {
					strcpy(code,(const char*)tempcode);
					code[ap] = 0;
				}
				else {
					return PROBLEMSREADINGFREEFORMAT;
				}
			}
		}
		else {
			continue;
		}

		if (var->IsCategorical) { // only a categorical var has a codelist
			if (var->IsHierarchical && var->nDigitSplit == 0) {
			// code should be in already defined list of codes
				if (var->FindHierarchicalCode(code) < 0) { // not found by normal basis ones
					if (var->Missing1 != code && var->Missing2 != code) {
						return CODENOTINCODELIST;
					}
				}
			}
			else { // check if its a missing code
				if (var->Missing1 != code && var->Missing2 != code) {
					// Only nonmissingcodes
					if (!var->AddCode(code, false) ) {   // adds if new, else does nothing
						return PROGRAMERROR;
					}
				}
			}
		}

		if (var->IsNumeric) {
			double d;
			// exclude missings for calculating min/max
			if (strcmp(code, (LPCTSTR) var->Missing1) != 0 &&
				strcmp(code, (LPCTSTR) var->Missing2) != 0 ) {
				if (!ConvertNumeric(code, d) ) {
					return ISNOTNUMERIC;   // is not numeric! (variabelenummer meegeven voor gebruiker)
				}
				if (d > var->MaxValue) var->MaxValue = d;
				if (d < var->MinValue) var->MinValue = d;
			}
		}
	}


	return true;
}


// returns: -1 in case not found
// IsMissing on true if x = Missing1 or x = Missing2
int TauArgus::BinSearchStringArray(CStringArray &s, CString x, int nMissing, bool &IsMissing)
{
	int mid, left = 0, right = s.GetSize() - 1 - nMissing;
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
		mis = s.GetSize() - nMissing;
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

/*
void TauArgus::QuickSortMaxScore(double &doub, int &intg, int first, int last)
{

  int i, j;
  CString mid, temp;

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
// sort strings
void TauArgus::QuickSortStringArray(CStringArray &s, int first, int last)
{
	int i, j;
	CString mid, temp;

	ASSERT(first >= 0 && last >= first);

	do {
		i = first;
		j = last;
		mid = s[(i + j) / 2];
		do {
			while (s[i] < mid) i++;
			while (s[j] > mid) j--;
			if (i < j) {
				temp = s[i];
				s[i] = s[j];
				s[j] = temp;
			}
			else {
				if (i == j) {
				i++;
				j--;
			}
			break;
		}
   } while (++i <= --j);
   if (j - first < last - i) {
      if (j > first) {
			QuickSortStringArray(s, first, j);
      }
      first = i;
	}
	else {
      if (i < last) {
			QuickSortStringArray(s, i, last);
      }
      last = j;
    }
	} while (first < last);
}

// converts code to double; leading and trailing spaces are ignored;
// the last chararter should be a zero-byte
BOOL TauArgus::ConvertNumeric(char *code, double &d)

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

BOOL TauArgus::FillInTable(long Index, CString *sCodes, double Cost,
							   double Resp,double Shadow, long Freq,
							   double *TopNCell, double *TopNHolding,
								double LPL, double UPL,
								long Status, long & error, long & ErrorVarNo)
{
	int j;
	CVariable *var;
	CDataCell *dc;
	CTable *tab = &(m_tab[Index]);
	CString tempcode;
	bool IsMissing;
	long *tabind = new long [tab->nDim];
	long CellIndex;

	// Find Variable indexes for a code
	ErrorVarNo = 0;
	for (j=0; j< tab->nDim; j++)  {
		var = &(m_var[tab->ExplVarnr[j]]);
		tempcode = sCodes[j];
		if (var->IsHierarchical && var->nDigitSplit == 0) {
			var->TableIndex = var->FindAllHierarchicalCode(tempcode);
		}
		else {
			var->TableIndex = BinSearchStringArray(var->sCode, tempcode, var->nMissing, IsMissing);
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
	dc = tab ->GetCell(tabind);
	// if cell already exists
	if ((dc->IsFilled) )  {
		error = CELLALREADYFILLED;
		return false;
	}
	// create new cell
	else {
		CDataCell *dcempty = new CDataCell(tab->NumberofMaxScoreCell,tab->NumberofMaxScoreHolding,tab->ApplyHolding, tab->ApplyWeight);
		CellIndex = tab->GetCellNrFromIndices(tabind);

		// check if cell index is valid
		tab->CellPtr.SetAt(CellIndex,dcempty);
		dc = tab ->GetCell(tabind);
		dc->IsFilled = true;

	}

	// NOT too sure about this
	// set value
	dc ->SetResp(Resp);
	dc ->SetShadow(Shadow);
	dc ->SetCost(Cost);
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
void TauArgus::FillTables(UCHAR *str)
{
	int i, j;
	char code[MAXCODEWIDTH];
	bool IsMissing;
	double Weight = 0;
	CString tempcode;
	CVariable *var;
	CDataCell dc;
	CString  tempPeepCode;


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
				if (ReadVariableFreeFormat(str,m_VarNrHolding,&(tempcode))) {
					strcpy(code,(const char*)tempcode);
					code[var->nPos] = 0;
				}
			}

			if (strcmp(LastHoldingCode,code) != 0){
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
					if (ReadVariableFreeFormat(str,tab->ExplVarnr[j],&(tempcode))) {
						strcpy(code,(const char*)tempcode);
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
				if (ReadVariableFreeFormat(str,tab->PeepVarnr,&(tempcode))) {
					strcpy(code,(const char*)tempcode);
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
					if (ReadVariableFreeFormat(str,tab->ShadowVarnr,&(tempcode))) {
						strcpy(code,(const char*)tempcode);
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
					if (ReadVariableFreeFormat(str,tab->CostVarnr,&(tempcode))) {
						strcpy(code,(const char*)tempcode);
						code[var->nPos] = 0;
					}
				}
				var->Value = atof(code);
				var->ValueToggle = 1;
			}
		}

		// RespVarnr
		if ((tab->ResponseVarnr > 0) && (tab->ResponseVarnr <m_nvar))	{
			var = &(m_var[tab->ResponseVarnr]);
			if (var->ValueToggle == 0) { // first time, so compute value
				if (InFileIsFixedFormat) {
					strncpy(code, (char *)&str[var->bPos], var->nPos);
					code[var->nPos] = 0;
				}
				else {
					if (ReadVariableFreeFormat(str,tab->ResponseVarnr,&(tempcode))) {
						strcpy(code,(const char*)tempcode);
						code[var->nPos] = 0;
					}
				}
				var->Value = atof(code);
				var->ValueToggle = 1;
			}
		}
		/*else	{
			var->Value = 1;
			var->ValueToggle = 1;
		}*/
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
				if (ReadVariableFreeFormat(str,m_VarNrWeight,&(tempcode))) {
					strcpy(code,(const char*)tempcode);
					code[var->nPos] = 0;
				}
			}
			var->Value = atof(code);
			var->ValueToggle = 1;
		}
	}

	// now tabulate all tables from list
	for (i = 0; i < m_ntab; i++) {
		// set peeps to peep 1 and peep 2

		if (m_tab[i].ApplyPeeper) {

			if ((tempPeepCode.Compare(m_var[m_tab[i].PeepVarnr].PeepCode1) == 0) ||
			  (tempPeepCode.Compare(m_var[m_tab[i].PeepVarnr].PeepCode2) == 0))	{
			  dc.SetPeepCell(m_var[m_tab[i].ShadowVarnr].Value);
			  dc.SetPeepHolding(m_var[m_tab[i].ShadowVarnr].Value);
			  if ( tempPeepCode.Compare(m_var[m_tab[i].PeepVarnr].PeepCode1) == 0)	{
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
		if ((m_tab[i].ResponseVarnr > 0) && (m_tab[i].ResponseVarnr < m_nvar))	{
			dc.SetResp(m_var[m_tab[i].ResponseVarnr].Value);
		}
		else	{	//freq table
				dc.SetResp(1);
		}
		if ((m_tab[i].ShadowVarnr > 0) && (m_tab[i].ShadowVarnr < m_nvar))	{

 			dc.SetShadow(m_var[m_tab[i].ShadowVarnr].Value);
		}
		if (m_tab[i].CostVarnr >= 0) {
 		  dc.SetCost(m_var[m_tab[i].CostVarnr].Value);
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
		bool IsMissing;
		int i, hIndex, n = 0;
		CString s;
		if (var->nDigitSplit > 0) {
			for (i = 0; i < var->nDigitSplit - 1; i++) {
				n += var->DigitSplit[i];
				s = var->sCode[var->TableIndex].Left(n);
				hIndex = BinSearchStringArray(var->sCode, s, var->nMissing, IsMissing);
				ASSERT(hIndex >= 0 && hIndex < var->GetnCode());
				AddTableCells(t, AddCell, niv + 1, cellindex + hIndex); // for subtotals in codelist
			}
		}
		else {
			i = var->TableIndex;
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
	CDataCell *dc;
	double weight = 1;
	bool IsexistingHolding = false;
	double x ;
	double dtemp;
	ASSERT(cellindex >= 0 && cellindex < t.nCell);
	dc = t.GetCell(cellindex);
	// Does a new cell have to be created
	if (!dc->IsFilled) {
		CDataCell *dcempty = new CDataCell(t.NumberofMaxScoreCell,t.NumberofMaxScoreHolding,t.ApplyHolding, t.ApplyWeight);
		t.CellPtr.SetAt(cellindex,dcempty);
		dc = t.GetCell(cellindex);
	}


	// copy weight
	if (t.ApplyWeight) {
		weight = m_var[m_VarNrWeight].Value;
	}
	// adjust the max score array to make way for a new greater score
	if (t.NumberofMaxScoreCell > 0) {
		x = fabs(m_var[t.ShadowVarnr].Value);
		for (i = 0; i < t.NumberofMaxScoreCell; i++) {
			if (x > dc->MaxScoreCell[i]) {
				// shift rest
				for (j = t.NumberofMaxScoreCell- 2; j >= i; j--) {
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
					dtemp = x;
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
void TauArgus::AddTableToTableCell(CTable &tabfrom, CTable &tabto,
												  long ifrom, long ito)
{
	CDataCell *dc1, *dc2;

	ASSERT(ifrom >= 0 && ifrom < tabfrom.nCell);
	dc1 = tabfrom.GetCell(ifrom);
	ASSERT(ito >= 0 && ito < tabto.nCell);
	dc2 = tabto.GetCell(ito);
	if (!dc2->IsFilled)	{
		CDataCell *dcempty = new CDataCell(tabto.NumberofMaxScoreCell,tabto.NumberofMaxScoreHolding, tabto.ApplyHolding, tabto.ApplyWeight);
		tabto.CellPtr.SetAt(ito,dcempty);
		dc2 = tabto.GetCell(ito);
	}
	// add and store
	*dc2 += *dc1;

}

// Frequencys and Topn are checked when holdings exist
void TauArgus::MergeLastTempShadow()
{
	int i,j,k,l;
	CTable *tab;
	CDataCell *dc;
	double x;
	for (i=0; i<m_ntab; i++)
	{
		tab = &(m_tab[i]);
		if (tab ->ApplyHolding)
		{
			for (j= 0; j<(tab->nCell);j++)
			{
				dc = tab->GetCell(j);
				if (dc->GetHoldingNr() != -1){
					if (tab->NumberofMaxScoreHolding > 0) {

						x = fabs(dc->GetTempShadow());
						for (k = 0; k < tab->NumberofMaxScoreHolding; k++) {
							if (x > dc->MaxScoreHolding[k]) {
							// shift rest
								for (l = tab->NumberofMaxScoreHolding - 2; l >= k; l--) {
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
	int i;
	CTable *tab;
	for (i=0; i<m_ntab; i++){
		tab = & (m_tab[i]);
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

BOOL TauArgus::ParseRecodeString(long VarIndex, LPCTSTR RecodeString, long FAR* ErrorType, long FAR* ErrorLine, long FAR* ErrorPos, int Phase)
{
	char *p;
	int PosInString = 0, LineNumber = 1;
	int oke;

	// first detect lines in RecodeString
	while (1) {
		oke = ParseRecodeStringLine(VarIndex, &RecodeString[PosInString], ErrorType,  ErrorPos, Phase);
		if (!oke) {
			*ErrorLine = LineNumber;
			return false;
		}
		LineNumber++;
		p = strstr(&RecodeString[PosInString], SEPARATOR);
		if (p == 0) break;
		PosInString = p - RecodeString + strlen(SEPARATOR);
	}

	return true;
}

// Parse a line of a recode string (until str[i] == 0 || str[i] == '\n' || str[i] == '\r')
BOOL TauArgus::ParseRecodeStringLine(long VarIndex, LPCTSTR str, long FAR* ErrorType, long FAR* ErrorPos, int Phase)
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

	//if (strlen(DestCode) > (UINT) nPos) {
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
		if (strlen(SrcCode1) > (UINT) nPos || strlen(SrcCode2) > (UINT) nPos) {
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

// add spaces before to make the string the right length
void TauArgus::AddSpacesBefore(char *str, int len)
{
	int lstr = strlen(str);

	if (lstr >= len) return;  // nothing to do

	char tempstr[MAXCODEWIDTH + 1];
	strcpy(tempstr, str);
	sprintf(str, "%*s%s", len - lstr, " ", tempstr);
}

void TauArgus::AddSpacesBefore(CString& str, int len)
{
	int width = str.GetLength();

	if (width >= len) return;  // nothing to do

	{
		char tempstr[100];
		sprintf(tempstr, "%*s", len - width, " ");
		str.Insert(0, tempstr);
	}
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
	int n_codes = v->sCode.GetSize() - v->nMissing;
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
	ASSERT(c1 >= 0 && c1 < v->sCode.GetSize() && c2 >= c1 && c2 < v->sCode.GetSize());

	// search destcode in list
	ASSERT(v->nMissing > 0 && v->nMissing < 3);

	AddSpacesBefore(DestCode, v->Recode.CodeWidth);
	DestIndex = BinSearchStringArray(v->Recode.sCode, DestCode, v->Recode.nMissing, DestMissing);

	// never make from a missing value a valid value!
	if (!DestMissing && (Src1Missing || Src2Missing) ) {
		return R_MISSING2VALID;
	}

	ASSERT(DestIndex >= 0 && DestIndex < v->Recode.sCode.GetSize() );
	if (DestIndex < 0 || DestIndex >= v->Recode.sCode.GetSize() ) {
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
BOOL TauArgus::ComputeRecodeTables()
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

#ifdef _DEBUGG
		{ int i;
			for (i = 0; i < m_ntab; i++) {
				CString fname;
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

	n = m_var[varnr].Recode.sCode.GetSize();
	// not already in list?
	for (i = 0; i < n; i++) {                               // optimizable with binary search (number > ??)
		if (m_var[varnr].Recode.sCode[i] == newcode) break;   // found!
	}
	if (i == n) {  // not found
		m_var[varnr].Recode.sCode.Add(newcode);
	}

	return i;
}

int TauArgus::MakeRecodelistEqualWidth(int VarIndex, LPCTSTR Missing1, LPCTSTR Missing2)
{
	int i, length, ncode = m_var[VarIndex].Recode.sCode.GetSize(), maxwidth = 0;

	// if missings empty take missings of variable[VarIndex]
	if (Missing1[0] == 0 && Missing2[0] == 0) {
		length = strlen(m_var[VarIndex].Missing1);
		if (length > maxwidth) maxwidth = length;
		length = strlen(m_var[VarIndex].Missing2);
		if (length > maxwidth) maxwidth = length;
	}

	for (i = 1; i < ncode; i++) {
		if (length = m_var[VarIndex].Recode.sCode[i].GetLength(), length > maxwidth) {
			maxwidth = length;
		}
	}
	if (strlen(Missing1) > (UINT) maxwidth) maxwidth = strlen(Missing1);
	if (strlen(Missing2) > (UINT) maxwidth) maxwidth = strlen(Missing2);
	m_var[VarIndex].Recode.CodeWidth = maxwidth;

	// spaces before shorter ones
	for (i = 1; i < ncode; i++) {
		AddSpacesBefore(m_var[VarIndex].Recode.sCode[i], maxwidth);
	}

	return maxwidth;
}
// from source table create recoded destination table
BOOL TauArgus::ComputeRecodeTable(CTable& srctab, CTable& dsttab)
{
	int d;

	dsttab.nCell = 1;
	// compute number of cells
	for (d = 0; d < dsttab.nDim; d++) {
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
	ComputeRecodeTableCells(srctab, dsttab, niv + 1,
									iCellSrc * srctab.SizeDim[niv],
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
	int c,j,St;
	CDataCell *dc;
	long *DimNr;
	DimNr = new long [tab.nDim];
	CVariable *var;


	for (c = 0; c < tab.nCell; c++) {
		dc = tab.GetCell(c);
		tab.GetIndicesFromCellNr(c,DimNr);
		if (tab.SetMissingAsSafe)	{
			for (j = 0; j<tab.nDim; j++)	{
				var = &(m_var[tab.ExplVarnr[j]]);
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
		St = dc->GetStatus();
		//Anco 2-2-2005 Toegevoegd als status = 0
		if ( (St == CS_EMPTY) || (St == 0) )	{
			dc->SetStatus(tab.ComputeCellSafeCode(*dc) );
		}
	}
	delete[] DimNr;
}

// Protection Levels for cells are calculated. first Cell Status
// has to be calculated
void TauArgus::SetProtectionLevels(CTable &tab)
{
	int c;
	CDataCell *dc;

	for (c = 0; c < tab.nCell; c++) {
		dc = tab.GetCell(c);
		tab.SetProtectionLevelCell(*dc);
	}
}

// set that the table has been recoded
void TauArgus::SetTableHasRecode()
{
	int t, v;

	for (t = 0; t < m_ntab; t++) {
		for (v = 0; v < m_tab[t].nDim; v++) {
			if (m_var[m_tab[t].ExplVarnr[v]].HasRecode) {
				break;
			}
		}
		m_tab[t].HasRecode = (v != m_tab[t].nDim);
	}
}

// write table in comma seperated format
void TauArgus::WriteCSVTable(FILE *fd, CTable *tab, long *DimSequence, long *Dims, int niv, char ValueSep, long RespType)
{
	int i, n;

	// write Cell
	if (niv == tab->nDim) {
		WriteCSVCell(fd, tab, Dims, false, 0, RespType);
		fprintf(fd, "%c", ValueSep);
		return;
	}

	if (tab->nDim == 1) {
		WriteCSVColumnLabels(fd, tab, 0, ValueSep);
		fprintf(fd, "%c", ValueSep);
	}

	n = m_var[tab->ExplVarnr[DimSequence[niv]]].GetnCode();
	for (i = 0; i < n; i++) {
		if (niv == tab->nDim - 2) { // row variable?
			if (i == 0) {
				if (tab->nDim > 2) { // one or more layers in table?
					for (int j = 0; j < niv; j++) {
  						fprintf(fd, "Var %d,", DimSequence[j] + 1);
  						WriteCSVLabel(fd, tab, DimSequence[j], Dims[DimSequence[j]]);  // show all previous layer label(s)
  						fprintf(fd, ", ");
					}
					fprintf(fd, "\n");
				}
				WriteCSVColumnLabels(fd, tab, DimSequence[tab->nDim - 1], ValueSep);
			}
			WriteCSVLabel(fd, tab, DimSequence[niv], i);  // show row label
			fprintf(fd, "%c", ValueSep);
		}

		Dims[DimSequence[niv]] = i;
		WriteCSVTable(fd, tab, DimSequence, Dims, niv + 1, ValueSep, RespType);
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
void TauArgus::WriteCSVColumnLabels(FILE *fd, CTable *tab, long dim, char ValueSep)
{
	int i, n = m_var[tab->ExplVarnr[dim]].GetnCode();

	fprintf(fd, "%c", ValueSep); // first label in table always empty
	for (i = 0; i < n; i++) {
		WriteCSVLabel(fd, tab, dim, i);
		if (i < n - 1) fprintf(fd, "%c", ValueSep);
	}
	fprintf(fd, "\n");
}

void TauArgus::WriteCSVLabel(FILE *fd, CTable *tab, long dim, int code)
{
	CString s;
	ASSERT(dim >= 0 && dim < tab->nDim);
	ASSERT(code >= 0 && code < m_var[tab->ExplVarnr[dim]].GetnCode() );

	if (code == 0) {s = m_var[tab->ExplVarnr[dim]].TotalCode;
	}
	else	{
		s = m_var[tab->ExplVarnr[dim]].GetCode(code);
	}
	if (s.IsEmpty() ) s = m_ValueTotal;
	s.Replace('"', '\'');

	fprintf(fd, "\"%s\"", (LPCTSTR) s);
}

// write cells
void TauArgus::WriteCSVCell(FILE *fd, CTable *tab, long *Dim, bool ShowUnsafe, int SBSCode, long RespType)
{
	CDataCell *dc;
	int nDec = m_var[tab->ResponseVarnr].nDec;

	dc = tab->GetCell(Dim);
	switch (dc->GetStatus() ) {
		case CS_SAFE:
		case CS_SAFE_MANUAL:
		case CS_EMPTY_NONSTRUCTURAL:
		case CS_PROTECT_MANUAL:
			switch (RespType ){
			case 0: fprintf(fd, "%.*f", nDec, dc->GetResp()); break;
			case 1: fprintf(fd, "%d", dc->GetRoundedResponse());	break;
			case 2: fprintf(fd, "%.*f", nDec, dc->GetCTAValue ());break;
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
			case 1: fprintf(fd, "%d", dc->GetRoundedResponse());	break;
			case 2: fprintf(fd, "%.*f", nDec, dc->GetCTAValue ());break;
			}

			//			if (RespType)	{
//				if (ShowUnsafe) {
//					fprintf(fd, "%.*f", nDec, dc->GetResp());
//				}
//				else {
//					fprintf(fd, "x");
//				}
//			}
//			else	{
//				fprintf(fd,"%d", dc->GetRoundedResponse());
//			}
			break;
		case CS_EMPTY:
			fprintf(fd, "-");
			break;
	}
	if ( ShowUnsafe && ( SBSCode == 0) ) {
		switch (RespType){
			case 0:	fprintf(fd, ",%d", dc->GetStatus()); 
				    break;
			case 1:// fprintf(fd, ",%.*f", nDec, dc->GetResp()); break;
			case 2: 
     				if (dc->GetStatus() == CS_EMPTY) {fprintf(fd, ",-");}
	    			else                             {fprintf(fd, ",%.*f", nDec, dc->GetResp());}
		    		if (RespType = 2){fprintf(fd, ",%d", dc->GetStatus());}
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
	CDataCell *dc;
	dc = tab->GetCell(Dim);
	int f1, f2;
	double X, X1, X2, XS;
	bool DomRule, PQRule, UnsafeRule;
	DomRule = tab->DominanceRule;
	PQRule = tab->PQRule;
	f1 = dc->GetFreq();
	f2 = dc->GetFreqHolding();
	if (f2 > 0) { f1 = f2; }

	fprintf(fd, ",%d%c", f1, ValueSep);
//		fprintf(fd, "%c", ValueSep);
	UnsafeRule = false;
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
void TauArgus::WriteCellRecord(FILE *fd, CTable *tab,
											 long *Dims, int niv, char ValueSep, long SBSCode,
											 bool bSBSLevel,
											 BOOL SuppressEmpty, bool ShowUnsafe, long RespType)
{
	int i, n;

	// write Cell
	if (niv == tab->nDim) {
		if (SuppressEmpty) {
			if (tab->GetCell(Dims)->GetFreq() == 0) return;
		}
		WriteCellDimCell(fd, tab, Dims, ValueSep, SBSCode, bSBSLevel, ShowUnsafe, RespType);
		return;
	}

	n = m_var[tab->ExplVarnr[niv]].GetnCode();
	for (i = 0; i < n; i++) {
		Dims[niv] = i;
		WriteCellRecord(fd, tab, Dims, niv + 1, ValueSep, SBSCode, bSBSLevel, SuppressEmpty, ShowUnsafe, RespType);
	}
}

void TauArgus::WriteCellDimCell(FILE *fd, CTable *tab,
											  long *Dims, char ValueSep, long SBSCode,
											  bool SBSLevel,
											  bool ShowUnsafe, long RespType)
{
	int i, l, n = tab->nDim;
	for (i = 0; i < n; i++) {
		WriteCSVLabel(fd, tab, i, Dims[i]);
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
{ int i, j;
  CString s;
  for (i = 0; i < m_nvar; i++) {
    CVariable *v = &(m_var[i]);
		s.Format("%3d %3d %3d %3d %8.0f %8.0f", v->bPos + 1, v->nPos, v->nDec, v->GetnCode(), v->MaxValue, v->MinValue);
    TRACE("Var %2d. %s\n", i + 1, (LPCTSTR) s);
		for (j = 0; j < v->GetnCode(); j++) {
      TRACE("  %4d. [%s]\n", j + 1, (LPCTSTR) v->GetCode(j) );
    }
	}
}

// only for debug
BOOL TauArgus::ShowTable(const char *fname, CTable& tab)
{ FILE *fd;
  CDataCell *dc;

  fd = fopen(fname, "w");
	if (fd == 0) return false;

  switch (tab.nDim) {
	case 1:
		{ int i;
		  for (i = 0; i < tab.SizeDim[0]; i++) {
				dc = tab.GetCell(i);
				if (dc->GetResp() == 0) {
				  fprintf(fd, "%3d. %12s \n", i + 1, "-");
				} else {
				  fprintf(fd, "%3d. %12.2f \n", i + 1, dc->GetResp());
				}
				if (dc->GetFreq() == 0) {
					fprintf(fd, "%5s \n", "-");
				} else {
					fprintf(fd, "%5d \n", dc->GetFreq());
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
				fprintf(fd, "\n*** Layer %d Code [%s] ***\n\n", i + 1, (LPCTSTR) m_var[tab.ExplVarnr[0]].GetCode(i) );
        ShowTableLayer(fd, 1, 2, i * tab.SizeDim[1] * tab.SizeDim[2], tab);
      }
		}
		break;
	case 4:
		{ int i, j;
			for (i = 0; i < tab.SizeDim[0]; i++) {
				fprintf(fd, "\n*** Layer %d Code %s ***\n", i + 1, (LPCTSTR) m_var[tab.ExplVarnr[0]].GetCode(i) );
				for (j = 0; j < tab.SizeDim[1]; j++) {
				  fprintf(fd, "   *** Layer %d *** Code %s\n\n", j + 1, (LPCTSTR) m_var[tab.ExplVarnr[1]].GetCode(j));
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
	else    	    c = sprintf(str, "%6d ", val);
  return c;
}


// only for debug
void TauArgus::ShowTableLayer(FILE *fd, int var1, int var2, int cellnr, CTable& tab)
{ int r, k, c, colwidth = 0;
  char str[200];
  CStringArray  *sCol, *sRow;  // pointer to codes for row and column

  sCol = m_var[tab.ExplVarnr[var2]].GetCodeList();
	sRow = m_var[tab.ExplVarnr[var1]].GetCodeList();

  { // compute width column
		k = c = 0;
		CDataCell *dc;
		dc = tab.GetCell(c + k);
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
  fprintf(fd, "%14s", (LPCTSTR) m_ValueTotal);
  for (k = 0; k < tab.SizeDim[var2]; k++) {
	  fprintf(fd, "%-*s", colwidth, (LPCTSTR) sCol->GetAt(k));
	}
  fprintf(fd, "\n\n");

  for (r = 0; r < tab.SizeDim[var1]; r++) {
		c = cellnr + r * tab.SizeDim[var2];
		if (r == 0) {
		  fprintf(fd, "%s  => ", (LPCTSTR) m_ValueTotal);
		} else {
		  fprintf(fd, "%-6s => ", (LPCTSTR) sRow->GetAt(r) );
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
	long d;
	bool IsGoodTable = true;

	for (d = 0; d < tab->nDim; d++) {
		//WriteRange(fd, tab, var, d, DimNr, 0, WithBogus, tdp);
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
	long DimNr[MAXDIM];
	long d;
	for (d = 0; d < tab->nDim; d++) {
		AdjustNonBasalCells(tab,d,DimNr,0);
	}
	CDataCell *dctemp;
	for (int i=0; i<tab->nCell; i++)	{
		dctemp = tab->GetCell(i);
	}

}

// Is good table is false if the table is not additive
void TauArgus::TestTable(CTable *tab, long TargetDim,
									 long *DimNr, long niv, bool	*IsGoodTable)
{
	CUIntArray Children;
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
						long RealCode = Children.GetAt(j);
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
int TauArgus::GetChildren(CVariable &var, int CodeIndex, CUIntArray &Children)
{
	int i, n = 0, LevelParent, LevelDesc, nCode;
	CCode *hCode = var.GethCode();
	ASSERT(var.IsHierarchical);

	Children.RemoveAll();
	nCode = var.GetnCode();
	// here I better do something
	if (hCode[CodeIndex].IsParent) {
		LevelParent = hCode[CodeIndex].Level;
		for (i = CodeIndex + 1; i<nCode; i++) {
			LevelDesc = hCode[i].Level;
			if (LevelDesc <= LevelParent) break;
			if (LevelDesc == LevelParent + 1) { // that's a child
				Children.Add(i);
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

	long ramya=1;
	long robbert = 1;
	CUIntArray Children;
	CDataCell *dc;
	CDataCell *dctemp, *dcramya;
   CDataCell *addcell;
	long tempDimNr;
	double sum,test;


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
						long RealCode = Children.GetAt(j);
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
							delete [] dctemp;
						}
						tab->CellPtr.SetAt(tempDimNr,addcell);
					}
					else {
						delete[] addcell;
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
			test = dctemp->GetResp();
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
					delete [] dctemp;
				}
				tab->CellPtr.SetAt(tempDimNr,addcell);
			}
			else {

				delete[] addcell;
			}
			dcramya = tab->GetCell(tempDimNr);
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
bool TauArgus::ReadVariableFreeFormat(UCHAR *Str, long VarIndex, CString *VarCode)
{
	CStringArray VarCodes;
	CString stempstr, stemp, tempvarcode;
	CVariable *var;
	VarCodes.SetSize(m_nvar);
	stempstr = Str;
	int inrem;
	long lseppos;
	long lcount= 0;
	if (InFileSeperator  != " ") {
		lseppos = stempstr.Find(InFileSeperator,0);
		while (lseppos != -1) {
			stemp = stempstr.Left(lseppos);
			VarCodes.SetAt(lcount,stemp);
			stempstr.Delete(0,lseppos + 1);
			lcount ++;
			lseppos = stempstr.Find(InFileSeperator, 0);
		}
		//fill the stuff here
		//Waarom mag de laatste niet leeg zijn?????
//		if ((stempstr.GetLength() == 0) || (lcount <m_nvar - 1)
//			|| (lcount >= m_nvar)) {
		if ((lcount <m_nvar - 1) || (lcount >= m_nvar)) {
			// string too short or too long

			return false;
		}
		else {
			VarCodes.SetAt(lcount,stempstr);
		}
		tempvarcode = VarCodes.GetAt(VarIndex);
		tempvarcode.TrimLeft();
		tempvarcode.TrimRight();
		inrem = tempvarcode.Remove('"');
		ASSERT ((inrem == 2) || (inrem == 0));
		var = &(m_var[VarIndex]);
		AddSpacesBefore(tempvarcode,var->nPos);
		//Now add leading spaces
		*VarCode = tempvarcode;
		return true;
	}
	else {
		return false;
	}
}

// returns depth of hierarchical tree.
long TauArgus::MaxDiepteVanSpanVariablen(CTable *tab)
{
	long max = 0, maxvardiepte;
	long i;
	CVariable *var;
	for (i= 0; i<tab->nDim; i++)	{
		var = &(m_var[tab->ExplVarnr[i]]);
		maxvardiepte = var->GetDepthOfHerarchicalBoom();
		max = __max(max,maxvardiepte);

	}
	return max;
}

// Test the subcode list (Networking. This is just a test program and can be converted to
// a real interface.
bool TauArgus::TestSubCodeList()
{
	CTable *tab;
	CVariable *var;
	long i;
	long j;
	long k,l, test;
	for (i=0; i<m_ntab; i++)	{
		tab = &(m_tab[i]);
		for (j = 0; j<tab->nDim; j++)	{
			var = &(m_var[tab->ExplVarnr[j]]);
			if (!var->PrepareSubCodeList())	{
				return false;
			}
			//create subcodelist for table
			if (!var->FillSubCodeList())	{
				return false;
			}
			// just to see sub codes are filled
			for (l = 0; l<var->NumSubCodes; l++)	{
				for (k = 0 ; k< var->m_SubCodes[l].NumberOfSubCodes(); k++)
				{
					test = var->m_SubCodes[l].
						GetSubCodeIndex(k);
				}
			}

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
	CTable *tab;
	CVariable *var;
	long i;
	long Num=1;

	tab = &(m_tab[IndexTable]);
	for (i= 0; i<tab->nDim; i++)	{
		var = &(m_var[tab->ExplVarnr[i]]);
		Num *= var->NumSubCodes;
	}
	return Num;
}

// Given a table and a subtable, Find out which Tuple the subtable is made of
// i.e. which subcode idices of the explanatory variable the subtale is made of of
bool TauArgus::SubTableTupleForSubTable(long TableIndex, long SubTableIndex,
													 long *SubCodeIndices)
{
	CTable *tab;
	CVariable *var;
	long i,c;
	c = SubTableIndex;

	tab = &(m_tab[TableIndex]);
	for (i = tab->nDim-1; i >= 0; i--)	{
		var = &(m_var[tab->ExplVarnr[i]]);
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
	long SubTableIndex = -1;
	long *SubTableTuple;
	long * MarginalCellIndex;
	long * SubTableCellIndex;
	long * TableCellIndex;
	long nCellSubTable;
	CTable *tab;
	tab = &(m_tab[TableIndex]);
	bool found = false;
	long SubTableCellNum;
	long teller = UnsafeCellNum;
	long NumberOfColumns;
	CVariable *var, *var1;
	CDataCell *dc;
	long nDec = 8;
	double UPL, LPL;
	var1 = &(m_var[tab->ExplVarnr[tab->nDim-1]]);
	NumberOfColumns = var1->m_SubCodes[0].NumberOfSubCodes();
	SubTableTuple = new long [tab->nDim];
	TableCellIndex = new long [tab->nDim];
	MarginalCellIndex = new long [tab->nDim];
	SubTableCellIndex  = new long [tab->nDim];
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
				fprintf(fdtemp, "%d    %d    " , teller, i);
				fprintf(fdtemp, "%d    %d    ", SubTableCellIndex[0], SubTableCellIndex[1]);
				UPL = dc->GetUpperProtectionLevel();
				LPL = dc->GetLowerProtectionLevel();
				if (MaxScale > 0){
				 if ( LPL > MaxScale * dc->GetResp()) LPL = MaxScale * dc->GetResp();
				 if ( UPL > MaxScale * dc->GetResp()) UPL = MaxScale * dc->GetResp();
				}
//				fprintf (fdtemp, "%.*f    %.*f\n", nDec, dc->GetLowerProtectionLevel(),nDec, dc->GetUpperProtectionLevel());
				fprintf (fdtemp, "%.*f    %.*f\n", nDec, LPL,nDec, UPL);
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
					fprintf(fdtemp, "%d    %d    " , teller, i);
					fprintf(fdtemp, "%d    %d    ", MarginalCellIndex[0], MarginalCellIndex[1]);
					UPL = dc->GetUpperProtectionLevel();
					LPL = dc->GetLowerProtectionLevel();
					if ( MaxScale > 0) {
					 if ( LPL > MaxScale * dc->GetResp()) LPL = MaxScale * dc->GetResp();
					 if ( UPL > MaxScale * dc->GetResp()) UPL = MaxScale * dc->GetResp();
					}
					//fprintf (fdtemp, "%.*f    %.*f\n", nDec, dc->GetLowerProtectionLevel(),nDec, dc->GetUpperProtectionLevel());
					fprintf (fdtemp, "%.*f    %.*f\n", nDec, LPL, nDec, UPL);
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
					fprintf(fdtemp, "%d    %d    " , teller, i);
					fprintf(fdtemp, "%d    %d    ", MarginalCellIndex[0], MarginalCellIndex[1]);
					UPL = dc->GetUpperProtectionLevel();
					LPL = dc->GetLowerProtectionLevel();
					if (MaxScale > 0 ) {
					 if ( LPL > MaxScale * dc->GetResp()) LPL = MaxScale * dc->GetResp();
					 if ( UPL > MaxScale * dc->GetResp()) UPL = MaxScale * dc->GetResp();
					}
//					fprintf (fdtemp, "%.*f    %.*f\n", nDec, dc->GetLowerProtectionLevel(),nDec, dc->GetUpperProtectionLevel());
					fprintf (fdtemp, "%.*f    %.*f\n", nDec, LPL, nDec, UPL);
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
			fprintf(fdtemp, "%d    %d    " , teller, i);
			fprintf(fdtemp, "%d    %d    ", MarginalCellIndex[0], MarginalCellIndex[1]);
			UPL = dc->GetUpperProtectionLevel();
			LPL = dc->GetLowerProtectionLevel();
			if (MaxScale > 0) {
			 if ( LPL > MaxScale * dc->GetResp()) LPL = MaxScale * dc->GetResp();
			 if ( UPL > MaxScale * dc->GetResp()) UPL = MaxScale * dc->GetResp();
			}
//			fprintf (fdtemp, "%.*f    %.*f\n", nDec, dc->GetLowerProtectionLevel(),nDec, dc->GetUpperProtectionLevel());
			fprintf (fdtemp, "%.*f    %.*f\n", nDec, LPL, nDec, UPL);

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
	long i;
	for (i = 0; i<nDim; i++)	{
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
	long i;
	CTable *tab;
	CVariable *var;
	long Num = 1;
	tab =  &(m_tab[TableIndex]);

	for (i = 0; i<tab->nDim; i++)	{
		var = &(m_var[tab->ExplVarnr[i]]);
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
	CTable *tab;
	CVariable *var;
	long i;
	long c = CellIndexInSubTable;
	//long * SubTableTupleIndex;


	tab = &(m_tab[TableIndex]);
	//SubTableTupleIndex = new long [tab->nDim];

	for (i = tab->nDim-1; i >= 0; i--)	{
	/*	var = &(m_var[tab->ExplVarnr[i]]);
		SubCodeIndices[i] = c%var->NumSubCodes;
		c	-= SubCodeIndices[i];
		c /= var->NumSubCodes;
		*/
		var = &(m_var[tab->ExplVarnr[i]]);
		SubTableCellIndex[i] = c%var->m_SubCodes[SubTableTuple[i]].NumberOfSubCodes();
		c -= SubTableCellIndex[i];
		c /= var->m_SubCodes[SubTableTuple[i]].NumberOfSubCodes();
	}
	// got the subtable cell index get dim nr in table
	for (i = 0; i<tab->nDim; i++)	{
		var = &(m_var[tab->ExplVarnr[i]]);
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
	long *SubTableTuple;
	long *SubCodeList;
	long nCellSubTable;
	CTable *tab;
	tab = &(m_tab[TableIndex]);
	bool found = false;

	SubTableTuple = new long [tab->nDim];
	SubCodeList = new long [tab->nDim];
	long nsubtable = NumberOfSubTables(TableIndex);
	for (i = 0; i<nsubtable; i++)	{
		//Find a SubTableTuple
		SubTableTupleForSubTable(TableIndex,i,SubTableTuple);
		nCellSubTable = FindNumberOfElementsInSubTable(SubTableTuple, TableIndex);
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
	long *dimnr;
	CTable *tab;
	tab = &(m_tab[TableIndex]);
	dimnr = new long[tab->nDim];
	long SubTableIndex;
	tab->GetIndicesFromCellNr(CellIndex,dimnr);
	SubTableIndex = SubTableForCellDimension(TableIndex,dimnr,SubTableTupleIndex);
	delete [] dimnr;
	return SubTableIndex;
}

// This is is not generalized but can be with some minor changes
// To generalize this all the help functions are there above
bool TauArgus::WriteAllSubTablesInAMPL(FILE *fd, long tabind)
{
	long i, j, k, cellnr;
	CTable *tab;
	CVariable *var1, *var;
	long sum;
	long marginal = 0;
	long NumberOfColumns;
	long NumSubTables, NumSubTableCells;

	long *SubTableTuple;
	long *SubTableCellIndex;
	long *TableCellIndex;
	long *MarginalCellIndex;
	long nDecResp = 8;
	CDataCell *dc;
	tab = &(m_tab[tabind]);
	var1 = &(m_var[tab->ExplVarnr[tab->nDim-1]]); // last variable at this point its 2
																// dim

	NumberOfColumns = var1->m_SubCodes[0].NumberOfSubCodes();

	SubTableTuple = new long[tab->nDim];
	SubTableCellIndex = new long[tab->nDim];
	TableCellIndex = new long[tab->nDim];
//	dimnr = new long [tab->nDim];
	MarginalCellIndex = new long [tab->nDim];
	NumSubTables = NumberOfSubTables(tabind);

	fprintf (fd, "%s\n", "#table values and weights 9indexed by (table,row,column)), marginals included");
	fprintf (fd, "%s\n", "param : a         weight   :=");
	for (i =0; i<NumSubTables; i++)	{
		SubTableTupleForSubTable(tabind,i,SubTableTuple);
		NumSubTableCells = FindNumberOfElementsInSubTable(SubTableTuple,tabind);
		marginal = 0; sum = 0;
		for (j= 0; j<NumSubTableCells; j++)	{
			// just print cell
			FindCellIndexForSubTable(TableCellIndex,tabind,SubTableTuple,j, SubTableCellIndex);
			cellnr = tab->GetCellNrFromIndices(TableCellIndex);
			dc = tab->GetCell(cellnr);
			fprintf (fd, "%d    %d    %d    ",  i, SubTableCellIndex[0], SubTableCellIndex[1]);
			fprintf (fd, "%.*f    %.*f\n", nDecResp, dc->GetResp(),nDecResp, dc->GetCost(tab->Lambda));

			marginal++;
			// Last Column (marginal column)
			if (marginal == NumberOfColumns)	{

				for (k= 0; k<tab->nDim; k++)	{
					var = &(m_var[tab->ExplVarnr[k]]);
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
				fprintf (fd, "%d    %d    %d    " , i, MarginalCellIndex[0], MarginalCellIndex[1]);
				fprintf(fd,"%.*f    %.*f\n",	nDecResp, dc->GetResp(),nDecResp, dc->GetCost(tab->Lambda));

				marginal = 0;
			}
		}

		// Now the last Rows which is the marginal row
		for (k = 0; k<NumberOfColumns; k++)	{
			for (long l= 0; l<tab->nDim; l++)	{
				var = &(m_var[tab->ExplVarnr[l]]);
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
			dc = tab->GetCell(cellnr);
			// print table number etc
			fprintf (fd, "%d    %d    %d    ", i, MarginalCellIndex[0], MarginalCellIndex[1]);

			fprintf(fd,"%.*f    %.*f\n",	nDecResp, dc->GetResp(),nDecResp, dc->GetCost(tab->Lambda));
		}
		//Now the last lot of the sub table i.e. the totaal generaal of the sub table
		for (long l= 0; l<tab->nDim; l++)	{
			var = &(m_var[tab->ExplVarnr[l]]);
			MarginalCellIndex[l] = var->m_SubCodes[SubTableTuple[l]].NumberOfSubCodes();	// No of Rows +1
			TableCellIndex[l] = var->m_SubCodes[SubTableTuple[l]].GetParentIndex();
		}
		cellnr = tab->GetCellNrFromIndices(TableCellIndex);
		dc = tab->GetCell(cellnr);
		// print table number etc
		fprintf (fd, "%d    %d    %d    ", i, MarginalCellIndex[0], MarginalCellIndex[1]);
		fprintf(fd,"%.*f    %.*f\n" ,nDecResp, dc->GetResp(),nDecResp, dc->GetCost(tab->Lambda));


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
	CTable *tab;
	CVariable *var;
	long i, j, k;
	long nsubtab;
	long *SubTableTuple;
	long RowNum,  CodeIndex;
	long SubTableIndex, ChildSubTableIndex;
	long SubCodeIndex;
	CSubCodeList* osubcodelist;

	CString scode;
	tab = &(m_tab[tabind]);

	// since the first explanatory variable is hierarchical
	var = &(m_var[tab->ExplVarnr[varind]]);
	if (!var->IsHierarchical)	{
		return false;

	}

	SubTableTuple = new long [tab->nDim];
	nsubtab = NumberOfSubTables(tabind);
	bool foundRowNum = false;
	bool foundParent = false;
	long param = 0;


	//Print the row number the table and the table where it is split into
	fprintf(fd, "%s\n", "#(T-1) lines with hierarchical info: row rh,of table th decomposed in table tdh");
	fprintf(fd, "%s\n", " param:     rh     th     tdh :=");
	for (i = 1; i < var->nCode; i++)	{
		scode = var->sCode.GetAt(i);
		CodeIndex = i;
		// Is a parent
		if (var->FindNumberOfChildren(CodeIndex) > 0)	{
			// find out which table this parent is in
			for (j =0; j <nsubtab; j++)	{
				SubTableTupleForSubTable(tabind,j,SubTableTuple);
				SubCodeIndex = SubTableTuple[varind];
				osubcodelist = &(var->m_SubCodes[SubCodeIndex]);
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
				for (k =0; k<nsubtab; k++)	{
					SubTableTupleForSubTable(tabind,k,SubTableTuple);
					SubCodeIndex = SubTableTuple[varind];
					osubcodelist = &(var->m_SubCodes[SubCodeIndex]);
					if (scode == osubcodelist->GetParentCode())	{
						foundParent = true;
						ChildSubTableIndex = k;
						break;
					}
				}
				if (foundParent)	{
					//Print the stuff
					fprintf (fd, "%d    %d    %d   %d\n", param,RowNum, SubTableIndex, ChildSubTableIndex);
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
bool TauArgus::WriteHierTableInAMPL(FILE *fd, long tabind, CString TempDir, double MaxScale)
{
	long nsubtab;
	CVariable *var0;
	CVariable *var1;
	long *DimNr;
	long *RowColIndex;
	long nUnsafe;
	CTable *tab;
	CDataCell *dc;
	FILE *fdtempw;
	FILE *fdtempr;
	UCHAR str[MAXRECORDLENGTH];
	fdtempw = fopen(TempDir +"/TMPAMPLU", "w");
	if (fd == 0)	{
		return false;
	}
	long status;
	tab = &(m_tab[tabind]);
	DimNr = new long [tab->nDim];
	RowColIndex = new long [tab->nDim];
	long i;
	long nDec = 8;
	fprintf(fd,"%s\n", "# AMPL generated By TauArgus");
	fprintf(fd,"%s\n", "#");
	// The number of sub tables
	fprintf(fd, "%s\n", "#T = number of 2D tables");
	nsubtab = NumberOfSubTables(tabind);
	fprintf (fd, "%s%d%s\n", "param T := ", nsubtab, ";");

	// Print Number of Rows
	fprintf (fd, "%s\n", "#M = number of rows of each table (without marginal row)");
	fprintf (fd,"%s\n", "param M := ");
	var0 = &(m_var[tab->ExplVarnr[0]]);
	var1 = &(m_var[tab->ExplVarnr[1]]);
	for (i =0; i<var0->NumSubCodes; i++)	{
		fprintf(fd,"%d                  %d\n", i, var0->m_SubCodes[i].NumberOfSubCodes());
	}
	fprintf(fd,"%s\n", ";");

	// Print Number Of Columns
	fprintf (fd, "%s\n", "#N = number of columns common to each table (without marginal row)");
	fprintf (fd,"%s%d%s\n", "param N := ", var1->m_SubCodes[0].NumberOfSubCodes(), ";");

	// Print Hierarchical info
	// since the first variable is hierarchical
	WriteTableSequenceHierarchyInAMPL(fd,tabind, 0);

	// Print Number of Unsafe Cells
	nUnsafe = 0;
	for (i=0; i <tab->nCell; i++)	{
		dc = tab->GetCell(i);
		status = dc->GetStatus();
		if ((status == CS_UNSAFE_RULE) || (status == CS_UNSAFE_PEEP) || (status == CS_UNSAFE_FREQ) ||
			(status == CS_UNSAFE_ZERO) || (status == CS_UNSAFE_SINGLETON) || (status == CS_UNSAFE_SINGLETON_MANUAL) ||
			(status == CS_UNSAFE_MANUAL) || (status == CS_PROTECT_MANUAL))	{
			nUnsafe = WriteCellInTempFile(nUnsafe,tabind, i, fdtempw,MaxScale);

		}
	}
	fclose(fdtempw);
	fprintf (fd, "%s\n", "#P = number of primary supression cells of each table;");
	fprintf (fd,"%s %d %s\n", "param P := ", nUnsafe, ";");
	//and where the unsafe cells occur with LPL and UPL.
	fprintf (fd, "%s\n", "# for each primary : table, row, column, lower prot, and upper prot.");
	fprintf  (fd,"%s\n", "param : p_t    p_r    p_c    lpl    upl  :=");
	// now open the temp file
	fdtempr = fopen(TempDir +"/TMPAMPLU", "r");
	if (fdtempr == 0)	{
		return false;
	}
	fseek( fdtempr, 0L, SEEK_SET );
	for (i=0; i<nUnsafe; i++)	{
		int res = ReadMicroRecord(fdtempr, str);
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

	CTable *tab;
	CVariable *var;
	long j;
	long k,l, test;
	CString sTempDir = "E:/Temp";

		tab = &(m_tab[ind]);
		for (j = 0; j<tab->nDim; j++)	{
			var = &(m_var[tab->ExplVarnr[j]]);
			if (!var->PrepareSubCodeList())	{
				return false;
			}
			//create subcodelist for table
			if (!var->FillSubCodeList())	{
				return false;
			}
			// just to see sub codes are filled
			for (l = 0; l<var->NumSubCodes; l++)	{
				for (k = 0 ; k< var->m_SubCodes[l].NumberOfSubCodes(); k++)
				{
					test = var->m_SubCodes[l].
						GetSubCodeIndex(k);
				}
			}

		}


	FILE *fd;
	fd = fopen ("E:/Temp/Hierampl.txt", "w");

	WriteHierTableInAMPL(fd,ind,sTempDir, 0.0);
	fclose(fd);
	return true;

}



//DEL STDMETHODIMP TauArgus::Anco(long longvar1, long *longvar2, VARIANT_BOOL *pVal)
//DEL {
//DEL 	bool bla;
//DEL 	if (bla)	{
//DEL 		*pVal = VARIANT_FALSE;
//DEL 		return S_OK;
//DEL 	}
//DEL
//DEL 	return S_OK;
//DEL }

