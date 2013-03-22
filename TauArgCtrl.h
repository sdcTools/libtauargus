// TauArgCtrl.h : Declaration of the CTauArgCtrl

#ifndef __TAUARGCTRL_H_
#define __TAUARGCTRL_H_

#include "resource.h"       // main symbols
#include "Variable.h"
#include "Table.h"
#include "DataCell.h"
#include "Ghmiter.h"
#include "Hitas.h"
#include "JJFormat.h"
#include "NewTauArgusCP.h"
#include "AMPL.h"
#include <locale.h>

/////////////////////////////////////////////////////////////////////////////
// CTauArgCtrl
class ATL_NO_VTABLE CTauArgCtrl :

	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTauArgCtrl, &CLSID_TauArgCtrl>,
	public IConnectionPointContainerImpl<CTauArgCtrl>,
	public IDispatchImpl<ITauArgCtrl, &IID_ITauArgCtrl, &LIBID_NEWTAUARGUSLib>,
	public CProxy_ITauArgCtrlEvents< CTauArgCtrl >
{
private:
	bool m_CompletedCodeList;
	CTable * GetTable(int t);
	int  m_VarNrWeight;
	int  m_VarNrHolding;
	char m_ValueSeparator;
	CString m_ValueTotal;

	CHitas m_hitas;
	CGhmiter m_ghmiter;
	CJJFormat m_jjformat;
	bool m_UsingMicroData;
	bool m_HasMaxScore;
	bool m_HasFreq;
	bool m_HasStatus;
	long m_nRecFile;
	long m_fixedlength;
	long m_fSize;

	long m_ntab;
	bool InFileIsFixedFormat;
	CString InFileSeperator;

	CVariable* m_var;
   CTable* m_tab;
	long m_nvar;
	int m_nNoSense;
	int m_nUntouched;
	int m_nOverlap;
	CString m_WarningRecode;


   void CleanUp();
	void CleanTables();
	void AddTableCells(CTable& t, CDataCell AddCell, int niv, long cellindex);
	void AddTableCell(CTable& t, CDataCell AddCell, long cellindex);
	void FillTables(UCHAR *str);
	BOOL ConvertNumeric(char *code, double &d);
	void QuickSortStringArray(CStringArray &s, int first, int last);
	int  BinSearchStringArray(CStringArray &s, CString x, int nMissing, bool& IsMissing);
	char m_fname[_MAX_PATH];
	int  DoMicroRecord(UCHAR *str, int *varindex);
	int  ReadMicroRecord(FILE *fd, UCHAR *str);
  // int  MakeSafeGHM(int TableIndex, long *nSetSecondary);

// for export files
	void WriteCSVTable(FILE *fd, CTable *tab, long *DimSequence, long *Dims, int niv, char ValueSep, long RespType);
	void WriteCSVColumnLabels(FILE *fd, CTable *tab, long dim, char ValueSep);
	void WriteCSVLabel(FILE *fd, CTable *tab, long dim, int code);
	void WriteCSVCell(FILE *fd, CTable *tab, long *Dim, bool ShowUnsafe, int SBSCode, long RespType);
	void WriteSBSStaart(FILE *fd, CTable *tab, long *Dim, char ValueSep, long SBSCode);
	void WriteCellRecord(FILE *fd, CTable *tab, long *Dims, int niv, char ValueSep, long SBSCode, bool bSBSLevel, BOOL SuppressEmpty, bool ShowUnsafe, long RespType);
   void WriteFirstLine(FILE *fd, LPCTSTR FirstLine);
   void WriteCellDimCell(FILE *fd, CTable *tab, long *Dims, char ValueSep, long SBSCode, bool SBSLevel, bool ShowUnsafe, long RespType);
	void ComputeCellStatuses(CTable &tab);
	void SetProtectionLevels(CTable &tab);

// for debug
	void ShowCodeLists();
	BOOL ShowTable(const char *fname, CTable& tab);
	void ShowTableLayer(FILE *fd, int var1, int var2, int cellnr, CTable &tab);
	int  ShowTableLayerCell(char *str, double val, int ndec);
	int  ShowTableLayerCell(char *str, long val);


	// for RECODE
	bool ReadVariableFreeFormat(UCHAR *Str, long VarIndex, CString *VarCode);
	void InitializeHoldingTables();
	void MergeLastTempShadow();
	BOOL ParseRecodeString(long VarIndex, LPCTSTR RecodeString, long FAR* ErrorType, long FAR* ErrorLine, long FAR* ErrorPos, int Phase);
	BOOL ParseRecodeStringLine(long VarIndex, LPCTSTR str, long FAR* ErrorType, long FAR* ErrorPos, int Phase);
	int  AddRecode(int varnr, const char *newcode);
	int  MakeRecodelistEqualWidth(int VarIndex, LPCTSTR Missing1, LPCTSTR Missing2);
	int  ReadWord(LPCTSTR str, char* CodeFrom, char *CodeTo, char EndCode, int& fromto, int& pos);
	void AddSpacesBefore(char *str, int len);
	void AddSpacesBefore(CString& str, int len);
	int  SetCode2Recode(int VarIndex, char *DestCode, char *SrcCode1, char *SrcCode2, int fromto);
	BOOL ComputeRecodeTables();
	BOOL ComputeRecodeTable(CTable& srctab, CTable& dsttab);
	void ComputeRecodeTableCells(CTable& srctab, CTable& dsttab, int niv, int iCellSrc, int iCellDst);
	void AddTableToTableCell(CTable &tabfrom, CTable &tabto, long ifrom, long ito);
	void SetTableHasRecode();
	BOOL FillInTable(long Index, CString *sCodes, double Cost, double Resp,
						double Shadow, long Freq, double *TopNCell, double *TopNHolding,
						double LPL, double UPL,long Status, long & error, long  & ErrorVarNo);
  	void FireUpdateProgress(short Perc);



	bool IsTable (CTable *tab);
	void TestTable(CTable *tab, long fixeddim, long *DimNr, long niv, bool *IsGoodTable);
	int GetChildren(CVariable &var, int CodeIndex, CUIntArray &Children);

	// To find rand totallen
	void AdjustNonBasalCells(CTable *tab, long TargetDim, long *DimNr, long niv);
	void AdjustTable(CTable *tab);
	long MaxDiepteVanSpanVariablen(CTable *tab);

	// Sub Tables
	long NumberOfSubTables(long IndexTable);
	bool SubTableTupleForSubTable(long TableIndex, long SubTableIndex, long *SubCodeIndices);
	long FindNumberOfElementsInSubTable(long *SubTableTuple, long TableIndex);
	bool FindCellIndexForSubTable(long *TableCellIndex, long TableIndex, long *SubTableTuple,long CellIndexInSubTable, long *SubTableCellIndex);
	long FindSubTableForCell(long CellIndex, long TableIndex, long *SubTableTupleIndex);
	bool WriteHierTableInAMPL(FILE *fd, long tabind, CString TempDir, double MaxScale);
	long SubTableForCellDimension(long TableIndex, long *CellDimension, long * SubTableTupleIndex);
	bool WriteTableSequenceHierarchyInAMPL(FILE *fd, long tabind, long varind);
	bool ArrayCompare (long *arr1, long *arr2, long nDim);
	bool WriteAllSubTablesInAMPL(FILE *fd, long tabind);
	long WriteCellInTempFile(long UnsafeCellNum, long TableIndex, long CellNum, FILE *fdtemp, double MaxScale);
	bool testampl(long ind);
	bool TestSubCodeList();

public:
	CTauArgCtrl()
	{

		//tyuuf
		setlocale(LC_NUMERIC,"english");
		m_var  = 0;
		m_nvar = 0;
		m_tab  = 0;
		m_ntab = 0;
		m_VarNrHolding = -1;
		// Initialize HoldingNr;
		CurrentHoldingNr = -1;
		LastHoldingCode = "";
		m_HasFreq = false;
		m_HasStatus = false;
		m_HasMaxScore = false;
		m_UsingMicroData = true;
		m_CompletedCodeList = false;
		InFileIsFixedFormat = true;

	}
	~CTauArgCtrl()
	{
	// TODO: Cleanup your control's instance data here.

		CleanUp();
	}

DECLARE_REGISTRY_RESOURCEID(IDR_TAUARGCTRL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTauArgCtrl)
	COM_INTERFACE_ENTRY(ITauArgCtrl)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CTauArgCtrl)
CONNECTION_POINT_ENTRY(DIID__ITauArgCtrlEvents)
END_CONNECTION_POINT_MAP()


// ITauArgCtrl
public:
	STDMETHOD(SetAllNonStructuralAsEmpty)(/*[in]*/ long TableIndex, /*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetSingleNonStructuralAsEmpty)(/*[in]*/ long TableIndex,/*[in,out]*/  long *DimIndex,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetSingleEmptyAsNonStructural)(/*[in]*/ long TableIndex, /*[in,out]*/ long *DimIndex, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetAllEmptyNonStructural)(/*[in]*/ long TableIndex, /*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetSecondaryFromHierarchicalAMPL)(/*[in]*/  BSTR FileName, /*[in]*/ long TableIndex,  /*[in,out]*/ long *ErrorCode, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(WriteHierarchicalTableInAMPLFormat)(/*[in]*/ BSTR AMPLFilename, BSTR TempDir, /*[in]*/  long Tableindex, /*[in]*/ double MaxScale, long *ErrorCode,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetRoundedResponse)(/*[in]*/  BSTR RoundedFile,  /*[in]*/ long TableIndex,   /*[retval,out]*/  VARIANT_BOOL *pVal );
	STDMETHOD(MaximumProtectionLevel)(/*[in]*/ long TableIndex, /*[retval,out]*/ double *Maximum);
	STDMETHOD(SetProtectionLevelsForFrequencyTable)(long TableIndex,/*[in]*/ long Base, /*[in]*/ long K, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetTableCellCost)(/*[in]*/ long TableIndex, /*[in,out]*/ long * DimIndex, /*[in]*/ double Cost, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetCellStatusStatistics)(/*[in]*/ long TableIndex, /*[in,out]*/ long * StatusFreq, /*[in,out]*/ long * StatusCellFreq, /*[in,out] */ long * StatusHoldingFreq, /*[in,out]*/ double *StatusCellResponse, /*[in,out]*/ double * StatusCellCost, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(WriteTableInAMPLFormat)(/*[in]*/ BSTR AMPLFileName, /*[in]*/ long TableIndex, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetInFileInfo)(/*[in]*/ VARIANT_BOOL IsFixedFormat, /*[in]*/ BSTR Seperator);
	STDMETHOD(ComputeCodesToIndices)(/*[in]*/ long TableIndex, /*[in,out]*/ VARIANT *sCode, /*[in,out]*/ long *dimIndex, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(CheckRealizedLowerAndUpperValues)(/*[in]*/long TabNr, /*[retval,out]*/  long *pVal);
    STDMETHOD(SetCTAValues)(/*[in]*/ long TabNr, /*[in]*/ long CelNr, /*[in]*/double OrgVal, /*[in]*/double CTAVal,/*[in,out]*/  long *Sec, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetRealizedLowerAndUpper)(/*[in]*/ long TabNr,/*[in]*/ long CelNr, /*[in]*/ double RealizedUpper, /*[in]*/ double RealizedLower, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(UndoSecondarySuppress)(/*[in]*/ long TableIndex, long SortSuppress, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetTableSafetyInfo)(long TabIndex,
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
											long *ErrorCode, VARIANT_BOOL *pVal);
	STDMETHOD(SetVariableForTable)(/*[in]*/ long Index, /*[in]*/long nMissing,  BSTR Missing1, /*[in]*/ BSTR Missing2,
		/*[in]*/ long nDec,VARIANT_BOOL IsPeeper, BSTR PeeperCode, /*[in]*/ VARIANT_BOOL IsHierarchical, /*[in]*/ VARIANT_BOOL IsNumeriek, /*[in]*/ long nPos, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(CompletedTable)(/*[in]*/ long Index, /*[in,out]*/ long * ErrorCode, /*[in]*/ BSTR FileName, /*[in]*/ VARIANT_BOOL CalculateTotals,/*[in]*/ VARIANT_BOOL SetCalculatedTotalsAsSafe,/*[in]*/ VARIANT_BOOL ForCoverTable,/*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetInTable)(/*[in]*/ long Index, /*[in]*/ VARIANT * sCode,
									/*[in]*/ double Shadow, /*[in]*/ double Cost,
									/*[in]*/ double Resp, /*[in]*/ long Freq, /*[in]*/ double * MaxScoreCell,
									double * MaxScoreHolding, /*[in]*/ long Status,double LowerProtectionLevel,
									double UpperProtectionLevel,
									/*[in,out]*/ long * ErrorCode, /*[in,out]*/ long * ErrVNum,/*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(ThroughTable)();
	STDMETHOD(SetTotalsInCodeList)(/*[in]*/ long NumberofVariables,/*[in]*/  long * VarIndex,/*[in,out]*/  long * ErrorCode, /*[in,out]*/  long *  ErrorInVarIndex, /*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetInCodeList)(/*[in]*/ long NumberofVar,  /*[in]*/ long * VarIndex, /*[in]*/ VARIANT * sCode, /*[in,out]*/ long * ErrorCode, /*[in,out]*/ long * ErrorInVarIndex, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(WriteCellRecords)(/*[in]*/ long TableIndex, /*[in]*/ BSTR FileName, /*[in]*/  long SBS, /*[in]*/ VARIANT_BOOL SBSLevel, /*[in]*/ VARIANT_BOOL SuppressEmpty, /*[in]*/ BSTR FirstLine, VARIANT_BOOL ShowUnsafe, long RespType,/*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetTotalTabelSize)(/*[in]*/ long TableIndex,/*[in,out]*/ long* nCell, /*[in,out]*/ long * SizeDataCell);
	STDMETHOD(SetSecondaryJJFORMAT)(/*[in]*/ long TableIndex, /*[in]*/ BSTR FileName, /*[in]*/ VARIANT_BOOL WithBogus, /*[in,out]*/ long * nSetSecondary, /*[retval,out]*/ long * pVal);
	STDMETHOD(WriteJJFormat)(/*[in]*/ long TableIndex, /*[in]*/ BSTR FileName, /*[in]*/ double LowerBound,  /*[in]*/ double UpperBound, /*[in]*/ VARIANT_BOOL WithBogus , VARIANT_BOOL AsPerc, VARIANT_BOOL ForRounding,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(WriteCSV)(/*[in]*/ long TableIndex, /*[in]*/ BSTR FileName, /*[in,out]*/ long * DimSequence,long RespType, /*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(GetCellDistance)(/*[in]*/ long TableIndex, /*[in,out]*/ long * Dims, /*[in,out]*/ long * Distance , /*[retval,out]*/ VARIANT_BOOL *pVal );
	STDMETHOD(PrepareCellDistance)(/*[in]*/ long TableIndex, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetSecondaryGHMITER)(/*[in]*/ BSTR FileName, /*[in]*/ long TableIndex, /*[in,out]*/ long *nSetSecondary, VARIANT_BOOL IsSingleton,/*[retval,out]*/ long *pVal );
	STDMETHOD(WriteGHMITERDataCell)(/*[in]*/ BSTR FileName, /*[in]*/ long TableIndex, VARIANT_BOOL IsSingleton, /*[retval,out]*/ long *pVal);
	STDMETHOD(WriteGHMITERSteuer)(/*[in]*/ BSTR FileName, /*[in]*/ BSTR EndString1, /*[in]*/ BSTR EndString2, /*[in]*/ long TableIndex, /*[retval,out]*/ long *pVal);
	STDMETHOD(GetVarCodeProperties)(/*[in]*/ long VarIndex, /*[in]*/ long CodeIndex, /*[in,out]*/  long * IsParent, /*[in,out]*/  long * IsActive, /*[in,out]*/ long * IsMissing, /*[in,out]*/ long * Level, /*[in,out]*/ long * nChildren, /*[in,out]*/ BSTR * Code, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(UnsafeVariableCodes)(/*[in]*/  long VarIndex, /*[in]*/ long CodeIndex, /*[in,out]*/ long * IsMissing, /*[in,out]*/ long *  Freq, /*[in,out]*/ BSTR * Code, /*[in,out]*/ long * Count, /*[in,out]*/ long * UCArray, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetVarCode)(/*[in]*/ long VarIndex, /*[in]*/ long CodeIndex, /*[in,out]*/ long * CodeType, /*[in,out]*/ BSTR * CodeString, /*[in,out]*/ long * IsMissing, /*[in,out]*/ long * Level, /*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetHierarchicalCodelist)(/*[in]*/  long VarIndex, /*[in]*/ BSTR FileName, /*[in]*/ BSTR LevelString, /*[retval,out]*/ long *pVal);
	STDMETHOD(SetSecondaryHITAS)(/*[in]*/ long TableIndex, /*[in,out]*/ long *nSetSecondary, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(PrepareHITAS)(/*[in]*/ long TableIndex, /*[in]*/ BSTR NameParameterFile, /*[in]*/ BSTR NameFilesFile, /*[in]*/ BSTR TauTemp, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetTableSafety)(	long Index, VARIANT_BOOL DominanceRule,
										long * DominanceNumber,long * DominancePerc,
										VARIANT_BOOL PQRule,long * PriorPosteriorP,
										long * PriorPosteriorQ,long * PriorPosteriorN,
										long * SafeMinRecAndHoldings,
										long * PeepPerc,long * PeepSafetyRange,long * PeepMinFreqCellAndHolding,
										VARIANT_BOOL ApplyPeep,VARIANT_BOOL ApplyWeight, VARIANT_BOOL ApplyWeightOnSafetyRule,
										VARIANT_BOOL ApplyHolding,VARIANT_BOOL ApplyZeroRule,
										VARIANT_BOOL EmptyCellAsNonStructural, long NSEmptySafetyRange,
										long ZeroSafetyRange,	long ManualSafetyPerc,
										long * CellAndHoldingFreqSafetyPerc,
										VARIANT_BOOL *pVal);
	STDMETHOD(GetTableCellValue)(/*[in]*/ long TableIndex, /*[in]*/  long CellIndex, /*[in,out]*/ double *CellResponse, VARIANT_BOOL *pVal);
	STDMETHOD(GetTableCell)(/*[in]*/ long TableIndex, /*[in,out]*/ long * DimIndex, /*[in,out]*/ double*CellResponse,
										/*[in,out] */ long *CellRoundedResp, /*[in,out] */double *CellCTAResp, /*[in,out]*/ double * CellShadow, /*[in,out]*/ double * CellCost,
										/*[in,out]*/ long * CellFreq, /*[in,out]*/ long * CellStatus,
										/*[in,out]*/  double * CellMaxScore, /*[in,out]*/ double * CellMaxScoreWeight,
										long *HoldingFreq,
													 double *HoldingMaxScore, long *HoldingNrPerMaxScore,
													 double * PeepCell, double * PeepHolding, long * PeepSortCell, long * PeepSortHolding,
										double * Lower, double * Upper, double * RealizedLower, double * RealizedUpper,  VARIANT_BOOL *pVal);
	STDMETHOD(SetTable)(/*[in]*/ long Index, /*[in]*/ long nDim,
		/*[in,out]*/ long * ExplanatoryVarList, VARIANT_BOOL IsFrequencyTable,
		/*[in]*/  long ResponseVar, /*[in]*/ long ShadowVar, /*[in]*/ long CostVar,
		double Lambda, double MaxScaledCost,
		long PeepVarnr,VARIANT_BOOL SetMissingAsSafe, VARIANT_BOOL *pVal);
	STDMETHOD(SetVariable)(/*[in]*/ long VarIndex, /*[in]*/ long bPos, /*[in]*/ long nPos,
		/*[in]*/ long nDec, long nMissing,/*[in]*/ BSTR Missing1, /*[in]*/ BSTR Missing2,/*[in]*/ BSTR TotalCode,VARIANT_BOOL IsPeeper, BSTR PeeperCode1, BSTR PeeperCode2, /*[in]*/ VARIANT_BOOL IsCategorical,
		/*[in]*/  VARIANT_BOOL IsNumeric, /*[in]*/  VARIANT_BOOL IsWeight, /*[in]*/ VARIANT_BOOL IsHierarchical,
		/*[in]*/ VARIANT_BOOL IsHolding,
		/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(DoActiveRecode)(/*[in]*/ long VarIndex, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetVarNumberOfCodes)(/*[in]*/ long VarIndex, /*[in,out]*/ long *NumberOfCodes, /*[in,out]*/ long * NumberOfActiveCodes, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetVarCodeActive)(/*[in]*/ long VarIndex, /*[in]*/ long CodeIndex, /*[in]*/ VARIANT_BOOL Active, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetStatusAndCostPerDim)(/*[in]*/ long TableIndex, /*[in,out]*/ long *Status, /*[in,out]*/ double *Cost, /*[retval,out]*/ VARIANT_BOOL  *pVal);
	STDMETHOD(SetTableCellStatus)(/*[in]*/ long TableIndex, /*[in,out]*/ long * DimIndex, /*[in]*/ long CelStatus, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(UndoRecode)(/*[in]*/ long VarIndex, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetMaxnUc)(/*[retval,out]*/ long *pVal);
	STDMETHOD(ExploreFile)(/*[in]*/ BSTR FileName, /*[in,out]*/ long * ErrorCode,  /*[in,out]*/ long * LineNumber, /*[in,out]*/ long * VarIndex, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(UnsafeVariable)( /*[in]*/ long VarIndex,/*[in,out]*/ long *Count, /*[in,out]*/ long * UCArray, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetTableRow)( /*[in]*/ long TableIndex, /*[in,out]*/ long * DimIndex, /*[in,out]*/ double * Cell, /*[in,out]*/ long *Status, /*[in]*/ long CountType, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetHierarchicalDigits)( /*[in]*/ long VarIndex, /*[in]*/ long nDigitPairs, /*[in]*/ long *nDigits, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(CleanAll)();
	STDMETHOD(ApplyRecode)();
	STDMETHOD(DoRecode)( /*[in]*/ long VarIndex, /*[in]*/ BSTR RecodeString, long nMissing,/*[in]*/ BSTR eMissing1, /*[in]*/ BSTR eMissing2,  /*[in.out]*/ long *ErrorType, /*[in,out]*/ long * ErrorLine, /*[in,out]*/ long * ErrorPos, /*[in,out]*/ BSTR * WarningString, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(ComputeTables)( /*[in,out]*/ long * ErrorCode, /*[in,out]*/ long * TableIndex, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetNumberTab)( /*[in]*/ long nTab, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetNumberVar)(/*[in]*/ long nVar, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetMinimumCellValue)(/*[in]*/ long TableIndex, /*[in,out]*/ double *Maximum, /*[retval,out]*/ double *Minimum);
	STDMETHOD(SetProtectionLevelsForResponseTable)(long TableIndex,/*[in,out]*/ long * DimIndex,/*[in]*/ double LowerBound, /*[in]*/ double UpperBound, /*[retval,out]*/ VARIANT_BOOL *pVal);

};

#endif //__TAUARGCTRL_H_
