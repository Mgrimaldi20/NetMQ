#include <mutex>
#include <vector>

#include "UnsubscribeCmd.h"

UnsubscribeCmd::UnsubscribeCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params)
	: Cmd(ioctx, manager),
	ackdata({ .type = Cmd::Type::Unsubscribe })
{
	size_t offset = 0;

	std::pair<size_t, uint32_t> ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t topiclen = std::get<1>(ret);
	topic = params.subspan(offset, topiclen);
}

void UnsubscribeCmd::ExecuteCmd()
{
	if (!ioctx->GetConnected().load())
		return;

	std::scoped_lock lock(manager.subsmtx);

	std::span<std::byte> topickey(topic.begin(), topic.end());
	auto iter = manager.subscriptions.find(topickey);

	if (iter != manager.subscriptions.end())
	{
		std::vector<std::shared_ptr<IOContext>> &sublist = iter->second;
		sublist.erase(std::remove(sublist.begin(), sublist.end(), ioctx), sublist.end());

		if (sublist.empty())
			manager.subscriptions.erase(iter);

		ackdata.reason = Cmd::ReasonCode::Success;
	}

	ackdata.reason = Cmd::ReasonCode::NoSubscriptionExisted;
}

void UnsubscribeCmd::ExecuteAck()
{
	ioctx->PostSend(
		ackbuilder
		.AppendUInt<Cmd::Type>(ackdata.type)
		.AppendUInt<Cmd::ReasonCode>(ackdata.reason)
		.Build()
	);
}
