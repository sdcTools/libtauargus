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
#include <cmath>
#include <limits>
#ifndef __TAUARGUSDEFINESH
#define __TAUARGUSDEFINESH

#define VERSIONNUMBER "1.1.4.9"

#define MAXCODEWIDTH 100
#define MAXRECORDLENGTH 32000
#define SEPARATOR "\r\n"
#define MAXMEMORYUSE 50000000

#define FIREPROGRESS 1000
#define MAXDIM 10
#define MAXDIGITGROUP 10
#define MAXLEVEL 10 //8
#define EPSILON 0.0001 //0.0000001

//inline bool DBL_EQ(double x, double v)
//{ 
//    return fabs(x - v) <= 4*std::numeric_limits<double>::epsilon();
//}

inline bool DBL_EQ(double x, double v)
// Using way of comparing two floats as suggested on http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
// std::fabs(x-v) < std::numeric_limits<double>::epsilon() * std::fabs(x+v) * ulp
//                  || std::fabs(x-v) < std::numeric_limits<double>::min()
// Using ULP = 2 as is usually appropriate for the values we are facing
// Using std::numeric_limits<double>::epsilon() = 1E-14 (should be enough, machine precision 64 bit in 32 bit application could
// give strange results)
{
    int ulp = 2;
    double eps = 1E-14;
    return ((std::fabs(x-v) < eps * std::fabs(x+v) * ulp) || (std::fabs(x-v) < eps));
}

enum CellHoldingSort	{
	WITHOUT_HOLDING = -2,
	WITH_HOLDING = -1
};

enum PeepSort	{
	EMPTY = -1,
	NOPEEP = 0,
	PEEP1,
	PEEP2
};
enum CodeType {
	CTY_BASIC = 1,
	CTY_TOTAL
};

enum CountType {
	CT_RESPONS = 1,
	CT_SHADOW,
	CT_COST,
	CT_ROUNDEDRESP,
	CT_CTA,
        CT_CKM
};

enum SafetyType {
	NONE = 0,
	DOMINANCE,
	PQRULE,
	FREQONLY
	//PEEP
};


enum CostVarType {
	CVT_DISTANCE = -3,
	CVT_UNITY    = -2,
	CVT_FREQ     = -1
};

enum SecondarySuppressInputParam {
	WITHOUT_SINGLETON = 1,
	WITH_SINGLETON,
	BOTH
};
  
// items should be a range, so don't use "= x" (except the first: always 1)
enum CellStatusItems {
	CS_SAFE = 1,                // always first! (see check in function SetTableCellStatus), value irrelevant
	CS_SAFE_MANUAL, 
	CS_UNSAFE_RULE,             // make constants whatever you like // should be first UNSAFE
	CS_UNSAFE_PEEP,
	CS_UNSAFE_FREQ,
	CS_UNSAFE_ZERO,
	CS_UNSAFE_SINGLETON,
	CS_UNSAFE_SINGLETON_MANUAL,
	CS_UNSAFE_MANUAL,            // Should be last UNSAFE
	CS_PROTECT_MANUAL,
	CS_SECONDARY_UNSAFE,         // source: CS_SAFE
	CS_SECONDARY_UNSAFE_MANUAL,  // source: CS_SAFE_MANUAL
	CS_EMPTY_NONSTRUCTURAL,
	CS_EMPTY                     // always last! (see check in function SetTableCellStatus)
};

#define NCELLSTATUS 14   // see above


enum RecodePhase {
	CHECK = 10,
	DESTCODE,
	SRCCODE
};

enum FromToCodes {
	FROMTO_TO = 10,   // -23
	FROMTO_SOLO,      // 34
	FROMTO_FROM,      // 23-
	FROMTO_RANGE      // 23-25
};


// ErrorCodes
enum ErrorCodes {
	// Keep under the 5000.
	FILENOTFOUND = 1000,
	CANTOPENFILE,
	EMPTYFILE,
	WRONGLENGTH,
	RECORDTOOSHORT,
	WRONGRECORD,
	NOVARIABLES,
	NOTABLES,
	NOTENOUGHMEMORY,
	NOTABLEMEMORY, 
	SUBTABLENOSUB,
	SUBTABLEWRONGVAR,
	NODATAFILE,
	PROGRAMERROR,
	WRONGHIERARCHY,
	TABLENOTSET,
	VARIABLENOTSET,
	CODENOTINCODELIST,
	ISNOTNUMERIC,
	FREQPERCOUTOFRANGE,
	STATUSPERCOUTOFRANGE,
	PROBLEMSREADINGFREEFORMAT,
	// From table inputs

	CELLALREADYFILLED,
	CANNOTFILLTABLE,
	TABLENOTADDITIVE,
	CANNOTMAKETOTALS,
	CANNOTCALCULATESAFETY,
	CODEDOESNOTEXIST,

	HC_FILENOTFOUND = 1500,
	HC_LEVELSTRINGEMPTY,
	HC_CODEISMISSING,
	HC_LEVELINCORRECT,
	HC_CODEEMPTY,
	HC_NOTHIERARCHICAL,
	HC_HASSPLITDIGITS,
	HC_CODETOOLONG,


	// parsing recode
	E_HARD = 2000,
	E_SOFT,
	E_NOVARTABDATA,
	E_LENGTHWRONG,
	E_RANGEWRONG,
	E_VARINDEXWRONG,
	E_EMPTYSPEC,
  
	// codes recode
	R_FROMTOOBIG = 3000,
	R_CODENOTINLIST,
	R_NOSENSE,
	R_MISSING2VALID,

	// GHMITER
	GHM_TABLEINDEXWRONG = 4000,
	GHM_NOTPREPARED,
	GHM_CODEWIDTHTOOBIG,
	GHM_TABELLEINCORRECT,
	GHM_STEUERINCORRECT,
	GHM_EINGABEINCORRECT,
	GHM_EXECUTEINCORRECT,
	GHM_SOURCECELLINCORRECT,
	GHM_NOFILEAUSGABE,

	// HITAS
	HITAS_ = 4100,

	// JJFormat
	JFF_NOFILE = 4200,
	JJF_SOURCECELLINCORRECT,
	JJF_TABLEINDEXWRONG,

	// AMPL
	NOTABLE = 4225,
	TABLEWITHMORETHAN2VARIABLES,
	FIRSTVARNOTHIERARCHICAL,
	SECONDVARNOTFLAT,
	NOMEMORYFORSUBCODES,
	COULDNOTCREATESUBCODES,
	COULDNOTOPENFILE,
	COULDNOTCREATEAMPL,
};

#define COMPACT // bij tonen tabellen alleen RESP tonen

#endif // __TAUARGUSDEFINESH
