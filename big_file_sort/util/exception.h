#pragma once

#include <stdexcept>
#include <string>
#include <sstream>

class Exception : public std::exception {
public:
    virtual ~Exception() throw() {}

    virtual const char* what() const throw() {
        return whatMessage.c_str();
    }

    template <typename T>
    Exception& operator << (const T& value) {
        if (!whatMessage.empty()) {
            whatMessage += " ";
        }

        std::stringstream sstr;
        sstr << whatMessage << value;

        whatMessage = sstr.str();

        return *this;
    }

private:
    std::string whatMessage;
};
