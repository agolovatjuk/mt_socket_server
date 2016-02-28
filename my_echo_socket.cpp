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

pthread_mutex_t     lock;
pthread_mutex_t     mLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      cond  = PTHREAD_COND_INITIALIZER;

int T_CNT = 0;

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
    
    if ( ! WORKDIR.empty()){
        pth->assign(WORKDIR);
        pth->append("//");
    }
    pth->append(index);
//    cout << pth->c_str() << endl;
//    pth->assign(index);
    if(cgi_query)
        cgi->assign(cgi_query);

    return 0;
}

ssize_t  read_index(const char* fname, std::string *data){

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

    return data->size();
}


void *proc2(void *arg){
    
    int SlaveSocket = * ((int *) arg);
    free(arg);
    
    static char buff[1024];
    std::string rbuff;
    std::string path;
    
    fd_set rfds;
    struct timeval tv;
    int retval = 0;

   /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(SlaveSocket, &rfds);

   /* Wait up to five seconds. */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    cout << SlaveSocket << endl;
    retval = select(SlaveSocket + 1, &rfds, NULL, NULL, &tv);
    /* Don't rely on the value of tv now! */
    
    bzero(buff, sizeof(buff));
    int recVal = recv (SlaveSocket, buff, 1024, MSG_NOSIGNAL);

//    if (n == -1) {
//    //something wrong
//    } else if (n == 0)
//    continue;//timeout
//    if (!FD_ISSET(sd, &input))
//   ;//again something wrong

//    if((retVal == 0) && errno != EAGAIN){
//        shutdown(SlaveSocket, SHUT_RDWR);
//        close(SlaveSocket);
////        SlaveSockets->erase(i);
//    }
    
    if (recVal > 0) {
//        strncpy(Buffer + RecvSize, "125", 3);
//        send(SlaveSocket, Buffer, RecvSize, MSG_NOSIGNAL);
        req_parser(buff, &path);
        read_index(path.c_str(), &rbuff);
        ssize_t snd = send(SlaveSocket, rbuff.c_str(), rbuff.size(), MSG_NOSIGNAL);   
    }   
   
    if (recVal <= 0)
        printf("No data %d:_%d.\n", recVal, retval);
//        perror("select()");
//    else if (recVal)
//        printf("Data is available now %d_%d.\n", recVal, retval);
//        /* FD_ISSET(0, &rfds) will be true. */
//    else
//        printf("No data within five seconds %d:.\n", recVal);

//    shutdown(SlaveSocket, SHUT_RDWR);
//    close(SlaveSocket);
   
//   exit(EXIT_SUCCESS);
    pthread_exit(0);

}


void *process(void *arg){
    
    int SlaveSocket = * ((int *) arg);
    free(arg);
    char buff[1024];

    std::string rbuff;
    std::string path;

//    sleep(.1);

    bzero(buff, sizeof(buff));
    ssize_t rcv = recv(SlaveSocket, buff, sizeof(buff), MSG_NOSIGNAL);
    pthread_mutex_lock(&lock);
    cout << "Pthread:" << pthread_self() << endl;
    cout << "recv:" << recv << ":Request:\n" << buff  <<  endl;
    pthread_mutex_unlock(&lock);
    if(-1 != rcv && buff){
        req_parser(buff, &path);
        read_index(path.c_str(), &rbuff);
        ssize_t snd = send(SlaveSocket, rbuff.c_str(), strlen(rbuff.c_str()), MSG_NOSIGNAL);
    }
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
    int port = 12345;
    std::string ip = "127.0.0.1";
    int rezopt;

    extern char *optarg;
    extern int optind, opterr, optopt;
    
    while ( (rezopt = getopt(argc, argv, "h:p:d:") ) != -1) {
        switch(rezopt) {
            case 'h':
                ip = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
                WORKDIR = optarg;
                break;
//            case '?': printf("Error found !\n");break;
            default:
                exit(EXIT_FAILURE);
        }
    }
    
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);//(12345);
    
    
//    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);    
//    sa.sin_addr.s_addr = htonl(INADDR_ANY); // all interfaces
//    int err = inet_pton(AF_INET, "127.0.0.1", &(sa.sin_addr));
    int err = inet_pton(AF_INET, ip.data(), &(sa.sin_addr));

    if (err <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
    
    
    MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // IPP
    
//    set_nonblock(MasterSocket);
    
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
    
    if(pthread_mutex_init(&lock, NULL) !=0){
        perror("init mutex");
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
        } 
//        setsockopt(SlaveSocket, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv, sizeof(tv));
//        setsockopt(SlaveSocket, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(tv));
//            set_nonblock(SlaveSocket);
        
        set_nonblock(*iptr);
//            pthread_create(&thread, 0, process, &SlaveSocket);
        pthread_create(&thread, 0, proc2, iptr);
//        pthread_create(&thread, 0, process, iptr);
//        pthread_join(thread, NULL);
        pthread_detach(thread);
//        proc2(iptr);
//        process(iptr);
    }

    pthread_mutex_destroy(&lock);

    shutdown(MasterSocket, SHUT_RDWR);
    close(MasterSocket);
    
    return 0;
}
