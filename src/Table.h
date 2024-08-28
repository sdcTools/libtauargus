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

#if !defined Table_h
#define Table_h

#include <vector>

#include "defines.h"
#include "DataCell.h"
#include "Variable.h"

class CTable
{
// Construction
public:
	CTable();

// Attributes
public:
	bool Prepared;          // whether table is already prepared

        // About the Table
	bool IsFrequencyTable;
	bool ApplyHolding;
	bool ApplyWeight;
	bool ApplyWeightOnSafetyRule;
	bool ApplyPeeper;
	bool ApplyZeroRule;
	bool EmptyCellsAsNSEmpty;
	int HasRecode;
	bool SetMissingAsSafe;

	int ExplVarnr[MAXDIM];  // index of each explanatory variable
	int ResponseVarnr;      // index of respons variable
	int ShadowVarnr;        // index of shadow variable
	int CostVarnr;          // index of cost variable; or CVT_FREQ etc, these values are always < 0
        int CellKeyVarnr;       // index of variable to compute cellkey
	int PeepVarnr;

	long nDim;              // number of variables (= dimensions) in table
        int  SizeDim[MAXDIM];   // = nCode of corresponding variable

//	CDataCell *Cell;          // counting space
	std::vector<CDataCell*> CellPtr;
	long nCell;               // number of cells
	//int nMaxCellValues;
        
	long NumberofMaxScoreCell; //Saves the number of MaxScore to be stored per cell--Cell Level
	long NumberofMaxScoreHolding; //Saves the number of MaxScore to be stored per cell--Holding Level
        
        long CKMTopK;           // defines number of largest values percell needed for CKMType = T 
        std::string CKMType;    // defines multiplier to be used in CKM for magnitude tables
        bool KeepMinScore;      // if true, save smallest value per cell
        
	// Transformation the Cost function.
	double Lambda;
	double MaxScaledCost;
	double MinLPL;          // Minimum protection level avoiding problems when writing intermediate files  

	// Add
//	int SafetyRule;         // DOMINANCE or PQRULE or None

//	bool TwoRulesForCell;
//	bool TwoRulesForHolding;
	// for dominance safety method
	bool DominanceRule;
	int DominanceNumberCell_1;
	int DominancePercCell_1;
	int DominanceNumberCell_2;
	int DominancePercCell_2;

	int DominanceNumberHolding_1;
	int DominancePercHolding_1;
	int DominanceNumberHolding_2;
	int DominancePercHolding_2;
	// for PriorPosterial safety method or PQ-Rule
	bool PQRule;
	int PQ_PCell_1;
	int PQ_QCell_1;
	int PQ_NCell_1;
	int PQ_PCell_2;
	int PQ_QCell_2;
	int PQ_NCell_2;

	int PQ_PHolding_1;
	int PQ_QHolding_1;
	int PQ_NHolding_1;
	int PQ_PHolding_2;
	int PQ_QHolding_2;
	int PQ_NHolding_2;

	// for Peep regel
	long PeepPercCell_1;
	long PeepPercCell_2;

	long PeepPercHolding_1;
	long PeepPercHolding_2;

	long PeepSafetyRangePercCell;
	long PeepSafetyRangePercHolding;

	//
	long PeepMinFreqCell;
	long PeepMinFreqHold;


	int SafeMinRec; // Minimum number of contributors
	int SafeMinHoldings; // Minimum number of holdings
/// Safety Margins

	long CellFreqSafetyPerc;
	long HoldingFreqSafetyPerc;
	long ManualSafetyPerc;
	long NSEmptySafetyRange;
	long SingletonSafetyRangePerc;
	double ZeroSafetyRange;
        
	// Operations

	// have to make the operator =
	// since it does not work with CPtrArray
	void operator = (CTable &table2);

public:

// Implementation
public:
	void InitializeHoldingNrs();
	long GetCellNrFromIndices(long *Indices);
	bool GetCellDistance(long CellNr, long *Dist);
	bool IsCellSafe(long c);
	bool IsCellUnsafe(long c);
	int * CellDistance;
	bool PrepareComputeDistance();
	int  GetCellDistance(long *dim1, long *dim2);
	void GetStatusStatistics(long *Freq,long *CellFreq, long *HoldingFreq, double *CellResp, double *CellCost);
	void SetCellSecondaryUnsafe(long *dims);
	void SetSecondary(CVariable *var, long *dims, long *CellDims, int niv, long *nSetSecondary);
	bool SetSecondaryHITAS(FILE *fd, CVariable *var, long *nSetSecondary);
	CDataCell * GetCell(long *VarValueList);
	CDataCell * GetCell(long CellNr);
	void GetStatusAndCostPerDim(long *Status, double *Cost);
	void UndoSecondarySuppress(long SortSuppress);
	bool GetIndicesFromCellNr(long CellNr, long* Indices);
	bool CleanUp();
	int  ComputeCellSafeCode(CDataCell &dc);
	bool PrepareTable();
	bool UnsafeCellDominance(CDataCell &dc);
	bool UnsafeCellPQRule(CDataCell &dc);
	bool UnsafeCellMinRec(CDataCell& dc);
	bool UnsafeCellPeep(CDataCell &dc);
	void CountUnSafe(long *DimList, int FixedDimNr, long *Unsafe, int niv);

	bool GetUnsafeCells(int VarIndex, int VarCodeIndex, long *nUnsafe);
	bool GetUnsafeCells(int VarIndex, long *nUnsafe);
	//bool GetCell(long *VarValueList, CDataCell& datacell);
	//bool GetCell(long CellNr, CDataCell& datacell);
	long GetMemSizeTable();
	long GetSizeTable();

	bool SetCell(long *VarValueList, CDataCell& datacell);
	bool SetCell(long CellNr, CDataCell& datacell);
	bool SetDimSize(int dim, int value);
//  bool SetDominance(long DominanceNumber, long DominancePerc);

	bool SetDominance(long *DominanceNumber, long *DominancePerc);
	//bool SetPQRule(long PriorPosteriorP, long PriorPosteriorQ, long PriorPosteriorN);
	bool SetPQRule(long *PriorPosteriorP,long *PriorPosteriorQ, long *PriorPosteriorN);
	bool SetSafeMinRecAndHold(long SafeMinRec, long SafeMinholdings);
	bool SetVariables(int inDim, long* ExplVar, long RespVar, long ShadVar, long CostVar, long CellKeyVar, long PeepVar);
	bool SetExplVariables(int nDim, long* ExplVar);
	bool SetProtectionLevelCell(CDataCell &datacell);
	bool SetProtectionLevelCellFrequency(CDataCell &datacell, long Base, long K);
        //bool ComputeCellKeyCell(CDataCell &datacell);
	virtual ~CTable();
};

#endif // Table_h
