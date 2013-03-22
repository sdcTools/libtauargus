// JJFormat.h: interface for the CJJFormat class.
//
//////////////////////////////////////////////////////////////////////

/*#if !defined(AFX_JJFORMAT_H__E9C08C63_E319_11D5_BCD0_00C04F9A7DB5__INCLUDED_)
#define AFX_JJFORMAT_H__E9C08C63_E319_11D5_BCD0_00C04F9A7DB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000*/
#if !defined JJFormat_h
#define JJFormat_h

#include "Table.h"
#include "DataCell.h"
#include "Variable.h"

struct TabDimProp {
	int nCode;
	int nParent;
};


class CJJFormat
{
public:
	int  nSetAtSec;
  long SetCellNrBogus(CTable *tab, CVariable *var, long CellNr);
	BOOL SetSecUnSafe(CTable *tab, CVariable *var,FILE *fd, BOOL WithBogus);
  BOOL SetSecondaryUnsafe(const char *FileName, CTable *tab, CVariable *var, long *nSetSecondary, long *ErrorCode, BOOL WithBogus);
	long GetCellNrFromIndices(int nDim, long *DimNr, TabDimProp *tdp);
	int  GetRange(CVariable &var, int CodeIndex, CUIntArray &Children, BOOL WithBogus);
  BOOL WriteCells(FILE *fd, FILE *fdFrq, double LowerBound, double UpperBound, CTable *tab, CVariable *var, int nDecResp, BOOL WithBogus, BOOL AsPerc, BOOL ForRounding, double MaxCost, double MaxResp);
  BOOL WriteRestrictions(FILE *fd, CTable *tab, CVariable *var, BOOL WithBogus);
  void WriteRange(FILE *fd, CTable *tab, CVariable *var, int dim, long *DimNr, int niv, BOOL WithBogus, TabDimProp *tdp);
	CJJFormat();
	virtual ~CJJFormat();

};
#endif

//#endif // !defined(AFX_JJFORMAT_H__E9C08C63_E319_11D5_BCD0_00C04F9A7DB5__INCLUDED_)
