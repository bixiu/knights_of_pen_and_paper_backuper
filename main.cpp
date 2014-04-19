#include <iostream>
#include <fstream>
#include "device.hpp"
#include <streambuf>

using namespace std;
void backup(){
    Device_ptr dptr(new Device{});
    App app{dptr, "br.com.beholdstudios.knightspp"};
    ofstream out{"data.koop", ios::binary};
    out << app.read_file("/Documents/data.kopp");
}

void restore(){
    Device_ptr dptr(new Device{});
    App app{dptr, "br.com.beholdstudios.knightspp"};
    ifstream in{"data.koop", ios::binary};
    std::string content{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    app.write_file("/Documents/data.kopp", content);
}

int main(int , char *argv[])
{
    if(argv[1] == nullptr)
        cout << "Usage: " << argv[0] << " " << "backup|restore \n";
    else if(argv[1] == std::string("backup"))
        backup();
    else if(argv[1] == std::string("restore"))
        restore();
    else
        cout << "Usage: " << argv[0] << " " << "backup|restore \n";
    return 0;
}

