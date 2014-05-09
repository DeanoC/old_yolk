#include "core/core.h"
#include "core/coreresources.h"
#include "core/fileio.h"
#include "core/file_path.h"
#include "core/rapidjson/document.h"
#include <boost/algorithm/string.hpp> 
#include "tof.h"
#include "scene/generictextureformat.h"
#include "export/texexport.h"

void loadTao(	const Core::FilePath& inPath,
				std::vector<std::string>& outFilenames, 
				std::vector<Export::SubTexture>& outSprites) {

	Core::MemFile file;
	Core::FilePath tofPath = inPath.ReplaceExtension(".tao");
	bool ok = file.loadTextFile(tofPath.value().c_str());

	if (ok) {
		auto filestring = std::string((char*)file.takeBufferOwnership());

		int texWidth = 0;
		int texHeight = 0;

		using namespace rapidjson;
		Document doc;
		doc.Parse<0>(filestring.c_str());
		for (Value::ConstMemberIterator val = doc.MemberBegin();
			val != doc.MemberEnd();
			++val) {
			std::string attr = val->name.GetString();
			boost::algorithm::to_lower(attr);

			if (attr == "meta") {
				for (Value::ConstMemberIterator mval = val->value.MemberBegin();
					mval != val->value.MemberEnd();
					++mval) {
					std::string mattr = mval->name.GetString();
					boost::algorithm::to_lower(mattr);
					if (mattr == "image") {
						if (mval->value.IsString() == false) {
							continue;
						}
						std::string mvalstr = mval->value.GetString();
						outFilenames.push_back(mvalstr);
						continue;
					}
					if (mattr == "size") {
						for (Value::ConstMemberIterator sval = mval->value.MemberBegin();
							sval != mval->value.MemberEnd();
							++sval) {
							if (sval->value.IsInt() == false) {
								continue;
							}
							std::string sattr = sval->name.GetString();
							boost::algorithm::to_lower(sattr);
							if (sattr == "w") {
								texWidth = sval->value.GetInt();
								continue;
							}
							if (sattr == "h") {
								texHeight = sval->value.GetInt();
								continue;
							}
						}
					}
				}
				continue;
			}

			if (attr == "frames") {
				for (Value::ConstMemberIterator aval = val->value.MemberBegin();
					aval != val->value.MemberEnd();
					++aval) {
					Export::SubTexture st;
					st.index = 0; // texpacker only supports one texture so far
					bool rotated = false;
					for (Value::ConstMemberIterator fval = aval->value.MemberBegin();
						fval != aval->value.MemberEnd();
						++fval) {

						std::string fattr = fval->name.GetString();
						boost::algorithm::to_lower(fattr);
						if (fattr == "frame") {
							for (Value::ConstMemberIterator sval = fval->value.MemberBegin();
								sval != fval->value.MemberEnd();
								++sval) {
								if (sval->value.IsInt() == false) {
									continue;
								}
								std::string sattr = sval->name.GetString();
								boost::algorithm::to_lower(sattr);
								if (sattr == "x") {
									st.u0 = (float)sval->value.GetDouble();
									continue;
								}
								if (sattr == "y") {
									st.v0 = (float)sval->value.GetDouble();
									continue;
								}
								if (sattr == "w") {
									st.u1 = (float)sval->value.GetDouble();
									continue;
								}
								if (sattr == "h") {
									st.v1 = (float)sval->value.GetDouble();
									continue;
								}
							}
							continue;
						}
						if (fattr == "rotated") {
							if (fval->value.IsBool() == false) {
								continue;
							}
							rotated = fval->value.GetBool();
							continue;
						}
					}
					// we placed the w and h in u/v1 now to make them proper
					st.u1 += st.u0;
					st.v1 += st.v0;

					// now rotate if required
					if (rotated) {
						std::swap(st.u0, st.v0);
						std::swap(st.u1, st.v1);
					}
					// now push but still not quite ready
					outSprites.push_back(st);
					continue;
				}
			}
		}
		// now we need to convert into normalised coords
		for (auto it = outSprites.begin(); it != outSprites.end(); ++it) {
			(*it).u0 /= (float)texWidth;
			(*it).u1 /= (float)texWidth;
			(*it).v0 /= (float)texHeight;
			(*it).v1 /= (float)texHeight;
		}
	}
}