// Hitas.cpp: implementation of the CHitas class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Hitas.h"
#include "math.h"

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
	NameTabFile = "hitastab.txt";
	NameSecFile = "hitassec.txt";
}

CHitas::~CHitas()
{

}


/*
[Frequency rule]
MINCOUNT=3    ; Cell should contain at least (MINCOUNT + 1) contributants  // SafeMinRec minus 1 in avdw's geval

[Dominance rule] //  !!! is vervallen !!!!
DOMCOUNT=1    ; Largest DOMCOUNT should contribute less than
DOMPRO=0.7    ; DOMPRO times cellvalue

[Safety ranges]
LOWERMARG=0.99    ; lowerbound in case of marginal cell // zo laten
UPPERMARG=1.01    ; same for upperbound                 // zo laten

[Costs]
DISTANCE=0    ; 0 if cost specified in BTab.dat, 1 if distance is to be used // 0 geen afstandsfunctie, 1 Anco
D1=1 3 5 9 17   ; costs var1 distance=1, 2, 3, 4, 5 (=max cost)
D2=1 3 5 9 17   ; costs var2 distance=1, 2, 3, 4, 5 (=max cost)
D3=1 3 5 17 17    ; costs var3 distance=1, 2, 3, 4, 5 (=max cost)

[Misc]
MINTABVAL=0     ; Each cell is non-negative      // ondergrens 
MAXTABVAL=150000000   ; and less than MAXTABVAL  // totaal generaal van Resp (Shadow? Anco)
OUTDIR=c:/knb/hitas/test/  ; Will contain ouput-files (= TEMP-dir?) // werkdirectory
*/


bool CHitas::WriteParameterFile(FILE *fd, CTable& tab)
{
	fprintf(fd, "[Frequency rule]\n");
	fprintf(fd, "MINCOUNT=%d\n\n", tab.SafeMinRec);

	/*
	fprintf(fd, "[Dominance rule]\n");
	fprintf(fd, "DOMCOUNT=%d\n", tab.DominanceNumber);  
	fprintf(fd, "DOMPRO=0.%d\n\n", tab.DominancePerc);   
  */
	
	fprintf(fd, "[Safety ranges]\n");
	fprintf(fd, "LOWERMARG=0.99\n");
	fprintf(fd, "UPPERMARG=1.01\n\n");

	fprintf(fd, "[Misc]\n");
	fprintf(fd, "MINTABVAL=0\n");
	fprintf(fd, "MAXTABVAL=%.0f\n\n", tab.GetCell(0L)->GetResp() * 1.5); // should be enough
//	MakeTempPath(); Wordt nu door TAU doorgegeven.
//	fprintf(fd, "OUTDIR=%s\n\n", (LPCTSTR) TempPath);

	fprintf(fd, "[Costs]\n");
	fprintf(fd, "DISTANCE=0\n");

	return true;
}

bool CHitas::MakeTempPath()
{	
	char path[_MAX_PATH];
	
	if (!GetTempPath(_MAX_PATH, path)) {
		return false;
	}

	TempPath = path;
	return true;
}

/*
3                                            // aantal dimensies tabel
c:\knb\hitas\test3\data\SBI3.txt             // naam bestand met opspanvariabele 1
c:\knb\hitas\test3\data\GK3.txt              // naam bestand met opspanvariabele 2
c:\knb\hitas\test3\data\Regio3.txt           // naam bestand met opspanvariabele 3
c:\knb\hitas\test3\data\BTab.dat             // naam bestand met basistabel
c:\knb\hitas\test\status3.dat                // naam bestand met resultaat (alleen secundaire cellen)
*/

bool CHitas::WriteFilesFile(FILE *fd, CTable &tab, CVariable *var)
{ 
	int i;
//	std::string fname/*, varname*/;
	char fname[MAX_PATH];
	char varname[10];
	fprintf(fd, "%d\n", tab.nDim); // dimensies

	// name files with codelists
	for (i = 0; i < tab.nDim; i++) {
		sprintf(fname, "%shitasv%d.txt", TempPath.c_str(), i + 1);
		sprintf(varname, "Var_%d", i + 1);
		fprintf(fd, "%s\n", fname);
		var[tab.ExplVarnr[i]].WriteCodelist(fname, ".", varname, true);
	}

	// name file with table
	sprintf(fname, "%s%s", TempPath.c_str(), NameTabFile.c_str());
	fprintf(fd, "%s\n", fname);
	// write cells
	WriteCellFile(fname, tab, var);

	// name file for result with secondary cell dimensions
	sprintf(fname, "%s%s", TempPath.c_str(), NameSecFile.c_str());
	fprintf(fd, "%s\n", fname);

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
			fprintf(fd, "%5d ", dimsCodeList[i]);
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
			double UPL, LPL, Sliding, Capacity;
			LPL =0; UPL =0; Sliding =0; Capacity = 0;
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

