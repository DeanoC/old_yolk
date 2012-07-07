//!-----------------------------------------------------
//!
//! \file dirwatch.cpp
//! functionality for monitoring directory and file
//! changes. So far windows only
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "dirwatch.h"
#include <process.h>


namespace Core
{
bool DirectoryWatcher::SafeParams::operator==( const DirectoryWatcher::SafeParams* b ) const
{
	return( (strcmp( this->pDirectory, b->pDirectory ) == 0) &&
			(this->bActive == b->bActive) && 
			(this->pThis == b->pThis) );
}

DirectoryWatcher::DirectoryWatcher() :
	m_pFileCallback(0)
{
}

DirectoryWatcher::~DirectoryWatcher()
{
	std::list<SafeParams*>::iterator diIt = m_Directories.begin();
	while( diIt != m_Directories.end() )
	{
		(*diIt)->bActive = false;
		++diIt;
	}
}


void DirectoryWatcher::AddFileChangeCallback( FileChange pCallback )
{
	m_pFileCallback = pCallback;
}

void DirWatchThread( void* arg )	
{
	DirectoryWatcher::SafeParams* pParams = (DirectoryWatcher::SafeParams*) arg;
	DirectoryWatcher *pThis = pParams->pThis;

	pParams->pThis->WatchThread( pParams );

	delete pParams; // parameters have been finished with
   _endthread();
}

void DirectoryWatcher::WatchThread( const SafeParams* params )
{
	HANDLE hHandle; 

	// Watch the directory for file creation and deletion. 
   hHandle = CreateFile( params->pDirectory,
							FILE_LIST_DIRECTORY,
							FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
							0 );
 
   if (hHandle == INVALID_HANDLE_VALUE) 
      ExitProcess(GetLastError()); 

   char Buffer[BUFFER_SIZE];
   DWORD BytesReturned;

   while(	params->bActive && 
			ReadDirectoryChangesW(	hHandle, 
									&Buffer, 
									BUFFER_SIZE, 
									TRUE,
									//FILE_NOTIFY_CHANGE_FILE_NAME | 
									FILE_NOTIFY_CHANGE_LAST_WRITE,
									//FILE_NOTIFY_CHANGE_CREATION, 
									&BytesReturned, 
									0, 0 ) )
	{
		FILE_NOTIFY_INFORMATION* pBuffer = (FILE_NOTIFY_INFORMATION*)&Buffer;

		while( pBuffer != 0 && params->bActive )
		{
			//nasty unicode to ansi conversion
			char strBuf[1024];
			for( unsigned int i=0;i < pBuffer->FileNameLength/2;i++)
			{
				strBuf[i] = (char) pBuffer->FileName[i];
			}
			strBuf[pBuffer->FileNameLength/2] = 0;

			switch(pBuffer->Action)
			{
			case FILE_ACTION_ADDED: 
				m_pFileCallback( FILE_ADDED, params->pDirectory, strBuf );
			break; 
			case FILE_ACTION_REMOVED: 
				m_pFileCallback( FILE_REMOVED, params->pDirectory, strBuf );
			break; 
			case FILE_ACTION_MODIFIED: 
				m_pFileCallback( FILE_MODIFIED, params->pDirectory, strBuf );
			break; 
			default:
			case FILE_ACTION_RENAMED_OLD_NAME: 
			case FILE_ACTION_RENAMED_NEW_NAME: 
			break;
			}

			int offset = pBuffer->NextEntryOffset;
			pBuffer = (FILE_NOTIFY_INFORMATION*) (offset != 0) ? (FILE_NOTIFY_INFORMATION*)(offset+(char*)pBuffer) : 0;
		}
	}

   CloseHandle( hHandle );

}

void DirectoryWatcher::AddDirectory( const char* pDirectory )
{
	CORE_ASSERT( strlen( pDirectory ) < 1023 );

	SafeParams* pParams = new SafeParams;
	pParams->pThis = this;
	strcpy( pParams->pDirectory, pDirectory );
	pParams->bActive = true;

	// one thread per directory
	if( std::find( m_Directories.begin(), m_Directories.end(), pParams ) == m_Directories.end() )
	{
		m_Directories.push_back( pParams );

		_beginthread( DirWatchThread, 0, pParams );
	} else
	{
		// already doing this directoty
		delete pParams;
	}
}

} //end namespace Core
