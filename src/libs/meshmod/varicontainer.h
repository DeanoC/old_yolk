#pragma once
/** \file MeshModVariContainer.h
   Used by the mesh system for holding variable amounts of vectors (vertex, faces).
   (c) 2002 Dean Calver
 */

#if !defined( MESH_MOD_VARICONTAINER_H_ )
#define MESH_MOD_VARICONTAINER_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod {

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------

	/**
	Short description.
	Detailed description
	*/
	template<typename T>
	class Elements {
	public:
		virtual ~Elements() {};

		// name of this type of element ("position", "UV2D", "Colour", etc)
		const std::string name;
		// subname. changeable for multiple sets of m_name's ("worldspace", "Tex0",etc)
		std::string subName;
		virtual void cloneElement( const size_t index ) = 0;
		virtual void resize( const size_t size ) = 0;
		virtual size_t size() const = 0;

		virtual Elements<T>* clone() const = 0;
	protected:
		Elements( const std::string& _name)
			:  name(_name) {};
		Elements& operator=( const Elements& );
	 	 
	};

	template< typename T, typename Type, int reserveAmnt = 1024>
	class ImplElements : public Elements<Type> {
	public:
		typedef T			DataType;
		typedef Type		ContainerType;
		typedef typename std::vector<DataType>::iterator iterator;
		typedef typename std::vector<DataType>::const_iterator const_iterator;

		std::vector<DataType>	elements;

		ImplElements() : 
			Elements<ContainerType>( DataType::getName() ) {
			elements.reserve( reserveAmnt );
		}

		static const std::string getName() {
			return DataType::getName();
		}

		const DataType& getElement(const size_t i) const { return elements[i]; };
		DataType& getElement(const size_t i) { return elements[i]; };

		const DataType& operator[](const size_t i) const { return getElement(i); };
		DataType& operator[](const size_t i) { return getElement(i); };

		iterator begin() { return elements.begin(); }
		iterator end() { return elements.end(); }
		const_iterator cbegin() { return elements.cbegin(); }
		const_iterator cend() { return elements.cend(); }

		void push_back( const DataType& data ) {
			elements.push_back( data );
		};

		template< typename IndexType>
		IndexType distance( iterator it ) { 
			return (IndexType) std::distance( elements.begin(), it );
		}

		template< typename IndexType>
		const IndexType distance( const_iterator it ) const { 
			return (IndexType) std::distance( elements.begin(), it );
		}

		virtual void cloneElement( const size_t index ) {
			elements.push_back( elements[index] );
		};

		virtual void resize( const size_t size ) {
			elements.resize( size );
		};
		virtual size_t size() const {
			return elements.size();
		}

		virtual Elements<Type>* clone() const {
			return CORE_NEW ImplElements<T,Type,reserveAmnt>( *this );
		}

		template< typename INTER_TYPE >
		DataType interpolate( const size_t i0, const size_t i1, INTER_TYPE t ) {
			return elements[i0].interpolate( elements[i1], t );
		}
		template< typename INTER_TYPE >
		DataType interpolate( const size_t i0, const size_t i1, const size_t i2, INTER_TYPE u, INTER_TYPE v ) {
			return elements[i0].interpolate( elements[i1], elements[i2], u, v );
		}
	};

	template< typename T >
	class VariContainer {
	public:
		~VariContainer();
		void resize(const size_t size);
		size_t size() const;

		void cloneTo( VariContainer<T>& nvc ) const;

	private:
		//! add the element with default subname
		T* addElements( T* face, const std::string& subName = std::string() );

		//! gets the first element named name (ther may be others with different subnames)
		T* getElements( const std::string& name  );
		//! gets the first element named name (ther may be others with different subnames) (const)
		const T* getElements( const std::string& name ) const;

		//! gets all elements of the input name
		void getAllElementsNamed( const std::string& name, std::vector<T*>& out );
		//! gets all elements of the input name (const)
		void getAllElementsNamed( const std::string& name, std::vector<const T*>& out ) const;

		// gets the element of name and sub name
		T* getElementsNameAndSubName( const std::string& name, const std::string& subName );
		// gets the element of name and sub name (const_
		const T* getElementsNameAndSubName( const std::string& name, const std::string& subName ) const;

	public:
		// how many elements do we have.
		size_t getElementCount() const {
			return elements.size();
		}


		template< typename Type>
		Type* addElements( const std::string& subName = std::string() ) {
			Type* ele = (Type*) addElements( CORE_NEW Type() );
			CORE_ASSERT( ele != 0);
			return ele;
		}

		template< typename Type>
		Type* getOrAddElements( const std::string& subName = std::string() ) {
			Type* ele;
			if( !subName.empty() ) {
				ele = (Type*) getElementsNameAndSubName( Type::getName(), subName );
				if( ele == 0 ) {
					ele = (Type*) addElements( CORE_NEW Type(), subName );
				}
			} else {
				ele = (Type*) getElements( Type::getName() );
				if( ele == 0 ) {
					ele = (Type*) addElements( CORE_NEW Type() );
				}
			}

			CORE_ASSERT( ele != 0);
			return ele;
		}

		template< typename Type>
		void pushBack( const std::string& subName = std::string(), const typename Type::DataType& data = typename Type::DataType() ) {
			// if we adding a new type we don't wanna always resize the buffer after the add
			// cos adding a new type resizes it to the size of the array. This encapsulates
			// the slight complicated and quite frankly odd logic.. it basically does
			// what I expect by pushBack
			if( getElementsNameAndSubName( Type::getName(), subName ) == 0 ) {
				Type* pType = getOrAddElements<Type>( subName );
				if( pType->size() == 0 ) {
					pType->push_back( data );
				} else {
					pType->getElement(0) = data;
				}
			} else {
				Type* pType = getElements<Type>( subName );
				pType->push_back( data );
			}
		}

		template< typename Type>
		Type* getElements() {
			return static_cast<Type*>( getElements( Type::DataType::getName() ) );
		}

		template< typename Type>
		const Type* getElements() const {
			return static_cast<const Type*>( getElements( Type::DataType::getName() ) );
		}

		template< typename Type>
		Type* getElements( const std::string& subName ) {
			return static_cast<Type*>( getElementsNameAndSubName( Type::DataType::getName() ,subName) );
		}
		template< typename Type>
		const Type* getElements( const std::string& subName ) const {
			return static_cast<const Type*>( getElementsNameAndSubName( Type::DataType::getName() ,subName) );
		}

		//! gets all elements of the types name
		template< typename Type>
		void getAllElements( std::vector<Type*>& out ) {
			typename std::vector<T*>::iterator feIt = elements.begin();		
			while( feIt != elements.end() ) {
				if( (*feIt)->name == Type::DataType::getName() ) {
					out.push_back( (Type*) (*feIt) );
				}
				++feIt;
			}
		}

		//! gets all elements of the types name (const)
		template< typename Type>
		void getAllElements( std::vector<const Type*>& out ) const {
			typename std::vector<T*>::const_iterator feIt = elements.begin();		
			while( feIt != elements.end() ) {
				if( (*feIt)->name == Type::DataType::getName() ) {
					out.push_back( (const Type*) (*feIt) );
				}
				++feIt;
			}
		}

		size_t cloneElement( const size_t elementToCopy );

	private:
		// elements
		std::vector<T*> elements;
	};

	template<typename T>
	inline VariContainer<T>::~VariContainer() {
		typename std::vector<T*>::iterator veIt = elements.begin();

		while( veIt != elements.end() ) {
			CORE_DELETE *(veIt);
			++veIt;
		}

		elements.clear();
	}

	template<typename T>
	inline size_t VariContainer<T>::size() const {
		if( elements.empty() ) {
			CORE_ASSERT( false );
			return 0;
		}

		return elements[0]->size();
	}

	template<typename T>
	inline void VariContainer<T>::resize(const size_t size )
	{
		if( elements.empty() ) {
			CORE_ASSERT( false );
		}

		typename std::vector<T*>::iterator veIt = elements.begin();
		while( veIt != elements.end() ) {
			(*veIt)->resize( size );
			++veIt;
		}
	}


	template<typename T>
	inline T* VariContainer<T>::addElements( T* face, const std::string& subName  ) {
		elements.push_back( face );
		face->resize( elements[0]->size() );
		face->subName = subName;
		return face;
	}

	template<typename T>
	inline T* VariContainer<T>::getElements( const std::string& name ) {
		typename std::vector<T*>::iterator feIt = elements.begin();
		
		while( feIt != elements.end() ) {
			if( (*feIt)->name == name ) {
				return (*feIt);
			}
			++feIt;
		}

		return 0;
	}

	template<typename T>
	inline const T* VariContainer<T>::getElements( const std::string& name ) const {
		typename std::vector<T*>::const_iterator feIt = elements.begin();
		
		while( feIt != elements.end() ) {
			if( (*feIt)->name == name ) {
				return (*feIt);
			}
			++feIt;
		}

		return 0;
	}


	template<typename T>
	inline void VariContainer<T>::getAllElementsNamed( const std::string& name, std::vector<T*>& out ) {
		typename std::vector<T*>::iterator feIt = elements.begin();
		
		while( feIt != elements.end() ) {
			if( (*feIt)->name == name ) {
				out.push_back( (*feIt) );
			}
			++feIt;
		}
	}

	template<typename T>
	inline void VariContainer<T>::getAllElementsNamed( const std::string& name, std::vector<const T*>& out ) const {
		typename std::vector<T*>::const_iterator feIt = elements.begin();
		
		while( feIt != elements.end() ) {
			if( (*feIt)->name == name ) {
				out.push_back( (*feIt) );
			}
			++feIt;
		}
	}

	template<typename T>
	inline T* VariContainer<T>::getElementsNameAndSubName( const std::string& name, const std::string& subname ) {
		typename std::vector<T*>::iterator feIt = elements.begin();
		
		while( feIt != elements.end() ) {
			if( ((*feIt)->name == name ) &&
				((*feIt)->subName == subname) ) {
				return (*feIt);
			}
			++feIt;
		}

		return 0;
	}

	template<typename T>
	inline const T* VariContainer<T>::getElementsNameAndSubName( const std::string& name, const std::string& subname ) const {
		typename std::vector<T*>::const_iterator feIt = elements.begin();
		
		while( feIt != elements.end() ) {
			if( ((*feIt)->name == name ) &&
				((*feIt)->subName == subname) ) {
				return (*feIt);
			}
			++feIt;
		}

		return 0;
	}

	template<typename T>
	inline size_t VariContainer<T>::cloneElement(const size_t elementToCopy ) {
		typename std::vector<T*>::iterator feIt = elements.begin();
		
		while( feIt != elements.end() ) {
			(*feIt)->cloneElement( elementToCopy );
			++feIt;
		}

		return size() - 1;
	}
	template<typename T>
	inline void VariContainer<T>::cloneTo( VariContainer<T>& nvc ) const {
		typename std::vector<T*>::const_iterator feIt = elements.cbegin();

		while( feIt != elements.cend() ) {
			nvc.elements.push_back( (*feIt)->clone() );
			++feIt;
		}
	}

} // end namespace

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Externals
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// End Header file
//---------------------------------------------------------------------------
#endif
