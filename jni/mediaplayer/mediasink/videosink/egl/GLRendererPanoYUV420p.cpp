/*
 * GLRendererPanoYUV420p.cpp
 *
 *  Created on: 2016年11月29日
 *      Author: leon
 */

#include <math.h>
#include "log.hpp"
#include "GLRendererPanoYUV420p.hpp"
#include "utils/math3d.hpp"

namespace whitebean
{

const float GLRendererPanoYUV420p::mR = 5.0f;
const float GLRendererPanoYUV420p::TOUCH_SCALE_FACTOR = 180.0f / 320 / 3.8f;

GLRendererPanoYUV420p::GLRendererPanoYUV420p()
: left(0)
, right(0)
, bottom(0)
, top(0)
, angleX(0)
, angleY(0)
{
	mVertexSize = 3;
	mTexCoordSize = 2;

    mVertexScript = STRINGIZE(
        attribute vec4 a_position;
        attribute vec2 a_tex_coord_in_y;
		attribute vec2 a_tex_coord_in_u;
		attribute vec2 a_tex_coord_in_v;
        varying vec2 v_tex_coord_out_y;
		varying vec2 v_tex_coord_out_u;
		varying vec2 v_tex_coord_out_v;
		uniform	mat4 m_projection;

        void main(void) {
            v_tex_coord_out_y = a_tex_coord_in_y;
			v_tex_coord_out_u = a_tex_coord_in_u;
			v_tex_coord_out_v = a_tex_coord_in_v;
			gl_Position = m_projection*a_position;
        }
    );

    // init data
	memset(mProjectionMatrix, 0, sizeof(mProjectionMatrix));
	memset(mViewMatrix, 0, sizeof(mViewMatrix));
	memset(mProjectionViewMatrix, 0, sizeof(mProjectionViewMatrix));
	memset(mRotationMatrixX, 0, sizeof(mRotationMatrixX));
	memset(mRotationMatrixY, 0, sizeof(mRotationMatrixY));
	memset(mRotationMatrix, 0, sizeof(mRotationMatrix));
	memset(mScrtch, 0, sizeof(mScrtch));
}

void GLRendererPanoYUV420p::initVertices()
{
    double phiStep = 2 * PI / mPhiSteps;
    double tettaStep = PI / mTettaSteps;

    int spherePointCounter = 0;
    int texturePointCounter = 0;

    for (double tetta = 0; tetta <= (PI - tettaStep) + tettaStep / 2 ; tetta += tettaStep) {
        for(double phi = 0; phi <= 2 * PI + phiStep / 2; phi += phiStep) {
            mVertices[spherePointCounter++] = (float)(mR * sin(tetta) * cos(phi));
            mVertices[spherePointCounter++] = (float)(mR * cos(tetta));
            mVertices[spherePointCounter++] = (float)(mR * sin(tetta) * sin(phi));

            mTexCoords[texturePointCounter++] = (float) (phi / (2 * PI));
            mTexCoords[texturePointCounter++] = 1.f - (float) (tetta / (PI));

            mVertices[spherePointCounter++] = (float)(mR * sin(tetta + tettaStep) * cos(phi));
            mVertices[spherePointCounter++] = (float)(mR * cos(tetta + tettaStep));
            mVertices[spherePointCounter++] = (float)(mR * sin(tetta + tettaStep) * sin(phi));

            mTexCoords[texturePointCounter++] = (float) (phi / (2 * PI));
            mTexCoords[texturePointCounter++] = 1.f - (float) ((tetta + tettaStep) / (PI));
        }
    }

    mVerticesPtr = mVertices;
    mVerticesNum = sizeof(mVertices) / sizeof(GLfloat) / mVertexSize;
}

void GLRendererPanoYUV420p::initTexCoords()
{
	mTexCoordsPtr = mTexCoords;
	mTexCoordsNum = sizeof(mTexCoords) / sizeof(GLfloat) / mTexCoordSize;
}

void GLRendererPanoYUV420p::loadProjection(float aspect)
{
    float ratio = aspect;
    left = -1.0f;
    right = 1.0f;
    bottom = -1.0f;
    top = 1.0f;
    left *= ratio;
    right *= ratio;

    frustumM(mProjectionMatrix, left, right, bottom, top, 1.2f, 5.0f);

    setLookAtM(mViewMatrix, 0, 0, 0, 0, 0, -1, 0, -1, 0);
    matrixMul4(mProjectionMatrix, mViewMatrix, mProjectionViewMatrix);
    setRotateM(mRotationMatrixX, angleY, -1.0f, 0.0f, 0);
    setRotateM(mRotationMatrixY, angleX, 0.0f, -1.0f, 0);
    matrixMul4(mRotationMatrixX, mRotationMatrixY, mRotationMatrix);
    matrixMul4(mProjectionViewMatrix, mRotationMatrix, mScrtch);
}

int GLRendererPanoYUV420p::render(GLFrame *pic)
{
	GLint rec[4];
	glGetIntegerv(GL_VIEWPORT, rec);
	int surWidth = rec[2];
	int surHeight = rec[3];

	glClear(GL_COLOR_BUFFER_BIT);

	if (!pic) {
		return -1;
	}

	loadProjection(surWidth*1.0/surHeight);

    GLuint matrix = glGetUniformLocation(mGlProgram, "m_projection");
	glUniformMatrix4fv(matrix, 1, 0, mScrtch);

	updateTexture(pic);

	glDrawArrays(GL_TRIANGLE_STRIP , 0, mVerticesNum);

	return 0;
}

void GLRendererPanoYUV420p::onTouchMoveEvent(float dx, float dy)
{
	angleX -= dx * TOUCH_SCALE_FACTOR;
	angleY -= dy * TOUCH_SCALE_FACTOR;
}

}
