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

long CSubCodeList::IsInSubCodes(string sCode)
{
	for (long i = 0; i < m_colSubCodeCollection.size(); i++)	{
		if (sCode.compare(m_colSubCodeCollection[i]) == 0)	{
			return i;
		}
	}
	return -1;
}


void CSubCodeList::SetParentCode(string code)
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