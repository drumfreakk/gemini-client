#include <stdio.h>
#include <tls.h>
#include <unistd.h>		/* close() */
#include <sys/socket.h>	/* SOCKETS (socket(), bind()...) */
#include <netinet/ip.h>	/* IPv4 () */
#include <arpa/inet.h>	/* network byte order */

#include "tools.h"

int main(){
/* SETTING UP TLS */
	struct tls_config* tls_conf = tls_config_new();
	if(tls_config_set_protocols(tls_conf, TLS_PROTOCOL_TLSv1_2) == -1)
		fatal("setting TLS protocols");
	if(tls_config_set_keypair_file(tls_conf, "/home/kip/gemini-server/keys/cert.pem",\
	                                         "/home/kip/gemini-server/keys/key.pem") == -1)
		fatal("setting key/certificate");

	struct tls* server = tls_server();
	if(server == NULL)
		fatal("creating server");
	if(tls_configure(server, tls_conf) == -1)
		fatal("configuring server");

/* SETTING UP THE SOCKET */
	int host_fd, client_fd, recv_len, yes = 1;
	struct sockaddr_in local_addr, client_addr;
	char buffer[1024];

	host_fd = socket(AF_INET, SOCK_STREAM, 0);

	if(setsockopt(host_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		fatal("setting socket options");

	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(1965);
	local_addr.sin_addr.s_addr = 0;	// Automatically sets to local address apparently

	if(bind(host_fd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1)
		fatal("binding socket");

	if(listen(host_fd, 5) == -1)
		fatal("starting listening");

/* MAIN LOOP */
	while(1){
		socklen_t addrlen = sizeof(struct sockaddr_in);
		client_fd = accept(host_fd, (struct sockaddr*)&client_addr, &addrlen);
		if(client_fd == -1)
			error("accepting connection");

		printf("New connection from %s:%d\n", \
			inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		struct tls* client_tls;
		if(tls_accept_socket(server, &client_tls, client_fd) == -1)
			error("upgrading to tls");
		
		if(tls_handshake(client_tls) == -1)
			error("performing handshake");
		
		recv_len = tls_read(client_tls, &buffer, 1024);
		while(recv_len > 0){
			printf("Recieved %d chars\n", recv_len);
			dump(buffer, recv_len);
			recv_len = tls_read(client_tls, &buffer, 1024);
		}	
		char to_write[] = "40 aaa\r\n";
		if(tls_write(client_tls, to_write, 8) == -1)
			error("sending message");

/*		send(client_fd, "Hello, world!\n", 14, 0);
		
		recv_len = recv(client_fd, &buffer, 1024, 0);
		while(recv_len > 0){
			printf("Recieved %d chars\n", recv_len);
			dump(buffer, recv_len);
			recv_len = recv(client_fd, &buffer, 1024, 0);
		}
*/
		close(client_fd);
	}
//TODO: properly clean up tls
	close(host_fd);

	return 0;
}


