/***************************************************************************

This class is a utility wrapper for 'C' code contained in internet RFC 1321, 
"The MD5 Message-Digest Algorithm".

It calculates a cryptological hash value, called a "digest" from a character 
string.  For every unique character string the MD5 hash is guaranteed to be 
unique.  The MD5 hash has the property that given the digest, it's 
thought to be impossible to get back to the plain text string with existing 
technology. In this implementation the digest is always a 32 digit hex number,
regardless of the length of the input plaintext.

This class is helpful for programs which store passwords.  Rather than storing 
the password directly, the programmer should store the MD5 digest of the password. 
Then when the user enters a password, compute the MD5 digest of the input password.
If it is identical to the stored digest, then the user
has entered the correct password.  It doesn't matter if an evil person sees the 
digest, since he or she can't get from the digest to the password. At least not 
unless the user enters a word out of the dictionary, since the evil person could 
hash the whole dictionary.  One way to defeat a dictionary attack is to append 
a non-text character onto the password, so that even if the user enters a dumb 
password like "password", you just append some non alpha character to the entered
password, i.e. password = "password" + "$".  By always appending a nonalpha 
character, your stored digest isn't in the attacker's dictionary. You can 
then safely post the digest of the password on a highway billboard.
    
Example pseudocode:
{
std::string storedPasswordDigest = GetPasswordDigestFromStorage();
std::string passwordEnteredbyUser;
cout << "Enter password:" ;
cin >> passwordEnteredbyUser;
	
CMD5 md5(passwordEnteredbyUser.c_str()); //note c_str() returns a pointer to the std::string's character buffer, just like CString's "GetBuffer" member function.
	
if(md5.getMD5Digest != storedPasswordDigest)
{
	//user has entered an invalid password
	cout << "Incorrect password!";
	exit(1);
}
	
//if we get here, then the user entered a valid password
}
**************************************************************************
Use this code as you see fit. It is provided "as is"
without express or implied warranty of any kind.
  
Jim Howard, jnhtx@jump.net
***************************************************************************/

#pragma once

#ifndef AFX_MD51_H__2A1EA377_D065_11D4_A8C8_0050DAC6D85C__INCLUDED_
#define AFX_MD51_H__2A1EA377_D065_11D4_A8C8_0050DAC6D85C__INCLUDED_


namespace Core // Deano placed in Core namespace
{


class MD5  
{
public:
	MD5( const char* plainText );  //set plaintext in ctor

	std::string getMD5String() const; 
	const uint8_t* getDigest() const { return m_digest; }
	void copyDigest( uint8_t* _digest ) const { memcpy( _digest, m_digest, 16 ); }

private:
	void calcDigest( const char* _plainText ); //this function computes the digest by calling the RFC 1321 'C' code
	uint8_t m_digest[16]; //the numerical value of the digest

};


}	//namespace Core


#endif
