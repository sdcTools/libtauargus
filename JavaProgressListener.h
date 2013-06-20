#if !defined JavaProgressListener_h
#define JavaProgressListener_h

#include <jni.h>
#include "IProgressListener.h"

class JavaProgressListener : public IProgressListener  
{
	JavaVM *jvm;
	jobject object;
	jmethodID methodId;
public:
	JavaProgressListener(JNIEnv *jenv, jobject object);
	~JavaProgressListener();
	void UpdateProgress(int perc);
};

#endif // JavaProgressListener_h
