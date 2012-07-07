#ifndef _YOLK_GL_RESOURCELOADER_H_
#define _YOLK_GL_RESOURCELOADER_H_ 1
#pragma once

namespace Gl {
// forward decl
class ResourceLoaderImpl;

class ResourceLoader {
public:
	friend class Gfx;
	ResourceLoader();
	~ResourceLoader();


protected:
	void installResourceTypes();
	ResourceLoaderImpl& impl;
};

}

#endif