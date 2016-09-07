/*
 * GLRenderer.cpp
 *
 * Created on: 2016Äê7ÔÂ3ÈÕ
 *     Author: loushuai
 */

#include <stdlib.h>
#include "log.hpp"
#include "GLRenderer.hpp"

namespace whitebean
{
	
GLRenderer::GLRenderer()
{

}

GLRenderer::~GLRenderer()
{
		
}

GLuint GLRenderer::loadShader(GLenum type, const char *shaderSrc)
{
    GLuint shader;
    shader = glCreateShader(type);
    if (shader) {
        glShaderSource(shader,1, &shaderSrc, NULL);
        glCompileShader(shader);
        GLint compile;
        compile = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compile);
        if (!compile) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if(infoLen > 1)
            {
                char* infoLog = (char*)malloc(infoLen);

                glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
                LOGE( "Error compiling shader: %s", infoLog);
                free(infoLog);
            }

            glDeleteShader(shader);
            return 0;
        }
    } else {
        return 0;
    }
	
    return shader;
}

GLuint GLRenderer::createProgram(const char *vshaderSrc, const char *fshaderSrc)
{
	GLuint programObject;
	GLint  linked;	

	mGlVShader = loadShader(GL_VERTEX_SHADER, vshaderSrc);
	if (!mGlVShader) {
		LOGE("Load vertex shader failed");
		return 0;
	}

	mGlFShader = loadShader(GL_FRAGMENT_SHADER, fshaderSrc);
	if (!mGlFShader)
	{
		LOGE("Load fragment shader failed");
		return 0;
	}

	programObject = glCreateProgram();

	if (programObject == 0) {
		LOGE("Create program failed");
		return  0;		
	}

	glAttachShader(programObject, mGlVShader);
	glAttachShader(programObject, mGlFShader);

	glLinkProgram(programObject);

	glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
	if(!linked) {
		GLint infoLen;

		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
		LOGE("Linking length: %d", infoLen);

		if(infoLen > 1) {
			char* infoLog = (char*)malloc(infoLen);

			glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
			LOGE("Error linking program: %s, %s", infoLog, glGetString(glGetError()));

			free(infoLog);
		}

		glDeleteProgram(programObject);
		return 0;
	}

	return programObject;
}

void GLRenderer::initVertices()
{
	mVertices[0] = 1.0f;
	mVertices[1] = -1.0f;
	mVertices[2] = 1.0f;
	mVertices[3] = 1.0f;
	mVertices[4] = -1.0f;
	mVertices[5] = 1.0f;
	mVertices[6] = -1.0f;
	mVertices[7] = -1.0f;
}
	
int GLRenderer::loadVertices()
{
    GLubyte indices[] = {0, 1, 2, 2, 3, 0};
    GLuint a_position;

    glGenBuffers(1, &mVertexBuffer);
    a_position = glGetAttribLocation(mGlProgram, "a_position");
    glEnableVertexAttribArray(a_position);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &mIndicesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
			GL_STATIC_DRAW);

    return 0;
}

void GLRenderer::initTexCoords()
{
	mTexCoords[0] = 1.0f;
	mTexCoords[1] = 1.0f;
	mTexCoords[2] = 1.0f;
	mTexCoords[3] = 0.0f;
	mTexCoords[4] = 0.0f;
	mTexCoords[5] = 0.0f;
	mTexCoords[6] = 0.0f;
	mTexCoords[7] = 1.0f;	
}

void GLRenderer::cropTexCoords(GLfloat ratio)
{
	mTexCoords[0] = ratio;
	mTexCoords[1] = 1.0f;
	mTexCoords[2] = ratio;
	mTexCoords[3] = 0.0f;
	mTexCoords[4] = 0.0f;
	mTexCoords[5] = 0.0f;
	mTexCoords[6] = 0.0f;
	mTexCoords[7] = 1.0f;
}
int GLRenderer::loadTexCoords(int id)
{	
	return 0;
}

int GLRenderer::initTexture()
{	
	return 0;
}

int GLRenderer::loadTexture(GLFrame *pic)
{
	return 0;
}

int GLRenderer::getLineSize(GLFrame *pic)
{
	return 0;
}
	
int GLRenderer::prepare()
{
	return 0;
}
	
int GLRenderer::render(GLFrame *pic)
{
	return 0;
}
	
}
