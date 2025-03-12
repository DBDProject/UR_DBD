/*
	Author : 변한빛
	Last Update : 2025-03-03
	Description : DBD 프	로토콜 정의
*/

#pragma once

#include "CoreMinimal.h"
#include "DBDProtocol.generated.h" // Add this if it's not already there

UENUM()
enum class HPACKET_TYPE : uint16
{
	CHAT_MSG = 0
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
