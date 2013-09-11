#if !defined Hitas_h
#define Hitas_h
 
#include "Variable.h"
#include "Table.h"
#include "DataCell.h"

class CHitas  
{
public:
	std::string NameSecFile;
	std::string NameTabFile;
  bool WriteCellDim(FILE *fd, CTable &tab, CVariable *var, long *dimsCell, long *dimsCodeList, int niv);
	bool WriteCellFile(LPCTSTR FileName, CTable &tab, CVariable *var);
	bool WriteFilesFile(FILE *fd, CTable& tab, CVariable *var);
	bool WriteParameterFile(FILE *fd, CTable& tab);
	std::string TempPath;
	CHitas();
	virtual ~CHitas();
};

#endif // Hitas_h

