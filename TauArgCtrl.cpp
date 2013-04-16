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
	return tauArgus.UndoSecondarySuppress(TableIndex, SortSuppress, pVal);  
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
	char *recodeString = _com_util::ConvertBSTRToString(RecodeString);
	char *missing1 = _com_util::ConvertBSTRToString(eMissing1);
	char *missing2 = _com_util::ConvertBSTRToString(eMissing2);
	const char* warningString;

	*pVal = tauArgus.DoRecode(VarIndex - 1, recodeString, nMissing, missing1, missing2, ErrorType, ErrorLine, ErrorPos, &warningString) ? VARIANT_TRUE : VARIANT_FALSE;

	*WarningString = _com_util::ConvertStringToBSTR(warningString);

	delete[] recodeString;
	delete[] missing1;
	delete[] missing2;

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
	return tauArgus.UnsafeVariable(VarIndex, Count, UCArray, pVal);
}

// In this function the input file is read and the code list is built
STDMETHODIMP CTauArgCtrl::ExploreFile(BSTR FileName, long *ErrorCode,
													long *LineNumber, long *VarIndex,
			
													VARIANT_BOOL *pVal)
{
	char* fileName = _com_util::ConvertBSTRToString(FileName);

	*pVal = tauArgus.ExploreFile(fileName, ErrorCode, LineNumber, VarIndex) ? VARIANT_TRUE : VARIANT_FALSE;
	*VarIndex = *VarIndex + 1;

	delete[] fileName;

	return S_OK;
}

// get maximum unsafe Combination
STDMETHODIMP CTauArgCtrl::GetMaxnUc(long *pVal)
{
	return tauArgus.GetMaxnUc(pVal);
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
	*pVal = tauArgus.SetTableCellCost(TableIndex -1, DimIndex, Cost) ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

// Get Status and Cost per dimensie
STDMETHODIMP CTauArgCtrl::GetStatusAndCostPerDim(long TableIndex, long *Status,
																double *Cost, VARIANT_BOOL *pVal)
{
	return tauArgus.GetStatusAndCostPerDim(TableIndex, Status, Cost, pVal);
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
	char *missing1 = _com_util::ConvertBSTRToString(Missing1);
	char *missing2 = _com_util::ConvertBSTRToString(Missing2);
	char *totalCode = _com_util::ConvertBSTRToString(TotalCode);
	char *peeperCode1 = _com_util::ConvertBSTRToString(PeeperCode1);
	char *peeperCode2 = _com_util::ConvertBSTRToString(PeeperCode2);

	*pVal = tauArgus.SetVariable(VarIndex - 1, bPos, nPos, nDec, nMissing, missing1, missing2, totalCode, IsPeeper==VARIANT_TRUE, peeperCode1, peeperCode2, IsCategorical==VARIANT_TRUE, IsNumeric==VARIANT_TRUE, IsWeight==VARIANT_TRUE, IsHierarchical==VARIANT_TRUE, IsHolding==VARIANT_TRUE)
		? VARIANT_TRUE 
		: VARIANT_FALSE;

	delete[] missing1;
	delete[] missing2;
	delete[] totalCode;
	delete[] peeperCode1;
	delete[] peeperCode2;

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
	long* ExpVar = new long[nDim];
	for (int i=0; i< nDim; i++) 
	{
		ExpVar[i] = ExplanatoryVarList[i] - 1;
	}

	*pVal = tauArgus.SetTable(Index - 1, nDim, ExpVar, IsFrequencyTable==VARIANT_TRUE, ResponseVar - 1, ShadowVar - 1, CostVar > 0 ? CostVar - 1 : CostVar,  Lambda, MaxScaledCost, PeepVarnr - 1, SetMissingAsSafe==VARIANT_TRUE)
		? VARIANT_TRUE 
		: VARIANT_FALSE;

	delete[] ExpVar;

	return S_OK;
}


STDMETHODIMP CTauArgCtrl::GetTableCellValue(long TableIndex, long CellIndex,
                                            double *CellResponse, VARIANT_BOOL *pVal)
{
	return tauArgus.GetTableCellValue(TableIndex -1, CellIndex, CellResponse, pVal);
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
	return tauArgus.PrepareHITAS(TableIndex, NameParameterFile, NameFilesFile, TauTemp, pVal);
}

// Cells that are found to be secondary unsafe by Hitas is set as Unsafe in the table
STDMETHODIMP CTauArgCtrl::SetSecondaryHITAS(long TableIndex, long *nSetSecondary,
														 VARIANT_BOOL *pVal)
{
	return tauArgus.SetSecondaryHITAS(TableIndex, nSetSecondary, pVal);
}

STDMETHODIMP CTauArgCtrl::SetHierarchicalCodelist(long VarIndex, BSTR FileName,
																 BSTR LevelString, long *pVal)
{
	char* fileName = _com_util::ConvertBSTRToString(FileName);
	char* levelString = _com_util::ConvertBSTRToString(LevelString);

	*pVal = tauArgus.SetHierarchicalCodelist(VarIndex - 1, fileName, levelString);

	delete[] fileName;
	delete[] levelString;

	return S_OK;
}

// Gets a code if given an index and a variable number
STDMETHODIMP CTauArgCtrl::GetVarCode(long VarIndex, long CodeIndex,
												long *CodeType, BSTR *CodeString,
												long *IsMissing, long *Level,
												VARIANT_BOOL *pVal)
{
	const char* codeString;

	*pVal = tauArgus.GetVarCode(VarIndex - 1, CodeIndex, CodeType, &codeString, IsMissing, Level)
		? VARIANT_TRUE 
		: VARIANT_FALSE;

	*CodeString = _com_util::ConvertStringToBSTR(codeString);

	return S_OK;
}


// return the codes for unsafe variables
STDMETHODIMP CTauArgCtrl::UnsafeVariableCodes(long VarIndex, long CodeIndex,
															long *IsMissing, long *Freq,
															BSTR *Code, long *Count,
															long *UCArray, VARIANT_BOOL *pVal)
{
	return tauArgus.UnsafeVariableCodes(VarIndex, CodeIndex, IsMissing, Freq, Code, Count, UCArray, pVal);
}

// return properties given a Variable and Code Index
STDMETHODIMP CTauArgCtrl::GetVarCodeProperties(long VarIndex, long CodeIndex,
															 long *IsParent, long *IsActive,
															 long *IsMissing, long *Level,
															 long *nChildren, BSTR *Code,
															 VARIANT_BOOL *pVal)
{
	const char* code;

	*pVal = tauArgus.GetVarCodeProperties(VarIndex - 1, CodeIndex, IsParent, IsActive, IsMissing, Level, nChildren, &code);

	*Code = _com_util::ConvertStringToBSTR(code);

	return S_OK;
}

// Write Table in GHmiter file. This is used in secondary supressions
STDMETHODIMP CTauArgCtrl::WriteGHMITERSteuer(BSTR FileName, BSTR EndString1,
														  BSTR EndString2, long TableIndex,
														  long *pVal)
{
	return tauArgus.WriteGHMITERSteuer(FileName, EndString1, EndString2, TableIndex, pVal);
}

// Write data Cells for GHMiter.
STDMETHODIMP CTauArgCtrl::WriteGHMITERDataCell(BSTR FileName, long TableIndex,
															  VARIANT_BOOL IsSingleton,
															 long *pVal)
{
	return tauArgus.WriteGHMITERDataCell(FileName, TableIndex, IsSingleton, pVal);
}

// Cells that are given as secondary Unsafe by GHmiter
// to be set in the table
STDMETHODIMP CTauArgCtrl::SetSecondaryGHMITER(BSTR FileName, long TableIndex,
															long *nSetSecondary,VARIANT_BOOL IsSingleton,
															long *pVal)
{
	return tauArgus.SetSecondaryGHMITER(FileName, TableIndex, nSetSecondary, IsSingleton, pVal);
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
	return tauArgus.GetCellStatusStatistics(TableIndex, StatusFreq, StatusCellFreq, StatusHoldingFreq, StatusCellResponse, StatusCellCost, pVal);
}


// This function is not usedb at the moment
STDMETHODIMP CTauArgCtrl::PrepareCellDistance(long TableIndex, VARIANT_BOOL *pVal)
{
	return tauArgus.PrepareCellDistance(TableIndex, pVal);
}

// This function is not used either at the moment
STDMETHODIMP CTauArgCtrl::GetCellDistance(long TableIndex, long *Dims,
													  long *Distance, VARIANT_BOOL *pVal)
{
	return tauArgus.GetCellDistance(TableIndex, Dims, Distance, pVal);
}

// Write a table as Comma seperated file
STDMETHODIMP CTauArgCtrl::WriteCSV(long TableIndex, BSTR FileName,
											 long *DimSequence, long RespType, VARIANT_BOOL *pVal)
{
	return tauArgus.WriteCSV(TableIndex, FileName, DimSequence, RespType, pVal);
}

// Write Table in JJ Format
STDMETHODIMP CTauArgCtrl::WriteJJFormat(long TableIndex, BSTR FileName,
													double LowerBound, double UpperBound,
													VARIANT_BOOL WithBogus, VARIANT_BOOL AsPerc,
													VARIANT_BOOL ForRounding,VARIANT_BOOL *pVal)
{
	return tauArgus.WriteJJFormat(TableIndex, FileName, LowerBound, UpperBound, WithBogus, AsPerc, ForRounding, pVal);
}
// Cells that are give as secondary unsafe by JJ to be set in the table
STDMETHODIMP CTauArgCtrl::SetSecondaryJJFORMAT(long TableIndex, BSTR FileName, VARIANT_BOOL WithBogus, long *nSetSecondary, long *pVal)
{
	return tauArgus.SetSecondaryJJFORMAT(TableIndex, FileName, WithBogus, nSetSecondary, pVal);
}

// Get Size of a tabel. This function has to be changed
STDMETHODIMP CTauArgCtrl::GetTotalTabelSize(long TableIndex,
														   long* nCell,long * SizeDataCell)
{
	return tauArgus.GetTotalTabelSize(TableIndex, nCell, SizeDataCell);
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
	return tauArgus.WriteCellRecords(TableIndex, FileName, SBS, SBSLevel, SuppressEmpty, FirstLine, ShowUnsafe, RespType, pVal);
}

// Code list to be created. This is a sibling of the explore file. This is needed
// for crerating a codelist when a table is directly given instead of being created from
// micro data.
STDMETHODIMP CTauArgCtrl::SetInCodeList(long NumberofVar, long *VarIndex,
													VARIANT *sCode, long *ErrorCode,
													long *ErrorInVarIndex, VARIANT_BOOL *pVal)
{
	return tauArgus.SetInCodeList(NumberofVar, VarIndex, sCode, ErrorCode, ErrorInVarIndex, pVal);
}

// Once all code list a finished. Set Hierarchies and totals. This happens automatically in
// the end of explore file
STDMETHODIMP CTauArgCtrl::SetTotalsInCodeList(long NumberofVariables, long *VarIndex,
															long *ErrorCode, long *ErrorInVarIndex,
															VARIANT_BOOL *pVal)
{
	return tauArgus.SetTotalsInCodeList(NumberofVariables, VarIndex, ErrorCode, ErrorInVarIndex, pVal);
}

// Tells you that you a given a table directly and not to
//create one from micro data
STDMETHODIMP CTauArgCtrl::ThroughTable()
{
	return tauArgus.ThroughTable();
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
	return tauArgus.SetInTable(Index, sCode, Shadow, Cost, Resp, Freq, MaxScoreCell, MaxScoreHolding, Status, LowerProtectionLevel, UpperProtectionLevel, ErrorCode, ErrVNum, pVal);
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
	return tauArgus.CompletedTable(Index, ErrorCode, FileName, CalculateTotals, SetCalculatedTotalsAsSafe, ForCoverTable, pVal);
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
	return tauArgus.SetVariableForTable(Index, nMissing, Missing1, Missing2, nDec, IsPeeper, PeeperCode, IsHierarchical, IsNumeriek, nPos, pVal);
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
	return tauArgus.SetTableSafetyInfo(TabIndex, HasMaxScore, DominanceRule, DominanceNumber, DominancePerc, PQRule, PriorPosteriorP, PriorPosteriorQ, PriorPosteriorN, HasFreq, CellFreqSafetyPerc, SafeMinRec, HasStatus, ManualSafetyPerc, ApplyZeroRule, ZeroSafetyRange, EmptyCellAsNonStructural, NSEmptySafetyRange, ErrorCode, pVal);
}

//CTA-value
STDMETHODIMP CTauArgCtrl::SetCTAValues(long TabNr, long CelNr,
                                         double OrgVal, double CTAVal,
                                         long *Sec , VARIANT_BOOL *pVal)
{
	return tauArgus.SetCTAValues(TabNr, CelNr, OrgVal, CTAVal, Sec , pVal);
}

// Set Lower and upper protection level. This is the one calculated by Anneke
STDMETHODIMP CTauArgCtrl::SetRealizedLowerAndUpper(long TabNr, long CelNr,
																	double RealizedUpper, double RealizedLower,
																	VARIANT_BOOL *pVal)
{
	return tauArgus.SetRealizedLowerAndUpper(TabNr, CelNr, RealizedUpper, RealizedLower, pVal);
}

// Check if Realized Upper and Lower bound are a better
// approximation than the Protection Level.

STDMETHODIMP CTauArgCtrl::CheckRealizedLowerAndUpperValues(long TabNr, long *pVal)
{
	return tauArgus.CheckRealizedLowerAndUpperValues(TabNr, pVal);
}

// given an array of codes, calculate the corresponding indexes or cell number
STDMETHODIMP CTauArgCtrl::ComputeCodesToIndices(long TableIndex, VARIANT *sCode, long *dimIndex, VARIANT_BOOL *pVal)
{
	return tauArgus.ComputeCodesToIndices(TableIndex, sCode, dimIndex, pVal);
}

//Give input file information. This important when you are reading a file with free format
STDMETHODIMP CTauArgCtrl::SetInFileInfo(VARIANT_BOOL IsFixedFormat, BSTR Seperator)
{
	char* Sep = _com_util::ConvertBSTRToString(Seperator);
	
	tauArgus.SetInFileInfo(IsFixedFormat==VARIANT_TRUE, Sep);

	delete[] Sep;

	return S_OK;
}

// Write Table in AMPL format. This is used in networking
STDMETHODIMP CTauArgCtrl::WriteTableInAMPLFormat(BSTR AMPLFileName, long TableIndex, VARIANT_BOOL *pVal)
{
	return tauArgus.WriteTableInAMPLFormat(AMPLFileName, TableIndex, pVal);
}


STDMETHODIMP CTauArgCtrl::SetProtectionLevelsForFrequencyTable(long TableIndex, long Base,
																					 long K, VARIANT_BOOL *pVal)
{
	return tauArgus.SetProtectionLevelsForFrequencyTable(TableIndex, Base, K, pVal);
}

STDMETHODIMP CTauArgCtrl::SetProtectionLevelsForResponseTable(long TableIndex, long *DimIndex,
															   double LowerBound,double UpperBound,
															   VARIANT_BOOL *pVal)
{
	return tauArgus.SetProtectionLevelsForResponseTable(TableIndex, DimIndex, LowerBound, UpperBound, pVal);
}


STDMETHODIMP CTauArgCtrl::MaximumProtectionLevel(long TableIndex, double *Maximum)
{
	return tauArgus.MaximumProtectionLevel(TableIndex, Maximum);
}

//GetMinimumCellValue
STDMETHODIMP CTauArgCtrl::GetMinimumCellValue(long TableIndex, double *Maximum, double *Minimum)
{
	*Minimum = tauArgus.GetMinimumCellValue(TableIndex - 1, Maximum);

	return S_OK;
}

STDMETHODIMP CTauArgCtrl::SetRoundedResponse(BSTR RoundedFile, long TableIndex, VARIANT_BOOL *pVal)
{
	return tauArgus.SetRoundedResponse(RoundedFile, TableIndex, pVal);
}


STDMETHODIMP CTauArgCtrl::WriteHierarchicalTableInAMPLFormat(BSTR AMPLFilename, BSTR TempDir,
															 long Tableindex, double MaxScale, long *ErrorCode,  VARIANT_BOOL *pVal)
{
	return tauArgus.WriteHierarchicalTableInAMPLFormat(AMPLFilename, TempDir, Tableindex, MaxScale, ErrorCode,  pVal);
}


STDMETHODIMP CTauArgCtrl::SetSecondaryFromHierarchicalAMPL(BSTR FileName,
																			  long TableIndex, long *ErrorCode,
																			  VARIANT_BOOL *pVal)
{
	return tauArgus.SetSecondaryFromHierarchicalAMPL(FileName, TableIndex, ErrorCode, pVal);
}

STDMETHODIMP CTauArgCtrl::SetAllEmptyNonStructural(long TableIndex, VARIANT_BOOL *pVal)
{
	return tauArgus.SetAllEmptyNonStructural(TableIndex, pVal);
}

STDMETHODIMP CTauArgCtrl::SetSingleEmptyAsNonStructural(long TableIndex, long *DimIndex, VARIANT_BOOL *pVal)
{
	return tauArgus.SetSingleEmptyAsNonStructural(TableIndex, DimIndex, pVal);
}


STDMETHODIMP CTauArgCtrl::SetSingleNonStructuralAsEmpty(long TableIndex, long *DimIndex, VARIANT_BOOL *pVal)
{
	return tauArgus.SetSingleNonStructuralAsEmpty(TableIndex, DimIndex, pVal);
}


STDMETHODIMP CTauArgCtrl::SetAllNonStructuralAsEmpty(long TableIndex, VARIANT_BOOL *pVal)
{
	return tauArgus.SetAllNonStructuralAsEmpty(TableIndex, pVal);
}
