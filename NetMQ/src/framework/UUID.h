#ifndef _NETMQ_UUID_H_
#define _NETMQ_UUID_H_

#include <string>

/*
* Namespace: NetMQ::UUID
* UUID generators supporting V4 (random) UUIDs. Generates 128-bit UUIDs with 122 bits of randomness.
*
*	GenerateV4: Generates a V4 (random) UUID with cryptographically secure randomness
*/
namespace NetMQ::UUID
{
	std::string GenerateV4();
}

#endif
