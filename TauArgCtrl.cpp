// TauArgCtrl.cpp : Implementation of CTauArgCtrl
#include "stdafx.h"

#include <comutil.h>

#include "TauArgus.h"
#include "NewTauArgus.h"
#include "TauArgCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Needed for conversion function from BSTR to char*
# pragma comment(lib, "comsupp.lib")

// --------------------------------------------------------------------------
// B2CString - helper class for conversion from BSTR to char* 
// --------------------------------------------------------------------------
class B2CString {

private:
	char *cstr;

public:
	B2CString(const BSTR &bstr) {
		cstr = _com_util::ConvertBSTRToString(bstr);
	}

	~B2CString() {
		delete [] cstr;
	}

	operator char*() {
		return cstr;
	}
};

// --------------------------------------------------------------------------
// C2BString - helper class for back conversion from const char** to BSTR* 
// --------------------------------------------------------------------------
class C2BString {

private:
	const char *cstr;
	BSTR* bstr;

public:
	C2BString(BSTR *bstr) : cstr(0), bstr(bstr) {
	}

	~C2BString() {
		*bstr = _com_util::ConvertStringToBSTR(cstr);
	}

	operator const char**() {
		return &cstr;
	}
};

// --------------------------------------------------------------------------
// B2CStringArray - helper class for conversion from VARIANT* to char[]
// --------------------------------------------------------------------------
class B2CStringArray {

private:
	char** cstr;
	long size;

public:
	B2CStringArray(const VARIANT* &bstrArray) {
		SAFEARRAY *sa = bstrArray->parray;
		SafeArrayGetUBound(sa, 1, &size);
		cstr = new char*[size];

		BSTR bstrtemp;
		// currently all supplied arrays have a lower bound index of 1
		for (long j = 1; j <= size; j++) {
			// To get a string from an array of strings. Use SafeArray
			HRESULT hresult = SafeArrayGetElement(sa, &j, &bstrtemp);
			cstr[j - 1] = _com_util::ConvertBSTRToString(bstrtemp);
		}
	}

	~B2CStringArray() {
		for (int i = 0; i < size; i++) {
			delete [] cstr[i];
		}

		delete[] cstr;
	}

	operator char**() {
		return cstr;
	}
};

// --------------------------------------------------------------------------
// DecrementIndex - helper class for decrementing indices (by 1) in arrays
// --------------------------------------------------------------------------
class DecrementIndex {

private:
	long *Index;

public:
	DecrementIndex(long size, long *Index) {
		this->Index = new long[size];
		for (int i=0; i<size; i++) {
			this->Index[i] = Index[i] - 1;
		}
	}

	~DecrementIndex() {
		delete[] Index;
	}

	operator long*() {
		return Index;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CTauArgCtrl

// This function is used for firing events.
void CTauArgCtrl::UpdateProgress (int Perc)
{
	Fire_UpdateProgress((short)Perc);
}

// cells that are set as secondary unsafe to be undone
STDMETHODIMP CTauArgCtrl::UndoSecondarySuppress(long TableIndex, long SortSuppress,
																VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.UndoSecondarySuppress(TableIndex - 1, SortSuppress) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Set number of Variables
STDMETHODIMP CTauArgCtrl::SetNumberVar(long nVar, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetNumberVar(nVar) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// set number of tables
STDMETHODIMP CTauArgCtrl::SetNumberTab(long nTab, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetNumberTab(nTab) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Compute the Tabels. In this function all the tables are filled.
STDMETHODIMP CTauArgCtrl::ComputeTables(long *ErrorCode, long *TableIndex, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.ComputeTables(ErrorCode, TableIndex) ? VARIANT_TRUE : VARIANT_FALSE;
	if (*TableIndex != -1) // -1 = error value
		*TableIndex = *TableIndex + 1;

	return S_OK;
}

STDMETHODIMP CTauArgCtrl::DoRecode(long VarIndex, BSTR RecodeString, long nMissing, BSTR eMissing1, BSTR eMissing2,
												long *ErrorType, long *ErrorLine, long *ErrorPos,
												BSTR *WarningString, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.DoRecode(VarIndex - 1, B2CString(RecodeString), nMissing, B2CString(eMissing1), B2CString(eMissing2), ErrorType, ErrorLine, ErrorPos, C2BString(WarningString)) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Apply Recoding. You need this for recoding tables
STDMETHODIMP CTauArgCtrl::ApplyRecode()
{
	tauArgus.ApplyRecode();

	return S_OK;
}

// Clean all allocated memory. Destructor does this
STDMETHODIMP CTauArgCtrl::CleanAll()
{
	tauArgus.CleanAll();

	return S_OK;
}

// Used for setting Hierarchical Variables with digit Split
STDMETHODIMP CTauArgCtrl::SetHierarchicalDigits(long VarIndex, long nDigitPairs, long *nDigits, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetHierarchicalDigits(VarIndex - 1, nDigitPairs, nDigits) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// For every row get all cells with corresponding information
STDMETHODIMP CTauArgCtrl::GetTableRow(long TableIndex, long *DimIndex, double *Cell,
													long *Status, long CountType,
													VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.GetTableRow(TableIndex - 1, DimIndex, Cell, Status, CountType) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// return information about Unsafe Variable
STDMETHODIMP CTauArgCtrl::UnsafeVariable(long VarIndex,
														long *Count, long *UCArray,
														VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.UnsafeVariable(VarIndex - 1, Count, UCArray) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// In this function the input file is read and the code list is built
STDMETHODIMP CTauArgCtrl::ExploreFile(BSTR FileName, long *ErrorCode,
													long *LineNumber, long *VarIndex,
			
													VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.ExploreFile(B2CString(FileName), ErrorCode, LineNumber, VarIndex) ? VARIANT_TRUE : VARIANT_FALSE;
	*VarIndex = *VarIndex + 1;

	return S_OK;
}

// get maximum unsafe Combination
STDMETHODIMP CTauArgCtrl::GetMaxnUc(long *pVal)
{
	*pVal = tauArgus.GetMaxnUc();

	return S_OK;
}

// Undo recode. Undo recodes for a variable. This is used when a table is
// created to be recoded
STDMETHODIMP CTauArgCtrl::UndoRecode(long VarIndex, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.UndoRecode(VarIndex - 1) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


//Sets the status of a cell to a given status
STDMETHODIMP CTauArgCtrl::SetTableCellStatus(long TableIndex, long *DimIndex,
														  long CelStatus, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetTableCellStatus(TableIndex - 1, DimIndex, CelStatus) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Set the cost function for a cell
STDMETHODIMP CTauArgCtrl::SetTableCellCost(long TableIndex, long *DimIndex, double Cost,
														 VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetTableCellCost(TableIndex - 1, DimIndex, Cost) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Get Status and Cost per dimensie
STDMETHODIMP CTauArgCtrl::GetStatusAndCostPerDim(long TableIndex, long *Status,
																double *Cost, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.GetStatusAndCostPerDim(TableIndex - 1, Status, Cost) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Set a Variable code as Active, This is important for
// recoding tables. This gives that the respective code is active
// and will be used in recoding
STDMETHODIMP CTauArgCtrl::SetVarCodeActive(long VarIndex, long CodeIndex,
														VARIANT_BOOL Active, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetVarCodeActive(VarIndex - 1, CodeIndex, Active==VARIANT_TRUE) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Get the number of codes for the given variable
STDMETHODIMP CTauArgCtrl::GetVarNumberOfCodes(long VarIndex, long *NumberOfCodes,
															long *NumberOfActiveCodes, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.GetVarNumberOfCodes(VarIndex - 1, NumberOfCodes, NumberOfActiveCodes) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Do recode for all active codes
STDMETHODIMP CTauArgCtrl::DoActiveRecode(long VarIndex, VARIANT_BOOL *pVal)

{
	*pVal = tauArgus.DoActiveRecode(VarIndex - 1) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Set Variable. All information to set in the variable object is given
STDMETHODIMP CTauArgCtrl::SetVariable(long VarIndex, long bPos,
												 long nPos, long nDec, long nMissing, BSTR Missing1,
												 BSTR Missing2,BSTR TotalCode,VARIANT_BOOL IsPeeper,
												 BSTR PeeperCode1, BSTR PeeperCode2,
												 VARIANT_BOOL IsCategorical,
												 VARIANT_BOOL IsNumeric, VARIANT_BOOL IsWeight,
												 VARIANT_BOOL IsHierarchical,
												 VARIANT_BOOL IsHolding,
												 VARIANT_BOOL *pVal)
{
	if (!isFixedFormat)
	{
		bPos = VarIndex;
	}
	*pVal = tauArgus.SetVariable(VarIndex - 1, bPos, nPos, nDec, nMissing, B2CString(Missing1), B2CString(Missing2), B2CString(TotalCode), IsPeeper==VARIANT_TRUE, B2CString(PeeperCode1), B2CString(PeeperCode2), IsCategorical==VARIANT_TRUE, IsNumeric==VARIANT_TRUE, IsWeight==VARIANT_TRUE, IsHierarchical==VARIANT_TRUE, IsHolding==VARIANT_TRUE)
		? VARIANT_TRUE 
		: VARIANT_FALSE;

	return S_OK;
}

// Sets all the information for the Table object this together with
// SetTableSafety does the trick.
STDMETHODIMP CTauArgCtrl::SetTable(	long Index, long nDim, long *ExplanatoryVarList,
												VARIANT_BOOL IsFrequencyTable,
												long ResponseVar, long ShadowVar, long CostVar,
												double Lambda,
												double MaxScaledCost,
												long PeepVarnr,
												VARIANT_BOOL SetMissingAsSafe,
												VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetTable(Index - 1, nDim, DecrementIndex(nDim, ExplanatoryVarList), IsFrequencyTable==VARIANT_TRUE, ResponseVar - 1, ShadowVar - 1, CostVar > 0 ? CostVar - 1 : CostVar,  Lambda, MaxScaledCost, PeepVarnr - 1, SetMissingAsSafe==VARIANT_TRUE)
		? VARIANT_TRUE 
		: VARIANT_FALSE;

	return S_OK;
}


STDMETHODIMP CTauArgCtrl::GetTableCellValue(long TableIndex, long CellIndex,
                                            double *CellResponse, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.GetTableCellValue(TableIndex - 1, CellIndex, CellResponse) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


// Returns the information in a cell.
STDMETHODIMP CTauArgCtrl::GetTableCell(long TableIndex, long *DimIndex,
													double *CellResponse, long *CellRoundedResp, double *CellCTAResp,
													double *CellShadow, double *CellCost,
													 long *CellFreq, long *CellStatus,
													 double *CellMaxScore,double *CellMAXScoreWeight,
													 long *HoldingFreq,
													 double *HoldingMaxScore, long *HoldingNrPerMaxScore,
													 double * PeepCell, double * PeepHolding, long * PeepSortCell, long * PeepSortHolding,
													 double *Lower, double *Upper,
													 double *RealizedLower,double * RealizedUpper,
													 VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.GetTableCell(TableIndex - 1, DimIndex, CellResponse, CellRoundedResp, CellCTAResp, CellShadow, CellCost, CellFreq, CellStatus, CellMaxScore, CellMAXScoreWeight, HoldingFreq, HoldingMaxScore, HoldingNrPerMaxScore, PeepCell, PeepHolding, PeepSortCell, PeepSortHolding, Lower, Upper, RealizedLower, RealizedUpper)
		? VARIANT_TRUE 
		: VARIANT_FALSE;

	return S_OK;
}

// Set information necessary to build a tableobject. This functions works together with SetTable.
STDMETHODIMP CTauArgCtrl::SetTableSafety( long Index, VARIANT_BOOL DominanceRule,
														long * DominanceNumber,
														long * DominancePerc,
														VARIANT_BOOL PQRule,
														long * PriorPosteriorP,
														long * PriorPosteriorQ,
														long * PriorPosteriorN,
														long * SafeMinRecAndHoldings,
														long * PeepPerc,
														long * PeepSafetyRange,
														long * PeepMinFreqCellAndHolding,
														VARIANT_BOOL ApplyPeep,
														VARIANT_BOOL ApplyWeight, VARIANT_BOOL ApplyWeightOnSafetyRule,
														VARIANT_BOOL ApplyHolding,VARIANT_BOOL ApplyZeroRule,
														VARIANT_BOOL EmptyCellAsNonStructural, long NSEmptySafetyRange,
														long ZeroSafetyRange,	long ManualSafetyPerc,
														long * CellAndHoldingFreqSafetyPerc,
														VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetTableSafety(Index - 1, DominanceRule==VARIANT_TRUE, DominanceNumber, DominancePerc, PQRule==VARIANT_TRUE, PriorPosteriorP, PriorPosteriorQ, PriorPosteriorN, SafeMinRecAndHoldings, PeepPerc, PeepSafetyRange, PeepMinFreqCellAndHolding, ApplyPeep==VARIANT_TRUE, ApplyWeight==VARIANT_TRUE, ApplyWeightOnSafetyRule==VARIANT_TRUE, ApplyHolding==VARIANT_TRUE, ApplyZeroRule==VARIANT_TRUE, EmptyCellAsNonStructural==VARIANT_TRUE, NSEmptySafetyRange, ZeroSafetyRange, ManualSafetyPerc, CellAndHoldingFreqSafetyPerc)
		? VARIANT_TRUE 
		: VARIANT_FALSE;

	return S_OK;
}

// Prepare the file for Hitas. The table is written in a file that could be used by
//Hitas for secondary supressions
STDMETHODIMP CTauArgCtrl::PrepareHITAS(long TableIndex, BSTR NameParameterFile,
												  BSTR NameFilesFile, BSTR TauTemp, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.PrepareHITAS(TableIndex - 1, B2CString(NameParameterFile), B2CString(NameFilesFile), B2CString(TauTemp))
		? VARIANT_TRUE 
		: VARIANT_FALSE;

	return S_OK;
}

// Cells that are found to be secondary unsafe by Hitas is set as Unsafe in the table
STDMETHODIMP CTauArgCtrl::SetSecondaryHITAS(long TableIndex, long *nSetSecondary,
														 VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetSecondaryHITAS(TableIndex - 1, nSetSecondary) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

STDMETHODIMP CTauArgCtrl::SetHierarchicalCodelist(long VarIndex, BSTR FileName,
																 BSTR LevelString, long *pVal)
{
	*pVal = tauArgus.SetHierarchicalCodelist(VarIndex - 1, B2CString(FileName), B2CString(LevelString));

	return S_OK;
}

// Gets a code if given an index and a variable number
STDMETHODIMP CTauArgCtrl::GetVarCode(long VarIndex, long CodeIndex,
												long *CodeType, BSTR *CodeString,
												long *IsMissing, long *Level,
												VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.GetVarCode(VarIndex - 1, CodeIndex, CodeType, C2BString(CodeString), IsMissing, Level)
		? VARIANT_TRUE 
		: VARIANT_FALSE;

	return S_OK;
}


// return the codes for unsafe variables
STDMETHODIMP CTauArgCtrl::UnsafeVariableCodes(long VarIndex, long CodeIndex,
															long *IsMissing, long *Freq,
															BSTR *Code, long *Count,
															long *UCArray, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.UnsafeVariableCodes(VarIndex - 1, CodeIndex, IsMissing, Freq, C2BString(Code), Count, UCArray) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// return properties given a Variable and Code Index
STDMETHODIMP CTauArgCtrl::GetVarCodeProperties(long VarIndex, long CodeIndex,
															 long *IsParent, long *IsActive,
															 long *IsMissing, long *Level,
															 long *nChildren, BSTR *Code,
															 VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.GetVarCodeProperties(VarIndex - 1, CodeIndex, IsParent, IsActive, IsMissing, Level, nChildren, C2BString(Code)) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Write Table in GHmiter file. This is used in secondary supressions
STDMETHODIMP CTauArgCtrl::WriteGHMITERSteuer(BSTR FileName, BSTR EndString1,
														  BSTR EndString2, long TableIndex,
														  long *pVal)
{
	*pVal = tauArgus.WriteGHMITERSteuer(B2CString(FileName), B2CString(EndString1), B2CString(EndString2), TableIndex - 1);

	return S_OK;
}

// Write data Cells for GHMiter.
STDMETHODIMP CTauArgCtrl::WriteGHMITERDataCell(BSTR FileName, long TableIndex,
															  VARIANT_BOOL IsSingleton,
															 long *pVal)
{
	*pVal = tauArgus.WriteGHMITERDataCell(B2CString(FileName), TableIndex - 1, IsSingleton==VARIANT_TRUE);

	return S_OK;
}

// Cells that are given as secondary Unsafe by GHmiter
// to be set in the table
STDMETHODIMP CTauArgCtrl::SetSecondaryGHMITER(BSTR FileName, long TableIndex,
															long *nSetSecondary,VARIANT_BOOL IsSingleton,
															long *pVal)
{
	*pVal = tauArgus.SetSecondaryGHMITER(B2CString(FileName), TableIndex - 1, nSetSecondary, IsSingleton==VARIANT_TRUE);

	return S_OK;
}

// Get information per status
STDMETHODIMP CTauArgCtrl::GetCellStatusStatistics(long TableIndex,
																  long *StatusFreq,
																  long *StatusCellFreq,
																  long *StatusHoldingFreq,
																  double *StatusCellResponse,
																  double *StatusCellCost,
																  VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.GetCellStatusStatistics(TableIndex - 1, StatusFreq, StatusCellFreq, StatusHoldingFreq, StatusCellResponse, StatusCellCost)
		? VARIANT_TRUE
		: VARIANT_FALSE;
	
	return S_OK;
}


// This function is not usedb at the moment
STDMETHODIMP CTauArgCtrl::PrepareCellDistance(long TableIndex, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.PrepareCellDistance(TableIndex - 1) ? VARIANT_TRUE : VARIANT_FALSE;
	
	return S_OK;
}

// This function is not used either at the moment
STDMETHODIMP CTauArgCtrl::GetCellDistance(long TableIndex, long *Dims,
													  long *Distance, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.GetCellDistance(TableIndex - 1, Dims, Distance) ? VARIANT_TRUE : VARIANT_FALSE;
	
	return S_OK;
}

// Write a table as Comma seperated file
STDMETHODIMP CTauArgCtrl::WriteCSV(long TableIndex, BSTR FileName,
											 long *DimSequence, long RespType, VARIANT_BOOL *pVal)
{
	// specialized Basic version decreases indices of DimSequence with 1. At this place there is no knowledge of the length of the DimSequence array.
	*pVal = tauArgus.WriteCSVBasic(TableIndex - 1, B2CString(FileName), DimSequence, RespType) ? VARIANT_TRUE : VARIANT_FALSE;
	
	return S_OK;
}

// Write Table in JJ Format
STDMETHODIMP CTauArgCtrl::WriteJJFormat(long TableIndex, BSTR FileName,
													double LowerBound, double UpperBound,
													VARIANT_BOOL WithBogus, VARIANT_BOOL AsPerc,
													VARIANT_BOOL ForRounding,VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.WriteJJFormat(TableIndex - 1, B2CString(FileName), LowerBound, UpperBound, WithBogus==VARIANT_TRUE, AsPerc==VARIANT_TRUE, ForRounding==VARIANT_TRUE) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Cells that are give as secondary unsafe by JJ to be set in the table
STDMETHODIMP CTauArgCtrl::SetSecondaryJJFORMAT(long TableIndex, BSTR FileName, VARIANT_BOOL WithBogus, long *nSetSecondary, long *pVal)
{
	*pVal = tauArgus.SetSecondaryJJFORMAT(TableIndex - 1, B2CString(FileName), WithBogus==VARIANT_TRUE, nSetSecondary);

	return S_OK;
}

// Get Size of a tabel. This function has to be changed
STDMETHODIMP CTauArgCtrl::GetTotalTabelSize(long TableIndex, long* nCell,long * SizeDataCell)
{
	tauArgus.GetTotalTabelSize(TableIndex - 1, nCell, SizeDataCell);

	return S_OK;
}

// Write Cell in file
STDMETHODIMP CTauArgCtrl::WriteCellRecords(long TableIndex, BSTR FileName,
									        long SBS,
									        VARIANT_BOOL SBSLevel,
											VARIANT_BOOL SuppressEmpty, BSTR FirstLine,
											VARIANT_BOOL ShowUnsafe,
										    long RespType,
											VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.WriteCellRecords(TableIndex - 1, B2CString(FileName), SBS, SBSLevel==VARIANT_TRUE, SuppressEmpty==VARIANT_TRUE, B2CString(FirstLine), ShowUnsafe==VARIANT_TRUE, RespType)
		? VARIANT_TRUE
		: VARIANT_FALSE;
	
	return S_OK;
}

// Code list to be created. This is a sibling of the explore file. This is needed
// for crerating a codelist when a table is directly given instead of being created from
// micro data.
STDMETHODIMP CTauArgCtrl::SetInCodeList(long NumberofVar, long *VarIndex,
													VARIANT *sCode, long *ErrorCode,
													long *ErrorInVarIndex, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetInCodeList(NumberofVar, DecrementIndex(NumberofVar, VarIndex), B2CStringArray(sCode), ErrorCode, ErrorInVarIndex) ? VARIANT_TRUE : VARIANT_FALSE;
	(*ErrorInVarIndex)++;

	return S_OK;
}

// Once all code list a finished. Set Hierarchies and totals. This happens automatically in
// the end of explore file
STDMETHODIMP CTauArgCtrl::SetTotalsInCodeList(long NumberofVariables, long *VarIndex,
															long *ErrorCode, long *ErrorInVarIndex,
															VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetTotalsInCodeList(NumberofVariables, DecrementIndex(NumberofVariables, VarIndex), ErrorCode, ErrorInVarIndex) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Tells you that you a given a table directly and not to
//create one from micro data
STDMETHODIMP CTauArgCtrl::ThroughTable()
{
	tauArgus.ThroughTable();

	return S_OK;
}

// Set data in table. Since table can be given in many ways.
// See SetTableSafetyInfo, not all input is valid input.
STDMETHODIMP CTauArgCtrl::SetInTable(long Index, VARIANT *sCode,
												double Shadow, double Cost,
												double Resp, long Freq,
												double *MaxScoreCell, double *MaxScoreHolding,
												long Status, double LowerProtectionLevel, double UpperProtectionLevel,
												long *ErrorCode, long *ErrVNum,
												VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetInTable(Index - 1, B2CStringArray(sCode), Shadow, Cost, Resp, Freq, MaxScoreCell, MaxScoreHolding, Status, LowerProtectionLevel, UpperProtectionLevel, ErrorCode, ErrVNum)
		? VARIANT_TRUE
		: VARIANT_FALSE;

	return S_OK;
}

// To state all the cells have been read and the table has to be built.
// In some case the marginals (or sub totals are given ) in other cases they have to be
// calculated
STDMETHODIMP CTauArgCtrl::CompletedTable(long Index, long *ErrorCode,
										 BSTR FileName,
										 VARIANT_BOOL CalculateTotals,
										 VARIANT_BOOL SetCalculatedTotalsAsSafe,
										 VARIANT_BOOL ForCoverTable,
										 VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.CompletedTable(Index - 1, ErrorCode, B2CString(FileName), CalculateTotals==VARIANT_TRUE, SetCalculatedTotalsAsSafe==VARIANT_TRUE, ForCoverTable==VARIANT_TRUE) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// variable set. This is incase table is given
//directly and not built from micro data
STDMETHODIMP CTauArgCtrl::SetVariableForTable(long Index, long nMissing, BSTR Missing1,
															BSTR Missing2, long nDec, VARIANT_BOOL IsPeeper,
															BSTR PeeperCode,
															VARIANT_BOOL IsHierarchical,
															VARIANT_BOOL IsNumeriek, long nPos,
															VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetVariableForTable(Index - 1, nMissing, B2CString(Missing1), B2CString(Missing2), nDec, IsPeeper==VARIANT_TRUE, B2CString(PeeperCode), IsHierarchical==VARIANT_TRUE, IsNumeriek==VARIANT_TRUE, nPos) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// set safety information. this function is used along with set table.
// this is the sibling of settablesafety. for instance holding and weight cannot be made
STDMETHODIMP CTauArgCtrl::SetTableSafetyInfo(long TabIndex,
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
	*pVal = tauArgus.SetTableSafetyInfo(TabIndex - 1, HasMaxScore==VARIANT_TRUE, DominanceRule==VARIANT_TRUE, DominanceNumber, DominancePerc, PQRule==VARIANT_TRUE, PriorPosteriorP, PriorPosteriorQ, PriorPosteriorN, HasFreq==VARIANT_TRUE, CellFreqSafetyPerc, SafeMinRec, HasStatus==VARIANT_TRUE, ManualSafetyPerc, ApplyZeroRule==VARIANT_TRUE, ZeroSafetyRange, EmptyCellAsNonStructural==VARIANT_TRUE, NSEmptySafetyRange, ErrorCode)
		? VARIANT_TRUE
		: VARIANT_FALSE;

	return S_OK;
}

//CTA-value
STDMETHODIMP CTauArgCtrl::SetCTAValues(long TabNr, long CelNr,
                                         double OrgVal, double CTAVal,
                                         long *Sec , VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetCTAValues(TabNr - 1, CelNr - 1, OrgVal, CTAVal, Sec) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Set Lower and upper protection level. This is the one calculated by Anneke
STDMETHODIMP CTauArgCtrl::SetRealizedLowerAndUpper(long TabNr, long CelNr,
																	double RealizedUpper, double RealizedLower,
																	VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetRealizedLowerAndUpper(TabNr - 1, CelNr - 1, RealizedUpper, RealizedLower) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Check if Realized Upper and Lower bound are a better
// approximation than the Protection Level.

STDMETHODIMP CTauArgCtrl::CheckRealizedLowerAndUpperValues(long TabNr, long *pVal)
{
	*pVal = tauArgus.CheckRealizedLowerAndUpperValues(TabNr - 1);

	return S_OK;
}

// given an array of codes, calculate the corresponding indexes or cell number
STDMETHODIMP CTauArgCtrl::ComputeCodesToIndices(long TableIndex, VARIANT *sCode, long *dimIndex, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.ComputeCodesToIndices(TableIndex - 1, B2CStringArray(sCode), dimIndex) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

//Give input file information. This important when you are reading a file with free format
STDMETHODIMP CTauArgCtrl::SetInFileInfo(VARIANT_BOOL IsFixedFormat, BSTR Seperator)
{
	isFixedFormat = IsFixedFormat==VARIANT_TRUE;
	tauArgus.SetInFileInfo(isFixedFormat, B2CString(Seperator));

	return S_OK;
}

// Write Table in AMPL format. This is used in networking
STDMETHODIMP CTauArgCtrl::WriteTableInAMPLFormat(BSTR AMPLFileName, long TableIndex, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.WriteTableInAMPLFormat(B2CString(AMPLFileName), TableIndex - 1) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


STDMETHODIMP CTauArgCtrl::SetProtectionLevelsForFrequencyTable(long TableIndex, long Base,
																					 long K, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetProtectionLevelsForFrequencyTable(TableIndex - 1, Base, K) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

STDMETHODIMP CTauArgCtrl::SetProtectionLevelsForResponseTable(long TableIndex, long *DimIndex,
															   double LowerBound,double UpperBound,
															   VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetProtectionLevelsForResponseTable(TableIndex - 1, DimIndex, LowerBound, UpperBound) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


STDMETHODIMP CTauArgCtrl::MaximumProtectionLevel(long TableIndex, double *Maximum)
{
	*Maximum = tauArgus.MaximumProtectionLevel(TableIndex - 1);

	return S_OK;
}

//GetMinimumCellValue
STDMETHODIMP CTauArgCtrl::GetMinimumCellValue(long TableIndex, double *Maximum, double *Minimum)
{
	*Minimum = tauArgus.GetMinimumCellValue(TableIndex - 1, Maximum);

	return S_OK;
}

STDMETHODIMP CTauArgCtrl::SetRoundedResponse(BSTR RoundedFile, long TableIndex, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetRoundedResponse(B2CString(RoundedFile), TableIndex - 1) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK; 
}


STDMETHODIMP CTauArgCtrl::WriteHierarchicalTableInAMPLFormat(BSTR AMPLFilename, BSTR TempDir,
															 long Tableindex, double MaxScale, long *ErrorCode,  VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.WriteHierarchicalTableInAMPLFormat(B2CString(AMPLFilename), B2CString(TempDir), Tableindex - 1, MaxScale, ErrorCode) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;  
}


STDMETHODIMP CTauArgCtrl::SetSecondaryFromHierarchicalAMPL(BSTR FileName, long TableIndex, long *ErrorCode,
																			  VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetSecondaryFromHierarchicalAMPL(B2CString(FileName), TableIndex - 1, ErrorCode) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

STDMETHODIMP CTauArgCtrl::SetAllEmptyNonStructural(long TableIndex, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetAllEmptyNonStructural(TableIndex - 1) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

STDMETHODIMP CTauArgCtrl::SetSingleEmptyAsNonStructural(long TableIndex, long *DimIndex, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetSingleEmptyAsNonStructural(TableIndex - 1, DimIndex) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


STDMETHODIMP CTauArgCtrl::SetSingleNonStructuralAsEmpty(long TableIndex, long *DimIndex, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetSingleNonStructuralAsEmpty(TableIndex - 1, DimIndex) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


STDMETHODIMP CTauArgCtrl::SetAllNonStructuralAsEmpty(long TableIndex, VARIANT_BOOL *pVal)
{
	*pVal = tauArgus.SetAllNonStructuralAsEmpty(TableIndex - 1) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}
