// Fill out your copyright notice in the Description page of Project Settings.

/*
	Author : 변한빛
	Last Update : 2025-03-03
	Description : DBD 수신 받는 쓰레드 정의
*/

#pragma once

#include "CoreMinimal.h"
#include "DBDNetManager.h"

class DBDNetWorker : public FRunnable
{
private:
	UDBDNetManager* m_pNetManager;
	char 			m_recvBuffer[MAX_BUFFER_SIZE];
	int				m_readPos;
	bool			m_bRunning;

public:
	DBDNetWorker(UDBDNetManager* pNetManager);
	~DBDNetWorker();

	void ReceivePacket();

public:
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
};
