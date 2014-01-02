#include <string.h>

void dns_convert_hostname(unsigned char *dns, unsigned char *host) {
	int lock = 0;
	strcat((char*)host, ".");
	
	for (int i = 0; i < strlen((char *)host); i++) {
		if (host[i] == '.') {
			*dns++ = i - lock;
			for (; lock < i; lock++) {
				*dns++ = host[lock];
			}
			lock++;
		}
	}
	*dns++ = '\0';
}
