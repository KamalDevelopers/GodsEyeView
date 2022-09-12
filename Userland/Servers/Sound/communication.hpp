#ifndef COMMUNICATION_HPP
#define COMMUNICATION_HPP

#include "sound.hpp"
#include <LibSound/connection.hpp>

int init_communications();
bool receive_connections(SoundServer* sound_server);

#endif
