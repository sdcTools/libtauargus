// DataCell.cpp : implementation file
//

#include "stdafx.h"
#include "DataCell.h"
#include "math.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataCell
// Constructor used in principle for creating a new cell that is added to the table
CDataCell::CDataCell(int NumberMaxScoreCell, int NumberMaxScoreHolding, int IsHolding, int IsWeight)
{
//	FILE *fd;
//	fd = fopen("E:/Temp/Debug.txt","a");
//		fprintf(fd,"%Xh%s\n", this,"		CDatacell		Const		()  ");
//	fclose(fd);
	PeepSortCell = EMPTY;
	PeepSortHolding = EMPTY;
	TempPeepSort = EMPTY;
	PeepCell = 0;
	PeepHolding = 0;
	Resp = 0;
	RoundedResp = 0;
	Cost = 0;
	Weight = 0;
	Shadow = 0;
	Freq = 0;
	FreqHolding = 0;
	TempShadow = 0;
	if (IsHolding) {
		HoldingNr = WITH_HOLDING;
	}
	else {
		HoldingNr = WITHOUT_HOLDING;
	}
	nMaxScoreCell = NumberMaxScoreCell;
	nMaxScoreHolding = NumberMaxScoreHolding;
	MaxScoreCell = 0;
	MaxScoreWeightCell = 0;
	MaxScoreHolding = 0;
	MaxScoreWeightHolding = 0;
	HoldingnrPerMaxScore = 0;
	IsFilled = false;
  
	Status = CS_EMPTY;
  //set ini values for realized upper and lower.as 0
	RealizedUpperValue = 0;
	RealizedLowerValue = 0;
	UpperProtectionLevel = 0;
	LowerProtectionLevel = 0;


	if (nMaxScoreCell > 0) {

			// allocate memory for MaxScore and MaxScoreWeight
			MaxScoreCell       = new double[nMaxScoreCell];
			if (IsWeight)	{
				MaxScoreWeightCell = new double[nMaxScoreCell];
			}
			// Allocate Memory for HoldingnrMaxScore

		
			//HoldingnrPerMaxScore = new int[nMaxScore];
			

			memset( MaxScoreCell,       0, sizeof(double) * nMaxScoreCell);
			if (IsWeight)	{
				memset( MaxScoreWeightCell, 0, sizeof(double) * nMaxScoreCell);
			}
			
			//memset( HoldingnrPerMaxScore,       -1, sizeof(int) * nMaxScore);
	}

	if (IsHolding && nMaxScoreHolding > 0)	{
			// allocate memory for MaxScore and MaxScoreWeight
			MaxScoreHolding       = new double[nMaxScoreHolding];
			if (IsWeight)	{
				MaxScoreWeightHolding = new double[nMaxScoreHolding];
			}
			// Allocate Memory for HoldingnrMaxScore

		
			HoldingnrPerMaxScore = new int[nMaxScoreHolding];
			

			memset( MaxScoreHolding,       0, sizeof(double) * nMaxScoreHolding);
			if (IsWeight)	{
				memset( MaxScoreWeightHolding, 0, sizeof(double) * nMaxScoreHolding);
			}
			
			memset( HoldingnrPerMaxScore,       -1, sizeof(int) * nMaxScoreHolding);

	}

}

// constructor used within functions. does not have the arrays for
// maxscore
CDataCell::CDataCell()
{
//	FILE *fd;
//	fd = fopen("E:/Temp/Debug.txt","a");
//	fprintf(fd,"%Xh%s\n", this,"		CDatacell		Const		()  ");
//	fclose(fd);
	PeepSortCell = EMPTY;
	PeepSortHolding = EMPTY;
	TempPeepSort = EMPTY;
	Weight = 0;
	PeepCell = 0;
	PeepHolding = 0;
	Resp = 0;
	RoundedResp = 0;
	CTAValue = 0;
	Cost = 0;
	Shadow = 0;
	Freq = 0;
	FreqHolding = 0;
	TempShadow = 0;
	HoldingNr = WITHOUT_HOLDING;
	nMaxScoreCell = 0;
	nMaxScoreHolding = 0;
	MaxScoreCell = 0;
	MaxScoreWeightCell = 0;
	MaxScoreHolding = 0;
	MaxScoreWeightHolding = 0;
	HoldingnrPerMaxScore = 0;
	IsFilled = false;
  
	Status = CS_EMPTY;
  //set ini values for realized upper and lower.as 0
	RealizedUpperValue = 0;
	RealizedLowerValue = 0;
	UpperProtectionLevel = 0;
	LowerProtectionLevel = 0;


	//LowerProtectionLevel = 0;
  //UpperProtectionLevel = 0;
  //SlidingProtectionLevel = 0;
  //ProtectionCapacity = 0;
}


// Destructor. All arrays that are created should be cleaned.
CDataCell::~CDataCell()
{
//	FILE *fd;
//	fd = fopen("E:/Temp/Debug.txt","a");
//	fprintf(fd,"%Xh%s\n", this,"		CDatacell		Dest  ");
//	fclose(fd);

	if (MaxScoreCell != 0) {
			delete [] MaxScoreCell;
	}
	if (MaxScoreWeightCell != 0) {
		delete MaxScoreWeightCell;
	}
	
	if (MaxScoreHolding != 0) {
			delete [] MaxScoreHolding;
	}
	if (MaxScoreWeightHolding != 0) {
		delete MaxScoreWeightHolding;
	}

	if (HoldingnrPerMaxScore != 0) {
		delete [] HoldingnrPerMaxScore;
	}
	
}
// merges the max score holding from two cells along with the holding numbers.
// this is used in recoded tables.
void CDataCell::MergeScoreHolding(double *a, int *ah, double *b, int *bh, int n)
{
	int i,j,ai = 0;
	
	if  (b == 0) return;
	double  * tempbval = new double [n] ;
	int * tempbhol = new int [n];

	for (i= 0; i< n; i++)
	{
		tempbval[i] = b[i];
		tempbhol[i] = bh[i];
	}

	// add holdings that are equal with b and put a[i] in the temp
	for (i=0; i<n; i ++){
		for (j= 0; j<n; j++){
			if ((ah[i] == tempbhol[j])) 
			{a[i] = a[i] + tempbval[j];
			 tempbval[j] = a[i] + tempbval[j];
			}
		}
	}
	
	for (i = 0; i < n; i++) {
		if (tempbval[i] > a[ai]) {
			// shift down
      for (j = n - 2; j >= ai; j--) {
				a[j + 1] = a[j];
				ah[j + 1] = ah[j];
			}
			a[ai] = tempbval[i];
			ah[ai] = tempbhol[i];
		} else {
			ai++;
			i--;
		}
		if (ai == n) break; // all elements from b smaller than a
	}
	delete [] tempbval;
	delete [] tempbhol;

}

// merges two arrays when cells are merged. This is used in recoded tables
void CDataCell::MergeScore(double *a, double *aw, double* b, double *bw, int n)  // for MaxScore and MaxScoreWeight
{ 
	int i, j, ai = 0;

	if (b == 0) return; // no scores

	for (i = 0; i < n; i++) {
		if (b[i] > a[ai]) {
			// shift down
			for (j = n - 2; j >= ai; j--) {
				a[j + 1] = a[j];
				if (aw != 0)	{
					aw[j + 1] = aw[j];
				}
			}
			a[ai] = b[i];
			if ((aw != 0) && (bw != 0 ))	{
				aw[ai++] = bw[i];
			}
		} 
		else {
			ai++;
			i--;
		}
		if (ai == n) break; // all elements from b smaller than a
	}
}

// Note the two functions below could be combined and made into one function with a boolean
// to direct you to use weight or not
// Gets the dominance Perc of cell.
double CDataCell::GetDominancePercCell(BOOL ApplyWeight, 
													BOOL ApplyWeightOnSafetyRule, long DominanceNumber)
{ 
	ASSERT(MaxScoreCell != 0);
	if (MaxScoreCell[0] == 0) return 0;
	if (Shadow == 0) return 0;

	return ComputeWeightedScoreCell(ApplyWeight && ApplyWeightOnSafetyRule,DominanceNumber ) * 100.0 / Shadow;
}

// Gets the dominance Perc of holding. These two 
double CDataCell::GetDominancePercHolding(BOOL ApplyWeight, 
													BOOL ApplyWeightOnSafetyRule, long DominanceNumber)
{ 
	ASSERT(MaxScoreHolding != 0);
	if (MaxScoreHolding[0] == 0) return 0;
	if (Shadow == 0) return 0;

	return ComputeWeightedScoreHolding(ApplyWeight && ApplyWeightOnSafetyRule,DominanceNumber ) * 100.0 / Shadow;
}

// Gets PQ for cell.

double CDataCell::GetPQCell(double p, double q, long n, 
									 BOOL ApplyWeight, BOOL ApplyWeightOnSafetyRule)
{
	if (!ApplyWeight || !ApplyWeightOnSafetyRule) {  
		int i;
		double s = 0;
		for (i = 0; i <= n; i++) {
			s += MaxScoreCell[i];
		}
		return (p / 100) * MaxScoreCell[0] + (q / 100) * (s - Shadow);
  }

  // Apply weight // Klopt dit?
	return (p / 100) * MaxScoreCell[0] + (q / 100) * 
		(ComputeWeightedScoreCell(true,n+1) - Shadow);
}

//  gets PQ holding
double CDataCell::GetPQHolding(double p, double q, long n, BOOL ApplyWeight, BOOL ApplyWeightOnSafetyRule)
{
	if (!ApplyWeight || !ApplyWeightOnSafetyRule) {  
		int i;
		double s = 0;
		for (i = 0; i <= n; i++) {
			s += MaxScoreHolding[i];
		}
		return (p / 100) * MaxScoreHolding[0] + (q / 100) * (s - Shadow);
	}

	// Apply weight
	return (p / 100) * MaxScoreHolding[0] + (q / 100) * (ComputeWeightedScoreHolding(true,n+1) - Shadow);
}

// Each of the top n is factored with the corresponding weights.
double CDataCell::ComputeWeightedScoreCell(BOOL DoWeight, long NumberOfScores )
{ 
	int i;
	double score = 0;

	if (DoWeight) {
		double WeightScore = 0;
		for (i = 0; i < NumberOfScores; i++) { 
			if (WeightScore + MaxScoreWeightCell[i] < NumberOfScores) {
				score += MaxScoreCell[i] * MaxScoreWeightCell[i];
				WeightScore += MaxScoreWeightCell[i];
			} 
			else {
				score += MaxScoreCell[i] * (NumberOfScores - WeightScore); // remaining part
				break;
			}
		}
		return score;
	}

	// no weight
	for (i = 0; i < NumberOfScores; i++) { 
  		score += MaxScoreCell[i];
	}
	return score;
}

// each of the top n holdings is factored with holdings.
double CDataCell::ComputeWeightedScoreHolding(BOOL DoWeight, long NumberOfScores )
{ 
	int i;
	double score = 0;

	if (DoWeight) {
		double WeightScore = 0;
		for (i = 0; i < NumberOfScores; i++) { 
			if (WeightScore + MaxScoreWeightHolding[i] < NumberOfScores) {
				score += MaxScoreHolding[i] * MaxScoreWeightHolding[i];
				WeightScore += MaxScoreWeightHolding[i];
			} 
			else {
				score += MaxScoreHolding[i] * (NumberOfScores - WeightScore); // remaining part
				break;
			}
		}
		return score;
	}

	// no weight
	for (i = 0; i < NumberOfScores; i++) { 
  		score += MaxScoreHolding[i];
	}
	return score;
}


//BEGIN_MESSAGE_MAP(CDataCell, CWnd)
	//{{AFX_MSG_MAP(CDataCell)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDataCell message handlers
// Not yet been done for the 2 rules
/*
BOOL CDataCell::GetProtectionLevel(long SafetyRule,
											  long ManualSafetyPerc, 
                                   long CellFreqSafetyPerc,
											  long HoldingFreqSafetyPerc,
											  long PeepSafetyRangePerc,
											  long HoldingPeepSafetyRangePerc,
											  long SingletonSafetyRangePerc,
											  double ZeroSafetyRangePerc,
											  long MinSafeRec,
											  long MinSafeHold,
											  bool WithHolding,
                                   BOOL DoWeight, 
											  int kCell_1,int kCell_2, int kHolding_1, int kHolding_2, 
											  int nkCell_1,int nkCell_2, int nkHolding_1, int nkHolding_2, 
											  int pCell_1,int pCell_2, int pHolding_1, int pHolding_2,
											  int qCell_1,int qCell_2, int qHolding_1, int qHolding_2, 
                                   int npqCell_1,int npqCell_2, int npqHolding_1, int npqHolding_2, 
											  double *Upper, double *Lower, 
                                   double *Sliding, double *Capacity)
{ 
	double LowerProtectionLevel = 0, UpperProtectionLevel = 0;
	double SlidingProtectionLevel = 0, ProtectionCapacity = 0;
	double Perc, Distance;
	double tempPerc = 0;
	long SafetyPerc =0;
	double tempProtectionLevel = 0;
	switch (Status) {
	case 	CS_UNSAFE_RULE:
		switch(SafetyRule) {
		case DOMINANCE:
			if ((kCell_1 >0) && (nkCell_1 >0))	{
				tempProtectionLevel = 100.0 * ComputeWeightedScoreCell(DoWeight,nkCell_1) / kCell_1;
				UpperProtectionLevel = tempProtectionLevel;
			}
			if ((kCell_2 >0) && (nkCell_2 >0))	{
				tempProtectionLevel = 100.0 * ComputeWeightedScoreCell(DoWeight,nkCell_2) / kCell_2;
				UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);
			}
			if (WithHolding)	{
				if ((kHolding_1 >0) && (nkHolding_1 >0))	{
					tempProtectionLevel = 100.0 * ComputeWeightedScoreHolding(DoWeight,nkHolding_1) / kHolding_1;
					UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);

				}
				if ((kHolding_2 >0) && (nkHolding_2 >0))	{
					tempProtectionLevel = 100.0 * ComputeWeightedScoreHolding(DoWeight,nkHolding_2) / kHolding_2;
					UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);

				}
			}
			break;
		case PQRULE:
			if ((pCell_1 >0) && (npqCell_1 >0))	{
				tempProtectionLevel = (pCell_1 * MaxScoreCell[0] / qCell_1) + ComputeWeightedScoreCell(DoWeight, npqCell_1);
				UpperProtectionLevel = tempProtectionLevel;				
			}
			if ((pCell_2 >0) && (npqCell_2 >0))	{
				tempProtectionLevel = (pCell_2 * MaxScoreCell[0] / qCell_2) + ComputeWeightedScoreCell(DoWeight, npqCell_2);
				UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);
			}
			if (WithHolding)	{
				if ((pHolding_1) && (npqHolding_1))	{
					tempProtectionLevel =  (pHolding_1 * MaxScoreHolding[0] / qHolding_1) + ComputeWeightedScoreHolding(DoWeight, npqHolding_1);
					UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);

				}
				if ((pHolding_2) && (npqHolding_2))	{
					tempProtectionLevel = (pHolding_2 * MaxScoreHolding[0] / qHolding_2) + ComputeWeightedScoreHolding(DoWeight, npqHolding_2);
					UpperProtectionLevel = __max(UpperProtectionLevel,tempProtectionLevel);

				}
			}
			break;

		//	ASSERT(q != 0);
		//	if (q == 0) return false;
			
			//UpperProtectionLevel = p * MaxScoreCell[0] / q + ComputeWeightedScoreCell(DoWeight, LenOfArray);
		
		}
		Distance = UpperProtectionLevel - Shadow;
		if (Distance <= 0) {//primary rule not violated, so minfreq or manual
			//if freq < SafeMinFreq
			if (Freq <= MinSafeRec)	{ 
				SafetyPerc = CellFreqSafetyPerc;
				if (WithHolding) {
					if (FreqHolding <= MinSafeHold) {
						SafetyPerc = __max(SafetyPerc,HoldingFreqSafetyPerc);
					}
				}
			}

			else   {                 
				SafetyPerc = ManualSafetyPerc;
			}
			//Distance = Shadow * (100.0 + SafetyPerc) / 100.0 - Shadow;// Distance = shadow*SafateyPerc/100.0
			Distance = (Shadow * SafetyPerc) / 100.0 ;// Distance = shadow*SafateyPerc/100.0
			if (Distance == 0) Distance = 1;
		} 

		if (Shadow == 0) {
		Perc = ManualSafetyPerc / 100.0;
		} else {
			Perc = Distance / Shadow;
		}
		break;
		// Not too sure about this
		case CS_UNSAFE_PEEP:
			Perc = PeepSafetyRangePerc / 100.0;
			if (WithHolding)	{
					tempPerc = HoldingPeepSafetyRangePerc/100.0;
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
	if (Resp != 0)	{
		UpperProtectionLevel = Resp * Perc;
		if (UpperProtectionLevel < 1) UpperProtectionLevel = 1;
		LowerProtectionLevel = UpperProtectionLevel;
		// For Note PQ rule it changes !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// still to do:
		// SlidingProtectionLevel and ProtectionCapacity
	}
	else	{
		//UpperProtectionLevel = 1;
		//LowerProtectionLevel = 0;
		UpperProtectionLevel = ZeroSafetyRangePerc / 100.0;
		LowerProtectionLevel = ZeroSafetyRangePerc / 100.0;
	
	}
	*Upper   = UpperProtectionLevel;
	
	*Lower   = LowerProtectionLevel;

	*Sliding = SlidingProtectionLevel;
	*Capacity = ProtectionCapacity;
	return true;
}
*/
/*
// Not too good so watch out
void CDataCell::SortMaxScoreHolding()
{
	int i,j;
	double dtemp;
	int itemp;

	for (i=0;i<nMaxScoreHolding-1; i++)
	{
		for(j=i+1;j<nMaxScoreHolding;j++)
		{
			if (MaxScoreHolding[i] < MaxScoreHolding[j])
			{
				dtemp = MaxScoreHolding[i];
				itemp = HoldingnrPerMaxScore[i];
				MaxScoreHolding[i] = MaxScoreHolding[j];
				HoldingnrPerMaxScore[i] = HoldingnrPerMaxScore[j];
				MaxScoreHolding[j] = dtemp;
				HoldingnrPerMaxScore[j] = itemp;
			}
		}
	}
}


*/
// compare tow cells. comparison bases is frequency, shadow, response and cost.
// returns true if the two cells are equal and false if not.
bool CDataCell::Compare(CDataCell &a)
{
	if (!DBL_EQ(Resp,a.Resp))	
	{
		return false;
	}
	if (!DBL_EQ(Cost,a.Cost))    
	{
		return false;
	}
	if (!DBL_EQ(Shadow,a.Shadow))
	{
		return false;
	}
	if (Freq != a.Freq)
	{
		return false;
	}
	return true;
}

double CDataCell::GetCost(double Lambda)
{
	if (Lambda >0)	{
		return pow(Cost,Lambda);

	}
	else
	{
		return log(Cost +1);
	}
}
