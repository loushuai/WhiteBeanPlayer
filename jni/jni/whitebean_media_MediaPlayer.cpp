#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "../mediaplayer/WhiteBeanPlayer.hpp"
#include "JNIHelp.h"

using namespace std;
using namespace whitebean;

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

struct fields_t {
    jfieldID    context;
};
static fields_t fields;

static mutex sLock;
static mutex sPlayerLock;
static unordered_multimap<void *, shared_ptr<WhiteBeanPlayer> > sPlayers;

static void incStrong(shared_ptr<WhiteBeanPlayer> sp)
{
	unique_lock<mutex> l(sPlayerLock);
	sPlayers.insert({sp.get(), sp});
}

static shared_ptr<WhiteBeanPlayer> decStrong(void *p)
{
	shared_ptr<WhiteBeanPlayer> ret;

	unique_lock<mutex> l(sPlayerLock);
	
	auto it = sPlayers.find(p);	
	if (it != sPlayers.end()) {
		ret = it->second;
		sPlayers.erase(it);
	}

	return ret;
}

static shared_ptr<WhiteBeanPlayer> getMediaPlayer(JNIEnv* env, jobject thiz)
{
	unique_lock<mutex> l(sLock);
	shared_ptr<WhiteBeanPlayer> ret;
	WhiteBeanPlayer* const p = (WhiteBeanPlayer*)env->GetIntField(thiz, fields.context);

	auto it = sPlayers.find(p);	
	if (it != sPlayers.end()) {
		ret = it->second;
	}
	
	return ret;
}

static shared_ptr<WhiteBeanPlayer> setMediaPlayer(JNIEnv* env, jobject thiz,
												  shared_ptr<WhiteBeanPlayer>& player)
{
	unique_lock<mutex> l(sLock);
	shared_ptr<WhiteBeanPlayer> ret;
	void * old = ((void*)env->GetIntField(thiz, fields.context));

	if (player) {
		incStrong(player);
	}

	
	if (old != 0) {
		ret = decStrong(old);
	}

	env->SetIntField(thiz, fields.context, (int)player.get());

	return ret;
}

static void whitebean_media_MediaPlayer_setDataSourcePath(
		    JNIEnv *env, jobject thiz, jstring path) {
	shared_ptr<WhiteBeanPlayer> mp = getMediaPlayer(env, thiz);
	if (!mp) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;		
	}

    if (path == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }

    const char *tmp = env->GetStringUTFChars(path, NULL);
    if (tmp == NULL) {  // Out of memory
        return;
    }
    LOGD("setDataSource: path %s", tmp);

	string pathStr(tmp);
    env->ReleaseStringUTFChars(path, tmp);
    tmp = NULL;

    int opStatus = mp->setDataSource(pathStr);
}

static void whitebean_media_MediaPlayer_prepare(JNIEnv *env, jobject thiz)
{
	shared_ptr<WhiteBeanPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

	int opStatus = mp->prepare();
}

static void whitebean_media_MediaPlayer_start(JNIEnv *env, jobject thiz)
{
	LOGD("start");
	shared_ptr<WhiteBeanPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

	int opStatus = mp->play();
}

static void whitebean_media_MediaPlayer_native_init(JNIEnv *env)
{
    jclass clazz;

    clazz = env->FindClass("whitebean/media/MediaPlayer");
    if (clazz == NULL) {
		LOGE("Find class failed");
        return;
    }

    fields.context = env->GetFieldID(clazz, "mNativeContext", "I");
    if (fields.context == NULL) {
		LOGE("Get field failed");
        return;
    }
}

static void whitebean_media_MediaPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
	LOGD("native setup");
	shared_ptr<WhiteBeanPlayer> mp(new WhiteBeanPlayer());
    if (mp == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return;
    }

	setMediaPlayer(env, thiz, mp);
}


static JNINativeMethod gMethods[] = {
	{"_setDataSource",        "(Ljava/lang/String;)V",          (void *)whitebean_media_MediaPlayer_setDataSourcePath},
    {"prepare",             "()V",                              (void *)whitebean_media_MediaPlayer_prepare},
    {"_start",              "()V",                              (void *)whitebean_media_MediaPlayer_start},	
	{"native_init",         "()V",                              (void *)whitebean_media_MediaPlayer_native_init},
	{"native_setup",        "(Ljava/lang/Object;)V",            (void *)whitebean_media_MediaPlayer_native_setup},
};

int jniRegisterNativeMethods(JNIEnv* env,
                             const char* className,
                             const JNINativeMethod* gMethods,
                             int numMethods)
{
    jclass clazz;

	LOGD("Registering %s natives", className);
    clazz = env->FindClass(className);
    if (clazz == NULL) {
		LOGD("Native registration unable to find class '%s'", className);
        return -1;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
		LOGD("RegisterNatives failed for '%s'", className);
        return -1;
    }
    return 0;
}

static int register_android_media_MediaPlayer(JNIEnv *env)
{
	return jniRegisterNativeMethods(env, "whitebean/media/MediaPlayer", gMethods, NELEM(gMethods));
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = nullptr;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed");
        goto bail;
    }

	if (register_android_media_MediaPlayer(env) < 0) {
        LOGE("ERROR: MediaPlayer native registration failed");
        goto bail;
	}

   result = JNI_VERSION_1_4;

bail:
    return result;	
}
