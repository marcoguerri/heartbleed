#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
int main() {
    FILE *fp;
    RSA *pk = NULL;
    char *q_hex;
    BN_CTX *temp;
    BIGNUM *result;
    int ret;
    FILE *fp_out;


    temp = BN_CTX_new();
    result = BN_new();
    fp = fopen("private_unencrypted.pem", "r");
    pk = PEM_read_RSAPrivateKey(fp,NULL,NULL,NULL);
    if(pk == NULL) {
        printf("Error while reading the private key");
        return 1;
    }
    ret = BN_mul(result,pk->q, pk->p, temp);
    printf("RET IS %d\n", ret);
    printf("p is: %s\n", BN_bn2hex(pk->p));    
    printf("q is: %s\n", BN_bn2hex(pk->q));    
    printf("result is %s\n", BN_bn2hex(result));
    printf("N is %s\n", BN_bn2hex(pk->n));
    fp_out = fopen("key_dumped", "w");
    BN_print_fp(fp_out,  pk->q);
    fclose(fp_out); 
}
