// Minetest
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "network/connection.h"
#include "network/mtp/impl.h"
#include "network/local/impl.h"

namespace con
{

IConnection *createMTP(float timeout, bool ipv6, PeerHandler *handler)
{
	// safe minimum across internet networks for ipv4 and ipv6
	constexpr u32 MAX_PACKET_SIZE = 512;
	return new con::Connection(MAX_PACKET_SIZE, timeout, ipv6, handler);
}

IConnection* createLocalConnection(bool is_client,
	std::shared_ptr<LocalNetwork> exchange,
	PeerHandler* peer_handler)
{
	return new con::LocalConnection(is_client, exchange, peer_handler);
}

}
