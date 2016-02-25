#include <iostream>
//#include <sstream>
#include <cstring>

using namespace std;
/*
int req_parser(std::string request, std::string* pth, std::string* cgi) {
    
    std::string index;
    char *t, *path, *cgi_query;
    
    pth->erase();
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


//int main(){
//    
//    std::string d[5];
//    d[0] = "GET / HTTP/1.1";
//    d[1] = "GET /test.html?a=4 HTTP/1.1";
//    d[2] = "GET /index.html/ HTTP/1.1";
//    d[3] = "GET /index.html?a=5&b=6 HTTP/1.1";
//    
//    std::string path; 
//    std::string cgi;
//    
//    for(int i=0; i<4; i++){
//        req_parser(d[i], &path, &cgi);
//        cout << path << ":" << cgi << endl;
//    }
//    return 0;
//}
 * 
 * */