
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tutil.h"
#include "crypt_aes.h"

const unsigned char *ddd = (unsigned char*)"12345678901234567890";

int main(int argc, char *argv[])
{
	int len = 0;
	unsigned char key[32];
	unsigned char *test_data = NULL;
	unsigned char *ciphertext = NULL;
	char *plaintext = NULL;
	EVP_CIPHER_CTX en, de;

	unsigned int salt[] = {12345, 54321};

	if (argc > 1) test_data = (unsigned char*)argv[1];
	else test_data = (unsigned char*)ddd;

	key[0] = 1;
	key[1] = 2;
	for (int i = 2; i < 32; i++) {
		key[i] = key[i-2] + key[i-1];	
	}

	if (aes_init(key, 32, (unsigned char *)&salt, &en, &de)) {        
		printf("Couldn't initialize AES cipher\n");
		return -1;
	}

	len = strlen((char*)test_data) + 1;
	fprintf(stdout, "before encrypt: [%s]\n", test_data);
	ciphertext = aes_encrypt(&en, (unsigned char *)test_data, &len);
	fprintf(stdout, "after encrypt: [%d]\n", len);
    plaintext = (char *)aes_decrypt(&de, ciphertext, &len);
	fprintf(stdout, "after decrypt: [%d]\n", len);
	fprintf(stdout, "after decrypt: [%s]\n", plaintext);

	free(ciphertext);
	free(plaintext);

	return 0;
}
