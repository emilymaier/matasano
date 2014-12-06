void rijndael_encrypt_ecb(unsigned char* data, size_t size, unsigned char* key);
void rijndael_decrypt_ecb(unsigned char* data, size_t size, unsigned char* key);
void rijndael_encrypt_cbc(unsigned char* data, size_t size, unsigned char* key, unsigned char* iv);
void rijndael_decrypt_cbc(unsigned char* data, size_t size, unsigned char* key, unsigned char* iv);
