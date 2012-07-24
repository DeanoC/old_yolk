#ifndef _YOLK_GL_RESOURCELOADER_H_
#define _YOLK_GL_RESOURCELOADER_H_ 1
#pragma once

namespace Gl {
// forward decl
class ResourceLoaderImpl;
class ImageComposer;

class ResourceLoader {
public:
	friend class Gfx;
	ResourceLoader();
	~ResourceLoader();

	void showLoadingIfNeeded( ImageComposer* composer );
protected:
	void installResourceTypes();
	ResourceLoaderImpl& impl;
};

}

#endif