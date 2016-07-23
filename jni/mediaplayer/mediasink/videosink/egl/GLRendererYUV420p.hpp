/*
 * GLRendererYUV420p.hpp
 *
 * Created on: 2016Äê7ÔÂ16ÈÕ
 *     Author: loushuai
 */

#ifndef JNI_MEDIASINK_VIDEOSINK_EGL_GLRENDERERYUV420P_H_
#define JNI_MEDIASINK_VIDEOSINK_EGL_GLRENDERERYUV420P_H_

#include "GLRenderer.hpp"

namespace whitebean
{
	
class GLRendererYUV420p: public GLRenderer
{
public:
	GLRendererYUV420p();
	~GLRendererYUV420p() {}

	int initTexture();
	int loadTexture(GLFrame *pic);
	int getLineSize(GLFrame *pic);
private:
	GLuint mTextures[GLES2_MAX_PLANE];
	GLuint mSamplers[GLES2_MAX_PLANE];
};
	
}

#endif
