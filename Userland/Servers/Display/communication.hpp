#ifndef COMMUNICATION_HPP
#define COMMUNICATION_HPP

#include "wm.hpp"
#include <LibC/unistd.hpp>
#include <LibDisplay/connection.hpp>

int init_communications();
bool receive_connections(WindowManager* wm);

#endif
