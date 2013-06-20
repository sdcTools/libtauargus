#if !defined Ghmiter_h
#define Ghmiter_h

#include "Variable.h"
#include "Table.h"

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
