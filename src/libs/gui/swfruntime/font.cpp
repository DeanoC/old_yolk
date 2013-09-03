#include "swfruntime.h"
#include "shape.h"
#include "utils.h"
#include "font.h"

namespace Swf {
	
	Font::~Font() {
		for( auto i : glyphShapes ) {
			CORE_DELETE( i );
		}
	}
}
