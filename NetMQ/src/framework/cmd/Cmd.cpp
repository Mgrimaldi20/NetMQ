#include "Cmd.h"

void Cmd::operator()() const
{
	ExecuteCmd();

	if (AckRequired())
		ExecuteAck();
}

Cmd::Cmd(std::shared_ptr<IOContext> ioctx, SubManager &manager) noexcept
	: ioctx(ioctx),
	manager(manager)
{
}

const bool Cmd::AckRequired() const noexcept
{
	return true;
}
