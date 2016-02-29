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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
//#include <algorithm>
#include <semaphore.h>

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
sem_t *semid;


//std::string* read_index(const char* fname);
//void *process(void *arg);
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
    std::string path2;
    
    pth->erase();
    
    size_t pos = request.find(' ',0);
    if(pos > 0 && "GET" == request.substr(0,pos)){
        size_t posf = request.find(' ',pos+1);
        path2 = request.substr(pos+1+1,posf-pos-1-1);
        pos = path2.find('?',0);
        if(pos!=-1)
            path2 = path2.substr(0,path2.find('?',0));
    }    
//    t = strtok(&request[0], " ");
//    while(t) {
//        if (strcmp(t, "GET") == 0){
//            t = strtok(NULL, " ");
//            path = t;
//            t = strtok(t, "?");
//            cgi_query = strtok(NULL, "?");
//            path = strtok(path, "/");
//            break;
//        }
//    }

//    istringstream iss(d4);
//    do {
//        string sub;
//        iss >> sub;
//        if (sub == "GET"){
//            iss >> index;
//            break;
//        }
//    } while(iss);
    
    if (path2.empty())
        index = "index.html";
    else
        index.assign(path2);
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

    return 0;
}

ssize_t  read_index(const char* fname, std::string *data){

    std::string buff, b; // = std::string("");
    std::ifstream f (fname, ios::in);
    char *a, *page;
    int sz;

    if(f.is_open()){
        while(getline(f, buff)) {
            data->append(buff);
        }
        f.close();
        page = (char *) templ;
        b = "HTTP/1.0 200 OK\r\n"
             "Content-length: ";
        b+=std::to_string(data->size());
        b+="\r\n"
            "Connection: close\r\n"
            "Content-Type: text/html\r\n"
            "\r\n";
        b+=data->c_str();
    }
    else {
//        page = (char *) not_found;
        b="HTTP/1.0 404 NOT FOUND\r\nContent-length: 3\r\nContent-Type: text/html\r\n\r\nNOT";
    }

//    if(page == templ)
//        sz = asprintf(&a, page, data->size(), data->c_str());
//    else
//        sz = asprintf(&a, page);
//
//    if (sz == -1) {
//        data->append("error memory alloc");
//    }
//    else {
//        data->assign(a);
//        delete (a);
//    }
//
//    return data->size();
    
        data->assign(b);
        
    return b.size();
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
    int recVal = -1;
    
    int r, v;
    
    FD_ZERO(&rfds);
    FD_SET(SlaveSocket, &rfds);

   /* Wait up to five seconds. */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

//    cout << SlaveSocket << endl;
    retval = select(SlaveSocket + 1, &rfds, NULL, NULL, &tv);
    
    bzero(buff, sizeof(buff));
//    do {
    recVal = recv (SlaveSocket, buff, 1024, MSG_NOSIGNAL);
//    } while (recVal <= 0);
    
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
   
//    if (recVal <= 0)
//        printf("No data %d:_%d.\n", recVal, retval);
//        perror("select()");
//    else if (recVal)
//        printf("Data is available now %d_%d.\n", recVal, retval);
//        /* FD_ISSET(0, &rfds) will be true. */
//    else
//        printf("No data within five seconds %d:.\n", recVal);

    shutdown(SlaveSocket, SHUT_RDWR);
    close(SlaveSocket);
    
    pthread_mutex_lock(&lock);
    r = sem_post(semid); // increment semafor
    r = sem_getvalue(semid, &v);
//    std::cout << "proc2 SEM_value:" << v << std::endl;
    pthread_mutex_unlock(&lock);
    
//   exit(EXIT_SUCCESS);
//    pthread_exit(0);

}


//void *process(void *arg){
//    
//    int SlaveSocket = * ((int *) arg);
//    free(arg);
//    char buff[1024];
//
//    std::string rbuff;
//    std::string path;
//
////    sleep(.1);
//
//    bzero(buff, sizeof(buff));
//    ssize_t rcv = recv(SlaveSocket, buff, sizeof(buff), MSG_NOSIGNAL);
//    pthread_mutex_lock(&lock);
//    cout << "Pthread:" << pthread_self() << endl;
//    cout << "recv:" << recv << ":Request:\n" << buff  <<  endl;
//    pthread_mutex_unlock(&lock);
//    if(-1 != rcv && buff){
//        req_parser(buff, &path);
//        read_index(path.c_str(), &rbuff);
//        ssize_t snd = send(SlaveSocket, rbuff.c_str(), strlen(rbuff.c_str()), MSG_NOSIGNAL);
//    }
//    shutdown(SlaveSocket, SHUT_RDWR);
//    close(SlaveSocket);
//    
////    return NULL;
//    pthread_exit(0);
//}


/*
 * 
 */

int main_loop(int argc, char** argv) {
    
//    /home/box/final/final -h <ip> -p <port> -d <directory>
    
    int MasterSocket, SlaveSocket, b, optval;
    pthread_t thread;
    const char *sname = "/test.sem";
    const int semcnt = 32;

    semid = sem_open(sname, O_CREAT, 0666, semcnt);
    sem_close(semid);
    sem_unlink(sname);
    semid = sem_open(sname, O_CREAT, 0666, semcnt);

    int r, v;
    
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

    

    while(1){
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        sem_wait(semid); // decrement
        r = sem_getvalue(semid, &v);
//        std::cout << "Main SEM_value:" << v << std::endl;
        if(v==0) {
//            std::cout << "sleep 1" << v << std::endl;
            sleep(.00001);
        }
        
//        http://www.iakovlev.org/index.html?p=95
//    int *iptr;
        int *iptr = (int *) malloc(sizeof(int));
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
//        pthread_create(&thread, 0, process, iptr);
        pthread_create(&thread, 0, proc2, iptr);
//        pthread_join(thread, NULL);
        pthread_detach(thread);
//        proc2(iptr);
//        process(iptr);
    }

    pthread_mutex_destroy(&lock);

    shutdown(MasterSocket, SHUT_RDWR);
    close(MasterSocket);

    sem_close(semid);
    sem_unlink(sname);
    
    return 0;
}

int main (int argc, char **argv){
    
    pid_t process_id = 0;
    pid_t sid = 0;

    process_id = fork();
    if (process_id > 0) {
//        printf("process_id of child process %d \n", process_id);
        // return success in exit status
        exit(0);
    }

    umask(0);

    //set new session
    sid = setsid();
    
    if(sid < 0) {
        // Return failure
        // exit(1);
    }
    // Change the current working directory to root.
    chdir("/");
    // Close stdin. stdout and stderr
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    return main_loop(argc, argv);
}