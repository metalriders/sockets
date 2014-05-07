#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "server.h"
#include "tcp.h"
#include "debug.h"
#include "global.h"
#include "errno.h"

//SSL
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <resolv.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

#define FAIL    -1

void clientProccess(SSL_CTX *ctx, const int clientSocket);
SSL_CTX* InitServerCTX(void);
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile);
void ShowCerts(SSL* ssl);

int startServer(const unsigned int port) {

	int serverSocket;
	int clientSocket;
	char clientIP[16];
	unsigned int clientPort;
	SSL_CTX *ctx;
	int pid;
	
	//Init SSL library
	SSL_library_init();

	//Init SSL socket
	ctx = InitServerCTX();

	//Load certificates
	LoadCertificates(ctx, "Certificates/cert.pem", "Certificates/key.pem");

	
	serverSocket = newTCPServerSocket4("0.0.0.0",port,5);
	if(serverSocket == -1) {
		error(errno,"Can't create Socket");
		return FALSE;
	}
	debug(4,"Server Socket Created");

	while(TRUE) {
		clientSocket = waitConnection4(serverSocket,clientIP,&clientPort);
		if(clientSocket == -1) {                        
		    debug(1,"%s","ERROR: Cannot get Client Socket");
		    continue;    
		}

		debug(2,"Connected Client %s:%u",clientIP,clientPort);
		
		pid = fork();
		if(pid == -1) {
			debug(0,"Can't Fork!");
			close(clientSocket);
			continue;
		} else if(pid == 0 ) {			//Create secure connection
			// Soy el Hijo.
			clientProccess(ctx, clientSocket);
		} else if (pid > 0 ) {
			// Soy el Padre
			close(clientSocket);
		}
    	}

	return TRUE;
	
}


void clientProccess(SSL_CTX *ctx, const int clientSocket) {
	
	char *buffer;
	char *firstLine, *host, *usr_agent, *content;		//REQUEST HEADERS
	char html[250];
	int file;
	int readBytes;
	int firstFlag;
	int tmp_size;
	
	char *type;
	char *request;
	char *protocol;
	char *pch;
	char *size;
	
	SSL *ssl;
	int sd;

	ssl = SSL_new(ctx); 				/* get new SSL state with context */
	SSL_set_fd(ssl, clientSocket);			/* set connection socket to SSL state */
	
	if ( SSL_accept(ssl) == FAIL ){     		/* do SSL-protocol accept */
		ERR_print_errors_fp(stderr);
		return;
	}
	
	ShowCerts(ssl);        				/* get any certificates */

	debug(2,"Inicio del proceso del Cliente\n");

	type = calloc(255,1);
	request = calloc(255,1);
	protocol = calloc(255,1);

	buffer = calloc(255,1);

	firstLine = calloc(255,1);
	host = calloc(255,1);
	usr_agent = calloc(255,1);
	content = calloc(255,1);

	firstFlag = 1;
	while(SSL_read(ssl,buffer,254)>0) {		/* get request */
		debug(4,"%s",buffer);
		if(strcmp(buffer,"\r\n")==0) {
			break;
		}

		switch(firstFlag){				
			case 1: strcpy(firstLine, buffer);	//Catch request method from client
				firstFlag++;
				break;
			case 2: strcpy(host, buffer);		//Catch host
				firstFlag++;
				break;
			case 3: strcpy(usr_agent, buffer);	//Catch user agent from client
				firstFlag++;
				break;
			case 4: strcpy(content, buffer);	//Catch all content-type used
				firstFlag++;
				break;
			default: break;
		}
		bzero(buffer,255);
	}	
	
	//REQUEST HEADERS
	debug(4,"%s",firstLine);
	debug(4,"%s",host);
	debug(4,"%s",usr_agent);
	debug(4,"%s",content);

	// PROCESAR EL GET
	debug(4,"First Line %s",firstLine);
	pch = strtok (firstLine," /\r\n");
	while (pch != NULL)
	{
		if(strlen(type) == 0) 			//Catch type of request
			strcpy(type, pch);
		else if(strlen(request) == 0)		//Catch URI 
			strcpy(request, pch);	
		else if(strlen(protocol) == 0)		//Catch protocol
			strcpy(protocol, pch);
		pch = strtok(NULL, " \r\n");

	}
	*request++;

	debug(4,"REQUEST %s", request);

	//RESPONDER CON 200 OK  SI EXISTE EL REQUEST
	if(strlen(request) == 0){
		sendStatus(clientSocket, "HTTP/1.1 200 OK");
	} else{
		file = open(request, O_RDONLY);			

		if(file == -1) {
			debug(4,"Error while open file %s", request);
			sendStatus(clientSocket, "HTTP/1.1 404 Not Found");
			strcpy(html, "<html><head><title>404 NOT FOUND</title></head><body>NO EXISTE!!!</body></html>\r\n");			
			//sendTCPLine4(clientSocket, html,strlen(html));
			SSL_write(ssl, html, strlen(html));

			error(errno, "No se pudo abrir el archivo");
		}else{
			debug(4,"Preparing to send file %s", request);

			sendStatus(clientSocket, "HTTP/1.1 200 OK");
			while((readBytes = read(file,buffer,255))>0) {		//Send file to client
				//sendTCPLine4(clientSocket,buffer,readBytes);
				SSL_write(ssl, buffer, readBytes);
			}

			strcpy(html, content);				//Send Content-type
			strcat(html, "\r\n");
			
			tmp_size = (int)fsize(request);			//Get size of file
			size = itoa (tmp_size);

			strcpy(html, "Content-lenght: ");		//Send Content-lenght
			strcat(html, size);
			strcat(html, "\r\n");
			//sendTCPLine4(clientSocket, html, strlen(html));
			SSL_write(ssl, html, strlen(html));
		}
	}
	
	//strcpy(html, "Content-Type: text/html\r\n");
	//sendTCPLine4(clientSocket, html, strlen(html));

	//strcpy(html, "Content-Lenght: 457838592\r\n");
	//sendTCPLine4(clientSocket, html,strlen(html));
	
	
	//CERRAMOS LA COMUNICACIÓN
	sd = SSL_get_fd(ssl); 			/* get socket connection */
	SSL_free(ssl); 				/* release SSL state */
	close(sd);				/* close connection */
	SSL_CTX_free(ctx);			/* release context */
	close(clientSocket);			/* close server socket */

	return;
}

/*+++++++++++++++++++*/
/*+++ SSL Methods +++*/
/*+++++++++++++++++++*/

SSL_CTX* InitServerCTX(void)
{   SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */
    SSL_load_error_strings();   /* load all error messages */
    method = SSLv23_server_method();  /* create new server-method instance */
    ctx = SSL_CTX_new(method);   /* create new context from method */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
 /* set the local certificate from CertFile */
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* verify private key */
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

void ShowCerts(SSL* ssl)
{   X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        printf("Client certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
     
		if(SSL_get_verify_result(ssl) == X509_V_OK) {
            printf("client verification with SSL_get_verify_result() succeeded.\n");                
		} else{
            printf("client verification with SSL_get_verify_result() fail.\n");      
		}
		
        free(line);
        X509_free(cert);
    }
    else
        printf("No certificates.\n");
}
