#include "stdafx.h"
#include "SubCodeList.h"

CString CSubCodeList::GetSubCode(long isubcodenum)
{
	if  (isubcodenum < 0)	{
		return m_colSubCodeCollection.GetAt(0);
	}
	if (m_colSubCodeCollection.GetUpperBound() < isubcodenum)	{
		return m_colSubCodeCollection.GetAt(m_colSubCodeCollection.GetUpperBound());
	}
	else
	{
		return m_colSubCodeCollection.GetAt(isubcodenum);
	}
}


void CSubCodeList::FillSubCodes(CStringArray &codes, long *indices)
{
	long i;
	m_colSubCodeCollection.SetSize(codes.GetSize());
	m_lSubCodeIndex = new long[codes.GetSize()];

	for (i= 0; i< codes.GetSize(); i++)
	{
		m_colSubCodeCollection.SetAt(i,codes.GetAt(i));
		m_lSubCodeIndex[i] = indices[i];
	}
	m_lNumberOfSubCodes = codes.GetSize();

}

long CSubCodeList::IsInSubCodes(CString sCode)
{
	long i;
	bool found = false;
	
	for (i= 0; i< m_colSubCodeCollection.GetSize(); i++)	{
		if (sCode == m_colSubCodeCollection.GetAt(i))	{
			found = true;
			break;
		}
	}
	if (found) {
		return i;
	}
	else {
		return -1;
	}

}


void CSubCodeList::SetParentCode(CString code)
{
	m_sParentCode = code;
}

CString CSubCodeList::GetParentCode()
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