#include "stdafx.h"
#include "TauArgus.h"
#include "JavaProgressListener.h"
#include "tauargus_extern_TauArgusX.h"

/*
 * Class:     tauargus_extern_TauArgusX
 * Method:    SetProgressListener
 * Signature: (JLtauargus/extern/IProgressListener;)V
 */
JNIEXPORT void JNICALL Java_tauargus_extern_TauArgusX_SetProgressListener(JNIEnv *jenv, jclass cls, jlong jarg1, jobject jarg2) {
  TauArgus *tauArgus = *(TauArgus **)&jarg1; 
  delete (JavaProgressListener*) tauArgus->GetProgressListener();
  tauArgus->SetProgressListener(new JavaProgressListener(jenv, jarg2));
}
