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
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>


//#include <sys/stat.h>
//#include <stdio.h>
//#include <stdlib.h>

//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/socket.h>
//#include <netinet/in.h>


using namespace std;

std::string WORKDIR;

std::string* read_index(const char* fname);
void *process(void *arg);
int req_parser(std::string request, std::string* pth, std::string* cgi);

static const char* templ = "HTTP/1.0 200 OK\r\n"
		           "Content-length: %d\r\n"
		       	   "Content-Type: text/html; charset=utf8\r\n"
		       	   "\r\n"
		       	   "%s";

static const char* not_found = "HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n";


int set_nonblock(int fd){
    
    int flags;
    
#if defined (O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags|O_NONBLOCK);
#else
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}


int req_parser(std::string request, std::string* pth, std::string* cgi = NULL) {
    
    std::string index;
    char *t, *path, *cgi_query;
    
    pth->erase();
    if(cgi)
        cgi->erase();
    
    t = strtok(&request[0], " ");
    while(t) {
        if (strcmp(t, "GET") == 0){
            t = strtok(NULL, " ");
            path = t;
            t = strtok(t, "?");
            cgi_query = strtok(NULL, "?");
            path = strtok(path, "/");
            break;
        }
//        t = strtok(NULL, " ");
    }

//    istringstream iss(d4);
//    do {
//        string sub;
//        iss >> sub;
//        if (sub == "GET"){
//            iss >> index;
//            break;
//        }
//    } while(iss);
    
    if (!path)
        index = "index.html";
    else
        index.assign(path);
//    else if (strcmp(path, "/") == 0) 
//        index = "index.html";
//    else if (strcmp(path, "/index.html") == 0)
//        index = "index.html";
//    else if (strcmp(path, "/index.html/") == 0)
//        index = "index.html";
    
    pth->assign(index);
    if(cgi_query)
        cgi->assign(cgi_query);

    return 0;
}

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
        data->assign(a);
        delete (a);
    }

    return data;
}

void *process(void *arg){
    
    int SlaveSocket = * (int *) arg;
    free(arg);
    char buff[1024];

    std::string* rbuff;
    std::string path;

//    sleep(.1);
    bzero(buff, sizeof(buff));
    ssize_t rcv = recv(SlaveSocket, buff, sizeof(buff), MSG_NOSIGNAL);
    cout << "recv:" << recv << ":Req:\n" << buff  <<  endl;
    if(-1 != rcv && buff){
        cout << "Pthread:" << pthread_self() << endl;
        req_parser(buff, &path);
        rbuff = read_index(path.c_str());
        ssize_t snd = send(SlaveSocket, rbuff->c_str(), strlen(rbuff->c_str()), MSG_NOSIGNAL);
    }
//    else {
//        strncpy(buff, "\n\ngood by\n", sizeof(buff));
//        ssize_t snd = send(SlaveSocket, buff, sizeof(buff), MSG_NOSIGNAL);
//    }
    shutdown(SlaveSocket, SHUT_RDWR);
    close(SlaveSocket);
    
//    return NULL;
    pthread_exit(0);
}


/*
 * 
 */

int main(int argc, char** argv) {
    
    int MasterSocket, SlaveSocket, b, optval;
//    char buff[256];
    pthread_t thread;
    int THREADS = 5;
    pthread_t th_pool[THREADS];
//    /home/box/final/final -h <ip> -p <port> -d <directory>
    int port;
    std::string ip;
    int rezopt;
    cout << port << endl;

//    while ( (rezopt = getopt(argc, argv, "h:p:d:") ) != 1) {
//        switch(rezopt) {
//            case 'h':
//                ip = optarg;
//                break;
//            case 'p':
//                port = atoi(optarg);
//                break;
//            case 'd':
//                WORKDIR = optarg;
//                break;
////            default:
////                exit(EXIT_FAILURE);
//        }
//    }
    
    cout << port << endl;
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
//    sa.sin_port = htons(port);//(12345);
    sa.sin_port = htons(12345);
    
    
//    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);    
//    sa.sin_addr.s_addr = htonl(INADDR_ANY); // all interfaces
    int err = inet_pton(AF_INET, "127.0.0.1", &(sa.sin_addr));
//    int err = inet_pton(AF_INET, ip.data(), &(sa.sin_addr));

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
    
    int *iptr;

    while(1){
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
//        http://www.iakovlev.org/index.html?p=95
        iptr = (int *) malloc(sizeof(int));
        *iptr = accept(MasterSocket, NULL, NULL);
//        SlaveSocket = accept(MasterSocket, NULL, NULL);
//        if(SlaveSocket <= 0){
        if(&iptr <= 0){
            perror("accept error");
            exit(EXIT_FAILURE);
        
//        setsockopt(SlaveSocket, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv, sizeof(tv));
//        setsockopt(SlaveSocket, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(tv));
        
        } 
        else {    
//            set_nonblock(SlaveSocket);
            set_nonblock(*iptr);
//            pthread_create(&thread, 0, process, &SlaveSocket);
            pthread_create(&thread, 0, process, iptr);
//            pthread_join(thread, NULL);
            pthread_detach(thread);
//            sleep(.01);

//            for (int i = 0; i < THREADS; i++ ){
//                cout << "thread:" << i << endl;
//                pthread_create(&th_pool[i], 0, process, &SlaveSocket);
//                pthread_join(th_pool[i], NULL);
//            }

        }
    
    }
    shutdown(MasterSocket, SHUT_RDWR);
    close(MasterSocket);
    
    return 0;
}
