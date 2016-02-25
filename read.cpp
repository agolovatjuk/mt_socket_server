#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

static const char* not_found = "HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n";
static const char* templ = "HTTP/1.0 200 OK\r\n"
		           "Content-length: %d\r\n"
		       	   "Content-Type: text/html; charset=utf8\r\n"
		       	   "\r\n"
		       	   "%s";

std::string* read_index(const char* fname = "index.html"){

    std::string *data = new std::string;
    std::string buff; // = std::string("");
    std::ifstream f (fname, ios::in);

    if(f.is_open()){
        while(getline(f, buff)) {
//            f >> buff;
            data->append(buff);
        }
        f.close();
        cout << data->c_str() << endl;
//        char *b = new char[data->size() + strlen(templ)];
        cout << strlen(templ)<< ":" << data->size() << endl;
//        snprintf(b, data->size()+strlen(templ), templ, data->size(), data->c_str());
        char *a;
        int sz = asprintf(&a, templ, data->size(), data->c_str());
        if (sz == -1) {
            data->append("error memory alloc");
        }
        else {
            cout << strlen(a) << endl;
            data->erase();
            data->append(a);
            delete (a);
        }
    }
    else {
        data->append(not_found);
    }

    return data;
}

int main(){

    std::string *buff;
    buff = read_index("index.html");
    cout << buff->size() << ":" << buff->c_str() << endl;

    buff->erase();
    buff = read_index("index.htm");
    cout << buff->size() << ":" << buff->c_str() << endl;

    return 0;
}
