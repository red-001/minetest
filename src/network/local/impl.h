#pragma once
// Minetest
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "network/address.h"
#include "network/connection.h"
#include "network/peerhandler.h"
#include "network/local/exchange.h"

#define PEER_ID_LOCAL_CLIENT (PEER_ID_SERVER + 1)

namespace con
{

	class LocalPeer : public IPeer {
	public:
		LocalPeer(session_t id) : IPeer(id) {};

		virtual const Address& getAddress() const override {
			const static Address empty_address = {};

			return empty_address;
		}
	};

	class LocalConnection : public IConnection
	{
	public:

		LocalConnection(bool is_client, std::shared_ptr<LocalNetwork> exchange, PeerHandler* peer_handler) :
			m_is_client(is_client),
			m_exchange(exchange),
			m_peer_handler(peer_handler) {
		}

		~LocalConnection() {
			if (m_connected)
				Disconnect();
		}

		bool Connected() override {
			return m_exchange->connected() && m_connected;
		}
		void Disconnect() override {
			DisconnectPeer(getOtherPeerId());
		}
		void DisconnectPeer(session_t peer_id) override {
			removePeer(peer_id);

			m_connected = false;
		}

		bool ReceiveTimeoutMs(NetworkPacket* pkt, u32 timeout_ms) override {
			if (!m_connected)
				return false;
			bool recieved = m_exchange->read(m_is_client, pkt, timeout_ms);
			// process disconnections only after all packets were processed
			if (!recieved && !m_exchange->connected())
				Disconnect();
			return recieved;
		}

		void Send(session_t peer_id, u8 channelnum, NetworkPacket* pkt, bool reliable) override {
			assert(peer_id == getOtherPeerId());
			NetworkPacket pkt_with_peer_id = pkt->convertToRecievedPacket(GetPeerID());

			m_exchange->send(m_is_client, std::move(pkt_with_peer_id));
		}

		session_t GetPeerID() const override {
			return m_is_client ? PEER_ID_LOCAL_CLIENT : PEER_ID_SERVER;
		}

		void Connect(Address address) override {
			m_connected = true;

			addPeer(PEER_ID_SERVER);
		}

		void Serve(Address bind_addr) override {
			m_connected = true;

			addPeer(PEER_ID_LOCAL_CLIENT);
		};

		// stubs
		Address GetPeerAddress(session_t peer_id) override {
			return Address();
		}
		float getPeerStat(session_t peer_id, rtt_stat_type type) override {
			return 0.0f;
		}
		float getLocalStat(rate_stat_type type) override {
			return 0.0f;
		}


	protected:

		session_t getOtherPeerId() const {
			return m_is_client ? PEER_ID_SERVER : PEER_ID_LOCAL_CLIENT;
		}

		void addPeer(session_t id) const {
			LocalPeer tmp(id);
			m_peer_handler->peerAdded(&tmp);
		}

		void removePeer(session_t id) const {
			LocalPeer tmp(id);
			m_peer_handler->deletingPeer(&tmp, /*timeout=*/false);
		}

	private:
		bool m_is_client;
		bool m_connected = false;
		PeerHandler* m_peer_handler;
		std::shared_ptr<LocalNetwork> m_exchange;
	};
}

#undef PEER_ID_LOCAL_CLIENT
