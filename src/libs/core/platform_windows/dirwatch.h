//!-----------------------------------------------------
//!
//! \file dirwatch.h
//! functionality for monitoring directory and file
//! changes. So far windows only
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_DIRWATCH_H
#define WIERD_CORE_DIRWATCH_H


#if PLATFORM != WINDOWS
#	error Currently supported on windows only
#endif


#include <list>



namespace Core
{


class DirectoryWatcher : public Singleton<DirectoryWatcher>
{
public:
	enum CHANGE_REQUEST
	{
		FILE_ADDED,
		FILE_REMOVED,
		FILE_MODIFIED,
	};


public:
	DirectoryWatcher();
	~DirectoryWatcher();

	typedef void (*FileChange)(CHANGE_REQUEST change, const char* pDirectory, const char* pName );

	void AddFileChangeCallback( FileChange pCallback );

	void AddDirectory( const char* pDirectory );


private:
	friend void DirWatchThread( void* arg );

	static const int BUFFER_SIZE = 1024;

	struct SafeParams
	{
		Core::DirectoryWatcher* pThis;
		char pDirectory[ 1024 ]; // MAX length of directory name
		bool bActive;
		bool operator==( const SafeParams* b ) const;
	};


private:
	FileChange m_pFileCallback;

	void WatchThread(const SafeParams* params );

	Core::list<SafeParams*>	m_Directories;
};


}	//namespace Core


#endif
