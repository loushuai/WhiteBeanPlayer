/*
 * EglSink.hpp
 *
 * Created on: 2016Äê7ÔÂ3ÈÕ
 *     Author: loushuai
 */

#ifndef JNI_MEDIASINK_VIDEOSINK_EGL_EGLSINK_H_
#define JNI_MEDIASINK_VIDEOSINK_EGL_EGLSINK_H_

#include <EGL/egl.h>
#include <android/native_window_jni.h>
#include <memory>
#include "../videosink.hpp"

namespace whitebean
{
class GLRenderer;	

class EglSink : public VideoSink
{
public:
	EglSink() = delete;
	EglSink(ANativeWindow *nwindow);
	~EglSink();

	int init(int type);
	int display(FrameBuffer &frm);
	void onTouchMoveEvent(float dx, float dy);
private:
	ANativeWindow               *mNativeWindow;
	EGLNativeDisplayType	     mEglDisplay;
	EGLSurface	                 mEglSurface;
	EGLContext	                 mEglContext;
	int                          mSurfaceWidth;
	int                          mSurfaceHeight;
	std::shared_ptr<GLRenderer>  mRenderPtr;
};
	
}

#endif
