%module(directors="1") TauArgusJava
%{
#include "IProgressListener.h"
%}

/* turn on director wrapping IProgressListener */
%feature("director") IProgressListener;

%include "IProgressListener.h"
