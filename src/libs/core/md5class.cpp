// md5class.cpp: implementation of the CMD5 class.
//See internet RFC 1321, "The MD5 Message-Digest Algorithm"
//
//Use this code as you see fit. It is provided "as is"
//without express or implied warranty of any kind.

//////////////////////////////////////////////////////////////////////

#include "core/core.h"
#include <string.h>	  
#include <stdio.h>
#include "md5class.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "md5_global.h"
typedef struct {
	UINT4 state[4];                                   /* state (ABCD) */
	UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;
extern "C" 
{
	void MD5Init(MD5_CTX *);
	void MD5Update(MD5_CTX *,  const unsigned char *, size_t);
	void MD5Final(unsigned char [16], MD5_CTX *);
}

namespace Core // Deano added to Core namespace
{

MD5::MD5( const char* plainText) {	
	calcDigest( plainText );
}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

std::string MD5::getMD5String() const {
	char digestString[33];
	//make a string version of the numeric digest value
	int p=0;
	for (int i = 0; i<16; i++) {
		::sprintf(&digestString[p],"%02x", m_digest[i]);
		p+=2;
	}
	digestString[32] = 0;
 
	return std::string( digestString );
}

void MD5::calcDigest( const char* _plainText ) {
	//See RFC 1321 for details on how MD5Init, MD5Update, and MD5Final 
	//calculate a digest for the plain text
	MD5_CTX context;
	MD5Init(&context); 

	//the alternative to these ugly casts is to go into the RFC code and change the declarations
	MD5Update(&context, reinterpret_cast<const unsigned char *>(_plainText), ::strlen(_plainText));
	MD5Final(reinterpret_cast <unsigned char *>(m_digest),&context);
}


} // end namespace Core
