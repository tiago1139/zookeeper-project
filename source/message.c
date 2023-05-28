#include <errno.h>
#include "inet.h"
#include "message-private.h"

/*
    GRUPO 33
    - Ricardo Soares fc54446
    - Miguel Reis fc54409
    - Tiago Pinto fc55172
*/


/** * * * * * * * * * * * * * * *
*                               *
*       FUNCOES / METODOS       *
*                               *
*                               *
* * * * * * * * * * * * * * * * */

int write_all(int sock, char *buf, int len) {
	printf("writing... \n");
	int buffersize = len;
	while (len > 0)
	{
		
		int res = write(sock, buf, len);
		if (res <= 0)
		{
			if (errno == EINTR)
				continue;
            
			perror("write failed: ");
			return res;
		}
		buf += res;
		len -= res;
	}

	return buffersize;
}

int read_all(int sock, char *buf, int len) {
	printf("reading... \n");
	int buffersize = len;
	while (len > 0)
	{
		
		int result = read(sock, buf, len);
		if (result <= 0)
		{
			if (errno == EINTR)
				continue;
             
			perror("read failed: ");
			return result;
		}

		buf += result;
		len -= result;
	}

	return buffersize;
}
