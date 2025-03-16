// Fill out your copyright notice in the Description page of Project Settings.

#include "DBDPacketProcessor.h"

void UDBDPacketProcessor::ProcessChatMsg(const TSharedPtr<HPACKET>& packet)
{
	HProtocol::Chat chatMsg;
	UDBDPacketProcessor::DeserializePacket(*packet, chatMsg);

	FString fMsg = FString(UTF8_TO_TCHAR(chatMsg.msg().c_str()));
	OnChatMessageReceived.Broadcast(fMsg);
}

void UDBDPacketProcessor::ProcessKillerMatchACK(const TSharedPtr<HPACKET>& packet)
{
}

void UDBDPacketProcessor::ProcessSurvivorMatchACK(const TSharedPtr<HPACKET>& packet)
{
}

void UDBDPacketProcessor::Init()
{
	m_callback.Add(HPACKET_TYPE::CHAT_MSG, [this](const TSharedPtr<HPACKET>& Packet) {
		this->ProcessChatMsg(Packet);
		});

	m_callback.Add(HPACKET_TYPE::ACK_KILLER_MATCH, [this](const TSharedPtr<HPACKET>& Packet) {
		this->ProcessKillerMatchACK(Packet);
		});

	m_callback.Add(HPACKET_TYPE::ACK_SURVIVOR_MATCH, [this](const TSharedPtr<HPACKET>& Packet) {
		this->ProcessSurvivorMatchACK(Packet);
		});
}


void UDBDPacketProcessor::Process(const TSharedPtr<HPACKET>& packet)
{
	if (m_callback.Contains(packet->ph.type))
		m_callback[packet->ph.type](packet);
	else
		UE_LOG(LogClass, Warning, TEXT("[DBDNet]Unknown packet type"));
}
