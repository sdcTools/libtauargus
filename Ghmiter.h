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

#if !defined Ghmiter_h
#define Ghmiter_h

#include "Variable.h"
#include "Table.h"

//class __declspec(dllexport) CGhmiter
class CGhmiter  
{
public:
	int nSetAtSec;
  bool SetSecUnSafe(CTable *tab, FILE *fd, long *Dims, int niv, bool IsSingleton);
  bool SetSecondaryUnsafe(const char* FileName, CTable *tab, long *nSetSecondary, int *ErrorCode, bool IsSingleton);
	// bool SetProgramPath(LPCTSTR ProgramPath);
	// bool MakeTempPath();
	// bool ExecuteProgram();
	// void DeleteSourceFiles();
	int  ComputeHierarchicalLevels(int VarNr, int *nGroup, CVariable *m_var);
  void WriteCell(FILE *fd, CTable *tab, CVariable *m_var, int FreqWidth, int RespWidth, int nDecResp, double MaxResp, long *Dim, int niv, bool IsSingleton);
  bool CellsTable(const char *FileName, CTable *tab, CVariable *m_var, bool IsSingleton);
  bool ControlDataTable(const char *FileName, const char *TableName, const char *EndStr1, const char *EndStr2, int nDim, int *ExpVarNr, CVariable *m_var);
	// bool ControlDataTables(long MemorySize, double MinValue, double MaxValue, int MaxnDim, int nTables);
	// void DeleteResultFiles();
	// std::string ProgramPath;
	// std::string TempPath;
	// std::string CurrentPath;
	CGhmiter();
	virtual ~CGhmiter();
};

#endif // Ghmiter_h
