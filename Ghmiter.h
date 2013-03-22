// Ghmiter.h: interface for the CGhmiter class.
//
//////////////////////////////////////////////////////////////////////

/*#if !defined(AFX_GHMITER_H__A93D13A1_ADCA_11D5_BC97_00C04F9A7DB5__INCLUDED_)
#define AFX_GHMITER_H__A93D13A1_ADCA_11D5_BC97_00C04F9A7DB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000*/
#if !defined Ghmiter_h
#define Ghmiter_h

#include "Variable.h"
#include "Table.h"

class CGhmiter  
{
public:
	int nSetAtSec;
  BOOL SetSecUnSafe(CTable *tab, FILE *fd, long *Dims, int niv, bool IsSingleton);
  BOOL SetSecondaryUnsafe(const char* FileName, CTable *tab, long *nSetSecondary, int *ErrorCode, bool IsSingleton);
	// BOOL SetProgramPath(LPCTSTR ProgramPath);
	// BOOL MakeTempPath();
	// BOOL ExecuteProgram();
	// void DeleteSourceFiles();
	int  ComputeHierarchicalLevels(int VarNr, int *nGroup, CVariable *m_var);
  void WriteCell(FILE *fd, CTable *tab, CVariable *m_var, int FreqWidth, int RespWidth, int nDecResp, double MaxResp, long *Dim, int niv, bool IsSingleton);
  BOOL CellsTable(const char *FileName, CTable *tab, CVariable *m_var, bool IsSingleton);
  BOOL ControlDataTable(const char *FileName, const char *TableName, const char *EndStr1, const char *EndStr2, int nDim, int *ExpVarNr, CVariable *m_var);
	// BOOL ControlDataTables(long MemorySize, double MinValue, double MaxValue, int MaxnDim, int nTables);
	// void DeleteResultFiles();
	// CString ProgramPath;
	// CString TempPath;
	// CString CurrentPath;
	CGhmiter();
	virtual ~CGhmiter();




};
#endif

//#endif // !defined(AFX_GHMITER_H__A93D13A1_ADCA_11D5_BC97_00C04F9A7DB5__INCLUDED_)
