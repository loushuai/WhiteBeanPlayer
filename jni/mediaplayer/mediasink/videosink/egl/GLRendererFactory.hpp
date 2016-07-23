/*
 * GLRendererFactory.hpp
 *
 * Created on: 2016Äê7ÔÂ17ÈÕ
 *     Author: loushuai
 */

#ifndef JNI_MEDIASINK_VIDEOSINK_EGL_GLRENDERERFACTORY_H_
#define JNI_MEDIASINK_VIDEOSINK_EGL_GLRENDERERFACTORY_H_

#include <memory>
#include "GLRendererYUV420p.hpp"

namespace whitebean
{

enum {
	GL_RENDERER_YUV420P,
};

class GLRendererFactory
{
public:
	static std::shared_ptr<GLRenderer> create(int type)
	{
		std::shared_ptr<GLRenderer> ret;
		
		switch(type) {
		case GL_RENDERER_YUV420P:
			ret = std::shared_ptr<GLRenderer>(new GLRendererYUV420p);
			break;
		default:

			break;
		}

		return ret;
	}
};
	
}

#endif
