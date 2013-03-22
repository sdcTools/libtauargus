/*#if !defined(AFX_DATACELL_H__87A5FD81_6949_11D5_BC5D_00C04F9A7DB5__INCLUDED_)
#define AFX_DATACELL_H__87A5FD81_6949_11D5_BC5D_00C04F9A7DB5__INCLUDED_

// Contents data cell TauArgus

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000 */
// DataCell.h : header file
#if !defined DataCell_h
#define DataCell_h 
extern int CurrentHoldingNr;
extern CString LastHoldingCode;
#include "defines.h"
//#include "Globals.h"

/////////////////////////////////////////////////////////////////////////////
// CDataCell window

class CDataCell
{
// Construction
public:
	CDataCell(int MaxScoreCell, int MaxScoreHolding, int IsHolding, int IsWeight );
	CDataCell();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataCell)
	//}}AFX_VIRTUAL

// Implementation
public:
	int nMaxScoreCell;
	double *MaxScoreCell;// Shadow, ongewogen!
	double *MaxScoreWeightCell; // bijbehorend gewicht van ongewogen MaxScores, 0 indien nvt of niet toe te passen
	
	int nMaxScoreHolding;
	double *MaxScoreHolding;
	double *MaxScoreWeightHolding;
	int *HoldingnrPerMaxScore;// keeps de holding number per Maxscore
	virtual ~CDataCell();
	bool IsFilled;
	void operator+=(CDataCell& a)
	{ 
		IsFilled = true;
		Resp += a.Resp;
		Shadow += a.Shadow;
		Cost += a.Cost;
		Weight += a.Weight;
		// Not too sure about this
		
		// add frequencies only if holding number is different
		// How do I know that this cell comes from an apply holding
		if(HoldingNr == WITHOUT_HOLDING){
				Freq += a.Freq;
				if (a.PeepCell > PeepCell) {
					PeepCell = a.PeepCell;
					if ((PeepSortCell == NOPEEP) || (a.PeepSortCell == NOPEEP))	{
						PeepSortCell = NOPEEP;
					}
					else	{
						PeepSortCell = a.PeepSortCell;
					}
				}
			}
			else {
				Freq += a.Freq;
				if (a.PeepCell > PeepCell) {
					PeepCell = a.PeepCell;
					if ((PeepSortCell == NOPEEP) || (a.PeepSortCell == NOPEEP))	{
						PeepSortCell = NOPEEP;
					}
					else	{
						PeepSortCell = a.PeepSortCell;
					}
				}
			
				// Now the Holdings
				// First Holding
				if ((HoldingNr == WITH_HOLDING) && (CurrentHoldingNr != -1))
				{
					// same as a standard cell
					// 4 juni 2010 Freqholding toegevoegd AHNL
					FreqHolding += a.FreqHolding;					
					TempShadow = a.Shadow;
					HoldingNr = CurrentHoldingNr;
					TempPeepSort = a.PeepSortCell;
				}
				else {
					// New Holding
					if (CurrentHoldingNr != HoldingNr){
						FreqHolding ++; //????? ANCO 14 maart 2006
						if (TempPeepSort != NOPEEP)	{
							// Largest Holding
							//if (Peep < TempShadow) {
							if (PeepHolding < TempShadow)	{
								//Peep = TempShadow;
								PeepHolding = TempShadow;
								PeepSortHolding= TempPeepSort;
							}
						}
						TempPeepSort = a.PeepSortCell;
						TempShadow = a.Shadow;
						HoldingNr = CurrentHoldingNr;
					}
					else{
						TempShadow += a.Shadow;
						
						/*if ((a.PeepSortCell != 0) && (TempPeepSort == 0))	{
							TempPeepSort = a.PeepSortCell;
						}*/
						TempPeepSort = a.PeepSortCell;
					}
				}
			}
			if (HoldingNr == WITHOUT_HOLDING)
			{
				MergeScore(MaxScoreCell, MaxScoreWeightCell, a.MaxScoreCell, a.MaxScoreWeightCell, nMaxScoreCell);}
			else{
				MergeScore(MaxScoreCell, MaxScoreWeightCell, a.MaxScoreCell, a.MaxScoreWeightCell, nMaxScoreCell);
				MergeScoreHolding(MaxScoreHolding, HoldingnrPerMaxScore, a.MaxScoreHolding, a.HoldingnrPerMaxScore, nMaxScoreHolding);
			}
	}

	CDataCell operator+(CDataCell &a)
	{ 
		CDataCell r = *this;
		r += a;
		return r;
	}

  // set counting data
	void SetResp(double Resp)								{	this->Resp = Resp; }
	void SetRoundedResponse(long roundedresp)			{	this ->RoundedResp = roundedresp;}
	void SetWeight(double weight)							{	this->Weight = weight;}
	void SetFreq(long Freq) 								{	this->Freq = Freq; }
	void SetFreqHolding(long FreqHolding)				{	this->FreqHolding = FreqHolding;}
	void SetShadow(double Shadow)							{	this->Shadow = Shadow; }
	void SetCost(double Cost)								{	this->Cost = Cost; }
	void SetStatus(long Status)							{	this->Status = Status; }
	void SetCTAValue(double CTAValue)                   { this->CTAValue = CTAValue; }
	void SetTempShadow(double TempShadow)				{	this->TempShadow = TempShadow; }
	void SetHoldingNr(int HoldingNr)						{	this->HoldingNr = HoldingNr; }
	void SetRealizedUpperValue(double UpperValue)	{	this-> RealizedUpperValue = UpperValue;}
	void SetRealizedLowerValue(double LowerValue)	{	this-> RealizedLowerValue = LowerValue;}
	void SetPeepCell(double PeepValue)					{	this->PeepCell = PeepValue;}
	void SetPeepHolding(double PeepValue)				{	this->PeepHolding = PeepValue;}
	void SetPeepSortCell(int iPeepsort)					{	this->PeepSortCell = iPeepsort;} 
	void SetPeepSortHolding(int iPeepsort)				{	this->PeepSortHolding = iPeepsort;}
	void SetUpperProtectionLevel(double UPL)			{	this->UpperProtectionLevel = UPL;}
	void SetLowerProtectionLevel(double LPL)			{	this->LowerProtectionLevel = LPL;}

	
	
	// get counting data
	double GetResp()							{	return Resp;  }
	long   GetRoundedResponse()			{	return RoundedResp; }
	double GetShadow()						{	return Shadow;}
	double GetCost(double Lambda);//	 { return Cost;  }
	long   GetFreq()							{	return Freq;  }
	double GetWeight()						{	return Weight; }
	long   GetFreqHolding()					{	return FreqHolding;}
	long   GetStatus()						{	return Status;}
	double GetCTAValue()                    {   return CTAValue; }
	double GetTempShadow()					{	return TempShadow;  }
	long   GetTempPeepSortCell()        {  return TempPeepSort; } //AHNL 5.1.2004
	int   GetHoldingNr()						{	return HoldingNr;  }
	double GetRealizedUpperValue()		{	return RealizedUpperValue;}
	double GetRealizedLowerValue()		{	return RealizedLowerValue;}
	double GetPeepCell()                {	return PeepCell;}
	double GetPeepHolding()					{	return PeepHolding; }
	long GetPeepSortCell()					{	return PeepSortCell;}
	long GetPeepSortHolding()				{	return PeepSortHolding;}
	double GetUpperProtectionLevel()		{	return UpperProtectionLevel;}
	double GetLowerProtectionLevel()		{	return LowerProtectionLevel;}


	void MergeScoreHolding(double *a, int *ah, double *b, int *bh, int n);
	void MergeScore(double *a, double *aw, double* b, double *bw, int n);  // for MaxScore and MaxScoreWeight
	double GetDominancePercCell(BOOL ApplyWeight, BOOL ApplyWeightOnSafetyRule, long DominanceNumber);
	double GetDominancePercHolding(BOOL ApplyWeight, BOOL ApplyWeightOnSafetyRule, long DominanceNumber);
	double GetPQCell(double p, double q, long n, BOOL ApplyWeight, BOOL ApplyWeightOnSafetyRule);
	double GetPQHolding(double p, double q, long n, BOOL ApplyWeight, BOOL ApplyWeightOnSafetyRule);
	 //BOOL GetProtectionLevel(long SafetyRule,long ManualSafetyPerc, long FreqSafetyPerc, long PeepSafetyRangePerc, long SingletonSafetyRangePerc,
		// double ZeroSafetyRangePerc, long MinSafeRec, long LenOfArray,BOOL DoWeight, 
		 //int k, int p, int q, double *Upper, double *Lower, double *Sliding, 
		// double *Capacity);
	double ComputeWeightedScoreCell(BOOL DoWeight, long NumberOfScores);
	double ComputeWeightedScoreHolding(BOOL DoWeight, long NumberOfScores);
	 /*BOOL GetProtectionLevel(long SafetyRule,
											  long ManualSafetyPerc, 
                                   long CellFreqSafetyPerc,
											  long HoldingFreqSafetyRangePerc,
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
                                   double *Sliding, double *Capacity); */
	bool Compare(CDataCell &a);
	 
	 // void SortMaxScoreHolding();


protected:
	double	Resp;
	double	Cost;    // value depends on CostVarnr
	double	Shadow;  // only relevant for primary suppression pattern
	long	Freq;
	double	Weight;
	long	FreqHolding;
	long    RoundedResp;
	double  CTAValue;

	int		Status;
	double	TempShadow;
	int		HoldingNr; //Holding number of the temp shadow
	int		PeepSortCell;
	int		PeepSortHolding;
	int		TempPeepSort;

//For Dick Windmeijer a few variable not relevant for him haved been shortened to long
//	long	PeepCell;
//	long	PeepHolding;
//
//	long	RealizedUpperValue; // Anneke's upper value
//	long	RealizedLowerValue; // Anneke's lower value
//
//	long	LowerProtectionLevel;
//	long	UpperProtectionLevel;

//Normal version
	double	PeepCell;
	double	PeepHolding;

	double	RealizedUpperValue; // Anneke's upper value
	double	RealizedLowerValue; // Anneke's lower value

	double	LowerProtectionLevel;
	double	UpperProtectionLevel;
  //double  SlidingProtectionLevel;
  //double  ProtectionCapacity;

	
	// Generated message map functions
	//{{AFX_MSG(CDataCell)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

//	DECLARE_MESSAGE_MAP()
};
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

//#endif // !defined(AFX_DATACELL_H__87A5FD81_6949_11D5_BC5D_00C04F9A7DB5__INCLUDED_)
