// Hitas.h: interface for the CHitas class.
//
//////////////////////////////////////////////////////////////////////

/*#if !defined(AFX_HITAS_H__D3B12B56_C9F5_11D5_BCB5_00C04F9A7DB5__INCLUDED_)
#define AFX_HITAS_H__D3B12B56_C9F5_11D5_BCB5_00C04F9A7DB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
*/

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
	bool MakeTempPath();
	std::string TempPath;
	CHitas();
	virtual ~CHitas();

};
#endif
//#endif // !defined(AFX_HITAS_H__D3B12B56_C9F5_11D5_BCB5_00C04F9A7DB5__INCLUDED_)
