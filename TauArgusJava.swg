%module TauArgusJava

%include <typemaps.i>
%include <windows.i>
%include <arrays_java.i>
%include <various.i>

#define FAR                     __far
#define STDMETHOD(method)       HRESULT STDMETHODCALLTYPE method
#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDMETHODCALLTYPE       __stdcall

// Treat the following types as simple output variables in Java
%apply bool *OUTPUT { bool * };
%apply long *OUTPUT { long * };
%apply double *OUTPUT { double * };
// Method parameters treated as array variables in Java
%apply long[] { long *ExplanatoryVarList, long *DominanceNumber, long *DominancePerc, long *PriorPosteriorP, long *PriorPosteriorQ, long *PriorPosteriorN, long *SafeMinRecAndHoldings, long *PeepPerc, long *PeepSafetyRange, long *PeepMinFreqCellAndHolding, long *CellAndHoldingFreqSafetyPerc, long *DimIndex, long *Status};
%apply double[] { double *Cell };
// Method parameters treated as String output variable in Java
%apply char **STRING_OUT { const char ** CodeString };

%{
#include "stdafx.h"
#include "TauArgus.h"
%}

%include "TauArgus.h"
