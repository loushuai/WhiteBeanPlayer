/*
 * GLRendererYUV420p.cpp
 *
 * Created on: 2016Äê7ÔÂ16ÈÕ
 *     Author: loushuai
 */

#include "log.hpp"
#include "GLRendererYUV420p.hpp"

namespace whitebean
{

GLRendererYUV420p::GLRendererYUV420p()
{
    mVertexScript = STRINGIZE(
        attribute vec4 a_position;
        attribute vec2 a_tex_coord_in_y;
		attribute vec2 a_tex_coord_in_u;
		attribute vec2 a_tex_coord_in_v;
        varying vec2 v_tex_coord_out_y;
		varying vec2 v_tex_coord_out_u;
		varying vec2 v_tex_coord_out_v;

        void main(void) {
            v_tex_coord_out_y = a_tex_coord_in_y;
			v_tex_coord_out_u = a_tex_coord_in_u;
			v_tex_coord_out_v = a_tex_coord_in_v;
            gl_Position = a_position;
        }
    );	
	
    mFragmentScript = STRINGIZE(
        precision highp float;
        varying highp vec2 v_tex_coord_out_y;
		varying highp vec2 v_tex_coord_out_u;
		varying highp vec2 v_tex_coord_out_v;
		uniform sampler2D u_texture_y;
		uniform sampler2D u_texture_u;
		uniform sampler2D u_texture_v;
        void main() {
			mat3 yuv2rgb = mat3(1, 0, 1.5958, 1, -0.39173, -0.81290, 1, 2.017, 0);
			vec3 yuv = vec3(1.1643 * (texture2D(u_texture_y, v_tex_coord_out_y).r - 0.0625),
							texture2D(u_texture_u,v_tex_coord_out_u).r - 0.5,
							texture2D(u_texture_v,v_tex_coord_out_v).r - 0.5);
			vec3 rgb = yuv * yuv2rgb;
			gl_FragColor = vec4(rgb, 1.0);
        }
    );
	
	mGlProgram = createProgram(mVertexScript, mFragmentScript);
}

int GLRendererYUV420p::initTexture()
{
	mSamplers[0] = glGetUniformLocation(mGlProgram, "u_texture_y");
	mSamplers[1] = glGetUniformLocation(mGlProgram, "u_texture_u");
	mSamplers[2] = glGetUniformLocation(mGlProgram, "u_texture_v");	
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_TEXTURE_2D);

	glUseProgram(mGlProgram);

	glGenTextures(3, mTextures);

	for (int i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, mTextures[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glUniform1i(mSamplers[i], i);
	}	
	
	return 0;
}

int GLRendererYUV420p::loadTexCoords(int id)
{
	GLfloat coords[8];
    GLuint a_tex_coord_in;
	const char *p = NULL;

	switch (id) {
	case 0:
		p = "a_tex_coord_in_y";
		break;
	case 1:
	    p = "a_tex_coord_in_u";
		break;
	case 2:
		p = "a_tex_coord_in_v";
		break;
	default:
		return -1;
	}

    glGenBuffers(1, &mTexCoordBuffer[id]);	
    a_tex_coord_in = glGetAttribLocation(mGlProgram, p);
    glEnableVertexAttribArray(a_tex_coord_in);
    glBindBuffer(GL_ARRAY_BUFFER, mTexCoordBuffer[id]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mTexCoords), mTexCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(a_tex_coord_in, 2, GL_FLOAT, GL_TRUE, 0, 0);
	
	return 0;
}	

int GLRendererYUV420p::loadTexture(GLFrame *pic)
{
    const GLsizei widths[3]  = {pic->pitches[0], pic->pitches[1], pic->pitches[2]};
	const GLsizei heights[3] = {pic->height, pic->height/2, pic->height/2};
	const GLubyte *pixels[3] = {pic->pixels[0], pic->pixels[1], pic->pixels[2]};

	if (!pixels[0] || !pixels[1] || !pixels[2]) {
		LOGE("Invalid data");
		return -1;
	}
		
	for (int i = 0; i < 3; ++i) {
		glBindTexture(GL_TEXTURE_2D, mTextures[i]);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_LUMINANCE,
                     widths[i],
                     heights[i],
                     0,
                     GL_LUMINANCE,
                     GL_UNSIGNED_BYTE,
                     pixels[i]);		
	}
	
	return 0;
}

int GLRendererYUV420p::prepare()
{
	mGlProgram = createProgram(mVertexScript, mFragmentScript);
	if (!mGlProgram) {
		LOGE("Create program failed");
		return -1;
	}

	initVertices();
	loadVertices();

	for (int i = 0; i < GLES2_MAX_PLANE; ++i) {
		initTexCoords();
		loadTexCoords(i);
	}

	initTexture();

	return 0;
}	

int GLRendererYUV420p::getLineSize(GLFrame *pic)
{
	if (!pic) {
		return 0;
	}

	return pic->pitches[0];
}

int GLRendererYUV420p::render(GLFrame *pic)
{
	int bufLineSize[GLES2_MAX_PLANE] = {0};
	int planes = 0;
	
	glClear(GL_COLOR_BUFFER_BIT);

	if (!pic) {
		return -1;
	}

	loadTexture(pic);

	GLfloat ratio = (GLfloat)pic->width / pic->getLineSize(0);
	cropTexCoords(ratio);
	loadTexCoords(0);

	ratio = (GLfloat)pic->width / 2 / pic->getLineSize(1);
	cropTexCoords(ratio);
	loadTexCoords(1);

	ratio = (GLfloat)pic->width / 2 / pic->getLineSize(2);
	cropTexCoords(ratio);
	loadTexCoords(2);	
	
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

	return 0;
}	
	
}
