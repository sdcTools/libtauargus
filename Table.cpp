// Table.cpp : implementation file
//

#include "stdafx.h"
#include "Table.h"
//////
//#include <stdio.h>
//#include <stdlib.h>

#include  <stdio.h>
#include  <math.h>
#include  <stdlib.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTable

// This is an operator comparison. This is used in recoded tables.
// The recoded table is the same as the original table. Only the cell ptr is empty.

void CTable:: operator = (CTable & table2)
{
	long  i;

	Prepared = table2.Prepared;
	nDim  = table2.nDim;
	for (i=0; i<MAXDIM; i++) {
		SizeDim[i] = table2.SizeDim[i];
	}

	for (i=0; i<MAXDIM; i++) {
		ExplVarnr[i] = table2.ExplVarnr[i];
	}

	CellFreqSafetyPerc = table2.CellFreqSafetyPerc;
	HoldingFreqSafetyPerc = table2.HoldingFreqSafetyPerc;
	ManualSafetyPerc = table2.ManualSafetyPerc;
	ApplyHolding	= table2.ApplyHolding;
	ApplyWeight	= table2.ApplyWeight;
	ApplyWeightOnSafetyRule = table2.ApplyWeightOnSafetyRule;
	ResponseVarnr = table2.ResponseVarnr;
	ShadowVarnr = table2.ShadowVarnr ;
	CostVarnr = table2.CostVarnr ;
	//SafetyRule = table2.SafetyRule ;
/*
	TwoRulesForCell = table2.TwoRulesForCell;
	TwoRulesForHolding = table2.TwoRulesForHolding;
*/
	DominanceRule = table2.DominanceRule;
	DominanceNumberCell_1 = table2.DominanceNumberCell_1 ;
	DominancePercCell_1 = table2.DominancePercCell_1 ;
	DominanceNumberCell_2 = table2.DominanceNumberCell_2 ;
	DominancePercCell_2 = table2.DominancePercCell_2;

	DominanceNumberHolding_1 = table2.DominanceNumberHolding_1 ;
	DominancePercHolding_1 = table2.DominancePercHolding_1 ;
	DominanceNumberHolding_2 = table2.DominanceNumberHolding_2 ;
	DominancePercHolding_2 = table2.DominancePercHolding_2;

	PQRule = table2.PQRule;
	PQ_PCell_1 = table2.PQ_PCell_1 ;
	PQ_QCell_1 = table2.PQ_QCell_1 ;
	PQ_NCell_1 = table2.PQ_NCell_1 ;
	PQ_PCell_2 = table2.PQ_PCell_2 ;
	PQ_QCell_2 = table2.PQ_QCell_2 ;
	PQ_NCell_2 = table2.PQ_NCell_2 ;


	PQ_PHolding_1 = table2.PQ_PHolding_1 ;
	PQ_QHolding_1 = table2.PQ_QHolding_1 ;
	PQ_NHolding_1 = table2.PQ_NHolding_1 ;
	PQ_PHolding_2 = table2.PQ_PHolding_2 ;
	PQ_QHolding_2 = table2.PQ_QHolding_2 ;
	PQ_NHolding_2 = table2.PQ_NHolding_2 ;

	PeepPercCell_1 = table2.PeepPercCell_1;
	PeepPercCell_2 = table2.PeepPercCell_2;
	PeepPercHolding_1 = table2.PeepPercHolding_1;
	PeepPercHolding_2 = table2.PeepPercHolding_2;

	PeepSafetyRangePercCell = table2.PeepSafetyRangePercCell;
	PeepSafetyRangePercHolding = table2.PeepSafetyRangePercHolding;

	PeepMinFreqCell =table2.PeepMinFreqCell;
	PeepMinFreqHold =table2.PeepMinFreqHold;

	SafeMinRec = table2.SafeMinRec ;
	SafeMinHoldings = table2.SafeMinHoldings;
	HasRecode = table2.HasRecode ;
	SetMissingAsSafe = table2.SetMissingAsSafe;
	nCell = 0 ;
	//nMaxCellValues = table2.nMaxCellValues;
	NumberofMaxScoreCell = table2.NumberofMaxScoreCell;
	IsFrequencyTable = table2.IsFrequencyTable;
	NumberofMaxScoreHolding = table2.NumberofMaxScoreHolding;
	ApplyPeeper=table2.ApplyPeeper;
	SingletonSafetyRangePerc = table2.SingletonSafetyRangePerc;
	Lambda = table2.Lambda;
	MaxScaledCost = table2.MaxScaledCost;
	EmptyCellsAsNSEmpty = table2.EmptyCellsAsNSEmpty;
	NSEmptySafetyRange = table2.NSEmptySafetyRange;
	CellPtr.SetSize(1);


}

// Table constructor. All values are initialized as zeros.
CTable::CTable()
{

   nDim = 0;
  // Cell = 0;
   nCell = 0;
	HasRecode = 0;
	SetMissingAsSafe = false;

	//SafetyRule = 0;
	DominanceRule = false;
	PQRule = false;

	NumberofMaxScoreCell = 0;
	NumberofMaxScoreHolding = 0;
	DominanceNumberCell_1 = 0;
	DominancePercCell_1 = 0;
	DominanceNumberCell_2 = 0;
	DominancePercCell_2 = 0;
	DominanceNumberHolding_1 = 0;
	DominancePercHolding_1 = 0;
	DominanceNumberHolding_2 = 0;
	DominancePercHolding_2 = 0;
//	TwoRulesForCell = false;
//	TwoRulesForHolding = false;
	PQ_PCell_1 =0; PQ_QCell_1 = 100;PQ_NCell_1 = 0; //AHNL initialiseer PQ_Q = 100 ipv 0
	PQ_PCell_2 =0; PQ_QCell_2 = 100;PQ_NCell_2 = 0; //AHNL initialiseer PQ_Q = 100 ipv 0
	PQ_PHolding_1 =0; PQ_QHolding_1 = 100;PQ_NHolding_1 = 0; //AHNL initialiseer PQ_Q = 100 ipv 0
	PQ_PHolding_2 =0; PQ_QHolding_2 = 100;PQ_NHolding_2 = 0; //AHNL initialiseer PQ_Q = 100 ipv 0
	PeepPercCell_1 = 0;PeepPercCell_2 = 0;
	PeepPercHolding_1 = 0;PeepPercHolding_2 = 0;
	PeepSafetyRangePercCell = 0;
	PeepSafetyRangePercHolding = 0;

	PeepMinFreqCell =0;
	PeepMinFreqHold =0;

	CellDistance = 0;

	Prepared = false;

	Lambda = 1;
	MaxScaledCost = 20000;
	MinLPL = 1;

	PeepVarnr = -1;
	SingletonSafetyRangePerc = 0;
	NSEmptySafetyRange= 0;
	SafeMinRec = 0;
	SafeMinHoldings = 0;
//	nMaxCellValues = 0;
	CellFreqSafetyPerc = 0;
	HoldingFreqSafetyPerc = 0;
	ManualSafetyPerc = 0;
	IsFrequencyTable = false;
	ApplyHolding = false;
	ApplyWeight = false;
	ApplyWeightOnSafetyRule = false;
	EmptyCellsAsNSEmpty = false;
	ApplyPeeper = false;
	ApplyZeroRule = false;

	//Not too sure what the consequences are if I add This
	//CellPtr.RemoveAll();
}

CTable::~CTable()
{

	CleanUp();
}

// explanatory variables are set
bool CTable::SetExplVariables(int inDim, long* ExplVar)
{
	nDim = inDim;

	for (int i = 0; i < nDim; i++) {
		ExplVarnr[i] = ExplVar[i] - 1;
	}

	ShadowVarnr = -1;
	CostVarnr = -1;
	ResponseVarnr = -1;
	return true;

}


// other variables are set. In case only frequency tables are created, this has to be changed
bool CTable::SetVariables(int inDim, long *ExplVar, long RespVar, long ShadVar, long CostVar, long PeepVar)
{
	// Add HoldingVarnr to this shit

	nDim = inDim;

	for (int i = 0; i < nDim; i++) {
		ExplVarnr[i] = ExplVar[i];
	}

	if (RespVar >= 0)	{
		ResponseVarnr = RespVar;
	}
	/*else {
		ResponseVarnr = -1;
	}*/

	if (ShadVar >= 0)	{
		ShadowVarnr = ShadVar;
	}
	/*else	{
		ShadowVarnr = -1;
	}*/

	CostVarnr = CostVar;
	PeepVarnr = PeepVar;

	return true;
}
// cell_1 = LongArray[0], cell_2 = LongArray[1],
//Holding_1 = LongArray[2],Holding_2,= LongArray[3];
bool CTable::SetDominance(long *DominanceNumber, long *DominancePerc)
{
	if (!ApplyHolding)	{
		if (DominanceNumber[0] < 1) return false;
		if (DominancePerc[0] < 0 || DominancePerc[0] > 100) return false;

		this->DominanceNumberCell_1 = DominanceNumber[0];
		this->DominancePercCell_1 = DominancePerc[0];
		this->PQ_QCell_1 = 100; //ahnl 30 jan 2002

		//if DominanceNumberCell = 0; don't do anything
		//other wiseset
		if (DominanceNumber[1] > 0)	{
			if (DominancePerc[1] < 0 || DominancePerc[1] > 100) return false;

			this->DominanceNumberCell_2 = DominanceNumber[1];
			this->DominancePercCell_2 = DominancePerc[1];
			this->PQ_QCell_2 = 100; //ahnl 30 jan 2000
		}
	}
	else	{
		if ((DominanceNumber[0] <1) && (DominanceNumber[2] <1)) return false;
			if (DominanceNumber[0] > 0)	{
				if (DominancePerc[0] < 0 || DominancePerc[0] > 100) return false;

					this->DominanceNumberCell_1 = DominanceNumber[0];
					this->DominancePercCell_1 = DominancePerc[0];
					this->PQ_QCell_1 = 100; //ahnl 30 jan 2000
				}
				if (DominanceNumber[1] > 0)	{
					if (DominancePerc[1] < 0 || DominancePerc[1] > 100) return false;

					this->DominanceNumberCell_2 = DominanceNumber[1];
					this->DominancePercCell_2 = DominancePerc[1];
					this->PQ_QCell_2 = 100; //ahnl 30 jan 2000
				}
				if (DominanceNumber[2] > 0)	{
					if (DominancePerc[2] < 0 || DominancePerc[2] > 100) return false;

					this->DominanceNumberHolding_1 = DominanceNumber[2];
					this->DominancePercHolding_1 = DominancePerc[2];
					this->PQ_QHolding_1 = 100; //ahnl 30 jan 2000
				}
				if (DominanceNumber[3] > 0)	{
					if (DominancePerc[3] < 0 || DominancePerc[3] > 100) return false;

					this->DominanceNumberHolding_2 = DominanceNumber[3];
					this->DominancePercHolding_2 = DominancePerc[3];
					this->PQ_QHolding_2 = 100; //ahnl 30 jan 2000
				}
		}


	// Holding
	DominanceRule = true;
	return true;


}

// Set PQ rule
bool CTable::SetPQRule(long *PriorPosteriorP,long *PriorPosteriorQ, long *PriorPosteriorN )

{
	if (!ApplyHolding)	{
		if (PriorPosteriorN[0] <1) return false;
		if (PriorPosteriorP[0] > PriorPosteriorQ[0]) return false;
		PQ_PCell_1 = PriorPosteriorP[0];
		PQ_QCell_1 = PriorPosteriorQ[0];
		PQ_NCell_1 = PriorPosteriorN[0];
		if (PriorPosteriorN[1] >0)	{
			if (PriorPosteriorP[1] > PriorPosteriorQ[1]) return false;
			PQ_PCell_2 = PriorPosteriorP[1];
			PQ_QCell_2 = PriorPosteriorQ[1];
			PQ_NCell_2 = PriorPosteriorN[1];
		}

	}
	else
	{
		if ((PriorPosteriorN[0] <1) && (PriorPosteriorN[2] <1)) return false;
		if (PriorPosteriorN[0] >0)	{
			if (PriorPosteriorP[0] > PriorPosteriorQ[0]) return false;
			PQ_PCell_1 = PriorPosteriorP[0];
			PQ_QCell_1 = PriorPosteriorQ[0];
			PQ_NCell_1 = PriorPosteriorN[0];
		}
		if (PriorPosteriorN[1] >0)	{
			if (PriorPosteriorP[1] > PriorPosteriorQ[1]) return false;
			PQ_PCell_2 = PriorPosteriorP[1];
			PQ_QCell_2 = PriorPosteriorQ[1];
			PQ_NCell_2 = PriorPosteriorN[1];
		}
		if (PriorPosteriorN[2] >0)	{
			if (PriorPosteriorP[2] > PriorPosteriorQ[2]) return false;
			PQ_PHolding_1 = PriorPosteriorP[2];
			PQ_QHolding_1 = PriorPosteriorQ[2];
			PQ_NHolding_1 = PriorPosteriorN[2];
		}
		if (PriorPosteriorN[3] >0)	{
			if (PriorPosteriorP[3] > PriorPosteriorQ[3]) return false;
			PQ_PHolding_2 = PriorPosteriorP[3];
			PQ_QHolding_2 = PriorPosteriorQ[3];
			PQ_NHolding_2 = PriorPosteriorN[3];
		}

	}
	PQRule = true;
	return true;
}

// safe minimum records and safe minimum holdings
bool CTable::SetSafeMinRecAndHold(long SafeMinRec, long SafeMinHoldings)
{


	if (ApplyHolding)	{
		if ((SafeMinRec < 1)&&(SafeMinHoldings < 1)) {
			return false;
		}
		this->SafeMinHoldings = SafeMinHoldings;
		this->SafeMinRec = SafeMinRec;
	}
	else	{
		if (SafeMinRec < 1) {
			return false;
		}
		this->SafeMinRec = SafeMinRec;
	}
	return true;
}

// memory for table is created. In this method the length of top n is calculated.
// The top n is used by datacell to reserve memory for top n inputs for the table.
// Since there could be two PQ and two Dominance rules. The top n is the maximum n.
// One cell is also created as an empty cell. This is the (Size of table + 1)th cell. This is used
// whenever information for a cell is asked that is not created.
bool CTable::PrepareTable()
{
	int nScoreCellDom, nScoreHoldingDom, i;
	int nScoreCellPQ, nScoreHoldingPQ;
	nCell = GetSizeTable();
	// The rest are null pointers.
	CellPtr.SetSize(nCell+1); // last pointer to emptycell;
	if (CellPtr.GetSize() == 0) {
		return false;
	}
	// one empty cell with last Cellptr pointing to.
	CDataCell *dcempty = new CDataCell(NumberofMaxScoreCell, NumberofMaxScoreHolding,ApplyHolding,ApplyWeight);
	//hier ook een nus struct empty maken; Size eentje hoger; Iedereen verwijst naaar de empty cell ipv NULL
	CellPtr.SetAt(nCell,dcempty);
	for (i=0; i<nCell; i++)	{
		CellPtr.SetAt(i,NULL);
	}

	nScoreCellDom = 0;
	nScoreHoldingDom = 0;
	nScoreCellPQ = 0;
	nScoreHoldingPQ = 0;

	// each cell has to maintain max score . it has to be the maximum
	// of the dom numbers and PQN +1
	if (DominanceRule)	{

		nScoreCellDom = __max(DominanceNumberCell_1, DominanceNumberCell_2);
		if (ApplyHolding)	{
			nScoreHoldingDom = __max(DominanceNumberHolding_1, DominanceNumberHolding_2);
		}
	}
	if (PQRule)	{
		nScoreCellPQ = __max(PQ_NCell_1,PQ_NCell_2) + 1;
		if ((ApplyHolding) && __max(PQ_NHolding_1,PQ_NHolding_2) > 0)	{
			nScoreHoldingPQ = __max(PQ_NHolding_1,PQ_NHolding_2) +1;
		}
	}

	NumberofMaxScoreCell= __max(nScoreCellDom, nScoreCellPQ);
	NumberofMaxScoreHolding = __max(nScoreHoldingDom,nScoreHoldingPQ);
	Prepared = true;
	return true;

}

// cleans up all allocated memory.
bool CTable::CleanUp()
{
	int i;
	if (nCell == 0) {
		return true;  // nothing to do
	}

	if (CellDistance != 0) {
		delete [] CellDistance;
	}

	for (i=0; i<nCell+1; i++) {
		if (CellPtr.GetAt(i) != NULL) {
			CDataCell *dc;
			dc = (CDataCell *)CellPtr.GetAt(i);
			delete[] dc;
			//delete[] CellPtr.GetAt(i);
			CellPtr.SetAt(i,NULL);
		}
	}
	CellPtr.RemoveAll();

	return true;
}



// got to do this

long CTable::GetMemSizeTable()
{ int nScoreCellDom = 1;
  int nScoreHoldingDom = 0;
  long MemUsage;

	MemUsage = GetSizeTable() * sizeof(CDataCell);

/*
	case DOMINANCE:
		nScoreCell = __max(DominanceNumberCell_1, DominanceNumberCell_2);
		if (ApplyHolding)	{
			nScoreHolding = __max(DominanceNumberHolding_1, DominanceNumberHolding_2);
		}
		break;
	case PQRULE:
		nScoreCell = __max(PQ_NCell_1,PQ_NCell_2) + 1;
		if ((ApplyHolding) && __max(PQ_NHolding_1,PQ_NHolding_2) > 0)	{
			nScoreHolding = __max(PQ_NHolding_1,PQ_NHolding_2) +1;
		}
		break;

	// Add memory for MaxScore and MaxScoreWeight
  MemUsage += nScoreCell * 2 * sizeof(double) * GetSizeTable();
*/
	return MemUsage;

}

// returns how large the table is.
// this function now does not make sense because not all cells are filled.
long CTable::GetSizeTable()
{
	int i;
	long SizeTable = 1;

	ASSERT(nDim > 0 && nDim <= MAXDIM);

	for (i = 0; i < nDim; i++) {
		ASSERT(SizeDim[i] > 0);
		SizeTable *= SizeDim[i]; // number of codes of variable
	}
	return SizeTable;
}

// sets per dimension the size of this dimension.
bool CTable::SetDimSize(int dim, int value)
{
	ASSERT(nDim != 0 && dim >= 0 && dim < nDim && value > 0);
	if (nDim == 0 || dim < 0 || dim >= nDim || value < 1) {
		return false;
	}
	SizeDim[dim] = value;
	return true;
}

// returns cell if an array of dimension positions are given.
CDataCell* CTable::GetCell(long *VarValueList)
{
  return GetCell(GetCellNrFromIndices(VarValueList));
}

// returns a datacell. if the datacell does not exist the empty cell is returned.
// The empty cell is described in Prepare table.
CDataCell* CTable::GetCell(long CellNr)
{
	CDataCell *dc;

	if (CellPtr.GetAt(CellNr) != NULL) {

		dc = (CDataCell *) CellPtr.GetAt(CellNr);
		switch (CostVarnr) {
		case CVT_DISTANCE:
			dc->SetCost(1); // provisional solution, Anco
			break;
		case CVT_UNITY:  // always one
			dc->SetCost(1);
		  break;
		case CVT_FREQ:  // frequency
			dc->SetCost(dc->GetFreq());
		  break;
	  }

		return dc;
	}

	else {
		dc = (CDataCell *) CellPtr.GetAt(nCell); // return last empty cell;
		return dc;
	}
}

// given a cell and an array of dimension positions.
//The cell can be placed on position in the CellPtr array.

bool CTable::SetCell(long *VarValueList, CDataCell &datacell)
{
	SetCell(GetCellNrFromIndices(VarValueList), datacell);
	return true;
}

// Given a cell an a position. It sets the cell.
bool CTable::SetCell(long CellNr, CDataCell &datacell)
{
	ASSERT(CellNr >= 0 && CellNr < nCell);


	CellPtr.SetAt(CellNr,& datacell);
	return true;
}
// Unsafe cell through Dominance rule. This is used to determine that the cell is
// primary unsafe.
bool CTable::UnsafeCellDominance(CDataCell &dc)
{
	if (DominanceNumberCell_1 > 0)	{
		if (dc.GetDominancePercCell(ApplyWeight, ApplyWeightOnSafetyRule,
			DominanceNumberCell_1) > DominancePercCell_1)	{
			return true;
		}
	}
	if (DominanceNumberCell_2 > 0)	{
		if (dc.GetDominancePercCell(ApplyWeight, ApplyWeightOnSafetyRule,
			DominanceNumberCell_2)  > DominancePercCell_2)	{
			return true;
		}
	}
	if (ApplyHolding)	{
		if (DominanceNumberHolding_1 > 0)	{
			if (dc.GetDominancePercHolding(ApplyWeight, ApplyWeightOnSafetyRule,
				DominanceNumberHolding_1) > DominancePercHolding_1)	{
				return true;
			}
		}
		if (DominanceNumberHolding_2 > 0)	{
			if (dc.GetDominancePercHolding(ApplyWeight, ApplyWeightOnSafetyRule,
				 DominanceNumberHolding_2) > DominancePercHolding_2)	{
				return true;
			}
		}
	}
	return false;
}
// Unsafe cell through PQ rule. This is used to determine that a cell is
// primary unsafe.
bool CTable::UnsafeCellPQRule(CDataCell &dc)
{
	if (PQ_PCell_1 > 0)	{
		if (dc.GetPQCell(PQ_PCell_1, PQ_QCell_1,
			PQ_NCell_1, ApplyWeight, ApplyWeightOnSafetyRule) > 0) {
  			return true;
		}
	}
	if (PQ_PCell_2 > 0)	{
		if (dc.GetPQCell(PQ_PCell_2, PQ_QCell_2,
			PQ_NCell_2, ApplyWeight, ApplyWeightOnSafetyRule) > 0) {
  			return true;
		}
	}
	if (ApplyHolding)	{
		if (PQ_PHolding_1 > 0)	{
			if (dc.GetPQHolding(PQ_PHolding_1, PQ_QHolding_1,
				PQ_NHolding_1, ApplyWeight, ApplyWeightOnSafetyRule) > 0) {
  				return true;
			}
		}
		if (PQ_PHolding_2 > 0)	{
			if (dc.GetPQHolding(PQ_PHolding_2, PQ_QHolding_2,
				PQ_NHolding_2, ApplyWeight, ApplyWeightOnSafetyRule) > 0) {
  				return true;
			}
		}
	}
	return false;
}

// Unsafe cell through peep rule. This is used to determine if the cell is unsafe
// because it fails in the peep rule.
bool CTable::UnsafeCellPeep(CDataCell &dc)
{

	if(dc.GetPeepSortCell() == PEEP1)	{
		if (PeepPercCell_1 >0)	{
			if (dc.GetPeepCell() > (PeepPercCell_1/100.0)*dc.GetShadow()) {
				return true;
			}
		}
	}
	if(dc.GetPeepSortCell() == PEEP2)	{
		if (PeepPercCell_2 >0)	{
			if (dc.GetPeepCell() > (PeepPercCell_2/100.0)*dc.GetShadow()) {
				return true;
			}
		}
	}
	if (ApplyHolding)	{
		if (dc.GetPeepSortHolding() == PEEP1)	{
			if (PeepPercHolding_1 >0)	{
				if (dc.GetPeepHolding() > (PeepPercHolding_1/100.0)*dc.GetShadow()) {
					return true;
				}
			}
		}
		if (dc.GetPeepSortHolding() == PEEP2)	{
			if (PeepPercHolding_2 >0)	{
				if (dc.GetPeepHolding() > (PeepPercHolding_2/100.0)*dc.GetShadow()) {
					return true;
				}
			}
		}
	}
// AHNL 6.1.2003: testen op PEEP1 en PEEP2  en niet op <> NOPEEP want soms staat de
//                statsu nog op EMPTY!!!
	if (dc.GetPeepSortCell() == PEEP1 || dc.GetPeepSortCell() == PEEP2)	{
		// freq
		if (dc.GetFreq() < PeepMinFreqCell)	{
			return true;
		}
	}
	if (ApplyHolding)	{
		if (dc.GetPeepSortHolding() == PEEP1 || dc.GetPeepSortHolding() == PEEP2)	{
		// freq
			if (dc.GetFreqHolding() < PeepMinFreqHold)	{
				return true;
			}
		}
	}



	return false;

}


// Unsafe cells through minimum records or minimum holdings.
bool CTable::UnsafeCellMinRec(CDataCell &dc)
{
	if (!ApplyWeight)	{
		if ((dc.GetFreq() > 0) && (dc.GetFreq() < SafeMinRec))	{
			return true;
		}
		if (ApplyHolding)	{
			if ((dc.GetFreqHolding() > 0) && (dc.GetFreqHolding() < SafeMinHoldings))	{
				return true;
			}
		}
	}
	// Now Holding and weight cannot happen at the same time. (atleast
	// not yet)
	else	{
		if ((dc.GetWeight() < SafeMinRec))	{
			return true;
		}
	}
	return false;
}

// This is used to determine, if a cell is safe or unsafe.
// 1) check if the cell is empty.
// 2) Check if the cell fails Dominance or PQ rule
// 3) Check if the Frequency rule fails.
//Know that apply holding and weight cannot occur at the same time.
// 4) Check if Zero rule fails.
// If the cell passes all checks then the cell is safe.
int CTable::ComputeCellSafeCode(CDataCell &dc)
{

	if (dc.GetFreq() == 0) { // empty cell
		if (EmptyCellsAsNSEmpty)	{
			return CS_EMPTY_NONSTRUCTURAL;
		}
		else	{
			return CS_EMPTY;
		}
		// NIET WAAR als de status gegeven is en de freq niet.
	}

	if (DominanceRule)	{
		if (UnsafeCellDominance(dc) ) { // dominance too big
    		return CS_UNSAFE_RULE;
		}
	}
	if (PQRule)	{
		if (UnsafeCellPQRule(dc) ) { // Prior Posteriority incorrect
   	  return CS_UNSAFE_RULE;
		}
	}
	if (ApplyPeeper) {
		if (UnsafeCellPeep(dc) ) {
			return CS_UNSAFE_PEEP;
		}
	}
	if (UnsafeCellMinRec(dc) ) { // safe minimal?
		return CS_UNSAFE_FREQ;
	}

	if (ApplyZeroRule)	{
		if ((dc.GetResp() == 0) && (dc.GetFreq() > 0))	{
			return CS_UNSAFE_ZERO;
		}
		else	{
			return CS_SAFE;
		}
	}
	else
	{
		return CS_SAFE;
	}

}

// nUnsafe: array with max MAXDIM longs.
// For a given explanatory variable this method returns the
//number of unsafe cells per code (index)
bool CTable::GetUnsafeCells(int VarIndex, long *nUnsafe)
{
	long DimList[MAXDIM], c;
	int d, i, n;

	memset(nUnsafe, 0, sizeof(long) * nDim);

	// find index variable
	for (i = 0; i < nDim; i++) {
		if (ExplVarnr[i] == VarIndex) {
			break;
		}
	}

	if (i == nDim) {
		return false;  // Variable not in table, stupid boy
	}
	long tempstatus;
	for (c = 0; c < nCell; c++) {
		tempstatus = GetCell(c)->GetStatus();
		if ((tempstatus == CS_UNSAFE_FREQ) || (tempstatus == CS_UNSAFE_RULE) ||
			(tempstatus == CS_UNSAFE_PEEP) || (tempstatus == CS_UNSAFE_SINGLETON)||
			(tempstatus == CS_UNSAFE_ZERO) ){
			GetIndicesFromCellNr(c, DimList);
			// count number of dimensions of this cell
			for (d = n = 0; d < nDim; d++) {
				if (DimList[d] != 0) n++;
			}
			if (DimList[i] != 0) {  // Variable VarIndex != total
				nUnsafe[n - 1]++;
			}
		}
	}

	return true;
}

// Given a variable and code. This method returns the number of cells
bool CTable::GetUnsafeCells(int VarIndex, int VarCodeIndex, long *nUnsafe)
{
	long DimList[MAXDIM];
	int i;
	CDataCell dc;

	memset(nUnsafe, 0, sizeof(long) * nDim);

	// find index variable
	for (i = 0; i < nDim; i++) {
		if (ExplVarnr[i] == VarIndex) {
			break;
		}
	}

	if (i == nDim) {
		return false;  // Variable not in table, stupid boy
	}

	if (VarCodeIndex < 0 || VarCodeIndex >= SizeDim[i]) {
		return false;
	}

	DimList[i] = VarCodeIndex;

	CountUnSafe(DimList, i, nUnsafe, 0);


	return true;

}

// Counts Unsafe
void CTable::CountUnSafe(long *DimList, int FixedDimNr, long *Unsafe, int niv)
{
	int i;

	if (niv == FixedDimNr) {
		CountUnSafe(DimList, FixedDimNr, Unsafe, niv + 1);
		return;
	}

	if (niv == nDim) {
		if ((GetCell(DimList)->GetStatus() == CS_UNSAFE_FREQ) ||
		(GetCell(DimList)->GetStatus() == CS_UNSAFE_PEEP) ||
		(GetCell(DimList)->GetStatus() == CS_UNSAFE_RULE) ||
		(GetCell(DimList)->GetStatus() == CS_UNSAFE_ZERO) ||
		(GetCell(DimList)->GetStatus() == CS_UNSAFE_FREQ)){
			int d, n;
			for (d = n = 0; d < nDim; d++) {
				if (DimList[d] != 0) n++;
			}
			if (n > 0) { // n == 0 in case of general total, no variable is connected
				Unsafe[n - 1]++;
			}
		}
		return;
	}

	for (i = 0; i < SizeDim[niv]; i++) {
		DimList[niv] = i;
		CountUnSafe(DimList, FixedDimNr, Unsafe, niv + 1);
 	}

}

// Returns the cell number given indices.
//For example (0,0,0..0) returns 0 and (1,0,0..0) returns 1.
long CTable::GetCellNrFromIndices(long *Indices)
{ int i, c = 0;

	for (i = 0; i < nDim; i++) {
    ASSERT(Indices[i] >= 0 && Indices[i] < SizeDim[i]);
		c *= SizeDim[i];
		c += Indices[i];
	}

	return c;
}

// Given a CellNr returns an array of indices.
bool CTable::GetIndicesFromCellNr(long CellNr, long *Indices)
{
	int i, c = CellNr;

	for (i = nDim - 1; i >= 0; i--) {
		Indices[i] = c % SizeDim[i];
		c -= Indices[i];
		c /= SizeDim[i];
	}

	return (c != 0);
}

// cells that are secondary unsafe are reset to safe.
void CTable::UndoSecondarySuppress(long SortSuppress)
{
	long i;
	CDataCell *dc;
// set realized upper and upper to initial values
	for (i = 0; i < nCell; i++) {
		dc = GetCell(i);
		dc->SetRealizedUpperValue(0);
		dc->SetRealizedLowerValue(0);
		dc->SetCTAValue(0);
		switch (SortSuppress)	{
			case WITHOUT_SINGLETON:
				switch (dc->GetStatus()) {
					case CS_SECONDARY_UNSAFE:
						if (dc->GetFreq() == 0 ) {
							dc->SetStatus (CS_EMPTY_NONSTRUCTURAL);}
								else {
								dc->SetStatus(CS_SAFE);};
						break;
					case CS_SECONDARY_UNSAFE_MANUAL:
						dc->SetStatus(CS_SAFE_MANUAL);
						break;
				}
				break;
			case WITH_SINGLETON:
				switch (dc->GetStatus()) {
					case CS_UNSAFE_SINGLETON:
						dc->SetStatus(CS_SAFE);
						break;
					case CS_UNSAFE_SINGLETON_MANUAL:
						dc->SetStatus(CS_SAFE_MANUAL);
						break;
				}
				break;

				case BOTH:
					switch (dc->GetStatus()) {
						case CS_SECONDARY_UNSAFE:
						if (dc->GetFreq() == 0 ) {
							dc->SetStatus (CS_EMPTY_NONSTRUCTURAL);}
								else {
								dc->SetStatus(CS_SAFE);};
							break;
						case CS_SECONDARY_UNSAFE_MANUAL:
							dc->SetStatus(CS_SAFE_MANUAL);
							break;
						case CS_UNSAFE_SINGLETON:
							dc->SetStatus(CS_SAFE);
							break;
						case CS_UNSAFE_SINGLETON_MANUAL:
							dc->SetStatus(CS_SAFE_MANUAL);
							break;
				}

			}
		}
}

// Get Status and Cost per Explanatory variable
void CTable::GetStatusAndCostPerDim(long *Status, double *Cost)
{
	int i, j, ndim;
	long Dims[MAXDIM], index;
	CDataCell *dc;
	// I am not scaling the cost here
	memset(Status, 0, sizeof(long)   * (nDim + 1) * NCELLSTATUS);
	memset(Cost,   0, sizeof(double) * (nDim + 1) * NCELLSTATUS);

	for (i = 0; i < nCell; i++) {
		dc = GetCell(i);
		GetIndicesFromCellNr(i, Dims);
		for (j = nDim - 1, ndim = 0; j >= 0; j--) {
			if (Dims[j] != 0) ndim++;
		}
		index = ndim * NCELLSTATUS + dc->GetStatus() - CS_SAFE;
		Status[index]++;
		Cost[index] += dc->GetCost(Lambda);
	}
}

// The secondary unsafe cells are set as unsafe
bool CTable::SetSecondaryHITAS(FILE *fd, CVariable *var, long *nSetSecondary)
{
	char *p, str[200];
	long dims[MAXDIM];
	long CellDims[MAXDIM];

	*nSetSecondary = 0;


	int n;
	while (!feof(fd) ) {
		str[0] = 0;
		fgets(str, 200, fd);
		if (str[0] == 0) break;

		n = 0;
		p = str;
		while (p != 0 && n < nDim) {
			if (n < nDim) {
				dims[n++] = strtol(p, &p, 10);
			}
		};
		if (n != nDim) return false;

		SetSecondary(var, dims, CellDims, 0, nSetSecondary);
  }

	return true;
}

void CTable::SetSecondary(CVariable *var, long *dims, long *CellDims, int niv, long *nSetSecondary)
{
	int i, j, c;

	if (niv == nDim) {
		SetCellSecondaryUnsafe(CellDims);
		(*nSetSecondary)++;
		return;
	}

	CVariable *v = &(var[ExplVarnr[niv]]);
	CCode *phCode = v->GethCode();


	ASSERT(dims[niv] >= 0 && dims[niv] < SizeDim[niv]);

	for (i = c = 0; i < SizeDim[niv]; i++, c++) {
		if (v->IsHierarchical && phCode[i].nChildren == 1) { // bogus?
			c--;
		}
		else {
			if (c == dims[niv]) {
				CellDims[niv] = i;
				SetSecondary(var, dims, CellDims, niv + 1, nSetSecondary); // basic Cell
				if (v->IsHierarchical) { // set also higher bogus levels
					for (j = i - 1; j >= 0; j--) {
						if (phCode[j].nChildren != 1) break;
						CellDims[niv] = j;
    					SetSecondary(var, dims, CellDims, niv + 1, nSetSecondary);  // bogus Cell
					}
				}
				break;
			}
		}
	}
}

// Set Secondary cells as unsafe
void CTable::SetCellSecondaryUnsafe(long *dims)
{
	CDataCell *dc;

	dc = GetCell(dims);
	switch (dc->GetStatus()) {
	case CS_SAFE:
		dc->SetStatus(CS_SECONDARY_UNSAFE);
		break;
	case CS_SAFE_MANUAL:
		dc->SetStatus(CS_SECONDARY_UNSAFE_MANUAL);
		break;
    case CS_EMPTY_NONSTRUCTURAL:
		dc->SetStatus (CS_SECONDARY_UNSAFE);
		break;
	}
}

// Get information per status
void CTable::GetStatusStatistics(long *Freq, long *CellFreq, long *HoldingFreq,
											double * CellResp, double *CellCost)
{
	int i;
	CDataCell *dc;
	// compute number of CellStatuses
	// Cell is not bounded
	for (i = 0; i < NCELLSTATUS; i++) {
		Freq[i] = 0;
		CellFreq[i] = 0;
		HoldingFreq[i] = 0;
		CellResp[i] = 0;
		CellCost[i] = 0;
	}

	for (i = 0; i < nCell; i++) {
		ASSERT(GetCell(i)->GetStatus() > 0 && GetCell(i)->GetStatus() <= NCELLSTATUS);
		dc= GetCell(i);
		Freq[GetCell(i)->GetStatus() - 1]++;
		CellFreq[GetCell(i)->GetStatus()-1] =	CellFreq[GetCell(i)->GetStatus()-1] + dc->GetFreq();
		if (ApplyHolding)	{
			HoldingFreq[GetCell(i)->GetStatus()-1] =	HoldingFreq[GetCell(i)->GetStatus()-1] + dc->GetFreqHolding();
		}
		CellResp[GetCell(i)->GetStatus() - 1]= CellResp[GetCell(i)->GetStatus() - 1] +dc->GetResp();
		CellCost[GetCell(i)->GetStatus() - 1] =CellCost[GetCell(i)->GetStatus() - 1] + dc->GetCost(Lambda);

	}
}

// returns true if the cell is safe
bool CTable::IsCellSafe(long c)
{
	int status = GetCell(c)->GetStatus();
	return status == CS_SAFE || status == CS_SAFE_MANUAL || status == CS_PROTECT_MANUAL;
}

// returns true if the cell is unsafe
bool CTable::IsCellUnsafe(long c)
{
	int status = GetCell(c)->GetStatus();
	return status == CS_UNSAFE_FREQ || status == CS_UNSAFE_PEEP || status == CS_UNSAFE_RULE
		|| status == CS_UNSAFE_SINGLETON
		|| status == CS_UNSAFE_MANUAL
		|| status == CS_UNSAFE_ZERO;
}

// this is not used. Maybe there are plans to use it in the future
int CTable::GetCellDistance(long *dim1, long *dim2)
{
	int i, n = 0;

	for (i = 0; i < nDim; i++) {
		n += abs(dim1[i] - dim2[i]);
	}

	return n;
}

bool CTable::PrepareComputeDistance()
{
// This could be used sometime in the future

	/*long dim1[MAXDIM], dim2[MAXDIM];
  long Dist, i, j;
  BYTE *SafeOrUnsafe;

  if (CellDistance != 0) {
		delete [] CellDistance;
	}

  CellDistance = new int[nCell];
  if (CellDistance == 0) return false;

	SafeOrUnsafe = new BYTE[nCell];
  if (SafeOrUnsafe == 0) return false;

	memset(CellDistance, 0, sizeof(int) * nCell);

	// compute safe/unsafe for all cells
	CDataCell *dc;
	for (i = 0; i < nCell; i++) {

		SafeOrUnsafe[i] = 0;
		if (IsCellSafe(i) )   SafeOrUnsafe[i] = CS_SAFE;
		if (IsCellUnsafe(i) ) SafeOrUnsafe[i] = CS_UNSAFE;
  }

	for (i = 0; i < nCell; i++) {
    if (SafeOrUnsafe[i] != CS_SAFE) continue;
		Dist = nCell;
    GetIndicesFromCellNr(i, dim1);
		for (j = 0; j < nCell; j++) {
    if (SafeOrUnsafe[j] != CS_UNSAFE) continue;
      GetIndicesFromCellNr(j, dim2);
      long d = GetCellDistance(dim1, dim2);
			if (d < Dist) {
				Dist = d;
			}
		}
		ASSERT(Dist > 0 && Dist < nCell);
    CellDistance[i] = Dist;
	}

	delete [] SafeOrUnsafe;
	*/
  return true;
}

// also not used
bool CTable::GetCellDistance(long CellNr, long *Dist)
{
	if (CellDistance == 0) return false;
	*Dist = CellDistance[CellNr];
	return true;
}

//	Holding number of cell is initialized as With Holding.
// This is important in the operator overloading of the datacell
void CTable::InitializeHoldingNrs()
{
	int i;
	CDataCell *dc;
	if (ApplyHolding)	{
		for (i=0; i<nCell; i++)
		{
			dc = GetCell(i);
			dc->SetHoldingNr(WITH_HOLDING);
		}
	}
}


// Protection levels for a cell is set.
// 1) If the cell fails a rule. The protection level for all rules that could trigger an unsafe cell are calculated
// if the unsafe is not triggerd by the rule then protection level - shadow <= 0; The Protection Level is the maximum
// of the protection levels calculated.
// 2) if the cell is unsafe through Freq then Freq safety perc,
// 3) if the cell is unsafe through Zero rule then zero safety perc
// 4) if the cell is unsafe through peep then peep safety perc
// and etc.
bool CTable::SetProtectionLevelCell(CDataCell &datacell)
{
	double LowerProtectionLevel = 0, UpperProtectionLevel = 0;
	//double SlidingProtectionLevel = 0, ProtectionCapacity = 0;
	double Perc, Distance;
	double tempPerc = 0;
	long SafetyPerc =0;
	double tempProtectionLevel = 0;
	long status;
	status = datacell.GetStatus();
//	switch (datacell.GetStatus()) {
	switch (status)	{
	case 	CS_UNSAFE_RULE:
		if (DominanceRule)	{
			if ((DominancePercCell_1 >0) && (DominanceNumberCell_1 >0))	{
				tempProtectionLevel = 100.0 * datacell.ComputeWeightedScoreCell(ApplyWeight,DominanceNumberCell_1) / DominancePercCell_1;
				if ((tempProtectionLevel - datacell.GetShadow()) > 0)	{
					UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);
				}
			}
			if ((DominancePercCell_2 >0) && (DominanceNumberCell_2 >0))	{
				tempProtectionLevel = 100.0 * datacell.ComputeWeightedScoreCell(ApplyWeight,DominanceNumberCell_2) / DominancePercCell_2;
				if ((tempProtectionLevel - datacell.GetShadow()) > 0)	{
					UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);
				}
			}
			if (ApplyHolding)	{
				if ((DominancePercHolding_1 >0) && (DominanceNumberHolding_1 >0))	{
					tempProtectionLevel = 100.0 * datacell.ComputeWeightedScoreHolding(ApplyWeight,DominanceNumberHolding_1) / DominancePercHolding_1;
					if ((tempProtectionLevel - datacell.GetShadow()) > 0)	{
						UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);
					}
				}
				if ((DominancePercHolding_2 >0) && (DominanceNumberHolding_2 >0))	{
					tempProtectionLevel = 100.0 * datacell.ComputeWeightedScoreHolding(ApplyWeight,DominanceNumberHolding_2) / DominancePercHolding_2;
					if ((tempProtectionLevel - datacell.GetShadow()) > 0)	{
						UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);
					}
				}
			}
		}
		if (PQRule)	{
			if ((PQ_PCell_1 >0) && (PQ_NCell_1 >0))	{
				tempProtectionLevel = (PQ_PCell_1 * datacell.MaxScoreCell[0] / PQ_QCell_1) + datacell.ComputeWeightedScoreCell(ApplyWeight, PQ_NCell_1+1);
				if ((tempProtectionLevel - datacell.GetShadow()) > 0)	{
					UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);
				}
			}
			if ((PQ_PCell_2 >0) && (PQ_NCell_2 >0))	{
				tempProtectionLevel = (PQ_PCell_2 * datacell.MaxScoreCell[0] / PQ_QCell_2) + datacell.ComputeWeightedScoreCell(ApplyWeight, PQ_NCell_2 +1);
				if ((tempProtectionLevel - datacell.GetShadow()) > 0)	{
					UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);
				}
			}
			if (ApplyHolding)	{
				if ((PQ_PHolding_1) && (PQ_NHolding_1))	{
					tempProtectionLevel =  (PQ_PHolding_1 * datacell.MaxScoreHolding[0] / PQ_QHolding_1) + datacell.ComputeWeightedScoreHolding(ApplyWeight, PQ_NHolding_1+1);
					if ((tempProtectionLevel - datacell.GetShadow()) > 0)	{

						UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);
					}
				}
				if ((PQ_PHolding_2) && (PQ_NHolding_2))	{
					tempProtectionLevel = (PQ_PHolding_2 * datacell.MaxScoreHolding[0] / PQ_QHolding_2) + datacell.ComputeWeightedScoreHolding(ApplyWeight, PQ_NHolding_2+1);
					if ((tempProtectionLevel - datacell.GetShadow()) > 0)	{
						UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);
					}
				}
			}
		}

		// In some cases even though the rule triggers an unsafe cell. The
		// protection Level is 0(or close to 0). In this case a protection level has to be created
		//thus the following:
		Distance = UpperProtectionLevel - datacell.GetShadow();
		if (Distance <= 0) {//primary rule not violated, so minfreq or manual
			//if freq < SafeMinFreq
			if (datacell.GetFreq() <= SafeMinRec)	{
				SafetyPerc = CellFreqSafetyPerc;
				if (ApplyHolding) {
					if (datacell.GetFreqHolding() <= SafeMinHoldings) {
						SafetyPerc = __max(SafetyPerc,HoldingFreqSafetyPerc);
					}
				}
			}

			else   {
				// set manual safety perc
				SafetyPerc = ManualSafetyPerc;
			}
			//Distance = Shadow * (100.0 + SafetyPerc) / 100.0 - Shadow;// Distance = shadow*SafateyPerc/100.0
			Distance = (datacell.GetShadow() * SafetyPerc) / 100.0 ;// Distance = shadow*SafateyPerc/100.0
			if (Distance == 0) Distance = 1;
		}

		if (datacell.GetShadow() == 0) {
			Perc = ManualSafetyPerc / 100.0;
		}
		else {
			Perc = Distance / datacell.GetShadow();
		}
		break;
		// Not too sure about this
		case CS_UNSAFE_PEEP:
			Perc = PeepSafetyRangePercCell / 100.0;
			if (ApplyHolding)	{
					tempPerc = PeepSafetyRangePercHolding/100.0;
					Perc= __max(Perc,tempPerc);
			}
			break;
		case CS_UNSAFE_FREQ:
			Perc = CellFreqSafetyPerc / 100.0;


			break;
		case CS_UNSAFE_SINGLETON:
			Perc = SingletonSafetyRangePerc / 100.0;
			break;
		case CS_UNSAFE_MANUAL:
			Perc = ManualSafetyPerc / 100.0;
			break;
		default:
			Perc = 0;
	}
	//Laatste twee toegevoegd door Anco, 28/3/2003



  //ASSERT(Perc >= 0 && Perc <= 100);
  // calculate lower protection level.
    if (Perc < 0 ) Perc = - Perc;  // om te voorkomen dat een nega.cell een negatieve Prot level krijgt
	if (datacell.GetResp() != 0)	{
		UpperProtectionLevel = fabs(datacell.GetResp()) * Perc;
		if (UpperProtectionLevel < MinLPL) UpperProtectionLevel = MinLPL;
		LowerProtectionLevel = UpperProtectionLevel;
		// For Note PQ rule it changes !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// still to do:
		// SlidingProtectionLevel and ProtectionCapacity
	}
	else	{
		//UpperProtectionLevel = 1;
		//LowerProtectionLevel = 0;
		if (datacell.GetFreq() != 0)	{
			UpperProtectionLevel = ZeroSafetyRange;
			LowerProtectionLevel = 0;
		}
		else	{
			if (datacell.GetStatus() == CS_EMPTY_NONSTRUCTURAL)	{
				UpperProtectionLevel = NSEmptySafetyRange;
				LowerProtectionLevel = 0;
			}
		}

	}

	datacell.SetUpperProtectionLevel(UpperProtectionLevel);
	datacell.SetLowerProtectionLevel(LowerProtectionLevel);
	//*Sliding = SlidingProtectionLevel;
	//*Capacity = ProtectionCapacity;
	return true;
}

bool CTable::SetProtectionLevelCellFrequency(CDataCell &datacell, long Base, long K)
{
	double UpperProtectionLevel = 0;
	double LowerProtectionLevel = 0;
	if (datacell.GetStatus() == CS_UNSAFE_FREQ)	{
		if (!ApplyHolding)	{
			UpperProtectionLevel = (double)__max(K,Base-SafeMinRec);
			LowerProtectionLevel = datacell.GetResp();
			datacell.SetUpperProtectionLevel(UpperProtectionLevel);
			datacell.SetLowerProtectionLevel(LowerProtectionLevel);
		}
		else	{
			UpperProtectionLevel = (double)__max(K,Base-SafeMinHoldings);
			LowerProtectionLevel = datacell.GetResp();
			datacell.SetUpperProtectionLevel(UpperProtectionLevel);
			datacell.SetLowerProtectionLevel(LowerProtectionLevel);
		}
	}
	return true;
}