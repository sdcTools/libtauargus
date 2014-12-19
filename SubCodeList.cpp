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

#include <vector>

#include "SubCodeList.h"

using namespace std;

string CSubCodeList::GetSubCode(long isubcodenum)
{
	if (isubcodenum < 0)	{
		return m_colSubCodeCollection[0];
	}
	if (isubcodenum > m_colSubCodeCollection.size() - 1) {
		return m_colSubCodeCollection[m_colSubCodeCollection.size() - 1];
	}
	else
	{
		return m_colSubCodeCollection[isubcodenum];
	}
}


void CSubCodeList::FillSubCodes(vector<string> &codes, long *indices)
{
	long i;
	m_colSubCodeCollection.resize(codes.size());
	m_lSubCodeIndex = new long[codes.size()];

	for (i= 0; i< codes.size(); i++)
	{
		m_colSubCodeCollection[i] = codes[i];
		m_lSubCodeIndex[i] = indices[i];
	}
	m_lNumberOfSubCodes = codes.size();

}

long CSubCodeList::IsInSubCodes(const string &sCode)
{
	for (long i = 0; i < m_colSubCodeCollection.size(); i++)	{
		if (sCode == m_colSubCodeCollection[i])	{
			return i;
		}
	}
	return -1;
}


void CSubCodeList::SetParentCode(const string &code)
{
	m_sParentCode = code;
}

string CSubCodeList::GetParentCode()
{
	return m_sParentCode;
}

void CSubCodeList::SetSequenceNumber(long num)
{
	m_lSequenceNumber = num;
}

long CSubCodeList::GetSequenceNumber()
{
	return m_lSequenceNumber;
}

long CSubCodeList::NumberOfSubCodes()
{
	return m_lNumberOfSubCodes;
}

long CSubCodeList::GetParentIndex()
{
	return m_lParentCodeIndex; 
}

void CSubCodeList::SetParentIndex(long Index)
{
	m_lParentCodeIndex = Index; 
}

long  CSubCodeList::GetSubCodeIndex(long i)
{
	return m_lSubCodeIndex[i];
}
