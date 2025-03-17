// Fill out your copyright notice in the Description page of Project Settings.

#include "DBDPacketProcessor.h"
#include "DBDNetManager.h"
#include "google/protobuf/message.h"

void UDBDPacketProcessor::ProcessChatMsg(const TSharedPtr<HPACKET>& packet)
{
	HProtocol::Chat chatMsg;
	UDBDPacketProcessor::DeserializePacket(*packet, chatMsg);

	FString fMsg = FString(UTF8_TO_TCHAR(chatMsg.msg().c_str()));
	OnChatMessageReceived.Broadcast(fMsg);
}

void UDBDPacketProcessor::ProcessKillerMatchACK(const TSharedPtr<HPACKET>& packet)
{
	OnKillerMatchACK.Broadcast();
}

void UDBDPacketProcessor::ProcessSurvivorMatchACK(const TSharedPtr<HPACKET>& packet)
{
	OnSurvivorMatchACK.Broadcast();
}

void UDBDPacketProcessor::ProcessMatchCancelACK(const TSharedPtr<HPACKET>& packet)
{
	OnMatchCancelACK.Broadcast();
}

void UDBDPacketProcessor::ProcessMatchReady(const TSharedPtr<HPACKET>& packet)
{
	FMatchInfo matchInfo;

	HProtocol::MatchReady matchReady;
	UDBDPacketProcessor::DeserializePacket(*packet, matchReady);

	matchInfo.maxPlayer = matchReady.maxplayer();
	matchInfo.killerIP = FString(UTF8_TO_TCHAR(matchReady.killerip().c_str()));
	matchInfo.killerCharacterType = matchReady.killercharacter();
	matchInfo.isServer = static_cast<uint8>(matchReady.isserver());

	for (int i = 0; i < matchReady.survivorip_size(); i++)
	{
		matchInfo.survivorIPs.Add(FString(UTF8_TO_TCHAR(matchReady.survivorip(i).c_str())));
		matchInfo.survivorCharacterTypes.Add(matchReady.survivorcharacter(i));
	}

	OnMatchReady.Broadcast(matchInfo);
}

void UDBDPacketProcessor::ProcessMatchAbandoned(const TSharedPtr<HPACKET>& packet)
{
	OnMatchAbandoned.Broadcast();
}

void UDBDPacketProcessor::Init(class UDBDNetManager* netManager)
{
	if (!IsValid(netManager))
	{
		UE_LOG(LogClass, Error, TEXT("[DBDNet]Invalid NetManager"));
		return;
	}

	m_pNetManager = netManager;

	m_callback.Add(HPACKET_TYPE::CHAT_MSG, [this](const TSharedPtr<HPACKET>& Packet) {
		this->ProcessChatMsg(Packet);
		});

	m_callback.Add(HPACKET_TYPE::ACK_KILLER_MATCH, [this](const TSharedPtr<HPACKET>& Packet) {
		this->ProcessKillerMatchACK(Packet);
		});

	m_callback.Add(HPACKET_TYPE::ACK_SURVIVOR_MATCH, [this](const TSharedPtr<HPACKET>& Packet) {
		this->ProcessSurvivorMatchACK(Packet);
		});

	m_callback.Add(HPACKET_TYPE::ACK_MATCH_CANCEL, [this](const TSharedPtr<HPACKET>& Packet) {
		this->ProcessMatchCancelACK(Packet);
		});

	m_callback.Add(HPACKET_TYPE::SEND_MATCH_READY, [this](const TSharedPtr<HPACKET>& Packet) {
		this->ProcessMatchReady(Packet);
		});

	m_callback.Add(HPACKET_TYPE::SEND_MATCH_ABANDONED, [this](const TSharedPtr<HPACKET>& Packet) {
		this->ProcessMatchAbandoned(Packet);
		});
}

void UDBDPacketProcessor::SendSurvivorMatchRequest(uint8 characterType)
{
	HProtocol::MatchPlayer matchPlayer;
	HPACKET packet;

	matchPlayer.set_playercharacter(static_cast<uint32_t>(characterType));
	UDBDPacketProcessor::SerializePacket(HPACKET_TYPE::SEND_SURVIROR_MATCH, matchPlayer, packet);
	m_pNetManager->SendPacket(packet);
}

void UDBDPacketProcessor::SendKillerMatchRequest(uint8 characterType)
{
	HProtocol::MatchPlayer matchPlayer;
	HPACKET packet;

	matchPlayer.set_playercharacter(static_cast<uint32_t>(characterType));
	UDBDPacketProcessor::SerializePacket(HPACKET_TYPE::SEND_KILLER_MATCH, matchPlayer, packet);
	m_pNetManager->SendPacket(packet);
}

void UDBDPacketProcessor::SendChatMessage(const FString& message)
{
	HProtocol::Chat chatMsg;
	HPACKET packet;

	std::string packetData = TCHAR_TO_UTF8(*message);

	chatMsg.set_msg(packetData);
	UDBDPacketProcessor::SerializePacket(HPACKET_TYPE::CHAT_MSG, chatMsg, packet);
	m_pNetManager->SendPacket(packet);
}

void UDBDPacketProcessor::SendMatchCancel()
{
	HPACKET packet;
	packet.ph.type = HPACKET_TYPE::SEND_MATCH_CANCEL;
	packet.ph.len = PACKET_HEADER_SIZE;
	m_pNetManager->SendPacket(packet);
}

void UDBDPacketProcessor::Release()
{
	m_callback.Empty();
	m_pNetManager = nullptr;
}


void UDBDPacketProcessor::Process(const TSharedPtr<HPACKET>& packet)
{
	if (m_callback.Contains(packet->ph.type))
		m_callback[packet->ph.type](packet);
	else
		UE_LOG(LogClass, Warning, TEXT("[DBDNet]Unknown packet type"));
}
