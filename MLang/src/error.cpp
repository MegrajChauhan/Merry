#include "../includes/error.hpp"

template <typename T>
void mlang::Error<T>::add_error(T error)
{
    this->_errors_.push(error);
}

template <typename T>
T mlang::Error<T>::get_next_error()
{
    return this->_errors_.pop();
}

template <typename T>
bool mlang::Error<T>::has_error()
{
    return (this->_errors_.size() > 0);
}