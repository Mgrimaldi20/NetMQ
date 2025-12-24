#include <mutex>
#include <vector>

#include "DisconnectCmd.h"

DisconnectCmd::DisconnectCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, ByteBuffer &params)
	: Cmd(ioctx, manager)
{
	(void)params;
}

void DisconnectCmd::ExecuteCmd()
{
	if (!ioctx->GetConnected().load())
		return;

	std::scoped_lock lock(manager.subsmtx);

	for (auto &[topic, sublist] : manager.subscriptions)
		sublist.erase(std::remove(sublist.begin(), sublist.end(), ioctx), sublist.end());

	std::erase_if(manager.subscriptions, [](const auto &pair) { return pair.second.empty(); });

	ioctx->CloseClient();
}

void DisconnectCmd::ExecuteAck()
{
}

const bool DisconnectCmd::AckRequired() const noexcept
{
	return false;
}
