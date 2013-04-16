// directors="1" needed for being able to use the director feature
%module(directors="1") TauArgusJava

// Include this block in the generated C++ wrapper code (IProgressListener_wrap.h)
%{
#include "IProgressListener.h"
%}

// Turn on director wrapping IProgressListener. This makes it possible to extend this class in Java
%feature("director") IProgressListener;

// Generate wrapper code for the class in this header file, so it can be called from Java
%include "IProgressListener.h"
