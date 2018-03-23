/*-
 * Copyright 2012 Matthew Endsley
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


/*
Compiling example:
$ g++ -o example example.cpp
*/

#include <string>
#include <vector>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "http.h"

// directly embed the source here
extern "C" {
	#include "http.c"
	#include "header.c"
	#include "chunk.c"
}

// return a socket connected to a hostname, or -1
int connectsocket(const char* host, int port)
{

    addrinfo* result = NULL;
    sockaddr_in addr = {0};
    int s;

    if (getaddrinfo(host, NULL, NULL, &result))
        goto error;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    for (addrinfo* ai = result; ai != NULL; ai = ai->ai_next) {
        if (ai->ai_family != AF_INET)
            continue;

        const sockaddr_in *ai_in = (const sockaddr_in*)ai->ai_addr;
        addr.sin_addr = ai_in->sin_addr;
        break;
    }

    freeaddrinfo(result);

    if (addr.sin_addr.s_addr == INADDR_ANY)
        goto error;

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1)
        goto error;

    if (connect(s, (const sockaddr*)&addr, sizeof(addr)))
        goto error;

    return s;

error:
    if (s != -1)
        close(s);
    if (result)
        freeaddrinfo(result);
    return -1;
}

// Response data/funcs
struct HttpResponse {
	std::vector<char> body;
    int code;
};

static void* response_realloc(void* opaque, void* ptr, int size)
{
    return realloc(ptr, size);
}

static void response_body(void* opaque, const char* data, int size)
{
    HttpResponse* response = (HttpResponse*)opaque;
    response->body.insert(response->body.end(), data, data + size);
}

static void response_header(void* opaque, const char* ckey, int nkey, const char* cvalue, int nvalue)
{ /* example doesn't care about headers */ }

static void response_code(void* opaque, int code)
{
    HttpResponse* response = (HttpResponse*)opaque;
    response->code = code;
}

static const http_funcs responseFuncs = {
    response_realloc,
    response_body,
    response_header,
    response_code,
};

char* ReadFile(char *filename)
{
   char *buffer = NULL;
   int string_size, read_size;
   FILE *handler = fopen(filename, "r");

   if (handler)
   {
       // Seek the last byte of the file
       fseek(handler, 0, SEEK_END);
       // Offset from the first to the last byte, or in other words, filesize
       string_size = ftell(handler);
       // go back to the start of the file
       rewind(handler);

       // Allocate a string that can hold it all
       buffer = (char*) malloc(sizeof(char) * (string_size + 1) );

       // Read it all in one operation
       read_size = fread(buffer, sizeof(char), string_size, handler);

       // fread doesn't set it so put a \0 in the last position
       // and buffer is now officially a string
       buffer[string_size] = '\0';

       if (string_size != read_size)
       {
           // Something went wrong, throw away the memory and set
           // the buffer to NULL
           free(buffer);
           buffer = NULL;
       }

       // Always remember to close the file.
       fclose(handler);
    }

    return buffer;
}

int main(int argc, char **argv){

if(argc > 1){
    HttpResponse response;
    response.code = 0;
    char * buf=ReadFile(argv[1]);
    int size = strlen(buf);
    int read;
    http_roundtripper rt;
    http_init(&rt, responseFuncs, &response);
    http_data(&rt, buf , size, &read);

    if (http_iserror(&rt)) {
        http_free(&rt);
        return -1;
    }

    http_free(&rt);

    return 0;
}
}
