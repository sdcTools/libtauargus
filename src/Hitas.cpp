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

#include <cstdio>
#include <cmath>

#include "Hitas.h"

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Write files to be used in Hitas
CHitas::CHitas()
{
	NameTabFile = "Hitastab.txt";
	NameSecFile = "hitassec.txt";
}

CHitas::~CHitas()
{
}

bool CHitas::WriteFilesFile(FILE *fd, CTable &tab, CVariable *var)
{ 
	int i;
        string fname;
        string varname;
	fprintf(fd, "%ld\n", tab.nDim); // number of dimensions

	// name files with codelists
	for (i = 0; i < tab.nDim; i++) {
                fname = TempPath + "hitasv" + std::to_string(i+1) + ".txt";
                varname = "Var_" + std::to_string(i+1);
		fprintf(fd, "%s\n", fname.c_str());
		var[tab.ExplVarnr[i]].WriteCodelist(fname.c_str(), ".", varname.c_str(), true);
	}

	// name file with table
        fname = TempPath + NameTabFile;
	fprintf(fd, "%s\n", fname.c_str());
	// write cells
	WriteCellFile(fname.c_str(), tab, var);

	// name file for result with secondary cell dimensions
        fname = TempPath + NameSecFile;
	fprintf(fd, "%s\n", fname.c_str());

	return true;
}

bool CHitas::WriteCellFile(LPCTSTR FileName, CTable &tab, CVariable *var)
{ 
	long dimsCell[MAXDIM];
	long dimsCodeList[MAXDIM];
	FILE *fd;

	fd = fopen(FileName, "w");
	if (fd == 0) return 0;

	WriteCellDim(fd, tab, var, dimsCell, dimsCodeList, 0);

	fclose(fd);
	return true;
}


bool CHitas::WriteCellDim(FILE *fd, CTable &tab, CVariable *var, 
							long *dimsCell, long *dimsCodeList, int niv)
{ 
	int i, c;
	int nDecResp, F;
	double C;
   
	if (niv == tab.nDim) {
		
		nDecResp = var[tab.ResponseVarnr].nDec;
//		double dRoundConst = (0.5)/pow(10,nDecResp);
		CDataCell *dc = tab.GetCell(dimsCell);
		double cell = dc->GetResp();
		for (i = 0; i < tab.nDim; i++) {
			fprintf(fd, "%5ld ", dimsCodeList[i]);
		}

		fprintf(fd, "%12.*f ", nDecResp, cell);

		double dRoundConst = (1.0)/pow(10,nDecResp);
		switch (dc->GetStatus() ) {
		case CS_SAFE:
		case CS_SAFE_MANUAL:
		case CS_EMPTY_NONSTRUCTURAL:
			fprintf(fd, "s ");
			break;
		case CS_UNSAFE_RULE:
		case CS_UNSAFE_FREQ:
		case CS_UNSAFE_PEEP:
		case CS_UNSAFE_ZERO:
		case CS_UNSAFE_SINGLETON:
		case CS_UNSAFE_SINGLETON_MANUAL:
		case CS_UNSAFE_MANUAL:

			fprintf(fd, "u ");
//			fprintf(fd, "%c", 117);
//			if (dc->GetFreq() <= tab.SafeMinRec) {
//  			fprintf(fd, "1 1 ");  // Anco, niet altijd sporend met de wensch van PP
//			} else {
			double UPL, LPL; // Sliding, Capacity;
			LPL =0; UPL =0; // Sliding =0; Capacity = 0;
  			//fprintf(fd, "%.0f %.0f ", UPL, LPL);
			LPL = dc->GetLowerProtectionLevel();
			UPL = dc->GetUpperProtectionLevel();
            nDecResp = nDecResp + 3;
//			if (LPL < dRoundConst) LPL  = dRoundConst;
//          lijkt me niet nodog. Alleen de bovengrens is voldoende
			if (UPL < dRoundConst) UPL  = dRoundConst;
//			fprintf(fd, "%12.*f ", nDecResp, LPL+dRoundConst);
//			fprintf(fd, "%12.*f ", nDecResp, UPL+dRoundConst);
//			fprintf(fd, "%12.*f ", nDecResp, LPL);
//			fprintf(fd, "%12.*f ", nDecResp, UPL);
//            fprintf(fd, "a  %12.*f %12.*f ",  nDecResp, LPL, nDecResp, UPL);
            fprintf(fd, "%12.*f %12.*f ", nDecResp, LPL, nDecResp, UPL);

//      }  
			break;
		case CS_PROTECT_MANUAL:
		case CS_EMPTY:
			fprintf(fd, "z ");
			break;
		case CS_SECONDARY_UNSAFE:
		case CS_SECONDARY_UNSAFE_MANUAL:
      return false;
      break;
		default:
			ASSERT(false);
		}

		// freq
		F = dc->GetFreq ();
		if (tab.ApplyHolding) F = dc->GetFreqHolding();
  		fprintf(fd, "%d ", F );   // AHNL 6.1.2004 Ik heb F nodig bij de minCost
		                          // MinCost => eps bij scorende nul-cellen 
		// cost
		C = dc->GetCost(tab.Lambda);
		if ( C == 0 && F > 0)  C = 0.00001;
		if (tab.CostVarnr < 0) {
		  //fprintf(fd, "%.0f ", dc->GetCost(tab.Lambda) );
		  //fprintf(fd, "%G ", dc->GetCost(tab.Lambda) );
			fprintf(fd, "%G ", C );
		} 
		else {
  		// int nDecCost = var[tab.CostVarnr].nDec;
		// fprintf(fd, "%.0f ", dc->GetCost(tab.Lambda) );
		//		fprintf(fd, "%G ", dc->GetCost(tab.Lambda) );
			fprintf(fd, "%G ", C );
		}
		fprintf(fd, "\n");
		return true;
	}

	CVariable *v = &(var[tab.ExplVarnr[niv]]);
	CCode *phCode = v->GethCode();	
	
	int k = v->GetnCode();
	for (i = c = 0; i < k; i++, c++) {
		dimsCell[niv] = i;
		if (!v->IsHierarchical || phCode[i].nChildren != 1) {
			dimsCodeList[niv] = c;
			WriteCellDim(fd, tab, var, dimsCell, dimsCodeList, niv + 1);
		} 
		else {
			c--;
		}
	}
	return true;
}
