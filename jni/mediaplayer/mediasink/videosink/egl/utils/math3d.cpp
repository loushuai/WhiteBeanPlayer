/*
 * math3d.cpp
 *
 *  Created on: 2016年12月3日
 *      Author: leon
 */

#include <cmath>
#include "math3d.hpp"

namespace whitebean
{

static inline float length(float x, float y, float z)
{
	return sqrt(x*x + y*y + z*z);
}

/**
 * Translates matrix m by x, y, and z in place.
 *
 * @param m matrix
 * @param mOffset index into m where the matrix starts
 * @param x translation factor x
 * @param y translation factor y
 * @param z translation factor z
 */
static inline void translateM(float m[MATRIX_SIZE],
        					  float x, float y, float z)
{
    for (int i=0 ; i<4 ; i++) {
        int mi = i;
        m[12 + mi] += m[mi] * x + m[4 + mi] * y + m[8 + mi] * z;
    }
}

void matrixMul4(float left[MATRIX_SIZE], float right[MATRIX_SIZE] ,float result[MATRIX_SIZE])
{
    float __attribute__((aligned(16))) resultMul[16];
    resultMul[0]  = left[0] * right[0]  + left[4] * right[1]  + left[8] * right[2]   + left[12] * right[3];
    resultMul[4]  = left[0] * right[4]  + left[4] * right[5]  + left[8] * right[6]   + left[12] * right[7];
    resultMul[8]  = left[0] * right[8]  + left[4] * right[9]  + left[8] * right[10]  + left[12] * right[11];
    resultMul[12] = left[0] * right[12] + left[4] * right[13] + left[8] * right[14]  + left[12] * right[15];

    resultMul[1]  = left[1] * right[0]  + left[5] * right[1]  + left[9] * right[2]   + left[13] * right[3];
    resultMul[5]  = left[1] * right[4]  + left[5] * right[5]  + left[9] * right[6]   + left[13] * right[7];
    resultMul[9]  = left[1] * right[8]  + left[5] * right[9]  + left[9] * right[10]  + left[13] * right[11];
    resultMul[13] = left[1] * right[12] + left[5] * right[13] + left[9] * right[14]  + left[13] * right[15];

    resultMul[2]  = left[2] * right[0]  + left[6] * right[1]  + left[10] * right[2]  + left[14] * right[3];
    resultMul[6]  = left[2] * right[4]  + left[6] * right[5]  + left[10] * right[6]  + left[14] * right[7];
    resultMul[10] = left[2] * right[8]  + left[6] * right[9]  + left[10] * right[10] + left[14] * right[11];
    resultMul[14] = left[2] * right[12] + left[6] * right[13] + left[10] * right[14] + left[14] * right[15];

    resultMul[3]  = left[3] * right[0]  + left[7] * right[1]  + left[11] * right[2]  + left[15] * right[3];
    resultMul[7]  = left[3] * right[4]  + left[7] * right[5]  + left[11] * right[6]  + left[15] * right[7];
    resultMul[11] = left[3] * right[8]  + left[7] * right[9]  + left[11] * right[10] + left[15] * right[11];
    resultMul[15] = left[3] * right[12] + left[7] * right[13] + left[11] * right[14] + left[15] * right[15];

    result[0] = resultMul[0];
    result[1] = resultMul[1];
    result[2] = resultMul[2];
    result[3] = resultMul[3];
    result[4] = resultMul[4];
    result[5] = resultMul[5];
    result[6] = resultMul[6];
    result[7] = resultMul[7];

    result[8] = resultMul[8];
    result[9] = resultMul[9];
    result[10] = resultMul[10];
    result[11] = resultMul[11];
    result[12] = resultMul[12];
    result[13] = resultMul[13];
    result[14] = resultMul[14];
    result[15] = resultMul[15];
}

int frustumM(float m[MATRIX_SIZE], float left, float right, float bottom, float top,
        	  float near, float far)
{
    if (left == right) {
    	return -1;
    }
    if (top == bottom) {
    	return -1;
    }
    if (near == far) {
    	return -1;
    }
    if (near <= 0.0f) {
    	return -1;
    }
    if (far <= 0.0f) {
    	return -1;
    }

    const float r_width  = 1.0f / (right - left);
    const float r_height = 1.0f / (top - bottom);
    const float r_depth  = 1.0f / (near - far);
    const float x = 2.0f * (near * r_width);
    const float y = 2.0f * (near * r_height);
    const float A = (right + left) * r_width;
    const float B = (top + bottom) * r_height;
    const float C = (far + near) * r_depth;
    const float D = 2.0f * (far * near * r_depth);
    m[0] = x;
    m[5] = y;
    m[8] = A;
    m[9] = B;
    m[10] = C;
    m[14] = D;
    m[11] = -1.0f;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[15] = 0.0f;

    return 0;
}

void setLookAtM(float rm[MATRIX_SIZE], float eyeX, float eyeY, float eyeZ,
        					  float centerX, float centerY, float centerZ, float upX, float upY,
							  float upZ)
{
    // See the OpenGL GLUT documentation for gluLookAt for a description
    // of the algorithm. We implement it in a straightforward way:

    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;

    // Normalize f
    float rlf = 1.0f / length(fx, fy, fz);
    fx *= rlf;
    fy *= rlf;
    fz *= rlf;

    // compute s = f x up (x means "cross product")
    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;

    // and normalize s
    float rls = 1.0f / length(sx, sy, sz);
    sx *= rls;
    sy *= rls;
    sz *= rls;

    // compute u = s x f
    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    rm[0] = sx;
    rm[1] = ux;
    rm[2] = -fx;
    rm[3] = 0.0f;

    rm[4] = sy;
    rm[5] = uy;
    rm[6] = -fy;
    rm[7] = 0.0f;

    rm[8] = sz;
    rm[9] = uz;
    rm[10] = -fz;
    rm[11] = 0.0f;

    rm[12] = 0.0f;
    rm[13] = 0.0f;
    rm[14] = 0.0f;
    rm[15] = 1.0f;

    translateM(rm, -eyeX, -eyeY, -eyeZ);
}

void setRotateM(float rm[MATRIX_SIZE], float a, float x, float y, float z)
{
    rm[3] = 0;
    rm[7] = 0;
    rm[11]= 0;
    rm[12]= 0;
    rm[13]= 0;
    rm[14]= 0;
    rm[15]= 1;
    a *= (float) (PI / 180.0f);
    float s = (float) sin(a);
    float c = (float) cos(a);
    if (1.0f == x && 0.0f == y && 0.0f == z) {
        rm[5] = c;   rm[10]= c;
        rm[6] = s;   rm[9] = -s;
        rm[1] = 0;   rm[2] = 0;
        rm[4] = 0;   rm[8] = 0;
        rm[0] = 1;
    } else if (0.0f == x && 1.0f == y && 0.0f == z) {
        rm[0] = c;   rm[10]= c;
        rm[8] = s;   rm[2] = -s;
        rm[1] = 0;   rm[4] = 0;
        rm[6] = 0;   rm[9] = 0;
        rm[5] = 1;
    } else if (0.0f == x && 0.0f == y && 1.0f == z) {
        rm[0] = c;   rm[5] = c;
        rm[1] = s;   rm[4] = -s;
        rm[2] = 0;   rm[6] = 0;
        rm[8] = 0;   rm[9] = 0;
        rm[10]= 1;
    } else {
        float len = length(x, y, z);
        if (1.0f != len) {
            float recipLen = 1.0f / len;
            x *= recipLen;
            y *= recipLen;
            z *= recipLen;
        }
        float nc = 1.0f - c;
        float xy = x * y;
        float yz = y * z;
        float zx = z * x;
        float xs = x * s;
        float ys = y * s;
        float zs = z * s;
        rm[ 0] = x*x*nc +  c;
        rm[ 4] =  xy*nc - zs;
        rm[ 8] =  zx*nc + ys;
        rm[ 1] =  xy*nc + zs;
        rm[ 5] = y*y*nc +  c;
        rm[ 9] =  yz*nc - xs;
        rm[ 2] =  zx*nc - ys;
        rm[ 6] =  yz*nc + xs;
        rm[10] = z*z*nc +  c;
    }
}

}
