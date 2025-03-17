/*
	Author : 변한빛
	Last Update : 2025-03-16
	Description : DBD 프	로토콜 정의
*/

#pragma once

#include "CoreMinimal.h"
#include "DBDProtocol.generated.h" // Add this if it's not already there

UENUM()
enum class HPACKET_TYPE : UINT16
{
	CHAT_MSG = 0,

	SEND_SURVIROR_MATCH,
	ACK_SURVIVOR_MATCH,

	SEND_KILLER_MATCH,
	ACK_KILLER_MATCH,

	SEND_MATCH_CANCEL,
	ACK_MATCH_CANCEL,

	SEND_MATCH_READY,
	SEND_MATCH_ABANDONED,

	SEND_MAP_LOAD_END,
	SEND_MAP_LOAD_INFO,

	SEND_GAME_START,
	ACK_GAME_START
};

#define PACKET_HEADER_SIZE 8
#define MAX_MSG_SIZE       1024
#define MAX_BUFFER_SIZE    MAX_MSG_SIZE + PACKET_HEADER_SIZE

#pragma pack(push, 1)

// Use struct instead of typedef struct for better C++ compatibility
struct DBDNETCORE_API PACKET_HEADER
{
	int          len;
	HPACKET_TYPE type;
	short        time;
};

struct DBDNETCORE_API HPACKET
{
	PACKET_HEADER ph;
	char          msg[MAX_MSG_SIZE];
};

#pragma pack(pop)
