//
// Created by aidoo on 2024/11/6.
//
#include "input.h"
#include "input/software_input.h"

namespace libRetroRunner {

    Input::Input() {

    }

    Input::~Input() {

    }

    std::unique_ptr<Input> Input::NewInstance() {
        return std::make_unique<SoftwareInput>();
    }
}
