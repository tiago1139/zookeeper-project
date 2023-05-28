#ifndef _MESSAGE_H
#define _MESSAGE_H /* Módulo data */

#include "inet.h"
/*
    GRUPO 33
    - Ricardo Soares fc54446
    - Miguel Reis fc54409
    - Tiago Pinto fc55172
*/


int write_all(int sock, char *buf, int len);
int read_all(int sock, char *buf, int len);

#endif
