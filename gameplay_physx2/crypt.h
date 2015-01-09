
#ifndef CRYPTOGRAPHY_INCLUDED
#define CRYPTOGRAPHY_INCLUDED

void encrypt(std::string& out, void* in, unsigned int insize, const char* key);
bool decrypt(void* out, unsigned int outsize, std::string& in, const char* key);
int checksum(void* in, unsigned int insize);

#endif