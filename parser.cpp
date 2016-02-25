#include <iostream>
#include <sstream>
#include <cstring>

using namespace std;

int main() {
    
    std::string d1 = "GET / HTTP/1.1";
    std::string d2 = "GET /index.html HTTP/1.1";
    std::string d3 = "GET /index.html/ HTTP/1.1";
    std::string d4 = "GET /index.html?a=5 HTTP/1.1";

    std::string index;
    istringstream iss(d4);

    char *t;
    t = strtok(&d4[0], " ");
    while(t) {
        if (strcmp(t, "GET") == 0){
            t = strtok(t, "?");
            t = strtok(NULL, "?");
            cout << t << endl;
            break;
        }
        t = strtok(NULL, " ");
    }

    cout << t << endl;

/*
    do {
        string sub;
        iss >> sub;
        if (sub == "GET"){
            iss >> index;
            break;
        }
    } while(iss);

*/
    

/*
    if (strcmp(t, "/") == 0) 
        r = "index.html";
    else if (strcmp(t, "/index.html") == 0)
        r = "index.html";
    else if (strcmp(t, "/index.html/") == 0)
        r = "index.html";

    cout << r << '\n';
*/
}
