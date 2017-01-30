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
#include <string>
#include <cmath>
#include <vector>

#include "General.h"
#include "Ghmiter.h"

//using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGhmiter::CGhmiter()
{ // char curdir[_MAX_PATH];

  // ProgramPath.Empty();
	// TempPath.Empty();
  // GetCurrentDirectory(_MAX_PATH, curdir);
	// CurrentPath = curdir;
}

CGhmiter::~CGhmiter()
{
  //if (!ProgramPath.IsEmpty() ) {
  //	DeleteResultFiles();
	//	DeleteSourceFiles();
	//}

  // SetCurrentDirectory((LPCTSTR) CurrentPath);
}

//////////////////////////////////////////////////////////////////////
// User functions
//////////////////////////////////////////////////////////////////////

/*
// delete previous result files
void CGhmiter::DeleteResultFiles()
{ string s;

  s = TempPath + "ausgabe";
	remove((LPCTSTR) s);
  s = TempPath + "proto001";
	remove((LPCTSTR) s);
  s = TempPath + "proto002";
	remove((LPCTSTR) s);
  s = TempPath + "proto003";
	remove((LPCTSTR) s);
}
*/

/*
void CGhmiter::DeleteSourceFiles()
{ string s;

  s = TempPath + "eingabe";
	remove((LPCTSTR) s);
  s = TempPath + "tabelle";
	remove((LPCTSTR) s);
  s = TempPath + "steuer";
	remove((LPCTSTR) s);
}
*/

/*
// Path: Path in which control files (tabelle, steuer, eingabe) are stored
// MemorySize: 4 - 64, in MB
// MinValue 1, MaxValue biggest value of all tables
// 0.0  value 0 safe
// 0    normal processing positive tables
// MaxnDim max number of dimensions of all tables
// nTables number of tables (usually 1)bool ControlDataTables(
// tabelle
bool CGhmiter::ControlDataTables(long MemorySizeMB, double MinValue, double MaxValue,
	int MaxnDim, int nTables)
{ FILE *fd;
  CString FileName;

	// DeleteResultFiles();

	FileName += TempPath + "tabelle";
  fd = fopen((LPCTSTR) FileName, "w");
	if (fd == 0) return false;

  // memory
	if (MemorySizeMB < 4 || MemorySizeMB > 64) goto error;
  fprintf(fd, "%d000000\n", MemorySizeMB);

	// min max value
	if (MaxValue < MinValue) goto error;
  fprintf(fd, "%f %f\n", MinValue, MaxValue + 10);

	fprintf(fd, "0.00\n");
	fprintf(fd, "0\n");
	fprintf(fd, "%d\n", MaxnDim);
	fprintf(fd, "%d\n", nTables);

	fclose(fd);
	return true;

error:
	fclose(fd);
	return false;
}
*/

// steuer
bool CGhmiter::ControlDataTable(const char *FileName, const char *TableName,
										  const char *EndStr1, const char *EndStr2,
										  int nDim, int *ExpVarNr, CVariable *m_var)
{
	FILE *fd;
	int i, nGroup[MAXLEVEL + 1];
	//char code[10];
        //char code[64];
        std::string code;

	fd = fopen(FileName, "w");
	if (fd == 0) return false;

	fprintf(fd, "'%s'\n", TableName);
	fprintf(fd, "'(5(A8,1X)) '\n");
	fprintf(fd, "%d\n", nDim);

	for (i = 0; i < nDim; i++) {
    //vector<string> *Codes = m_var[ExpVarNr[i]].GetCodeList();
	/*	if (m_var[ExpVarNr[i]].GetCodeWidth() > 8) {
			fclose(fd);
			return false;
		}Niet meer nodig omdat we nu nummertjes ipv de echte codes gebruiken*/
		fprintf(fd, "%d ", ComputeHierarchicalLevels(ExpVarNr[i], nGroup, m_var) );
	}
	fprintf(fd, "\n");

	// "mit Randsummenschranke"
 	for (i = 0; i < nDim; i++) {
		fprintf(fd, "1 ");
	}
	fprintf(fd, "\n");

	// "Abstandmass"
 	for (i = 0; i < nDim; i++) {
		fprintf(fd, "0.0 ");
	}
	fprintf(fd, "\n");

	for (i = 0; i < nDim; i++) {
		std::vector<std::string> *Codes = m_var[ExpVarNr[i]].GetCodeList();
		int j, k, s;
		fprintf(fd, "'Var %d'\n", i + 1);

		// number per code group
		ComputeHierarchicalLevels(ExpVarNr[i], nGroup, m_var);
		for (j = MAXLEVEL - 1; j >= 0; j--) {
			if (nGroup[j] != 0) {
    			fprintf(fd, "%d ", nGroup[j]);
			}
		}
		fprintf(fd, "\n");

		// now each code per code group
		k = -1;
		for (s = MAXLEVEL; s >= 0; s--) {
			if (nGroup[s] == 0) continue;
			for (j = Codes->size() - 1; j >= 0; j--) { // sorted decreasing
				if (m_var[ExpVarNr[i]].IsHierarchical) {
					if (m_var[ExpVarNr[i]].GethCode()[j].Level != s) continue;
				}
				else {
// waarom zeuren over de lengte bij non hier codes?
// altijd goed Anco 19 5 2003
//				  if (Codes->GetAt(j).GetLength() != s) continue;
					if (s != 0)	continue;
				}
				if (++k == 5) {
					fprintf(fd,"\n");
					k = 0;
				}
				m_var[ExpVarNr[i]].GetGHMITERCode(j, code);
//				fprintf(fd, "%s ",  code );
//              Anco Aanpassing voor linked tables 17 04 2003
				fprintf(fd, "%08d ", j );
			}
		}
		fprintf(fd,"\n");
	}

	fprintf(fd, "%s\n", EndStr1); // 0.00 0.00 or something like that
	fprintf(fd, "%s\n", EndStr2); // 1 1 1 or something like that

	fclose(fd);
	return true;
}

// eingabe
bool CGhmiter::CellsTable(const char *FileName, CTable *tab, CVariable *m_var, bool IsSingleton)
{
	FILE *fd;
	int n, m;
	long Dim[MAXDIM];
	char str[256];


	fd = fopen(FileName, "w");
	if (fd == 0) return false;

	// compute fixed data
	n = sprintf(str, "%ld", tab->GetCell(0L)->GetFreq());
	m = sprintf(str, "%.*f", (int)m_var[tab->ResponseVarnr].nDec, tab->GetCell(0L)->GetResp());

	// write for each cell a record in "eingabe"
	WriteCell(fd, tab, m_var, n, m, m_var[tab->ResponseVarnr].nDec,
	tab->GetCell(0L)->GetResp(), Dim, tab->nDim - 1, IsSingleton);

	fclose(fd);
	return true;

}

void CGhmiter::WriteCell(FILE *fd, CTable *tab, CVariable *m_var, int FreqWidth, int RespWidth, int nDecResp, double MaxResp, long *Dim, int niv, bool IsSingleton)
{
	int i, nCode, WertArt;
	double MinResp;
	long Freq;

	long tempFreq;


	if (niv < 0) {
		CDataCell *dc;
		dc = tab->GetCell(Dim);
		switch (dc->GetStatus() ) {
		case CS_UNSAFE_RULE:
		case CS_UNSAFE_FREQ:
		case CS_UNSAFE_PEEP:
		case CS_UNSAFE_SINGLETON:
		case CS_UNSAFE_ZERO:
		case CS_UNSAFE_MANUAL:
			WertArt = 129;
			break;
		default:
			WertArt = 1;
	}

   // protected: 6e + 7e = Resp may be, AHNL aangepast 30 jan 2002
	MinResp = dc->GetResp() * tab->PQ_QCell_1 / 100.0;
	MinResp = fabs ( MinResp);
	MaxResp = MinResp;
   if (dc->GetStatus() == CS_PROTECT_MANUAL) {
			MaxResp = 0.0;
			MinResp = 0.0;
	}

	//Als Safe (WERTART = 1) dan de freq minimaal op 2 zetten AHNL
   //Holdingfreq toegevoegd 23/8/2012
	Freq = dc->GetFreq();
	if (tab->ApplyHolding) Freq = dc->GetFreqHolding();


	if (Freq == 1 && WertArt == 1) {
		tempFreq = 2;
	}
	else {
		tempFreq = Freq;
	}
	if (!IsSingleton) {
		fprintf(fd, "%5d %*ld %*.*f 1   1  %.*f %*.*f ",
				WertArt,
				FreqWidth, tempFreq,
				RespWidth, nDecResp, dc->GetResp(),
				nDecResp, MaxResp,
				RespWidth, nDecResp, MinResp);
	}
	else { // Is Singleton
		if (WertArt == 1) {
			fprintf(fd, "%5d %*ld %*.*f 1   1   1  %.*f %*.*f ",
				WertArt,
				FreqWidth, tempFreq,
				RespWidth, nDecResp, dc->GetResp(),
				nDecResp, MaxResp,
				RespWidth, nDecResp, MinResp
			);
		}
		else {
			if (Freq == 1) {
				 fprintf(fd, "%5d %*ld %*.*f 1   1   1  %.*f %*.*f ",
								WertArt,
								FreqWidth, tempFreq,
								RespWidth, nDecResp, dc->GetResp(),
								nDecResp, MaxResp,
								RespWidth, nDecResp, MinResp);
			}
			else {
				WertArt = 1;
				fprintf(fd, "%5d %*ld %*.*f 1   9   1  %.*f %*.*f ",
				WertArt,
				FreqWidth, tempFreq,
				RespWidth, nDecResp, dc->GetResp(),
				nDecResp, MaxResp,
				RespWidth, nDecResp, MinResp);

			}
		}

	}
	// Anco toegevoegd op verzoek van Sarah 17 4 2003
   for (i = 0; i < tab->nDim; i++) {
		fprintf(fd, "'%08ld' ", Dim[i]);
	}

   for (i = 0; i < tab->nDim; i++) {
		//char code[10];
                //char code[64];
                std::string code;
		m_var[tab->ExplVarnr[i]].GetGHMITERCode((int) Dim[i], code);
		fprintf(fd, "'%s' ", code.c_str());
	}
   fprintf(fd, "\n");
	return;
}

	nCode = tab->SizeDim[niv];
	for (i = nCode - 1; i >= 0; i--) {
		Dim[niv] = i;
		WriteCell(fd, tab, m_var, FreqWidth, RespWidth, nDecResp, MaxResp, Dim, niv - 1, IsSingleton);
	}

}

// computes number of codes per hierarchical level (in nGroup)
// returns number of hierarchical levels
// total assumed to be a hierarchical level, so every variable is hierarchical
// GHMITER has a max of 200 for hierarchical subcodes??
int CGhmiter::ComputeHierarchicalLevels(int VarNr, int *nGroup, CVariable *m_var)
{
	int i, n = m_var[VarNr].GetnCode();
	int GroupExists[MAXLEVEL + 1];

	memset(GroupExists, 0, sizeof(GroupExists) );
	for (i = 0; i < n; i++) {
		if (m_var[VarNr].IsHierarchical) {
			GroupExists[m_var[VarNr].GethCode()[i].Level]++;
		}
		else {
		// Waarom Aad????? Als niet hierarchisch dan automatisch alles op een niveau behalve het totaal
		//        GroupExists[m_var[VarNr].GetCodeList()->GetAt(i).GetLength()]++;
		}
	}

	if (!m_var[VarNr].IsHierarchical) {
		GroupExists[1] = n-1;
		GroupExists[0] = 1;
	}

	// count and return number of groups
	for (i = n = 0; i <= MAXLEVEL; i++) {
		if (GroupExists[i] != 0) n++;
		nGroup[i] = GroupExists[i];
	}
	return n;
}

/*
bool CGhmiter::ExecuteProgram()
{ int result;

  result = _spawnl(_P_WAIT, (LPCTSTR) ProgramPath, (LPCTSTR) ProgramPath, NULL);
	if (result == -1)	{
		return false;
	}

  return true;
}
*/

/*
bool CGhmiter::MakeTempPath()
{	char path[_MAX_PATH];

	if (!GetTempPath(_MAX_PATH, path)) {
		return false;
	}

	TempPath = path;
	return true;
}
*/

/*
bool CGhmiter::SetProgramPath(LPCTSTR mProgramPath)
{
	// Check for existence
  if (_access(mProgramPath, 0) == -1 )	return false;
	ProgramPath = mProgramPath;
  return true;
}
*/

// secondary unsafe cells that are calculated by GHmiter are set as unsafe.
bool CGhmiter::SetSecondaryUnsafe(const char *FileName, CTable *tab, long *nSetSecondary, int *ErrorCode, bool IsSingleton)
{
	FILE *fd;
	*ErrorCode = 0;

	fd = fopen(FileName, "r");
	if (fd == 0) {
		*ErrorCode = GHM_NOFILEAUSGABE;
		return false;
	}

	long Dims[MAXDIM];
	nSetAtSec = 0;
	bool result = SetSecUnSafe(tab, fd, Dims, tab->nDim - 1,IsSingleton);
	if (!result) *ErrorCode = GHM_SOURCECELLINCORRECT;

	*nSetSecondary = nSetAtSec;
	fclose(fd);

	return result;
}

//Secondary unsafe set as unsafe in the table cells.

bool CGhmiter::SetSecUnSafe(CTable *tab, FILE *fd, long *Dims, int niv, bool IsSingleton)
{
	int i, n;

	if (niv < 0) {
		CDataCell *dc;
		char str[2048];

		dc = tab->GetCell(Dims);
		fgets(str, 2048, fd);
		if (atoi(str) == 1129)
		{ // first number 1129 = secondary set
			int Status = dc->GetStatus();
			switch (Status) {
				case CS_SAFE:
					if (IsSingleton) {
						dc->SetStatus(CS_UNSAFE_SINGLETON);
					}
					else {
						dc->SetStatus(CS_SECONDARY_UNSAFE);
					}
					break;
				case CS_SAFE_MANUAL:
					if (IsSingleton) {// Anco says CS_UNSAFE_Singleton_Manual
						dc->SetStatus(CS_UNSAFE_SINGLETON_MANUAL);
					}
					else {
						dc->SetStatus(CS_SECONDARY_UNSAFE_MANUAL);
					}
					break;
				default:
					if (!IsSingleton) {
						return false;
					}
			}
			nSetAtSec++;
			// tab->SetCell(Dims, *dc);
		}

		return true;
	}

	n = tab->SizeDim[niv];
	for (i = n - 1; i >= 0; i--) {
		Dims[niv] = i;
		if (!SetSecUnSafe(tab, fd, Dims, niv - 1, IsSingleton) ) {
			return false;
		}
	}

	return true;
}
