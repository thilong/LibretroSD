//
// Created by aidoo on 2024/11/1.
//

#ifndef _RR_TYPES_H
#define _RR_TYPES_H


#include <string>

struct Variable {
    std::string key;
    std::string value;
    std::string description;
};

enum AppCommands {
    kNone = 0,
    kLoadCore = 1 ,

};

#endif
