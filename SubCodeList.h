#if ! defined SubCodeList_h
#define SubCodeList_h
#include <afx.h>
class CSubCodeList
{
private:
	long m_lSequenceNumber;
	CString m_sParentCode;
	long m_lParentCodeIndex;
	long m_lNumberOfSubCodes;
	CStringArray m_colSubCodeCollection;
	long * m_lSubCodeIndex;

public:
	CSubCodeList()
	{
		m_lSequenceNumber = 0;
		m_sParentCode = "";
		m_lParentCodeIndex = 0;
		m_lNumberOfSubCodes = 0;
		m_lSubCodeIndex = 0;
		m_colSubCodeCollection.SetSize(0) ;
	}
	~CSubCodeList()
	{
		if (m_colSubCodeCollection.GetSize != 0)	{
			m_colSubCodeCollection.SetSize(0);
		}
		if (m_lSubCodeIndex != 0)	{
			delete[] m_lSubCodeIndex;
		}
	}
	CString GetSubCode(long icodenum);
	void FillSubCodes(CStringArray &codes, long *indices);
	long IsInSubCodes(CString sCode);
	void SetSequenceNumber(long num);
	void SetParentCode(CString code);
	void SetParentIndex(long Index);

	long GetSubCodeIndex(long i);
	long GetParentIndex();
	long GetSequenceNumber();
	CString GetParentCode();
	long NumberOfSubCodes();

};
#endif

