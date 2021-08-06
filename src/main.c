#include <stdio.h>
#include <tls.h>
#include <unistd.h>		/* close() */
#include <sys/socket.h>	/* SOCKETS (socket(), bind()...) */
#include <netinet/ip.h>	/* IPv4 () */
#include <arpa/inet.h>	/* network byte order */

#include "tools.h"

//#define NOTLS

int main(){
	char buffer[1024];
	int recv_len;

#ifndef NOTLS
/* SETTING UP TLS */
	struct tls_config* tls_conf = tls_config_new();
	if(tls_config_set_protocols(tls_conf, TLS_PROTOCOL_TLSv1_2 | TLS_PROTOCOL_TLSv1_3) == -1)
		fatal("setting TLS protocols");
//	if(tls_config_set_keypair_file(tls_conf, "/home/kip/gemini-server/keys/cert.pem",\
	                                         "/home/kip/gemini-server/keys/key.pem") == -1)
//		fatal("setting key/certificate");

	struct tls* client = tls_client();
	if(client == NULL)
		fatal("creating client");
	if(tls_configure(client, tls_conf) == -1)
		fatal("configuring client");

	if(tls_connect(client, "192.168.2.45", "1965") == -1)
		fatal("connecting to server")
	


#endif

#ifdef NOTLS
/* SETTING UP THE SOCKET */
	int local_fd, yes = 1;
	struct sockaddr_in server_addr;

	local_fd = socket(AF_INET, SOCK_STREAM, 0);

	if(setsockopt(local_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		fatal("setting socket options");

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(1965);
	server_addr.sin_addr.s_addr = 0;	// Automatically sets to local address apparently

	if(connect(local_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
		fatal("connecting to server");

	send(local_fd, "Good day!\n", 10, 0);
	
	recv_len = recv(local_fd, &buffer, 1024, 0);
	while(recv_len > 0){
		printf("Recieved %d chars\n", recv_len);
		dump(buffer, recv_len);
		recv_len = recv(local_fd, &buffer, 1024, 0);
	}
#endif

//TODO: properly clean up tls
	close(local_fd);

	return 0;
}


