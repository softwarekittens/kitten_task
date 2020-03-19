#pragma once

#include <exception>

class TaskSystemException : public std::exception {
public:
    TaskSystemException(std::exception& exception)
        : m_exception(exception)
    { }

    const char* what() const throw () {
        return m_exception.what();
    }

private:
    std::exception& m_exception;
};
