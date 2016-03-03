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
#include <semaphore.h>
#include <sstream>
#include <signal.h>


using namespace std;

std::string WORKDIR;

pthread_mutex_t     lock;
pthread_mutex_t     mLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      cond  = PTHREAD_COND_INITIALIZER;

sem_t semaphore;


static const char* templ = "HTTP/1.0 200 OK\r\n"
		           "Content-length: %d\r\n"
		       	   "Content-Type: text/html; charset=utf8\r\n"
		       	   "\r\n"
		       	   "%s";

//static const char* not_found = "HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n";
static const char* not_found = "HTTP/1.0 404 NOT FOUND\r\nContent-length: 9\r\nContent-Type: text/html\r\n\r\nNOT FOUND";


int set_nonblock(int fd){
    
    int flags;
    
#if defined (O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags|O_NONBLOCK);
#else
    flags = 1;
    return ioctl(fd, FIONBIO, &flags);
#endif
}

void handle_signal(int signum){
    // do nothing, ignore SIGHUP, SIGTERM
    ;
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

    if ( ! WORKDIR.empty()){
        pth->assign(WORKDIR);
        pth->append("//");
    }
    pth->append(index);

    return 0;
}

ssize_t  read_index(const char* fname, std::string *data){

    std::string b; // = std::string("");
    std::stringstream buffer;

    std::ifstream f (fname, ios::in);

    if(f.is_open()){
        
        buffer << f.rdbuf();
        data->assign(buffer.str());

        f.close();
        
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
        b = (char *) not_found;
    }

        data->assign(b);
        
    return b.size();
}


void *proc_select(void *arg){
    
    int SlaveSocket = * ((int *) arg);
    free(arg);
    /*static*/ char buff[1024];
    std::string rbuff;
    std::string path;
    std::string tbuff;
    
    fd_set rfds;
    struct timeval tv;
    int sck_cnt = 0;
    int recVal  = 0;
    
    FD_ZERO(&rfds);
    FD_SET(SlaveSocket, &rfds);

   /* Wait up to five seconds. */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    sck_cnt = select(SlaveSocket + 1, &rfds, NULL, NULL, &tv);
    
    tbuff.clear();
    do {
        bzero(buff, sizeof(buff));
        recVal = recv (SlaveSocket, buff, sizeof(buff), MSG_NOSIGNAL);
        tbuff += buff;
    } while (recVal > 0);
   
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
//    if (recVal <= 0)
//        printf("No data %d:_%d.\n", recVal, retval);
//        perror("select()");
//    else if (recVal)
//        printf("Data is available now %d_%d.\n", recVal, retval);
//        /* FD_ISSET(0, &rfds) will be true. */
//    else
//        printf("No data within five seconds %d:.\n", recVal);


    if ( ! tbuff.empty()) {
        req_parser(tbuff.c_str(), &path);
        read_index(path.c_str(), &rbuff);
        ssize_t snd = send(SlaveSocket, rbuff.c_str(), rbuff.size(), MSG_NOSIGNAL);   
    }   
   
    shutdown(SlaveSocket, SHUT_RDWR);
    close(SlaveSocket);

//    The function sem_post() is thread safe?
//    pthread_mutex_lock(&lock);
    sem_post(&semaphore);
//    pthread_mutex_unlock(&lock);
    
//   exit(EXIT_SUCCESS);
//    pthread_exit(0);

}

void *proc_socket(void *arg){
    
    int SlaveSocket = * ((int *) arg);
    free(arg);

    char buff[1024];
    ssize_t rcv;
    std::string nbuff;

    nbuff.clear();

    do {
        bzero(buff, sizeof(buff));
        rcv = recv(SlaveSocket, buff, sizeof(buff), MSG_NOSIGNAL);
        nbuff += buff;
    } while (rcv > 0);
//    pthread_mutex_lock(&lock);
//    cout << "Pthread:" << pthread_self() << endl;
//    cout << "recv:" << recv << ":Request:\n" << buff  <<  endl;
//    pthread_mutex_unlock(&lock);
    if( ! nbuff.empty() ){
        
        std::string rbuff;
        std::string path;

        req_parser(nbuff.c_str(), &path);
        read_index(path.c_str(), &rbuff);
        ssize_t snd = send(SlaveSocket, rbuff.c_str(), strlen(rbuff.c_str()), MSG_NOSIGNAL);
    }

    shutdown(SlaveSocket, SHUT_RDWR);
    close(SlaveSocket);
    
//    The function sem_post() is thread safe?
//    pthread_mutex_lock(&lock);
    sem_post(&semaphore);
//    pthread_mutex_unlock(&lock);

//    pthread_exit(0);
}


/* 
 * 
 */

int main_loop(int argc, char** argv) {
    
//    /home/box/final/final -h <ip> -p <port> -d <directory>
    
    int MasterSocket, b, optval;
    pthread_t thread;
    
    int semcnt = 512;

    int r, v;
    
    int port = 12345;
    std::string ip = "127.0.0.1";
    int rezopt;

    extern char *optarg;
    extern int optind, opterr, optopt;
    
    while ( (rezopt = getopt(argc, argv, "h:p:d:t:") ) != -1) {
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
            case 't':
                semcnt = atoi(optarg);
                break;
//            case '?': printf("Error found !\n");break;
            default:
                exit(EXIT_FAILURE);
        }
    }
    
    sem_init(&semaphore, 0, semcnt);

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
        tv.tv_sec = 0;
        tv.tv_usec = 0.5;
                
//        http://www.iakovlev.org/index.html?p=95
        int *iptr = (int *) malloc(sizeof(int));
        *iptr = accept(MasterSocket, NULL, NULL);
        if(&iptr <= 0){
            perror("accept error");
            exit(EXIT_FAILURE);
        } 

        sem_wait(&semaphore);
//        sem_getvalue(&semaphore, &v);
//        cout << "Sem:" << v << endl;

//        setsockopt(*iptr, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv, sizeof(tv));
//        setsockopt(*iptr, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(tv));
        
        set_nonblock(*iptr);
//        pthread_create(&thread, 0, proc_socket, iptr);
        pthread_create(&thread, 0, proc_select, iptr);
        pthread_detach(thread);
//        pthread_join(thread, NULL); //for further threadpool 

//        proc2(iptr);
    }

    pthread_mutex_destroy(&lock);

    shutdown(MasterSocket, SHUT_RDWR);
    close(MasterSocket);

    return 0;
}

/*
 * TODO: realize 
 * ThreadPool with epoll/dequeue
 * libev
 * libuv
 * boost::asio
 */

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
    
    signal(SIGHUP, handle_signal);
//    signal(SIGTERM, handle_signal);
    
    // Change the current working directory to root.
    chdir("/");
    // Close stdin. stdout and stderr
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    return main_loop(argc, argv);
}
