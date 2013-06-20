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

