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

#ifndef TauArgus_h
#define TauArgus_h

#include <map>
#include <clocale>
#include <vector>
#include <string>

//#include "resource.h"       // main symbols
#include "Variable.h"
#include "Table.h"
#include "DataCell.h"
#include "Ghmiter.h"
#include "Hitas.h"
#include "JJFormat.h"
#include "AMPL.h"
#include "IProgressListener.h"
#include "PTableCont.h"

//class __declspec(dllexport) TauArgus
class TauArgus
{
private:
	IProgressListener* m_ProgressListener;
  	void FireUpdateProgress(int Perc);

	bool m_CompletedCodeList;
	CTable * GetTable(int t);
	int  m_VarNrWeight;
	int  m_VarNrHolding;
        int  m_VarNrRecordKey;
	char m_ValueSeparator;
	std::string m_ValueTotal;

	CHitas m_hitas;
	CGhmiter m_ghmiter;
	CJJFormat m_jjformat;
	bool m_UsingMicroData;
	bool m_HasMaxScore;
	bool m_HasFreq;
	bool m_HasStatus;
	long m_nRecFile;
	long m_fixedlength;
        long m_maxBPos;
	long m_fSize;

	long m_ntab;
	bool InFileIsFixedFormat;
	std::string InFileSeperator;

	CVariable* m_var;
	CTable* m_tab;
	long m_nvar;
	int m_nNoSense;
	int m_nUntouched;
	int m_nOverlap;
	std::string m_WarningRecode;


        void CleanUp();
	void CleanTables();
	void AddTableCells(CTable& t, CDataCell AddCell, int niv, long cellindex);
	void AddTableCell(CTable& t, CDataCell AddCell, long cellindex);
	void FillTables(char *str);
	bool ConvertNumeric(char *code, double &d);
	char m_fname[MAX_PATH];
	int  DoMicroRecord(char *str, int *varindex);
	int  ReadMicroRecord(FILE *fd, char *str);
  // int  MakeSafeGHM(int TableIndex, long *nSetSecondary);

// for export files
	void WriteCSVTable(FILE *fd, CTable *tab, bool EmbedQuotes, long *DimSequence, long *Dims, int niv, char ValueSep, long RespType);
	void WriteCSVColumnLabels(FILE *fd, CTable *tab, long dim, char ValueSep, bool EmbedQuotes);
	void WriteCSVLabel(FILE *fd, CTable *tab, long dim, int code, bool EmbedQuotes);
	void WriteCSVCell(FILE *fd, CTable *tab, long *Dim, bool ShowUnsafe, int SBSCode, long RespType);
	void WriteSBSStaart(FILE *fd, CTable *tab, long *Dim, char ValueSep, long SBSCode);
	void WriteCellRecord(FILE *fd, CTable *tab, long *Dims, int niv, char ValueSep, long SBSCode, bool bSBSLevel, bool SuppressEmpty, bool ShowUnsafe, bool EmbedQuotes, long RespType);
        void WriteFirstLine(FILE *fd, LPCTSTR FirstLine);
        void WriteCellDimCell(FILE *fd, CTable *tab, long *Dims, char ValueSep, long SBSCode, bool SBSLevel, bool ShowUnsafe, bool EmbedQuotes, long RespType);
	void ComputeCellStatuses(CTable &tab);
	void SetProtectionLevels(CTable &tab);
        void ComputeCellKeys(CTable &tab);

// for debug
	void ShowCodeLists();
	bool ShowTable(const char *fname, CTable& tab);
	void ShowTableLayer(FILE *fd, int var1, int var2, int cellnr, CTable &tab);
	int  ShowTableLayerCell(char *str, double val, int ndec);
	int  ShowTableLayerCell(char *str, long val);


	// for RECODE
        bool ReadVariablesFreeFormat(char *Str, std::vector<char *> &VarCodes);
	void InitializeHoldingTables();
	void MergeLastTempShadow();
	bool ParseRecodeString(long VarIndex, LPCTSTR RecodeString, long FAR* ErrorType, long FAR* ErrorLine, long FAR* ErrorPos, int Phase);
	bool ParseRecodeStringLine(long VarIndex, LPCTSTR str, long FAR* ErrorType, long FAR* ErrorPos, int Phase);
	int  AddRecode(int varnr, const char *newcode);
	int  MakeRecodelistEqualWidth(int VarIndex, LPCTSTR Missing1, LPCTSTR Missing2);
	int  ReadWord(LPCTSTR str, char* CodeFrom, char *CodeTo, char EndCode, int& fromto, int& pos);
	int  SetCode2Recode(int VarIndex, char *DestCode, char *SrcCode1, char *SrcCode2, int fromto);
	bool ComputeRecodeTables();
	bool ComputeRecodeTable(CTable& srctab, CTable& dsttab);
	void ComputeRecodeTableCells(CTable& srctab, CTable& dsttab, int niv, int iCellSrc, int iCellDst);
	void AddTableToTableCell(CTable &tabfrom, CTable &tabto, long ifrom, long ito);
	void SetTableHasRecode();
	bool FillInTable(long Index, std::string *sCodes, double Cost, double Resp,
						double Shadow, long Freq, double *TopNCell, double *TopNHolding,
						double LPL, double UPL,long Status, long & error, long  & ErrorVarNo);


	bool IsTable (CTable *tab);
	void TestTable(CTable *tab, long fixeddim, long *DimNr, long niv, bool *IsGoodTable);
	int GetChildren(CVariable &var, int CodeIndex, std::vector<unsigned int> &Children);

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
	bool WriteHierTableInAMPL(FILE *fd, long tabind, const std::string &TempDir, double MaxScale);
	long SubTableForCellDimension(long TableIndex, long *CellDimension, long * SubTableTupleIndex);
	bool WriteTableSequenceHierarchyInAMPL(FILE *fd, long tabind, long varind);
	bool ArrayCompare (long *arr1, long *arr2, long nDim);
	bool WriteAllSubTablesInAMPL(FILE *fd, long tabind);
	long WriteCellInTempFile(long UnsafeCellNum, long TableIndex, long CellNum, FILE *fdtemp, double MaxScale);
	bool testampl(long ind);
	bool TestSubCodeList();

public:
	TauArgus()
	{

		//tyuuf
            // REALY NEEDED ??????
		setlocale(LC_NUMERIC,"english"); // REALY NEEDED ??????
            // REALY NEEDED ??????
		m_ProgressListener = NULL;;
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

	~TauArgus()
	{
		CleanUp();
	}

public:
	void SetProgressListener(IProgressListener *ProgressListener);
	bool SetAllNonStructuralAsEmpty(/*[in]*/ long TableIndex);
	bool SetSingleNonStructuralAsEmpty(/*[in]*/ long TableIndex,/*[in,out]*/  long *DimIndex);
	bool SetSingleEmptyAsNonStructural(/*[in]*/ long TableIndex, /*[in,out]*/ long *DimIndex);
	bool SetAllEmptyNonStructural(/*[in]*/ long TableIndex);
	bool SetSecondaryFromHierarchicalAMPL(/*[in]*/  const char* FileName, /*[in]*/ long TableIndex,  /*[in,out]*/ long* ErrorCode);
	bool WriteHierarchicalTableInAMPLFormat(/*[in]*/ const char* AMPLFilename, const char* TempDir, /*[in]*/  long Tableindex, /*[in]*/ double MaxScale, long* ErrorCode);
	bool SetRoundedResponse(/*[in]*/  const char* RoundedFile,  /*[in]*/ long TableIndex);
	double MaximumProtectionLevel(/*[in]*/ long TableIndex);
	bool SetProtectionLevelsForFrequencyTable(long TableIndex,/*[in]*/ long Base, /*[in]*/ long K);
	bool SetTableCellCost(/*[in]*/ long TableIndex, /*[in,out]*/ long* DimIndex, /*[in]*/ double Cost);
	bool GetCellStatusStatistics(/*[in]*/ long TableIndex, /*[in,out]*/ long* StatusFreq, /*[in,out]*/ long* StatusCellFreq, /*[in,out] */ long* StatusHoldingFreq, /*[in,out]*/ double *StatusCellResponse, /*[in,out]*/ double* StatusCellCost);
	bool WriteTableInAMPLFormat(/*[in]*/ const char* AMPLFileName, /*[in]*/ long TableIndex);
	void SetInFileInfo(/*[in]*/ bool IsFixedFormat, /*[in]*/ const char* Seperator);
	bool ComputeCodesToIndices(/*[in]*/ long TableIndex, /*[in,out]*/ char* sCode[], /*[in,out]*/ long *DimIndex);
	long CheckRealizedLowerAndUpperValues(/*[in]*/long TabNr);
        bool SetCTAValues(/*[in]*/ long TabIndex, /*[in]*/ long CelNr, /*[in]*/double OrgVal, /*[in]*/double CTAVal,/*[in,out]*/  long *Sec);
	bool SetRealizedLowerAndUpper(/*[in]*/ long TabNr,/*[in]*/ long CelNr, /*[in]*/ double RealizedUpper, /*[in]*/ double RealizedLower);
	bool UndoSecondarySuppress(/*[in]*/ long TableIndex, long SortSuppress);
	bool SetTableSafetyInfo(long TabIndex, bool HasMaxScore, bool DominanceRule, long* DominanceNumber, long* DominancePerc, bool PQRule,
				long* PriorPosteriorP, long* PriorPosteriorQ, long* PriorPosteriorN,	bool HasFreq, long CellFreqSafetyPerc, 
                                long SafeMinRec, bool HasStatus, long ManualSafetyPerc,	bool ApplyZeroRule, double ZeroSafetyRange, 
                                bool EmptyCellAsNonStructural, long NSEmptySafetyRange,	long *ErrorCode);
	bool SetVariableForTable(/*[in]*/ long Index, /*[in]*/long nMissing,  const char* Missing1, /*[in]*/ const char* Missing2, /*[in]*/ const char* TotalCode,
                                 /*[in]*/ long nDec, bool IsPeeper, const char* PeeperCode, /*[in]*/ bool IsHierarchical, /*[in]*/ bool IsNumeriek, 
                                 /*[in]*/ long nPos);
	bool CompletedTable(/*[in]*/ long Index, /*[in,out]*/ long * ErrorCode, /*[in]*/ const char* FileName, /*[in]*/ bool CalculateTotals,
                            /*[in]*/ bool SetCalculatedTotalsAsSafe,/*[in]*/ bool ForCoverTable);
	bool SetInTable(/*[in]*/ long Index, /*[in]*/ char* sCode[], /*[in]*/ double Shadow, /*[in]*/ double Cost, /*[in]*/ double Resp, 
                        /*[in]*/ long Freq, /*[in]*/ double* MaxScoreCell, /*[in]*/ double* MaxScoreHolding, /*[in]*/ long Status,double LowerProtectionLevel,
                        double UpperProtectionLevel, /*[in,out]*/ long * ErrorCode, /*[in,out]*/ long * ErrVNum);
	void ThroughTable();
	bool SetTotalsInCodeList(/*[in]*/ long NumberofVariables,/*[in]*/  long* VarIndex,/*[in,out]*/  long* ErrorCode, /*[in,out]*/  long*  ErrorInVarIndex);
	bool SetInCodeList(/*[in]*/ long NumberofVar,  /*[in]*/ long* VarIndex, /*[in]*/ char* sCode[], /*[in,out]*/ long* ErrorCode, /*[in,out]*/ long* ErrorInVarIndex);
	bool WriteCellRecords(/*[in]*/ long TableIndex, /*[in]*/ const char* FileName, /*[in]*/  long SBS, /*[in]*/ bool SBSLevel, /*[in]*/ bool SuppressEmpty, /*[in]*/ const char* FirstLine, bool ShowUnsafe, /*[in]*/ bool EmbedQuotes, long RespType);
	void GetTotalTabelSize(/*[in]*/ long TableIndex,/*[in,out]*/ long* nCell, /*[in,out]*/ long* SizeDataCell);
	long SetSecondaryJJFORMAT(/*[in]*/ long TableIndex, /*[in]*/ const char* FileName, /*[in]*/ bool WithBogus, /*[in,out]*/ long * nSetSecondary);
	bool WriteJJFormat(/*[in]*/ long TableIndex, /*[in]*/ const char* FileName, /*[in]*/ double LowerBound,  /*[in]*/ double UpperBound, /*[in]*/ bool WithBogus , bool AsPerc, bool ForRounding);
	bool WriteCSV(/*[in]*/ long TableIndex, /*[in]*/ const char* FileName, /*[in]*/ bool EmbedQuotes, /*[in,out]*/ long * DimSequence,long RespType);
	//Not used! bool WriteCSVBasic(/*[in]*/ long TableIndex, /*[in]*/ const char* FileName, /*[in,out]*/ long * DimSequence,long RespType);
	bool GetCellDistance(/*[in]*/ long TableIndex, /*[in,out]*/ long *DimIndex, /*[in,out]*/ long * Distance);
	bool PrepareCellDistance(/*[in]*/ long TableIndex);
	long SetSecondaryGHMITER(/*[in]*/ const char* FileName, /*[in]*/ long TableIndex, /*[in,out]*/ long *nSetSecondary, bool IsSingleton);
	long WriteGHMITERDataCell(/*[in]*/ const char* FileName, /*[in]*/ long TableIndex, bool IsSingleton);
	long WriteGHMITERSteuer(/*[in]*/ const char* FileName, /*[in]*/ const char* EndString1, /*[in]*/ const char* EndString2, /*[in]*/ long TableIndex);
	bool GetVarCodeProperties(/*[in]*/ long VarIndex, /*[in]*/ long CodeIndex, /*[in,out]*/  long * IsParent, /*[in,out]*/  long * IsActive, /*[in,out]*/ long * IsMissing, /*[in,out]*/ long * Level, /*[in,out]*/ long * nChildren, /*[in,out]*/ const char** Code);
        //bool UnsafeVariableCodes(/*[in]*/  long VarIndex, /*[in]*/ long CodeIndex, /*[in,out]*/ long * IsMissing, /*[in,out]*/ long *  Freq, /*[in,out]*/ const char** Code, /*[in,out]*/ long * Count, /*[in,out]*/ long * UCArray);
        bool UnsafeVariableCodes(/*[in]*/  long VarIndex, /*[in]*/ long CodeIndex, /*[in,out]*/ long * IsMissing, /*[in,out]*/ long *  Freq, /*[in,out]*/ std::string * Code, /*[in,out]*/ long * Count, /*[in,out]*/ long * UCArray);
	//bool GetVarCode(/*[in]*/ long VarIndex, /*[in]*/ long CodeIndex, /*[in,out]*/ long * CodeType, /*[in,out]*/ const char ** CodeString, /*[in,out]*/ long * IsMissing, /*[in,out]*/ long * Level);
        bool GetVarCode(/*[in]*/ long VarIndex, /*[in]*/ long CodeIndex, /*[in,out]*/ long * CodeType, /*[in,out]*/ std::string * CodeString, /*[in,out]*/ long * IsMissing, /*[in,out]*/ long * Level);
	long GetVarHierarchyDepth(/*[in]*/long VarIndex, /*[in]*/ bool Recoded);
        long SetHierarchicalCodelist(/*[in]*/  long VarIndex, /*[in]*/ const char* FileName, /*[in]*/ const char* LevelString);
	bool SetSecondaryHITAS(/*[in]*/ long TableIndex, /*[in,out]*/ long *nSetSecondary);
	bool PrepareHITAS(/*[in]*/ long TableIndex, /*[in]*/ const char* NameParameterFile, /*[in]*/ const char* NameFilesFile, /*[in]*/ const char* TauTemp);
	bool SetTableSafety(long Index, bool DominanceRule, long* DominanceNumber, long* DominancePerc, bool PQRule, long* PriorPosteriorP,
          		    long* PriorPosteriorQ, long* PriorPosteriorN, long* SafeMinRecAndHoldings, long* PeepPerc, long* PeepSafetyRange,
                            long* PeepMinFreqCellAndHolding, bool ApplyPeep, bool ApplyWeight, bool ApplyWeightOnSafetyRule, bool ApplyHolding,
                            bool ApplyZeroRule, bool EmptyCellAsNonStructural, long NSEmptySafetyRange,	double ZeroSafetyRange,	long ManualSafetyPerc,
                            long* CellAndHoldingFreqSafetyPerc);
	bool GetTableCellValue(/*[in]*/ long TableIndex, /*[in]*/  long CellIndex, /*[in,out]*/ double *CellResponse);
        bool GetTableCellProtectionLevels(/*[in]*/ long TableIndex,/*[in]*/ long CellIndex,/*[in,out]*/ double *lpl,/*[in,out]*/ double *upl);
        bool GetTableCellStatus(/*[in]*/ long TableIndex, /*[in]*/ long CellIndex, /*[in,out]*/ long *CellStatus);
	bool GetTableCell(/*[in]*/ long TableIndex, /*[in,out]*/ long* DimIndex, /*[in,out]*/ double *CellResponse, /*[in,out] */ double *CellRoundedResp,
                          /*[in,out] */double *CellCTAResp, /*[in,out] */ double *CellCKMResp, /*[in,out]*/ double *CellShadow, 
                          /*[in,out]*/ double *CellCost, /*[in,out]*/ double *CellKey, /*[in,out]*/ double *CellKeyNoZeros,
                          /*[in,out]*/ long *CellFreq, /*[in,out]*/ long *CellStatus, /*[in,out]*/  double *CellMaxScore, 
                          /*[in,out]*/ double *CellMaxScoreWeight, long *HoldingFreq, double *HoldingMaxScore, long *HoldingNrPerMaxScore,
                          double *PeepCell, double *PeepHolding, long *PeepSortCell, long *PeepSortHolding, double *Lower, double *Upper, 
                          double *RealizedLower, double *RealizedUpper);
	bool SetTable(/*[in]*/ long Index, /*[in]*/ long nDim,
		/*[in,out]*/ long * ExplanatoryVarList, bool IsFrequencyTable,
		/*[in]*/  long ResponseVar, /*[in]*/ long ShadowVar, /*[in]*/ long CostVar,
                /*[in]*/  long CellKeyVar,
                std::string CKMType, long CKMTopK,
		double Lambda, double MaxScaledCost,
		long PeepVarnr,bool SetMissingAsSafe);
	bool SetVariable(/*[in]*/ long VarIndex, /*[in]*/ long bPos, /*[in]*/ long nPos,
		/*[in]*/ long nDec, long nMissing,/*[in]*/ const char* Missing1, /*[in]*/ const char* Missing2,/*[in]*/ const char* TotalCode,bool IsPeeper, const char* PeeperCode1, const char* PeeperCode2, /*[in]*/ bool IsCategorical,
		/*[in]*/  bool IsNumeric, /*[in]*/ bool IsWeight, /*[in]*/ bool IsHierarchical,
		/*[in]*/ bool IsHolding, /*[in]*/ bool IsRecordKey);
	bool DoActiveRecode(/*[in]*/ long VarIndex);
	bool GetVarNumberOfCodes(/*[in]*/ long VarIndex, /*[in,out]*/ long *NumberOfCodes, /*[in,out]*/ long * NumberOfActiveCodes);
        bool GetTotalNumberOfCodes(/*[in]*/ long VarIndex, /*[in,out]*/ long *NumberOfCodes);
	bool SetVarCodeActive(/*[in]*/ long VarIndex, /*[in]*/ long CodeIndex, /*[in]*/ bool Active);
	bool GetStatusAndCostPerDim(/*[in]*/ long TableIndex, /*[in,out]*/ long *Status, /*[in,out]*/ double *Cost);
	bool SetTableCellStatus(/*[in]*/ long TableIndex, /*[in,out]*/ long * DimIndex, /*[in]*/ long CelStatus);
        bool SetTableCellProtectionLevels(/*[in]*/ long TableIndex,/*[in]*/ long CellIndex,/*[in]*/ double LPL,/*[in]*/ double UPL);
        bool SetTableCellStatus(/*[in]*/ long TableIndex, /*[in,out]*/ long CellIndex, /*[in]*/ long CellStatus);
	bool UndoRecode(/*[in]*/ long VarIndex);
	//long GetMaxnUc();
	bool ExploreFile(/*[in]*/ const char* FileName, /*[in,out]*/ long * ErrorCode,  /*[in,out]*/ long * LineNumber, /*[in,out]*/ long * ErrorVarIndex);
	bool UnsafeVariable( /*[in]*/ long VarIndex,/*[in,out]*/ long *Count, /*[in,out]*/ long * UCArray);
	//bool GetTableRow( /*[in]*/ long TableIndex, /*[in,out]*/ long *DimIndex, /*[in,out]*/ double * Cell, /*[in,out]*/ long *Status, /*[in]*/ long CountType);
	bool SetHierarchicalDigits( /*[in]*/ long VarIndex, /*[in]*/ long nDigitPairs, /*[in]*/ long *nDigits);
	void CleanAll();
	void ApplyRecode();
	bool DoRecode( /*[in]*/ long VarIndex, /*[in]*/ const char* RecodeString, long nMissing,/*[in]*/ const char* eMissing1, /*[in]*/ const char* eMissing2,  /*[in.out]*/ long *ErrorType, /*[in,out]*/ long * ErrorLine, /*[in,out]*/ long * ErrorPos, /*[in,out]*/ const char** WarningString);
	bool ComputeTables( /*[in,out]*/ long * ErrorCode, /*[in,out]*/ long * TableIndex);
	bool SetNumberTab( /*[in]*/ long nTab);
	bool SetNumberVar(/*[in]*/ long nVar);
	double GetMinimumCellValue(/*[in]*/ long TableIndex, /*[in,out]*/ double *Maximum);
	bool SetProtectionLevelsForResponseTable(/*[in]*/ long TableIndex,/*[in,out]*/ long * DimIndex,/*[in]*/ double LowerBound, /*[in]*/ double UpperBound);
	std::string GetErrorString(/*[in]*/ long ErrorNumber);
        std::string GetVersion();
        int SetCellKeyValuesFreq(/*[in]*/ long TabNo, /*[in]*/ std::string PTableFile, /*[out]*/ int *MinDiff, /*[out]*/ int *MaxDiff);
        int SetCellKeyValuesCont(/*[in]*/ long TabNo, /*[in]*/ std::string PTableFileCont, /*[in]*/ std::string PTableFileSep, 
                                 /*[in]*/ std::string CKMType, /*[in]*/ int topK, /*[in]*/ bool IncludeZeros, /*[in]*/ bool Parity, 
                                 /*[in]*/ bool Separation, /*[in]*/ double m1sqr, /*[in]*/ std::string Scaling, 
                                 /*[in]*/ double s0, /*[in]*/ double s1, /*[in]*/ double z_f, /*[in]*/ double q, /*[in]*/ double* epsilon,
                                 /*[in]*/ double muC);

private:
        double ShiftFirstDigit(double key, int nDec);
        double flexfunction(double z, double z_s, double s0, double s1, double z_f, double q, std::string Scaling);
        double LookUpVinptable(std::map<int,PTableDRow> ptable, double z, double RKey);
        double GetXj(std::string CKMType, int j, CDataCell &dc, bool WeightApplied);
};

#endif // TauArgus_h
