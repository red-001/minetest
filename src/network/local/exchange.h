#pragma once
// Minetest
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "network/networkpacket.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace con
{

	// thread safe packet queue, handles locking and timeout
	class PacketQueue
	{
	public:
		// fully thread-safe
		void push(NetworkPacket&& pkt) {
			{
				std::lock_guard lock{ m_mutex };
				m_shared_queue.push(std::move(pkt));
			}

			m_shared_modified_cv.notify_one();
		}

		// not thread safe, only call on the recieving side
		bool read(NetworkPacket* pkt, u32 timeout_milliseconds) {
			// grab the shared queue if we ran out of local packets
			if (m_recieved_queue.empty()) {
				std::lock_guard lock{ m_mutex };
				std::swap(m_recieved_queue, m_shared_queue);
			}

			if (readLocalQueue(pkt))
				return true;

			// no packets waiting for us!
			// wait for one if there's a timeout set
			if (timeout_milliseconds != 0) {
				std::unique_lock lock{ m_mutex };
				m_shared_modified_cv.wait_for(lock, std::chrono::microseconds(timeout_milliseconds));

				std::swap(m_recieved_queue, m_shared_queue);
				lock.unlock();

				// see if we now have a packet
				if (readLocalQueue(pkt))
					return true;
			}

			return false;
		}


	private:

		bool readLocalQueue(NetworkPacket* pkt) {
			if (m_recieved_queue.empty())
				return false;
			*pkt = std::move(m_recieved_queue.front());
			m_recieved_queue.pop();
			return true;
		}


		std::mutex m_mutex;
		std::queue<NetworkPacket> m_shared_queue;
		std::condition_variable m_shared_modified_cv;

		std::queue<NetworkPacket> m_recieved_queue;
	};

	// handles exchanging packets between the server and client thread
	// as well as keeping track of "connection" status
	class LocalNetwork
	{
	public:

		void send(bool is_client, NetworkPacket&& pkt) {
			PacketQueue* queue = is_client ? &m_server_pending_queue : &m_client_pending_queue;
			queue->push(std::move(pkt));
		}

		bool read(bool is_client, NetworkPacket* pkt, u32 timeout_milliseconds = 0) {
			PacketQueue* queue = is_client ? &m_client_pending_queue : &m_server_pending_queue;
			return queue->read(pkt, timeout_milliseconds);
		}

		bool connected() {
			return m_is_connected.load(std::memory_order_relaxed);
		}

		bool disconnect() {
			m_is_connected = false;
		}

	private:
		// thread-safe queues
		PacketQueue m_client_pending_queue;
		PacketQueue m_server_pending_queue;

		std::atomic<bool> m_is_connected = true;
	};
}
