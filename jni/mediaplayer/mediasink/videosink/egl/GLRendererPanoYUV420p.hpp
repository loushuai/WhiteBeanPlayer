/*
 * GLRendererPanoramaYUV420p.h
 *
 *  Created on: 2016年11月29日
 *      Author: leon
 */

#ifndef MEDIAPLAYER_MEDIASINK_VIDEOSINK_EGL_GLRENDERERPANOYUV420P_HPP_
#define MEDIAPLAYER_MEDIASINK_VIDEOSINK_EGL_GLRENDERERPANOYUV420P_HPP_

#include "GLRendererYUV420p.hpp"

namespace whitebean
{

class GLRendererPanoYUV420p : public GLRendererYUV420p {
public:
	GLRendererPanoYUV420p();
	virtual ~GLRendererPanoYUV420p() {}

	virtual void initVertices();
	virtual void initTexCoords();
	virtual void cropTexCoords(GLfloat ratio) {}
	int render(GLFrame *pic);
	virtual void onTouchMoveEvent(float dx, float dy);

private:
	static const int mTettaSteps = 10;
	static const int mPhiSteps = 18;
	static const float mR;
	static const float TOUCH_SCALE_FACTOR;

	GLfloat mVertices[((mPhiSteps + 1) * (2 * (mTettaSteps + 1) - 2)) * 3];
	GLfloat mTexCoords[((mPhiSteps + 1) * (2 * (mTettaSteps + 1) - 2)) * 2];

	void loadProjection(int viewDegree, float aspect);

	float left;
	float right;
	float bottom;
	float top;
	float angleX;
	float angleY;
	GLfloat __attribute__((aligned(16))) mProjectionMatrix[16];
	GLfloat __attribute__((aligned(16))) mViewMatrix[16];
	GLfloat __attribute__((aligned(16))) mProjectionViewMatrix[16];
	GLfloat __attribute__((aligned(16))) mRotationMatrixX[16];
	GLfloat __attribute__((aligned(16))) mRotationMatrixY[16];
	GLfloat __attribute__((aligned(16))) mRotationMatrix[16];
	GLfloat __attribute__((aligned(16))) mScrtch[16];

	int vertexStride;
};

}

#endif /* MEDIAPLAYER_MEDIASINK_VIDEOSINK_EGL_GLRENDERERPANOYUV420P_HPP_ */
