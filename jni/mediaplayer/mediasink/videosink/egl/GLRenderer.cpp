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
: mVertexSize(2)
, mTexCoordSize(2)
, mVerticesPtr(nullptr)
, mTexCoordsPtr(nullptr)
, mIndicesPtr(nullptr)
, mVerticesNum(0)
, mTexCoordsNum(0)
, mIndicesNum(0)
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

}

int GLRenderer::loadVertices()
{
    GLuint a_position;

    glGenBuffers(1, &mVertexBuffer);
    a_position = glGetAttribLocation(mGlProgram, "a_position");
    glEnableVertexAttribArray(a_position);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, mVerticesNum * mVertexSize * sizeof(GLfloat), mVerticesPtr, GL_STATIC_DRAW);
    glVertexAttribPointer(a_position, mVertexSize, GL_FLOAT, GL_FALSE, 0, 0);

    if (mIndicesPtr) {
    	glGenBuffers(1, &mIndicesBuffer);
    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesBuffer);
    	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndicesNum * sizeof(GLubyte), mIndicesPtr,
    			GL_STATIC_DRAW);
    }

    return 0;
}

void GLRenderer::initTexCoords()
{

}

void GLRenderer::cropTexCoords(GLfloat ratio)
{

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
