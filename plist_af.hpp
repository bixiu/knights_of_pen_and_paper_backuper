#ifndef PLIST_AF_HPP
#define PLIST_AF_HPP
#include <plist/plist.h>
struct Plist_af{
    plist_t pt;
    Plist_af(plist_t pt_):pt{pt_}{}
    ~Plist_af(){
        plist_free(pt);
    }
};
#endif // PLIST_HPP
