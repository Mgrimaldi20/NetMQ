#ifndef _NETMQ_CMD_H_
#define _NETMQ_CMD_H_

#include <cstdint>
#include <cstddef>
#include <concepts>
#include <type_traits>
#include <algorithm>
#include <memory>
#include <bit>
#include <iterator>
#include <vector>
#include <span>

#include "framework/SubManager.h"
#include "framework/ByteBuffer.h"

class Cmd
{
public:
	enum class Type : std::underlying_type_t<std::byte>
	{
		Reserved = 0,
		Connect = 1 << 4,
		ConnAck = 2 << 4,
		Publish = 3 << 4,
		PubAck = 4 << 4,
		PubRec = 5 << 4,
		PubRel = 6 << 4,
		PubComp = 7 << 4,
		Subscribe = 8 << 4,
		SubAck = 9 << 4,
		Unsubscribe = 10 << 4,
		UnsubAck = 11 << 4,
		PingReq = 12 << 4,
		PingResp = 13 << 4,
		Disconnect = 14 << 4,
		Auth = 15 << 4
	};

	enum class Property : std::underlying_type_t<std::byte>
	{
		PayloadFormatIndicator = 1,
		MessageExpiryInterval = 2,
		ContentType = 3,
		ResponseTopic = 8,
		CorrelationData = 9,
		SubscriptionIdentifier = 11,
		SessionExpiryInterval = 17,
		AssignedClientIdentifier = 18,
		ServerKeepAlive = 19,
		AuthenticationMethod = 21,
		AuthenticationData = 22,
		RequestProblemInformation = 23,
		WillDelayInterval = 24,
		RequestResponseInformation = 25,
		ResponseInformation = 26,
		ServerReference = 28,
		ReasonString = 31,
		ReceiveMaximum = 33,
		TopicAliasMaximum = 34,
		TopicAlias = 35,
		MaximumQoS = 36,
		RetainAvailable = 37,
		UserProperty = 38,
		MaximumPacketSize = 39,
		WildcardSubscriptionAvailable = 40,
		SubscriptionIdentifierAvailable = 41,
		SharedSubscriptionAvailable = 42
	};

	enum class ReasonCode : std::underlying_type_t<std::byte>
	{
		Success = 0,
		NormalDisconnection = 0,
		GrantedQoS0 = 0,
		GrantedQoS1 = 1,
		GrantedQoS2 = 2,
		DisconnectWithWillMessage = 4,
		NoMatchingSubscribers = 16,
		NoSubscriptionExisted = 17,
		ContinueAuthentication = 24,
		Reauthenticate = 25,
		UnspecifiedError = 128,
		MalformedPacket = 129,
		ProtocolError = 130,
		ImplementationSpecificError = 131,
		UnsupportedProtocolVersion = 132,
		InvalidClientID = 133,
		BadUsernameOrPassword = 134,
		NotAuthorized = 135,
		ServerUnavailable = 136,
		ServerBusy = 137,
		Banned = 138,
		ServerShuttingDown = 139,
		BadAuthenticationMethod = 140,
		KeepAliveTimeout = 141,
		SessionTakenOver = 142,
		TopicFilterInvalid = 143,
		TopicNameInvalid = 144,
		PacketIdentifierInUse = 145,
		PacketIdentifierNotFound = 146,
		ReceiveMaximumExceeded = 147,
		TopicAliasInvalid = 148,
		PacketTooLarge = 149,
		MessageRateTooHigh = 150,
		QuotaExceeded = 151,
		AdministrativeAction = 152,
		PayloadFormatInvalid = 153,
		RetainNotSupported = 154,
		QoSNotSupported = 155,
		UseAnotherServer = 156,
		ServerMoved = 157,
		SharedSubscriptionsNotSupported = 158,
		ConnectionRateExceeded = 159,
		MaximumConnectTime = 160,
		SubscriptionIdentifiersNotSupported = 161,
		WildcardSubscriptionsNotSupported = 162
	};

	virtual ~Cmd() = default;

	void operator()();

protected:
	Cmd(std::shared_ptr<IOContext> ioctx, SubManager &manager) noexcept;

	ByteBuffer ackbuffer;

	std::shared_ptr<IOContext> ioctx;
	SubManager &manager;

private:
	virtual void ExecuteCmd() = 0;
	virtual void ExecuteAck() = 0;

	virtual const bool AckRequired() const noexcept;
};

#endif
