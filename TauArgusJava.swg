%module TauArgusJava

%include <typemaps.i>
%include <windows.i>
%include <arrays_java.i>
%include <various.i>
%include "IProgressListener.swg"

// Defines not covered by the above ".i" files of SWIG
#define FAR                     __far
#define STDMETHOD(method)       HRESULT STDMETHODCALLTYPE method
#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDMETHODCALLTYPE       __stdcall

// By default treat method parameters of the following types as simple output variables in Java
%apply bool *OUTPUT { bool * };
%apply long *OUTPUT { long * };
%apply double *OUTPUT { double * };

// Instead of the default, treat these method parameters as array variables in Java
%apply long[] { long *ExplanatoryVarList, long *DominanceNumber, long *DominancePerc, long *PriorPosteriorP, long *PriorPosteriorQ, long *PriorPosteriorN, long *SafeMinRecAndHoldings, long *PeepPerc, long *PeepSafetyRange, long *PeepMinFreqCellAndHolding, long *CellAndHoldingFreqSafetyPerc, long *DimIndex, long *Status, long *nDigits, long *HoldingNrPerMaxScore };
%apply double[] { double *Cell, double *CellMaxScore, double *CellMAXScoreWeight, double *HoldingMaxScore };

// Method parameters treated as String output variable in Java
%apply char **STRING_OUT { char ** };

// Include this block in the generated C++ wrapper code (TauArgus_wrap.cpp)
%{
#include "stdafx.h"
#include "TauArgus.h"
%}

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

// Contains the class for which a Java wrapper/interface will be generated (only public methods will be handled of course)
%include "TauArgus.h"
