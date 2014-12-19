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

#if !defined SubCodeList_h
#define SubCodeList_h

#include <string>
#include <vector>

class CSubCodeList
{
private:
	long m_lSequenceNumber;
	std::string m_sParentCode;
	long m_lParentCodeIndex;
	long m_lNumberOfSubCodes;
	std::vector<std::string> m_colSubCodeCollection;
	long * m_lSubCodeIndex;

public:
	CSubCodeList()
	{
		m_lSequenceNumber = 0;
		m_sParentCode = "";
		m_lParentCodeIndex = 0;
		m_lNumberOfSubCodes = 0;
		m_lSubCodeIndex = 0;
		m_colSubCodeCollection.resize(0) ;
	}
	~CSubCodeList()
	{
		if (m_colSubCodeCollection.size() != 0)	{
			m_colSubCodeCollection.resize(0);
		}
		if (m_lSubCodeIndex != 0)	{
			delete[] m_lSubCodeIndex;
		}
	}
	std::string GetSubCode(long icodenum);
	void FillSubCodes(std::vector<std::string> &codes, long *indices);
	long IsInSubCodes(const std::string &sCode);
	void SetSequenceNumber(long num);
	void SetParentCode(const std::string &code);
	void SetParentIndex(long Index);

	long GetSubCodeIndex(long i);
	long GetParentIndex();
	long GetSequenceNumber();
	std::string GetParentCode();
	long NumberOfSubCodes();
};

#endif // SubCodeList_h

