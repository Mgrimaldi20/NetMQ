#ifndef _NETMQ_UUID_H_
#define _NETMQ_UUID_H_

#include <string>

/*
* Namespace: UUID
* UUID generators currently only supporting V4 UUIDs, but be warned, this UUID generator is not to spec.
* This UUID generation isnt perfect, and they arent real UUIDs since they are not 128 bits of randomness.
* This code may eventually be deprecated and removed all together in the future in place of system libraries.
*
*	GenerateV4: Generates a V4 (random) UUID
*/
namespace UUID
{
	std::string GenerateV4();
}

#endif
