/*
 * GLRenderer.hpp
 *
 * Created on: 2016Äê7ÔÂ3ÈÕ
 *     Author: loushuai
 */

#ifndef JNI_MEDIASINK_VIDEOSINK_EGL_GLRENDERER_H_
#define JNI_MEDIASINK_VIDEOSINK_EGL_GLRENDERER_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "../../../mediabase/FrameBuffer.hpp"

namespace whitebean
{

#define STRINGIZE(x) #x
#define GLES2_MAX_PLANE 3	

class GLFrame{
public:
	GLFrame(): mPlanes(0) {
		for (int i = 0; i < GLES2_MAX_PLANE; ++i) {
			pixels[i] = nullptr;
			pitches[i] = 0;
		}
		width = 0;
		height = 0;
	}

	GLFrame(FrameBuffer &frm): mFrameBuffer(frm) {		
		int planes = GLES2_MAX_PLANE < mFrameBuffer.getNumDataPlanes() ?
									   GLES2_MAX_PLANE : mFrameBuffer.getNumDataPlanes();

		mPlanes = planes;
		
		for (int i = 0; i < planes; ++i) {
			pixels[i] = mFrameBuffer.getDataPlane(i);
			pitches[i] = mFrameBuffer.getLineSize(i);
		}
		width = mFrameBuffer.getWidth();
		height = mFrameBuffer.getHeight();
	}
	
	~GLFrame() {}

	int getPlanes() const {
		return mPlanes;
	}

	int getLineSize(int i) const {
		if (i >= GLES2_MAX_PLANE) {
			return 0;
		}

		return pitches[i];
	}
	
	GLubyte *pixels[GLES2_MAX_PLANE];
	int pitches[GLES2_MAX_PLANE];
	int width;
	int height;
private:
	int mPlanes;
	FrameBuffer mFrameBuffer;
};
	
class GLRenderer
{
public:
	GLRenderer();
	virtual ~GLRenderer();

	virtual void initVertices();
	virtual int loadVertices();
	virtual void initTexCoords();
	virtual void cropTexCoords(GLfloat ratio);
	virtual int loadTexCoords(int id);
	virtual int initTexture();
	virtual int loadTexture(GLFrame *pic);
	virtual int getLineSize(GLFrame *pic);
	virtual int prepare();
	virtual int render(GLFrame *pic);
	
protected:
	const char *mVertexScript;
	const char *mFragmentScript;
	GLuint loadShader(GLenum type, const char *shaderSrc);
	GLuint createProgram(const char *vshaderSrc, const char *fshaderSrc);

	GLuint mGlProgram;
	GLubyte *mIndices;
	int mVertexSize;
	int mIndicesSize;
	GLuint mVertexBuffer;
	GLuint mTexCoordBuffer[GLES2_MAX_PLANE];
	GLuint mIndicesBuffer;
	GLuint mGlVShader;
	GLuint mGlFShader;
	GLfloat mVertices[8];
	GLfloat mTexCoords[8];
	int mBufferLineSize;
};
	
}

#endif
