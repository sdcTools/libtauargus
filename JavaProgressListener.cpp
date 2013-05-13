// JavaProgressListener.cpp: implementation of the JavaProgressListener class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JavaProgressListener.h"

//////////////////////////////////////////////////////////////////////
// Helper class
//////////////////////////////////////////////////////////////////////

class EnvWrapper {
	JavaVM *jvm;
	JNIEnv *jenv;
	int env_status;

public:
	EnvWrapper(JavaVM *jvm) {
		this->jvm = jvm;
        env_status = jvm->GetEnv((void **)&jenv, JNI_VERSION_1_2);
        if (env_status == JNI_EDETACHED)
			jvm->AttachCurrentThread((void **)&jenv, NULL);
	}

	~EnvWrapper() {
        if (env_status == JNI_EDETACHED)
			jvm->DetachCurrentThread();
	}

	JNIEnv* getJNIEnv() {
		return jenv;
	}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

JavaProgressListener::JavaProgressListener(JNIEnv *jenv, jobject object) : IProgressListener() {
    jenv->GetJavaVM(&jvm);
	this->object = jenv->NewGlobalRef(object);
	jclass cls = jenv->GetObjectClass(object);
	this->methodId = jenv->GetMethodID(cls, "UpdateProgress", "(I)V");
}

JavaProgressListener::~JavaProgressListener() {
	EnvWrapper envWrapper(jvm);
	envWrapper.getJNIEnv()->DeleteGlobalRef(object);
}

//////////////////////////////////////////////////////////////////////
// Public functions
//////////////////////////////////////////////////////////////////////

void JavaProgressListener::UpdateProgress(int perc) {
	EnvWrapper envWrapper(jvm);
	envWrapper.getJNIEnv()->CallVoidMethod(object, methodId, (jint)perc);
}
