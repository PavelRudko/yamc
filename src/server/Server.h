#ifndef YAMC_SERVER_SERVER_H
#define YAMC_SERVER_SERVER_H

#include "../Sockets.h"
#include "../Network.h"
#include "../Terrain.h"
#include <thread>
#include <atomic>
#include <set>
#include <unordered_map>
#include <queue>
#include <mutex>

namespace yamc
{
	struct ClientInfo
	{
		uint32_t id;
		SOCKET sock;
		bool isConnected;
		std::thread thread;
		std::mutex blockDiffsToSendMutex;
		std::queue<BlockDiff> blockDiffsToSend;
	};

	class Server
	{
	public:
		Server(int port, int seed);
		void start();
		void stop();
		~Server();

	private:
		static constexpr int ConnectionTimeout = 1000;

		int port;
		int seed;
		uint32_t clientIdCounter;
		std::thread serverThread;
		std::atomic_bool isRunning;
		std::set<ClientInfo*> clients;
		std::unordered_map<uint64_t, Chunk*> chunks;
		std::mutex terrainMutex;

		static void startMainLoop(Server* server);
		static void startHandlerThread(Server* server, ClientInfo* client);

		void mainLoop();
		void clientHandler(ClientInfo* client);
		void processConnect(PackageBuffer& packageBuffer, ClientInfo* client);
		void processBlockDiffs(PackageBuffer& packageBuffer, ClientInfo* client);
		void processLoadChunk(PackageBuffer& packageBuffer, ClientInfo* client);
		void broadcastBlockDiffs(const std::vector<BlockDiff>& blockDiffs, uint32_t clientId);
		void applyBlockDiffs(const std::vector<BlockDiff>& blockDiffs);
		bool sendPackage(PackageBuffer& packageBuffer, ClientInfo* client);
		void connectClient(SOCKET clientSock);
		void cleanupCompletedHandlers();
	};
}

#endif