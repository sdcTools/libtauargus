// Code.h: interface for the CCode class.
//
//////////////////////////////////////////////////////////////////////

/*#if !defined(AFX_CODE_H__E4A6F926_B7E3_11D5_BCA0_00C04F9A7DB5__INCLUDED_)
#define AFX_CODE_H__E4A6F926_B7E3_11D5_BCA0_00C04F9A7DB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
  */

/*
Code      Parent nChildren IsBogus Level

13             Y         1       N     1 
  130          Y         1       Y     2
    1300       N         0       Y     3
*/
#if !defined Code_h
#define Code_h
class CCode  
{
public:
	BOOL IsParent;    // nChildren >= 1
	BOOL IsBogus;     // only child of a parent
	BOOL Active;      // if so, always all descendants from an IsParent-item
	int Level;        // 0 = total, 1, 2 ... 
	int nChildren;    // number of childs 
	CCode();
	virtual ~CCode();
};

#endif
//#endif // !defined(AFX_CODE_H__E4A6F926_B7E3_11D5_BCA0_00C04F9A7DB5__INCLUDED_)
