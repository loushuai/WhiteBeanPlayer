/*
 * EglSink.cpp
 *
 * Created on: 2016Äê7ÔÂ3ÈÕ
 *     Author: loushuai
 */

#include "log.hpp"
#include "EglSink.hpp"
#include "GLRendererFactory.hpp"

using namespace std;

namespace whitebean
{

EglSink::EglSink(ANativeWindow *nwindow)
: mNativeWindow(nwindow)
, mEglDisplay(EGL_NO_DISPLAY)
, mEglContext(EGL_NO_CONTEXT)
, mEglSurface(EGL_NO_SURFACE)
, mSurfaceWidth(0)
, mSurfaceHeight(0)  
{
		
}

EglSink::~EglSink() {
	if (mEglDisplay != EGL_NO_DISPLAY) {
		eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (mEglContext != EGL_NO_CONTEXT) {
			eglDestroyContext(mEglDisplay, mEglContext);
		}
		if (mEglSurface != EGL_NO_SURFACE) {
			eglDestroySurface(mEglDisplay, mEglSurface);

		}
		if (!eglTerminate(mEglDisplay)) {
			LOGE( "Egl terminate error");
		}		
	}
}

int EglSink::init(int type)
{
	GLint majorVersion;
	GLint minorVersion;
	
	if (!mNativeWindow) {
		return -1;
	}

	mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if(EGL_NO_DISPLAY == mEglDisplay) {
		LOGE("Egl get display error");
		return -1;
	}

	if(!eglInitialize(mEglDisplay, &majorVersion, &minorVersion)) {
		LOGE("Egl initialize error");
		return -1;
	}

	EGLint config_attribs[] = {
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	int num_configs = 0;
	EGLConfig 	eglConfig;
	if(!eglChooseConfig(mEglDisplay, config_attribs, &eglConfig, 1, &num_configs))
	{
		LOGE("EGL eglChooseConfig error");
		return -1;
	}

	EGLint surf_attribs[] =
	{
		EGL_COLORSPACE, EGL_COLORSPACE_sRGB,
		EGL_NONE
	};

	mEglSurface = eglCreateWindowSurface(mEglDisplay, eglConfig, mNativeWindow, NULL);
	if(EGL_NO_SURFACE == mEglSurface) {
		LOGE("EGL eglCreateWindowSurface error");
		return -1;
	}

	if(!eglQuerySurface(mEglDisplay, mEglSurface, EGL_WIDTH, &mSurfaceWidth) ||
	   !eglQuerySurface(mEglDisplay, mEglSurface, EGL_HEIGHT, &mSurfaceHeight)) {
		LOGE("EGL eglQuerySurface error");
		return -1;
	}

	if(mSurfaceWidth <= 0 || mSurfaceHeight <= 0) {
		LOGE("EGL invalid mSurfaceWidth or mSurfaceHeight");
		return -1;
	}

	EGLint context_attrib[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	mEglContext = eglCreateContext(mEglDisplay, eglConfig, EGL_NO_CONTEXT, context_attrib);
	if(EGL_NO_CONTEXT == mEglContext) {
		LOGE("EGL eglCreateContext error");
		return -1;
	}

	if(!eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext)) {
		LOGE("EGL eglMakeCurrent error");
		return -1;
	}

	glViewport(0, 0, mSurfaceWidth, mSurfaceHeight);
	glClearColor(1.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);	
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	eglSwapBuffers(mEglDisplay, mEglSurface);

	LOGD("EGL init success");

	if (type == VIDEO_SINK_TYPE_PANORAMIC) {
		mRenderPtr = GLRendererFactory::create(GL_RENDERER_PANORAMIC_YUV420P);
	} else {
		mRenderPtr = GLRendererFactory::create(GL_RENDERER_YUV420P);
	}

	if (!mRenderPtr) {
		LOGE("EGL create render error");
		return -1;
	}

	if (mRenderPtr->prepare()) {
		LOGE("Render prepare failed");
		return -1;
	}
	
	return 0;
}

int EglSink::display(FrameBuffer &frm)
{
	GLFrame glfrm(frm);
	
	mRenderPtr->render(&glfrm);
	
	eglSwapBuffers(mEglDisplay, mEglSurface);
	
	return 0;
}

void EglSink::onTouchMoveEvent(float dx, float dy)
{
	if (mRenderPtr) {
		mRenderPtr->onTouchMoveEvent(dx, dy);
	}
}
	
}
