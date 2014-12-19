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

#if !defined JJFormat_h
#define JJFormat_h

#include <vector>

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
	bool SetSecUnSafe(CTable *tab, CVariable *var,FILE *fd, bool WithBogus);
  bool SetSecondaryUnsafe(const char *FileName, CTable *tab, CVariable *var, long *nSetSecondary, long *ErrorCode, bool WithBogus);
	long GetCellNrFromIndices(int nDim, long *DimNr, TabDimProp *tdp);
	int  GetRange(CVariable &var, int CodeIndex, std::vector<unsigned int> &Children, bool WithBogus);
  bool WriteCells(FILE *fd, FILE *fdFrq, double LowerBound, double UpperBound, CTable *tab, CVariable *var, int nDecResp, bool WithBogus, bool AsPerc, bool ForRounding, double MaxCost, double MaxResp);
  bool WriteRestrictions(FILE *fd, CTable *tab, CVariable *var, bool WithBogus);
  void WriteRange(FILE *fd, CTable *tab, CVariable *var, int dim, long *DimNr, int niv, bool WithBogus, TabDimProp *tdp);
	CJJFormat();
	virtual ~CJJFormat();
};

#endif // JJFormat_h
