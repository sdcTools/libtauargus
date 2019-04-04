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

#if !defined DataCell_h
#define DataCell_h 

#include <string>

#include "defines.h"

extern int CurrentHoldingNr;
extern std::string LastHoldingCode;

class CDataCell
{
protected:
	double	Resp;    // Response, weighted if needed
        double  NWResp;  // Non-weighted response, needed to apply cell key method to weighted tables
	double	Cost;    // value depends on CostVarnr
	double	Shadow;  // only relevant for primary suppression pattern
        double  CellKey; // Total of record-keys of units in the cell, needed to apply cell key method
	long	Freq;
	double	Weight;
	long	FreqHolding;
	//long    RoundedResp;
        double  RoundedResp;
	double  CTAValue;
        double  CKMValue;

	int	Status;
	double	TempShadow;
	int	HoldingNr; //Holding number of the temp shadow
	int	PeepSortCell;
	int	PeepSortHolding;
	int	TempPeepSort;

	double	PeepCell;
	double	PeepHolding;

	double	RealizedUpperValue; // Anneke's upper value
	double	RealizedLowerValue; // Anneke's lower value

	double	LowerProtectionLevel;
	double	UpperProtectionLevel;
        //double  SlidingProtectionLevel;
        //double  ProtectionCapacity;
        
        
// Construction
public:
	CDataCell(int MaxScoreCell, int MaxScoreHolding, int IsHolding, int IsWeight );
	CDataCell();

// Operations
public:

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
                NWResp += a.NWResp;
		Resp += a.Resp;
		Shadow += a.Shadow;
		Cost += a.Cost;
		Weight += a.Weight;
                CellKey += a.CellKey;
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

        // set data
	void SetResp(double Resp)			{ this->Resp = Resp; }
        void SetNWResp(double NWResp)			{ this->NWResp = NWResp; }
	void SetRoundedResponse(double roundedresp)	{ this->RoundedResp = roundedresp; }
	void SetWeight(double weight)			{ this->Weight = weight; }
	void SetFreq(long Freq) 			{ this->Freq = Freq; }
	void SetFreqHolding(long FreqHolding)		{ this->FreqHolding = FreqHolding; }
	void SetShadow(double Shadow)			{ this->Shadow = Shadow; }
	void SetCost(double Cost)			{ this->Cost = Cost; }
        void SetCellKey(double CellKey)                 { this->CellKey = CellKey; }
        void SetCKMValue(double CKMValue)               { this->CKMValue = CKMValue; }
	void SetStatus(long Status)			{ this->Status = Status; }
	void SetCTAValue(double CTAValue)               { this->CTAValue = CTAValue; }
	void SetTempShadow(double TempShadow)		{ this->TempShadow = TempShadow; }
	void SetHoldingNr(int HoldingNr)		{ this->HoldingNr = HoldingNr; }
	void SetRealizedUpperValue(double UpperValue)	{ this->RealizedUpperValue = UpperValue; }
	void SetRealizedLowerValue(double LowerValue)	{ this->RealizedLowerValue = LowerValue; }
	void SetPeepCell(double PeepValue)		{ this->PeepCell = PeepValue; }
	void SetPeepHolding(double PeepValue)		{ this->PeepHolding = PeepValue; }
	void SetPeepSortCell(int iPeepsort)		{ this->PeepSortCell = iPeepsort; }
	void SetPeepSortHolding(int iPeepsort)		{ this->PeepSortHolding = iPeepsort; }
	void SetUpperProtectionLevel(double UPL)	{ this->UpperProtectionLevel = UPL; }
	void SetLowerProtectionLevel(double LPL)	{ this->LowerProtectionLevel = LPL; }

	
	
	// get data
        double GetResp()			{ return Resp; }
	double GetNWResp()			{ return NWResp; }
        double GetRoundedResponse()		{ return RoundedResp; }
	double GetShadow()			{ return Shadow; }
        double GetCellKey()                     { return CellKey; }
        double GetCKMValue()                    { return CKMValue; }
	double GetCost(double Lambda);
	long   GetFreq()			{ return Freq; }
	double GetWeight()			{ return Weight; }
	long   GetFreqHolding()			{ return FreqHolding; }
	long   GetStatus()			{ return Status; }
	double GetCTAValue()                    { return CTAValue; }
	double GetTempShadow()			{ return TempShadow; }
	long   GetTempPeepSortCell()            { return TempPeepSort; } //AHNL 5.1.2004
	int    GetHoldingNr()			{ return HoldingNr; }
	double GetRealizedUpperValue()		{ return RealizedUpperValue; }
	double GetRealizedLowerValue()		{ return RealizedLowerValue; }
	double GetPeepCell()                    { return PeepCell; }
	double GetPeepHolding()			{ return PeepHolding; }
	long   GetPeepSortCell()		{ return PeepSortCell; }
	long   GetPeepSortHolding()		{ return PeepSortHolding; }
	double GetUpperProtectionLevel()	{ return UpperProtectionLevel; }
	double GetLowerProtectionLevel()	{ return LowerProtectionLevel; }


	void MergeScoreHolding(double *a, int *ah, double *b, int *bh, int n);
	void MergeScore(double *a, double *aw, double* b, double *bw, int n);  // for MaxScore and MaxScoreWeight
	double GetDominancePercCell(bool ApplyWeight, bool ApplyWeightOnSafetyRule, long DominanceNumber);
	double GetDominancePercHolding(bool ApplyWeight, bool ApplyWeightOnSafetyRule, long DominanceNumber);
	double GetPQCell(double p, double q, long n, bool ApplyWeight, bool ApplyWeightOnSafetyRule);
	double GetPQHolding(double p, double q, long n, bool ApplyWeight, bool ApplyWeightOnSafetyRule);
	double ComputeWeightedScoreCell(bool DoWeight, long NumberOfScores);
	double ComputeWeightedScoreHolding(bool DoWeight, long NumberOfScores);
	bool Compare(const CDataCell &a) const;
};

#endif // DataCell_h

