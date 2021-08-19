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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>

#include "General.h"
#include "JJFormat.h"

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJJFormat::CJJFormat()
{

}

CJJFormat::~CJJFormat()
{

}

// Write cells to be used in JJ format
bool CJJFormat::WriteCells(FILE *fd, FILE *fdFrq, double LowerBound, double UpperBound,
									CTable *tab, CVariable *var, int nDecResp,
									bool WithBogus, bool AsPerc, bool ForRounding,
									double MaxCost, double MaxResp)
{
	long i, j, n, frq, nCell, ScaledCost, nDecRespPlus;
	CDataCell *dc;
//	double MaxCost = tab->GetCell(0L)->GetCost(tab->Lambda)
	double RespValue;
//	double MaxResp = tab->GetCell(0L)->GetResp();
	// DIT INS ONZIN VOOR Pos/Neg tabellen!!!
	double LBound, UBound;
	double UPL, LPL, Sliding; // Capacity;


//	double dRoundConst = 0; //= (0.5)/pow(10,nDecResp);
	double dRoundConst = (1.0)/pow(10,nDecResp);
	nDecRespPlus = nDecResp + 3;

	fprintf(fd,"0\n");  // Fixed value
	nCell = tab->GetSizeTable();

	// compute nCell without all bogus cells

	if (WithBogus) {
		for (i = 0, n = 1; i < tab->nDim; i++) {
			n *= var[tab->ExplVarnr[i]].GetnCode() - var[tab->ExplVarnr[i]].GetnBogus();
		}
	}
	else {
		n = nCell;
	}
	fprintf(fd,"%ld\n", n);  // write number of cells

	for (i = j = 0; i < nCell; i++, j++) {
		if (WithBogus) {
			long DimIndices[MAXDIM]; // work, work, work...
			tab->GetIndicesFromCellNr(i, DimIndices);
			int d;
			for (d = 0; d < tab->nDim; d++) {
				if (var[tab->ExplVarnr[d]].IsHierarchical) {
					if (var[tab->ExplVarnr[d]].GethCode()[DimIndices[d]].IsBogus) {
						break;
					}
				}
			}
			if (d != tab->nDim) { // bogus?
				j--;
				continue;
			}
		}
		// no bogus
		dc = tab->GetCell(i);
		RespValue = dc->GetResp();
		fprintf(fd, "%ld ", j);  // index cell, base zero!
		fprintf(fd, "%.*f ", nDecResp, RespValue);

		//write the freqs to a seperate file for the singletons tricks
		frq = dc->GetFreq();
		fprintf(fdFrq, "%ld ", frq);

		frq = dc->GetFreqHolding();;
		fprintf(fdFrq, "%ld\n", frq);

		// compute scaled cost value
		if (tab->CostVarnr < 0) { // special value
			switch (tab->CostVarnr) {
				case CVT_UNITY:  // always one
					ScaledCost = 1;
					break;
				case CVT_DISTANCE:
				case CVT_FREQ:  // frequency
					ScaledCost = (long) (dc->GetCost(tab->Lambda) * tab->MaxScaledCost / MaxCost + .5);
					if (ScaledCost == 0) ScaledCost = 1;
					break;
				default:
					return false;
			}
		}
		else {
			ScaledCost = (long) (dc->GetCost(tab->Lambda) * tab->MaxScaledCost / MaxCost + .5);
			if (ScaledCost == 0) ScaledCost = 1;
		}

		fprintf(fd, "%ld ", ScaledCost);

		UPL = 0; LPL = 0; Sliding = 0; // Capacity = 0;
		// status
		switch (dc->GetStatus() ) {
		case CS_SAFE:
		case CS_SAFE_MANUAL:
		case CS_EMPTY_NONSTRUCTURAL:
			fprintf(fd,"s");
			break;
		case CS_UNSAFE_RULE:
		case CS_UNSAFE_FREQ:
		case CS_UNSAFE_PEEP:
		case CS_UNSAFE_ZERO:
		case CS_UNSAFE_SINGLETON:
		case CS_UNSAFE_MANUAL:
			fprintf(fd,"u");
// 	        dRoundConst = (0.1)/pow(10,nDecRespPlus); //was 0.5
			// en we foezelen alleen voor onveilige cellen.
			LPL = dc->GetLowerProtectionLevel();//+ dRoundConst;
			UPL = dc->GetUpperProtectionLevel();//+ dRoundConst;
		 	if (UPL < dRoundConst) UPL  = dRoundConst;
  			break;
		case CS_PROTECT_MANUAL:
		case CS_EMPTY:
			fprintf(fd,"z");
			break;
		case CS_SECONDARY_UNSAFE:
		case CS_SECONDARY_UNSAFE_MANUAL:
 //	        dRoundConst = (0.1)/pow(10,nDecRespPlus); //was 0.5
			fprintf(fd,"m");
			break;
			default:
			return false;
		}
//		if (ForRounding)	{
//			LBound=  0;
//			/Bound = 2*MaxResp;
//		}
//		else	{
		// Ook bij rounding gewoon de normale weg volgen
		// lower- upperbound
			if (AsPerc)  {
				LBound = RespValue * LowerBound;
				UBound = RespValue * UpperBound;

	       //fprintf(fd, " %.*f %.*f ", nDecResp, RespValue * LowerBound, nDecResp, RespValue * UpperBound);
			}
			else  {
				LBound = LowerBound;
				UBound = UpperBound;
				//fprintf(fd, " %.*f %.*f ", nDecResp,  LowerBound, nDecResp, UpperBound);
			}
//		}


/*	   if (AsPerc)  {
			if (!(dc->GetStatus() == CS_UNSAFE_ZERO))	{
				if (LPL > (RespValue - RespValue * LowerBound) ) LPL = RespValue - RespValue * LowerBound;
				if (UPL > (UpperBound *RespValue - RespValue) ) UPL = UpperBound *  RespValue - RespValue;
			}
		}*/
		//Check the feasibility of the levels
		double TB; //Aux variable for the protection bound
		if (!(dc->GetStatus() == CS_UNSAFE_ZERO))	{
			//First lower levels Niet negativiteit is onzin nu, Anco
			// if (LPL < 0 ) LPL = 0;
			// if (LBound < 0) LBound = 0;
                        TB = RespValue - LPL;
			if ((LBound > TB) || (TB > RespValue) ) {//Something is incorrect TB <= RespVar is always true because LPL >= 0
			 if (LBound > TB)  LBound = TB;
			}

//			if (LPL > (RespValue - LBound) ) LPL = RespValue - LBound - dRoundConst;
//			if (LPL < 0)  LPL = 0;
			if (UPL < 0) UPL = 0;
			TB = RespValue + UPL;
			if ((RespValue > TB ) || (TB > UBound )) {//Something is incorrect TB <= RespVar is always true because LPL >= 0
			 if (TB > UBound ) UBound = TB;
			}
//			if (UPL > (UBound - RespValue) ) UPL = UBound - RespValue - dRoundConst;
//			if (UPL < 0) UPL = 0;

		}
		else	{ // If CS_UNSAFE_ZERO
			if (RespValue + UPL > UBound) UBound = RespValue + UPL;
		}

     /* if (LBound > RespValue - LPL) LBound = RespValue - LPL;
	   if (UBound < RespValue + UPL) UBound = RespValue + UPL; */

//	fprintf(fd, " %.*f %.*f ", nDecRespPlus,  LBound, nDecRespPlus, UBound + 2*dRoundConst);
//	fprintf(fd, "%.*f %.*f %.*f\n", nDecRespPlus, LPL , nDecRespPlus, UPL+dRoundConst, nDecRespPlus, Sliding);
	fprintf(fd, " %.*f %.*f ", nDecResp,  LBound, nDecResp, UBound );
	fprintf(fd, "%.*f %.*f %.*f\n", (int)nDecRespPlus, LPL , (int)nDecRespPlus, UPL, nDecResp, Sliding);
  }

  return true;
}




// Restriction = Cell 1 = Cell 2 + Cell 3 + Cell 4 in complicated form:
// 0.0   4  :  1 (-1) 2 (1) 3 (1) 4 (1)
// so, there are four cells and 0.0 == -1 * Cell[1] + 1 * Cell[2] + 1 * Cell[3] + 1 * Cell[4]
bool CJJFormat::WriteRestrictions(FILE *fd, CTable *tab, CVariable *var, bool WithBogus)
{
	int d, d1, d2, nRestrictions, nDim = tab->nDim;

	TabDimProp tdp[MAXDIM];
	long DimNr[MAXDIM];


	// first compute number of Restrictions
	for (d = 0; d < nDim; d++) {
		CVariable *v = &(var[tab->ExplVarnr[d]]);

		tdp[d].nCode = v->GetnCode();
		if (WithBogus && v->IsHierarchical) tdp[d].nCode -= v->nBogus;
		if (v->IsHierarchical) {
			// count number of parents
			vector<unsigned int> Children;
			tdp[d].nParent = 1; // total always parent
			int nCode = v->GetnCode();
			for (int i = 1; i < nCode; i++) {
				/* int numRange = */ GetRange(*v, i, Children, WithBogus);
				//fprintf(ftemp,"%d  %d\n",i, numRange);
				//	for (int j = 0; j < numRange; j++)  {

				//	fprintf(ftemp, "    %d", Children.GetAt(j));

			//	}
			//	fprintf(ftemp,"\n");

				if (GetRange(*v, i, Children, WithBogus) > 0 ) {
					tdp[d].nParent++;
				}
			}
		}
		else {
			tdp[d].nParent = 1; // only total
		}
	}

	nRestrictions = 0;
	for (d1 = 0; d1 < nDim; d1++) {
		int n = 1;
		for (d2 = 0; d2 < nDim; d2++) {
			if (d2 == d1) {
				n *= tdp[d2].nParent;
			}
			else {
				n *= tdp[d2].nCode;
			}
		}
		nRestrictions += n;
	}

	// Write number of restrictions
	fprintf(fd,"%d\n", nRestrictions);

	// now write restriction ranges
	for (d = 0; d < nDim; d++) {
		//WriteRange(fd, tab, var, d, DimNr, 0, WithBogus, tdp);

		WriteRange(fd, tab, var, d, DimNr, 0, WithBogus, tdp);
	}
	return true;

}

// right hand side of the equation. Normally that parent = sum of children
void CJJFormat::WriteRange(FILE *fd, CTable *tab, CVariable *var,
									int TargetDim, long *DimNr, int niv,
									bool WithBogus, TabDimProp *tdp)
{
	vector<unsigned int> Children;
	if (niv == tab->nDim) {
		CVariable *v = &(var[tab->ExplVarnr[TargetDim]]);
		int nCode = v->GetnCode();
		if (v->IsHierarchical) {
			int i, j, k, r;
			for (i = 0, k = 0; i < nCode; i++) {
				int n = GetRange(*v, i, Children, WithBogus);
				// fprintf(ftemp,"  %s  %d","no Children", n);
				if (n > 0) {
				// count number of bogus codes before code i
					int nBogusBefore = 0;
					if (WithBogus) {
						for (r = i - 1; r >= 0; r--) {
							if (v->GethCode()[r].IsBogus) {
								nBogusBefore++;
							}
						}
					}
					DimNr[TargetDim] = k;
    				fprintf(fd, "0 %d : %ld (-1) ", n + 1, GetCellNrFromIndices(tab->nDim, DimNr, tdp) );
					for (j = 0; j < n; j++) {
						long RealCode = Children[j];
						if (WithBogus) {
							for (r = RealCode - 1; r >= i; r--) {
								if (v->GethCode()[r].IsBogus) {
									RealCode--;
								}
							}
							RealCode -= nBogusBefore;  // number of bogus codes before code i
						}
  						DimNr[TargetDim] = RealCode;

    					fprintf(fd, "%ld (1) ", GetCellNrFromIndices(tab->nDim, DimNr, tdp));
					}


	 				fprintf(fd,"\n");
				}
				if (!WithBogus || !v->GethCode()[i].IsBogus) k++;
			}
		}
		else {  // not hierarchical
			DimNr[TargetDim] = 0;

			fprintf(fd, "0 %d : %ld (-1) ", nCode, GetCellNrFromIndices(tab->nDim, DimNr, tdp) );
//			fprintf(fd, "%.*f %d : %d (-1) ", 1,0, nCode, GetCellNrFromIndices(tab->nDim, DimNr, tdp) );
			for (int i = 1; i < nCode; i++) {
				DimNr[TargetDim] = i;
				fprintf(fd, "%ld (1) ", GetCellNrFromIndices(tab->nDim, DimNr, tdp) );
			}
			fprintf(fd,"\n");
		}

	}
	else {
		if (niv != TargetDim) {
			int i, j;
			CVariable *v = &(var[tab->ExplVarnr[niv]]);
			int nCode = v->GetnCode();
  			for (i = 0, j = 0; i < nCode; i++) {
				if (!v->IsHierarchical || !WithBogus || !v->GethCode()[i].IsBogus) {
					DimNr[niv] = j++;
					//WriteRange(fd, tab, var, TargetDim, DimNr, niv + 1, WithBogus, tdp);
					WriteRange(fd, tab, var, TargetDim, DimNr, niv + 1, WithBogus, tdp);
				}
			}
		}
		else {
			// DimNr[niv (= TargetDim)] will be filled above
			//WriteRange(fd, tab, var, TargetDim, DimNr, niv + 1, WithBogus, tdp);
			WriteRange(fd, tab, var, TargetDim, DimNr, niv + 1, WithBogus, tdp);
		}
	}
}


// if WithBogus: No-Bogus-Parents with all descendants bogus: no Parent any more
int CJJFormat::GetRange(CVariable &var, int CodeIndex, vector<unsigned int> &Children,
								bool WithBogus)
{
	int i, n = 0, LevelParent, LevelDesc=0, nCode;
	CCode *hCode = var.GethCode();
	ASSERT(var.IsHierarchical);

	Children.clear();
	nCode = var.GetnCode();
	if (WithBogus) {
		if (!hCode[CodeIndex].IsBogus) {
			if (hCode[CodeIndex].IsParent) {
  				LevelParent = hCode[CodeIndex].Level;
				//
				for (i = CodeIndex + 1;i<nCode ; i++ ) {
  					LevelDesc = hCode[i].Level;
					if (LevelDesc <= LevelParent) break; // end descendants
					if (!hCode[i].IsBogus) break;
				}
				if (LevelDesc > LevelParent) {
					// not all bogus, so compute children
					// LevelDesc = new level
					for (i = CodeIndex + 1; ; i++) {
						if (hCode[i].Level <= LevelParent) break; // end descendants
						if (!hCode[i].IsBogus && hCode[i].Level == LevelDesc) {
							Children.push_back(i);
							n++;
						}
					}

				}
			}
		}
	}
	else {
		// here I better do something
		if (hCode[CodeIndex].IsParent) {
			LevelParent = hCode[CodeIndex].Level;
			for (i = CodeIndex + 1; i<nCode; i++) {
				// if i > n then break;
				LevelDesc = hCode[i].Level;
				if (LevelDesc <= LevelParent) break;
				if (LevelDesc == LevelParent + 1) { // that's a child
					Children.push_back(i);
					n++;
				}
			}

		}
	}

	return n;
}

// To get Cell Nr. In principle this is also there in CTable.
//I don't know why that is not used
long CJJFormat::GetCellNrFromIndices(int nDim, long *DimNr, TabDimProp *tdp)
{
	int i, c = 0;

	for (i = 0; i < nDim; i++) {
    //ASSERT(DimNr[i] >= 0 && DimNr[i] < tdp[i].nCode);
		c *= tdp[i].nCode;
		c += DimNr[i];
	}

	return c;

}

// Secondary Unsafes returned by JJ set as Unsafe cells in the table
bool CJJFormat::SetSecondaryUnsafe(const char *FileName, CTable *tab, CVariable *var, long *nSetSecondary, long *ErrorCode, bool WithBogus)
{
	FILE *fd;
	*ErrorCode = 0;

	fd = fopen(FileName, "r");
	if (fd == 0) {
		*ErrorCode = JFF_NOFILE;
		return false;
	}


	nSetAtSec = 0;
	bool result = SetSecUnSafe(tab, var, fd, WithBogus);
	if (!result) *ErrorCode = JJF_SOURCECELLINCORRECT;
	*nSetSecondary = nSetAtSec;

	fclose(fd);

	return result;
}

bool CJJFormat::SetSecUnSafe(CTable *tab,  CVariable *var, FILE *fd, bool WithBogus)
{
	int i, n = 2;
	char str[1000];
	CDataCell *dc;
	// lees eerst n lege regels als nodig
	for (i = 0; i < n; i++) {
	  fgets(str, 1000, fd);
	}

	while (!feof(fd) ) {
		str[0] = 0;
		fgets(str, 1000, fd);
		if (str[0] == 0) break;
		if (strchr(str, 'm') != 0) {
			// first integer: cellnr
			long CellNr = atol(str);
			ASSERT(CellNr >= 0 && CellNr < tab->nCell);
			if (CellNr < 0 || CellNr >= tab->nCell) {
				return false;
			}
     		// correct for Bogus
			if (WithBogus) {
				CellNr = SetCellNrBogus(tab, var, CellNr);
			}

			dc = tab->GetCell(CellNr);

			switch (dc->GetStatus() ) {
				case CS_SAFE:
					dc->SetStatus(CS_SECONDARY_UNSAFE);
					break;
				case CS_SAFE_MANUAL:
					dc->SetStatus(CS_SECONDARY_UNSAFE_MANUAL);
					break;
				default:
					return false;
			}
			nSetAtSec++;
			// tab->SetCell(CellNr, *dc);
		}
	}

	return true;


}

// only in case of WithBogus
// recalculate CellNr
long CJJFormat::SetCellNrBogus(CTable *tab, CVariable *var, long CellNr)
{
	int i, d, n, t;
	CVariable *v;
	long DimNr[MAXDIM];

	// compute index of each dimension
	for (d = tab->nDim - 1; d >= 0; d--) {
		// pointer to dimension variable
		v = &(var[tab->ExplVarnr[d]]);
		// size of dimension without bogus
		n = v->GetnCode() - v->GetnBogus();
		ASSERT(n > 0);
		DimNr[d] = CellNr % n;
		CellNr -= DimNr[d];
		CellNr /= n;
	}

	ASSERT(CellNr == 0);

	// now recompute position in dimension without bogus
	for (d = tab->nDim - 1; d >= 0; d--) {
		v = &(var[tab->ExplVarnr[d]]);
		if (v->GetnBogus() == 0) continue; // no bogus
		n = v->GetnCode();
		for (i = t = 0; i < n; i++) {
			if (t == DimNr[d]) break;
			if (!v->GethCode()[i].IsBogus) t++;
		}
		ASSERT(i < n);
		DimNr[d] = i;
	}

	return tab->GetCellNrFromIndices(DimNr);
}


