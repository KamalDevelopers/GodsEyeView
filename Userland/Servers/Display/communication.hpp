#ifndef COMMUNICATION_HPP
#define COMMUNICATION_HPP

#include "wm.hpp"
#include <LibDisplay/connection.hpp>

int init_communications();
bool receive_connections(WindowManager* wm);

#endif
