#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "includes/dns.h"

#define DNSPORT 53
#define HOSTNAME "google.com"
#define PACKETSIZE 72
#define TIMEOUT 5

int sockfd;

void usage(char *cmd) {
	printf("Usage: %s <address/mask> <bandwith in kbps>\n", cmd);
}

void error (char * msg) {
	fprintf(stderr, "Fatal Error: %s\n", msg);
	exit(EXIT_FAILURE);
}

void *receive(void *arg) {
	char buffer[65536];
	socklen_t addrlen;
	struct sockaddr_in rsystem;
	struct DNS_HEADER *dns = NULL;
	
	addrlen = sizeof(rsystem);
	
	for(;;) {
		memset(buffer, 0x00, 65536);
		if (recvfrom(sockfd, (char*)buffer, 65536, 0, (struct sockaddr*)&rsystem, &addrlen) >= 0) {
			dns = (struct DNS_HEADER*) buffer;
			if (ntohs(dns->ans_count) > 0) {
				printf("%s\n", inet_ntoa(rsystem.sin_addr));
			}
		}
	}
}

int main(int argc, char **argv) {
	pthread_t tid;
	int query_type;
	int end_ip;
	int sleeptime;
	int len;
	int bandwith;
	int mask;
	char buffer[16];
	unsigned char hostname[] = HOSTNAME;
	char *start_ip;
	unsigned char buf[65536];
	unsigned char *qname;
	struct sockaddr_in addr;
	struct DNS_HEADER *dns = NULL;
	struct QUESTION *qinfo = NULL;
	
	if (argc != 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	
	len = strlen(argv[1]);
	start_ip = (char *)malloc(sizeof(char) * len);
	if (start_ip == NULL) {
		error("Could not allocate memory\n");
	}
	
	for (int i = 0; i < len; i++) {
		if (argv[1][i] != '/') {
			start_ip[i] = argv[1][i];
		}
		else {
			start_ip[i] = '\0';
			mask = atoi(&argv[1][i + 1]);
			break;
		}
	}
	
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		error("Could not open socket\n");
	}
	
	bandwith = atoi(argv[2]);
	end_ip = pow(2, 32 - mask);
	sleeptime = 1000000 / bandwith * 1000 / (PACKETSIZE * 8);
	addr.sin_family = AF_INET;
    addr.sin_port = htons(DNSPORT);
	
	dns = (struct DNS_HEADER *)&buf;

	dns->qr = 0;
	dns->opcode = 0;
	dns->aa = 0; 
	dns->tc = 0;
	dns->rd = 1;
	dns->ra = 0;
	dns->z = 0;
	dns->ad = 0;
	dns->cd = 0;
	dns->rcode = 0;
	dns->q_count = htons(1);
	dns->ans_count = 0;
	dns->auth_count = 0;
	dns->add_count = 0;
	
	qname = (unsigned char *)&buf[sizeof(struct DNS_HEADER)];
	query_type = A_RECORD;
	dns_convert_hostname(qname , hostname);
	
	qinfo = (struct QUESTION *)&buf[sizeof(struct DNS_HEADER) + (strlen((const char *)qname) + 1)];
	qinfo->qtype = htons(query_type);
	qinfo->qclass = htons(1);

	if (pthread_create(&tid, NULL, receive, NULL)) {
		error("Could not create thread\n");
	}
	
	addr.sin_addr.s_addr = inet_addr(start_ip);
	for (int i = 0; i < end_ip; i++) {
		dns->id = (unsigned short)i;
		memset(buffer, 0x00, 16);
		inet_ntop(AF_INET, &addr.sin_addr.s_addr, buffer, 16);
		do {
		} while (sendto(sockfd, (char*)buf,sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0);
		addr.sin_addr.s_addr = htonl(addr.sin_addr.s_addr);
		addr.sin_addr.s_addr++;
		addr.sin_addr.s_addr = ntohl(addr.sin_addr.s_addr);
		usleep(sleeptime);
	}
	
	sleep(TIMEOUT);
	return 0;
}
