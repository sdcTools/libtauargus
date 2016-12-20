// %module TauArgusJava
// directors="1" needed for being able to use the director feature
%module(directors="1") TauArgusJava

%include <windows.i>
%include <typemaps.i>
%include <arrays_java.i>
%include <various.i>
%include <std_string.i>

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

// Instead of the default, treat these method parameters as array variables in Java...

// Arrays used as input
%apply long[] { long *ExplanatoryVarList, long *DominanceNumber, long *DominancePerc, long *PriorPosteriorP, long *PriorPosteriorQ, long *PriorPosteriorN, long *SafeMinRecAndHoldings, long *PeepPerc, long *PeepSafetyRange, long *PeepMinFreqCellAndHolding, long *CellAndHoldingFreqSafetyPerc, long *DimIndex, long *nDigits, long *DimSequence, long *VarIndex };
%apply char **STRING_ARRAY { char* [] }
%apply double[] { double *MaxScoreCell, double *MaxScoreHolding };

// Arrays used as output
%apply long[] { long *Status, long *HoldingNrPerMaxScore, long *UCArray };
%apply double[] { double *Cell, double *CellMaxScore, double *CellMAXScoreWeight, double *HoldingMaxScore, double *Cost };
// GetCellStatusStatistics
%apply long[] { long *StatusFreq, long *StatusCellFreq, long *StatusHoldingFreq };
%apply double[] { double *StatusCellResponse, double *StatusCellCost };

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

// Generate (Java) code for automatically loading the generated C++ library in Java
//%pragma(java) jniclasscode=%{
//  static {
//    try {
//        System.loadLibrary("TauArgusJava");
//    } catch (UnsatisfiedLinkError e) {
//      System.err.println("Native code library failed to load. \n" + e);
//      System.exit(1);
//    }
//  }
//%}
