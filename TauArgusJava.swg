// %module TauArgusJava
// directors="1" needed for being able to use the director feature
%module(directors="1") TauArgusJava

%include <windows.i>
%include <typemaps.i>
%include <arrays_java.i>
%include <various.i>
%include <std_string.i> // For const std::string *

//For non-const std::string
%typemap(jni) std::string *INOUT, std::string &INOUT %{jobjectArray%}
%typemap(jtype) std::string *INOUT, std::string &INOUT "java.lang.String[]"
%typemap(jstype) std::string *INOUT, std::string &INOUT "java.lang.String[]"
%typemap(javain) std::string *INOUT, std::string &INOUT "$javainput"

%typemap(in) std::string *INOUT (std::string strTemp ), std::string &INOUT (std::string strTemp ) {
  if (!$input) {
    SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
    return $null;
  }
  if (JCALL1(GetArrayLength, jenv, $input) == 0) {
    SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
    return $null;
  }

  jobject oInput = JCALL2(GetObjectArrayElement, jenv, $input, 0); 
  if ( NULL != oInput ) {
    jstring sInput = static_cast<jstring>( oInput );

    const char * $1_pstr = (const char *)jenv->GetStringUTFChars(sInput, 0); 
    if (!$1_pstr) return $null;
    strTemp.assign( $1_pstr );
    jenv->ReleaseStringUTFChars( sInput, $1_pstr);  
  }

  $1 = &strTemp;
}

%typemap(freearg) std::string *INOUT, std::string &INOUT ""

%typemap(argout) std::string *INOUT, std::string &INOUT
{ 
  jstring jStrTemp = jenv->NewStringUTF( strTemp$argnum.c_str() );
  JCALL3(SetObjectArrayElement, jenv, $input, 0, jStrTemp ); 
}

// Defines not covered by the above ".i" files of SWIG
#define FAR                     __far
#define STDMETHOD(method)       HRESULT STDMETHODCALLTYPE method
#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDMETHODCALLTYPE       __stdcall

// By default treat method parameters of the following types as simple output variables in Java
// Arrays of size 1 are generated, so output can be retrieved (we can't pass the address of a variable in Java)
%apply bool *OUTPUT { bool * };
%apply long *OUTPUT { long * };
%apply double *OUTPUT { double * };
%apply char **STRING_OUT { char ** };
%apply std::string *INOUT { std::string * };

// Instead of the default, treat these method parameters as array variables in Java...

// Arrays used as input
%apply long[] { long *ExplanatoryVarList, long *DominanceNumber, long *DominancePerc, long *PriorPosteriorP, long *PriorPosteriorQ, long *PriorPosteriorN, long *SafeMinRecAndHoldings, long *PeepPerc, long *PeepSafetyRange, long *PeepMinFreqCellAndHolding, long *CellAndHoldingFreqSafetyPerc, long *DimIndex, long *nDigits, long *DimSequence, long *VarIndex };
%apply char **STRING_ARRAY { char* [] }
%apply double[] { double *MaxScoreCell, double *MaxScoreHolding };

// Arrays used as output
%apply long[] { long *Status, long *HoldingNrPerMaxScore, long *UCArray };
%apply double[] { double *Cell, double *CellMaxScore, double *CellMAXScoreWeight, double *HoldingMaxScore, double *Cost, double *CKMValue, double *CellKey};
// GetCellStatusStatistics
%apply long[] { long *StatusFreq, long *StatusCellFreq, long *StatusHoldingFreq };
%apply double[] { double *StatusCellResponse, double *StatusCellCost, double *epsilon };
%apply int[] {int *MaxDiff, int *MinDiff};

// Include this block in the generated C++ wrapper code (TauArgus_wrap.cpp)
%{
#include "IProgressListener.h"
#include "TauArgus.h"
%}

%feature("director") IProgressListener;

// Turn on director wrapping IProgressListener. This makes it possible to extend this class in Java
// Generate wrapper code for the classes in these header files, so they can be called from Java
%include "IProgressListener.h"
%include "TauArgus.h"
