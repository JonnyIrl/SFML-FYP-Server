#pragma once

#include "SFML/System.hpp"

typedef sf::Uint8 PacketType;
const PacketType INITIAL_CONNECT_DATA = 0;
const PacketType GENERAL_MSG = 1;
const PacketType SERVER_MSG = 2;
const PacketType UPDATE_MSG = 3;
const PacketType SERVER_REPLY_MSG =4;
const PacketType PLAYER_POSITION_UPDATE = 5;
const PacketType NEW_PLAYER_CONNECTED = 6;
const PacketType PLAYER_READY_CHANGED = 7;
const PacketType BULLET_MESSAGE = 8;
const PacketType RESPAWN_MESSAGE = 9;
const PacketType ROOM_UPDATE = 10;
const PacketType KILL_CONFIRMED = 11;
const PacketType CHEST_MESSAGE = 12;
const PacketType CHEST_OPEN_MESSAGE = 13;
const PacketType CHEST_ITEM_TAKEN_MESSAGE = 14;
const PacketType GAME_STARTED_MESSAGE = 15;
const PacketType GAME_OVER_MESSAGE = 16;