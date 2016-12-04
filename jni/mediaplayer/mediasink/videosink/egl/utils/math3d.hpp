/*
 * math3d.hpp
 *
 *  Created on: 2016年12月3日
 *      Author: leon
 */

#ifndef MEDIAPLAYER_MEDIASINK_VIDEOSINK_EGL_UTILS_MATH3D_HPP_
#define MEDIAPLAYER_MEDIASINK_VIDEOSINK_EGL_UTILS_MATH3D_HPP_

namespace whitebean
{

#define PI 3.14159265358979323846264338327950288
const int MATRIX_SIZE = 16;

/*
 * Multiply two 4x4 matrixes
 * @para left the left matrix
 * @para right the right matrix
 * @para result
 */
void matrixMul4(float left[MATRIX_SIZE], float right[MATRIX_SIZE] ,float result[MATRIX_SIZE]);

/*
 * Defines a projection matrix in terms of six clip planes.
 * @param m the float array that holds the output perspective matrix
 * @param left
 * @param right
 * @param bottom
 * @param top
 * @param near
 * @param far
 * @return 0 ok, -1 error
 */
int frustumM(float m[MATRIX_SIZE], float left, float right, float bottom, float top,
        	  float near, float far);

/**
 * Defines a viewing transformation in terms of an eye point, a center of
 * view, and an up vector.
 *
 * @param rm returns the result
 * @param eyeX eye point X
 * @param eyeY eye point Y
 * @param eyeZ eye point Z
 * @param centerX center of view X
 * @param centerY center of view Y
 * @param centerZ center of view Z
 * @param upX up vector X
 * @param upY up vector Y
 * @param upZ up vector Z
 * @return 0 ok, -1 error
 */
void setLookAtM(float rm[MATRIX_SIZE], float eyeX, float eyeY, float eyeZ,
        					  float centerX, float centerY, float centerZ, float upX, float upY,
							  float upZ);

/**
 * Creates a matrix for rotation by angle a (in degrees)
 * around the axis (x, y, z).
 * <p>
 * An optimized path will be used for rotation about a major axis
 * (e.g. x=1.0f y=0.0f z=0.0f).
 *
 * @param rm returns the result
 * @param a angle to rotate in degrees
 * @param x X axis component
 * @param y Y axis component
 * @param z Z axis component
 */
void setRotateM(float rm[MATRIX_SIZE], float a, float x, float y, float z);

}

#endif /* MEDIAPLAYER_MEDIASINK_VIDEOSINK_EGL_UTILS_MATH3D_HPP_ */
