// TauArgCtrl.h : Declaration of the CTauArgCtrl

#ifndef __TAUARGCTRL_H_
#define __TAUARGCTRL_H_

#include <locale.h>

#include "TauArgus.h"
#include "resource.h"       // main symbols
#include "NewTauArgusCP.h"

/////////////////////////////////////////////////////////////////////////////
// CTauArgCtrl
class ATL_NO_VTABLE CTauArgCtrl :

	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTauArgCtrl, &CLSID_TauArgCtrl>,
	public IConnectionPointContainerImpl<CTauArgCtrl>,
	public IDispatchImpl<ITauArgCtrl, &IID_ITauArgCtrl, &LIBID_NEWTAUARGUSLib>,
	public CProxy_ITauArgCtrlEvents< CTauArgCtrl >,
	public IProgressListener
{
private:
	TauArgus tauArgus;

public:
	CTauArgCtrl() : tauArgus()
	{
		tauArgus.SetProgressListener(this);
	}

  	void UpdateProgress(int Perc);

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
