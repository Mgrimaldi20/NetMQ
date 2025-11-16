#include "Cmd.h"

void Cmd::operator()() const
{
	ExecuteCmd();

	if (AckRequired())
		ExecuteAck();
}

const bool Cmd::AckRequired() const noexcept
{
	return true;
}
