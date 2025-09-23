#ifndef _NETMQ_CMD_H_
#define _NETMQ_CMD_H_

class Cmd
{
public:
	enum class Type
	{
		Connect,
		Publish,
		Subscribe,
		Unsubscribe,
		Disconnect
	};

	void operator()()
	{
		Process();
	}

	virtual void Process() = 0;
};

#endif
