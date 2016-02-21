/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   my_socket.cpp
 * Author: golo
 *
 * Created on 21 февраля 2016 г., 16:11
 */

//#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "set_nonblock.h"

//#include <sys/stat.h>
//#include <stdio.h>
//#include <stdlib.h>

//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/socket.h>
//#include <netinet/in.h>


using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    
    int s, s_1, b, optval;
    char buff[256];
    struct sockaddr_in sa;
    std::cout << "AF_INET:" << AF_INET <<  std::endl;
    std::cout << "SOCK_STREAM:" << SOCK_STREAM <<  std::endl;
//    std::cout << "SOCK_STREAM:" << IP <<  std::endl;
    
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    
    sa.sin_family = AF_INET;
    sa.sin_port = htons(12345);
//    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);    
//    sa.sin_addr.s_addr = htonl(INADDR_ANY); // all interfaces
    int err = inet_pton(AF_INET, "127.0.0.1", &(sa.sin_addr));
    if (err <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    
    int snb = set_nonblock(s);
    cout << "Set NON block:" << snb << endl; 
    
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    err = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (-1 == err){
        perror("SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }
    
    b = bind(s, (struct sockaddr *) &sa, sizeof(sa));
    if(b == -1){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    std::cout << s << '\n';
    
    err = listen(s, SOMAXCONN);
    if (err == -1){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    while(1){
        
        s_1 = accept(s, NULL, NULL);
        setsockopt(s_1, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv, sizeof(tv));
        setsockopt(s_1, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(tv));
        if(s_1 == -1){
            perror("accept error");
            exit(EXIT_FAILURE);
        }
        
        bzero(buff, sizeof(buff));
        strcpy(buff, "You have only 5 sec\n\n");
        ssize_t snd = send(s_1, buff, sizeof(buff), MSG_NOSIGNAL);
        
        bzero(buff, sizeof(buff));
        ssize_t rcv = recv(s_1, buff, sizeof(buff), MSG_NOSIGNAL);
        if(-1 != rcv){
            ssize_t snd = send(s_1, buff, rcv, MSG_NOSIGNAL);
        } 
        else {    
            strcpy(buff, "\n\ngood by\n");
            ssize_t snd = send(s_1, buff, sizeof(buff), MSG_NOSIGNAL);
        }
        shutdown(s_1, SHUT_RDWR);
        close(s_1);
    }
    shutdown(s, SHUT_RDWR);
    close(s);
    
    return 0;
}

