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
	virtual ~GLRendererYUV420p() {}

	virtual void initVertices();
	virtual void initTexCoords();
	virtual void cropTexCoords(GLfloat ratio);
	virtual int initTexture();
	virtual int loadTexture(GLFrame *pic);
	virtual int loadTexCoords(int id);
	virtual void updateTexture(GLFrame *pic);
	virtual int prepare();
	virtual int getLineSize(GLFrame *pic);
	virtual int render(GLFrame *pic);

protected:
	GLfloat mVertices[8];
	GLfloat mTexCoords[8];
	GLubyte mIndices[6];
	GLuint mTextures[GLES2_MAX_PLANE];
	GLuint mSamplers[GLES2_MAX_PLANE];
};
	
}

#endif
