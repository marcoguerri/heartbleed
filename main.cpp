#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <string>
#include <iostream>

using namespace std;

#define BIT_KEY 128
#define SIZE_DUMP 334

int main() {

    FILE *fp_pk, *fp_bleed;
    RSA *pk = NULL;
    BN_CTX *temp;
    BIGNUM *pk_q_test, *reminder;
    unsigned int cycle, num_read, i;
    char *buff;
    
    temp = BN_CTX_new();
    pk_q_test = BN_new();
    reminder = BN_new();
    //
    // Extracting info from private key pem file
    //
    fp_pk = fopen("private_unencrypted.pem", "rb");
    pk = PEM_read_RSAPrivateKey(fp_pk,NULL,NULL,NULL);
    if(pk == NULL) {
       printf("Error while reading the private key");
       return 1;
    }

    buff = (char *) malloc(SIZE_DUMP*sizeof(char));
    fp_bleed = fopen("key_dumped_scrambled", "r");
    cycle = 0;
    num_read = fread((void*) buff, SIZE_DUMP, 1, fp_bleed);

    for(i=0; i<SIZE_DUMP-BIT_KEY+1; i++) {
        pk_q_test = BN_new();
        BN_bin2bn((unsigned char*)buff+cycle ,BIT_KEY, pk_q_test);
        BN_mod(reminder, pk->n, pk_q_test, temp);
        if(BN_is_zero(reminder)) {
            printf("I FOUND THE KEYYYYYY!\n");
            printf("%s\n",BN_bn2hex(pk_q_test));
            return 0;
        }
        cycle++;
    }
    return 0;
   
} 
