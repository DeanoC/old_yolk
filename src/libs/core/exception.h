/** \file exception.h
 * Exception Header.
 * provides macros to enable easy Exception handling
 * (c) 2000 Dean Calver
 */

#pragma once

#ifndef WIERD_CORE_EXCEPTION_H
#define WIERD_CORE_EXCEPTION_H


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#if defined(USE_CPP_EXCEPTIONS)

/// begin a try/catch block
#	define CoreTry			try
/// catch x type of exception 
#	define CoreCatch(x)	catch(x)
/// catch our Exception class of type x
#	define CoreCatchException(x)	catch( x ## _Exception const& err)
/// catch any and all exception
#	define CoreCatchAll	catch( ... )
/// this functions makes no throws
#	define CoreNoThrows	throw()
/// throw an x type of exception
#	define CoreThrow(x)	throw(x)

/// throw our exception class of type x and with some extra text "" for none
#	define CoreThrowException(x,ExtraText) throw x ## _Exception( ExtraText, __FILE__, __LINE__ )

/// pass this exception back up the catch chain
#	define CoreReThrow		throw

#else

/// begin a try/catch block
#	define CoreTry			
/// catch x type of exception 
#	define CoreCatch(x)	if( false )
/// catch our Exception class of type x
#	define CoreCatchException(x)	if( false )
/// catch any and all exception
#	define CoreCatchAll	if( false )
/// this functions makes no throws
#	define CoreNoThrows	
/// throw an x type of exception
#	define CoreThrow(x)	assert( false && #x )

/// throw our exception class of type x and with some extra text "" for none
#	define CoreThrowException(x,ExtraText) assert( false && #x && #ExtraText )

/// pass this exception back up the catch chain
#	define CoreReThrow		

#endif

/// Our Exception class, has Name, description, UserText, file and line exception occured at
#define DECLARE_EXCEPTION(Name, Description) 						\
class Name ## _Exception : public Except	 						\
{								                					\
public:																\
	Name ## _Exception ( const Core::string& ExtraText, 				\
			const Core::string& file, unsigned int line ) 			\
		: Except(ExtraText, file, line ){};							\
	const Core::string getName( void ) const CoreNoThrows			\
		{ return Core::string( #Name " Exception" ); };				\
	const Core::string getDescription( void ) const CoreNoThrows		\
		{ return Core::string( #Description ); }; 					\
}

/// Log an exception in VC6 IDE format for quick finding (just double click on the log)
#define LOG_EXCEPTION(Ex)											\
		IF_DEBUG(													\
			LOG_FILE_LINE( Ex.getFile(), Ex.getLine() )				\
			Log << Ex.getDescription() << "\n";						\
			Log << Ex.getName();									\
			if(Ex.getUserText() != "")								\
				Log << " : " << Ex.getUserText();					\
			Log << "\n"												\
				);													

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------

/**
Base exception class.
Exception inherits off this to have a uniform way of specifying exception data.
*/
class Except
#if defined( USE_CPP_EXCEPTION )
: Core::exception 
#endif	     
{
public:
	Except()
		: UserText(),File(), Line(0){};
	Except(const Core::string UserData, const Core::string file, unsigned int line )
#if defined( USE_CPP_EXCEPTION )
     : Core::exception( UserData.c_str() ), UserText( UserData ), File(file), Line(line) 
#endif       
       {};

	virtual ~Except() CoreNoThrows {};
	//! text name of exception
	virtual const Core::string getName() const CoreNoThrows { return Core::string("Except"); };
	//! text description of exception
	virtual const Core::string getDescription() const CoreNoThrows { return Core::string("Unspecified exception occured"); };
	//! user text from description
	const Core::string getUserText() const CoreNoThrows { return UserText; };
	//! File exception was thrown from
	const Core::string getFile() const CoreNoThrows { return File; };
	//! Line exception was thrown from
	const unsigned int getLine() const CoreNoThrows { return Line; };

private:
	Core::string UserText;
	Core::string File;
	unsigned int Line;

};


// standard errors
DECLARE_EXCEPTION(Objdelete, Object was deleted before all releases);
DECLARE_EXCEPTION(PtrError, The pointer was incorrect);

DECLARE_EXCEPTION(MemError, Unable to allocate any system memory);

DECLARE_EXCEPTION(FileNotFoundError, Unable to find the file);
DECLARE_EXCEPTION(FileError, Generic File error);

DECLARE_EXCEPTION(TodoError, TODO Not yet implemented);
DECLARE_EXCEPTION(ParamError, Some parameter was incorrect);
DECLARE_EXCEPTION(BoundCheckFail, A boundary check has failed);
DECLARE_EXCEPTION(AssertError, Assert Failed);

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
