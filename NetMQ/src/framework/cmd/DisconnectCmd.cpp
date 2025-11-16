#include <mutex>
#include <vector>

#include "DisconnectCmd.h"

DisconnectCmd::DisconnectCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params)
	: Cmd(ioctx)
{
	std::pair<size_t, uint8_t> ret = CmdUtil::ReadUInt<uint8_t>(params, 0);
}

void DisconnectCmd::ExecuteCmd() const
{
	if (!ioctx->GetConnected().load())
		return;

	std::scoped_lock lock(subsmtx);

	for (auto &[topic, sublist] : subscriptions)
		sublist.erase(std::remove(sublist.begin(), sublist.end(), ioctx), sublist.end());

	std::erase_if(subscriptions, [](const auto &pair) { return pair.second.empty(); });

	ioctx->CloseClient();
}

void DisconnectCmd::ExecuteAck() const
{
}

const bool DisconnectCmd::AckRequired() const noexcept
{
	return false;
}
