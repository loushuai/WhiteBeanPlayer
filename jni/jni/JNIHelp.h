#ifndef _JNI_HELP_H
#define _JNI_HELP_H

#include "log.hpp"
#include "jni.h"

int jniThrowException(JNIEnv* env, const char* className, const char* msg)
{
    jclass exceptionClass = env->FindClass(className);
    if (exceptionClass == NULL) {
		LOGE("Unable to find exception class %s,%s", className,msg);
        return -1;
    }

    if (env->ThrowNew(exceptionClass, msg) != JNI_OK) {
		LOGE("Failed throwing '%s' '%s'", className, msg);
		return -1;
    }
    return 0;	
}

#endif
