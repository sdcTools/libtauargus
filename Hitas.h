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

