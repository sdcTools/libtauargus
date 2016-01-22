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

#if !defined Variable_h
#define Variable_h

#include <vector>

#include "General.h"
#include "defines.h"
#include "Code.h"
#include "SubCodeList.h"

typedef struct {
	std::vector<std::string> sCode;			// list of codes generated from recode, sorted upwards
	int nCode;              // number of codes, should be equal to sCode.GetSize()
	std::string Missing1;       // first missing value
	std::string Missing2;       // second missing value
	int nMissing;	          // number of missing codes (usually 1 or 2)
	int *DestCode;          // m_var[SrcVar].nCode times an index to sCode
	int CodeWidth;          // max length sCodes
	int nBogus;
	CCode *hCode;           // Properties hierarchical codes
} RECODE;

class CVariable
{
// Construction
public:
	CVariable();

// Attributes
public:
	long bPos;              // starting position in record (first byte 0)
	long nPos;              // number of bytes
	long nDec;              // number of decimals

	bool IsPeeper;					//variable is Pieper
	bool IsCategorical;			// variable is categorical (sex, regio)
	bool IsNumeric;		      // variable is counting item (income, number of persons)
	bool IsWeight;		    	// variable is weight
	bool IsHierarchical;   	// variable is hierarchical
	bool IsHolding;       	// variable is holding indicator

	int nMissing;	          // number of missing codes (usually 1 or 2)
	std::string Missing1;       // first missing value
	std::string Missing2;       // second missing value
	std::string TotalCode;      // code for Total

  // computed during ExploreFile
	double MinValue;        // For numeric variables : minimum in data file
	double MaxValue;        //                         maximum in data file


	std::vector<std::string> sCode;			// For categorical variables: list of codes generated from microfile or from a file
	std::vector<unsigned char> hLevel;      // Levels hierarchical codelist, nCodes long
	std::vector<unsigned char> hLevelBasic; // Basic code hierarchical codelist, nCodes long

	int nCode;              // number of codes, should be equal to sCode.GetSize()

	// only relevant if IsHierarchical
	int nDigitSplit;                // number of digit groups (only if hierarchical)
	int DigitSplit[MAXDIGITGROUP];  // Digit groups
	CCode *hCode;                   // Properties hierarchical codes

	long TableIndex;        // used in FillTables
	int  ValueToggle;       // used in FillTables
	double Value;           // used in FillTables
	bool HasRecode;        // toggle for Recode specification
	bool HasCodeList;       // toggle for Codelist specification form file
	RECODE Recode;          // recode specification, only relevant if HasRecode = true
	std::string PeepCode1;
	std::string PeepCode2;

	bool PositionSet;
	unsigned long NumSubCodes;
	CSubCodeList *m_SubCodes;
// Operations
public:

// Implementation
public:
	int nBogus;
	int  hfCodeWidth; // max width code hierarchical codelist from file
	void UndoRecode();
	CCode* GethCode();
	bool SetHierarchicalRecode();
	void PrintLevelStrings(FILE *fd, int nLevel, LPCTSTR LevelString);
	void PrintLevelCode(FILE *fd, LPCTSTR code, LPCTSTR LevelString);
	void WriteBogusCodelist(FILE *fd, LPCTSTR LevelString, int index, int level, int boguslevel, int ncode, std::vector<std::string> *CodeList);
	bool WriteCodelist(LPCTSTR FileName, LPCTSTR LevelString, LPCTSTR Name, bool bogus);
	bool SetHierarchicalDigits(long nDigitPairs, long *nDigits);
	int  FindHierarchicalCode(LPCTSTR code);
	int  FindAllHierarchicalCode(LPCTSTR code);
	int  SetCodeList(LPCTSTR FileName, LPCTSTR LevelString);
	void SetActive(long CodeIndex, bool active);
	bool SetHierarch();
	void GetGHMITERCode(int i, char* code);
	int  GetCodeWidth();
	bool IsCodeBasic(int i);
	std::vector<std::string> * GetCodeList();
	int  GetnMissing();
	std::string GetCode(int i);
	int GetLevel(int i);
	long GetDepthOfHerarchicalBoom(bool Recoded);
	int  GetnCode();
	int  GetnBogus();
	int  GetnCodeActive();
	int  GetnCodeInActive();
	bool AddCode(const char *newcode, bool tail);
	bool ComputeHierarchicalCodes();
	bool SetMissing(LPCTSTR Missing1, LPCTSTR Missing2, long NumMissing);
	bool SetTotalCode(LPCTSTR sTotalCode);
	bool SetType(bool IsCategorical, bool IsNumeric, bool IsWeight, bool IsHierarchical, bool IsHolding, bool IsPeeper);
	bool SetPeepCodes(const std::string &Peep1, const std::string &Peep2);
	bool SetPosition(long bPos, long nPos, long nDec);
	bool SetDecPosition(long lnDec);
	virtual ~CVariable();
	long OrganizeCodelist();
	long NumberOfSubCodeList();
	bool PrepareSubCodeList();
	bool CreateSubCodeForHierarchicalCode(long CodeIndex, long SubCodeSequenceNumber);
	bool CreateSubCodeForNonHierarchicalCode();
	bool FillSubCodeList();
	long FindNumberOfChildren(long CodeIndex);
	bool FindChildren(long NumChild, std::vector<std::string> &Child, long CodeIndex, long *Index);
        bool NormaliseCode(char *Code);
};

#endif // Variable_h
