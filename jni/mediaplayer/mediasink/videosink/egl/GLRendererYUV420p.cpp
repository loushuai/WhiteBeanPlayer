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
    mFragmentScript = STRINGIZE(
        precision highp float;
        varying highp vec2 v_tex_coord_out;
		uniform sampler2D u_texture_y;
		uniform sampler2D u_texture_u;
		uniform sampler2D u_texture_v;
        void main() {
			mat3 yuv2rgb = mat3(1, 0, 1.5958, 1, -0.39173, -0.81290, 1, 2.017, 0);
			vec3 yuv = vec3(1.1643 * (texture2D(u_texture_y, v_tex_coord_out).r - 0.0625),
							texture2D(u_texture_u,v_tex_coord_out).r - 0.5,
							texture2D(u_texture_v,v_tex_coord_out).r - 0.5);
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

int GLRendererYUV420p::getLineSize(GLFrame *pic)
{
	if (!pic) {
		return 0;
	}

	return pic->pitches[0];
}
	
}
