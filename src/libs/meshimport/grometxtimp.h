#ifndef _YOLK_MESHIMPORT_GROMETXTIMP_H_
#define _YOLK_MESHIMPORT_GROMETXTIMP_H_ 1

namespace MeshImport {

	class GromeTxtVariable {
	public:
		GromeTxtVariable( const std::string& _name, const boost::any& _any ) {
			name = _name;
			value = _any;
		}

		std::string name;
		boost::any 	value;
	};

	typedef std::vector<GromeTxtVariable> GromeTxtObject;

	/**
	Short description.
	Detailed description
	*/
	class GromeTxtImp : public ImportInterface {
	public:
		GromeTxtImp( const std::string& filename  );
		virtual ~GromeTxtImp();

		virtual bool loadedOk() const override { return loadedOkay; }
		virtual MeshMod::ScenePtr toMeshMod() override { return MeshMod::ScenePtr(); }

		struct ObjectLookup {
			const char* text;
			void (GromeTxtImp::*func)( FILE *f );
		};
	public:
		void skipKey( FILE *f );
		void defaultVariableKey( FILE *f );
		void defaultObjectKey( FILE* f );

		void nameVarKey( FILE* f );

		void terrainZoneObjectKey( FILE* f );
		void heightfieldVariableKey( FILE* f );

	private:
		bool loadedOkay;

		void readLine( FILE* F);
		void objectReader( FILE *f, const ObjectLookup* keyArray );
		const ObjectLookup& findKey(const std::string& text, const ObjectLookup* keyArray ) const;

		// grome store some *HUGE* data on a single text line, so to make it simple
		// we keep even huger line buffers, twice... but still drop in the ocean of modern
		// memory sizes
		static const int LINE_BUFFER_SIZE = 200 * 1024 * 1024;

		unsigned int					lineNum;
		char*							lineBuffer;
		bool							ungetLastLine;
		char*							lastLineBuffer;
		std::stack<GromeTxtObject*>		objStack;
		std::vector<std::shared_ptr<GromeTxtObject>> 	objects;

	};
}

#endif