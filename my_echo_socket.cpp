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

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "set_nonblock.h"
#include <pthread.h>

//#include <sys/stat.h>
//#include <stdio.h>
//#include <stdlib.h>

//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/socket.h>
//#include <netinet/in.h>


using namespace std;

std::string* read_index(const char* fname);
void *process(void *arg);

static const char* templ = "HTTP/1.0 200 OK\r\n"
		           "Content-length: %d\r\n"
		       	   "Content-Type: text/html; charset=utf8\r\n"
		       	   "\r\n"
		       	   "%s";

static const char* not_found = "HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n";


std::string* read_index(const char* fname = "index.html"){

    std::string *data = new std::string;
    std::string buff; // = std::string("");
    std::ifstream f (fname, ios::in);
    char *a, *page;
    int sz;

    if(f.is_open()){
        while(getline(f, buff)) {
            data->append(buff);
        }
        f.close();
        page = (char *) templ;
    }
    else {
        page = (char *) not_found;
    }

    if(page == templ)
        sz = asprintf(&a, page, data->size(), data->c_str());
    else
        sz = asprintf(&a, page);

    if (sz == -1) {
        data->append("error memory alloc");
    }
    else {
        data->erase();
        data->append(a);
        delete (a);
    }
    
    return data;
}

void *process(void *arg){
    
    int SlaveSocket = * (int *) arg;
    char buff[512];
    char *b;
    std::string* rbuff;

//    sleep(10);
    bzero(buff, sizeof(buff));
    ssize_t rcv = recv(SlaveSocket, buff, sizeof(buff), MSG_NOSIGNAL);
    cout << "Req:\n" << buff << endl;
    if(-1 != rcv){
        cout << "Pthread:" << pthread_self() << endl;      
        rbuff = read_index("index.html");
        //strncpy(buff, rbuff->c_str(), sizeof(buff));
        //ssize_t snd = send(SlaveSocket, buff, sizeof(buff), MSG_NOSIGNAL);
        ssize_t snd = send(SlaveSocket, rbuff->c_str(), strlen(rbuff->c_str()), MSG_NOSIGNAL);
    }
    else {
        ;
//        strncpy(buff, "\n\ngood by\n", sizeof(buff));
//        ssize_t snd = send(SlaveSocket, buff, sizeof(buff), MSG_NOSIGNAL);
    }
    shutdown(SlaveSocket, SHUT_RDWR);
    close(SlaveSocket);
    
    pthread_exit(0);
}


/*
 * 
 */
int main(int argc, char** argv) {
    
    int MasterSocket, SlaveSocket, b, optval;
//    char buff[256];
    pthread_t thread;

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(12345);
    
    
//    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);    
//    sa.sin_addr.s_addr = htonl(INADDR_ANY); // all interfaces
    int err = inet_pton(AF_INET, "127.0.0.1", &(sa.sin_addr));
    if (err <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
    
    set_nonblock(MasterSocket);
    
    MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // IPP
    if (MasterSocket < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    err = setsockopt(MasterSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (-1 == err){
        perror("SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }
    
    b = bind(MasterSocket, (struct sockaddr *) &sa, sizeof(sa));
    if(b == -1){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    err = listen(MasterSocket, SOMAXCONN);
    if (err == -1){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    while(1){
//        struct timeval tv;
//        tv.tv_sec = 5;
//        tv.tv_usec = 0;
        
        SlaveSocket = accept(MasterSocket, NULL, NULL);
        if(SlaveSocket == -1){
            perror("accept error");
            exit(EXIT_FAILURE);
        
//        setsockopt(SlaveSocket, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv, sizeof(tv));
//        setsockopt(SlaveSocket, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(tv));
        
        } 
        else {    
//            set_nonblock(SlaveSocket);
            pthread_create(&thread, 0, process, &SlaveSocket);
            pthread_detach(thread);
        }
    
    }
    shutdown(MasterSocket, SHUT_RDWR);
    close(MasterSocket);
    
    return 0;
}

