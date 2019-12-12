#include "Server.h"
#include <stdexcept>
#include "../Compression.h"
#include <iostream>
#include <memory>

namespace yamc
{
	Server::Server(int port, int seed) :
		serverThread(),
		isRunning(false),
		port(port),
		seed(seed),
		clientIdCounter(0)
	{
		if (initSockets() != 0) {
			throw std::runtime_error("Cannot initialize sockets.");
		}
	}

	void Server::start()
	{
		isRunning = true;
		serverThread = std::move(std::thread(startMainLoop, this));
	}

	void Server::mainLoop()
	{
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (IS_INVALID_SOCKET(sock)) {
			throw std::runtime_error("Cannot create socket.");
		}

		struct sockaddr_in serverAddress;
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = INADDR_ANY;
		serverAddress.sin_port = htons(port);

		if (bind(sock, (struct sockaddr*) & serverAddress, sizeof(serverAddress)) < 0) {
			throw std::runtime_error("Cannot bind socket.");
		}

		listen(sock, 5);

		if (!setToNonBlockingMode(sock)) {
			throw std::runtime_error("Cannot set socket to non-blocking mode.");
		}

		while (isRunning) {
			struct sockaddr_in clientAddress;
			socklen_t clientAddressLength = sizeof(clientAddress);
			SOCKET clientSock = accept(sock, (struct sockaddr*)&clientAddress, &clientAddressLength);
			if (!IS_INVALID_SOCKET(clientSock)) {
				connectClient(clientSock);
			}

			cleanupCompletedHandlers();
		}

		for (auto client : clients) {
			client->thread.join();
			delete client;
		}

		safelyCloseSocket(sock);
	}

	void Server::clientHandler(ClientInfo* client)
	{
		PackageBuffer packageBuffer(PackageBufferCapacity);
		setToNonBlockingMode(client->sock);

		auto lastTickTime = std::chrono::high_resolution_clock::now();

		while (isRunning && client->isConnected) {
			packageBuffer.rewind();
			auto bytesRead = recv(client->sock, (char*)packageBuffer.getData(), PackageBufferCapacity, 0);
			if (bytesRead <= 0) {
				auto currentTime = std::chrono::high_resolution_clock::now();
				auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTickTime).count();
				if (elapsedMilliseconds > ConnectionTimeout) {
					client->isConnected = false;
					break;
				}
				continue;
			}

			uint8_t requestCode = packageBuffer.readByte();
			switch (requestCode) {
			case (uint8_t)RequestCodes::Connect:
				processConnect(packageBuffer, client);
				break;
			case (uint8_t)RequestCodes::UpdateBlockDiffs:
				processBlockDiffs(packageBuffer, client);
				break;
			case (uint8_t)RequestCodes::LoadChunk:
				processLoadChunk(packageBuffer, client);
				break;
			}

			lastTickTime = std::chrono::high_resolution_clock::now();
		}
	}

	void Server::processConnect(PackageBuffer& packageBuffer, ClientInfo* client)
	{
		packageBuffer.rewind();
		packageBuffer.writeByte((uint8_t)ResponseCodes::WorldSettings);
		packageBuffer.writeInt32(seed);

		sendPackage(packageBuffer, client);
	}

	void Server::processBlockDiffs(PackageBuffer& packageBuffer, ClientInfo* client)
	{
		std::vector<BlockDiff> blockDiffs;
		readBlockDiffsFromPackage(packageBuffer, blockDiffs);
		applyBlockDiffs(blockDiffs);
		
		if (blockDiffs.size() > 0) {
			broadcastBlockDiffs(blockDiffs, client->id);
		}

		packageBuffer.rewind();
		packageBuffer.writeByte((uint8_t)ResponseCodes::UpdateBlockDiffs);
		std::lock_guard<std::mutex> guard(client->blockDiffsToSendMutex);
		writeBlockDiffsToPackage(packageBuffer, client->blockDiffsToSend);

		sendPackage(packageBuffer, client);
	}

	void Server::processLoadChunk(PackageBuffer& packageBuffer, ClientInfo* client)
	{
		std::lock_guard<std::mutex> guard(terrainMutex);
		uint64_t chunkKey = packageBuffer.readUint64();

		auto chunkEntry = chunks.find(chunkKey);
		if (chunkEntry == chunks.end()) {
			packageBuffer.rewind();
			packageBuffer.writeByte((uint8_t)ResponseCodes::ChunkIsUnchanged);
			sendPackage(packageBuffer, client);
			return;
		}

		auto chunk = chunkEntry->second;

		std::vector<uint8_t> compressedData;
		compressData(chunk->getData(), Chunk::MaxHeight * Chunk::MaxLength * Chunk::MaxWidth * sizeof(uint32_t), compressedData);

		size_t offset = 0;
		size_t totalSize = compressedData.size();

		packageBuffer.rewind();
		packageBuffer.writeByte((uint8_t)ResponseCodes::ChunkDataStart);
		packageBuffer.writeUint32(totalSize);
		size_t bytesToWrite = std::min(packageBuffer.getAvailableSpace(), totalSize);
		packageBuffer.writeBuffer(compressedData.data(), bytesToWrite);
		if (!sendPackage(packageBuffer, client)) {
			return;
		}
		offset += bytesToWrite;

		while (totalSize - offset > 0) {
			packageBuffer.rewind();
			packageBuffer.writeByte((uint8_t)ResponseCodes::ChunkDataPart);
			bytesToWrite = (std::min)(packageBuffer.getAvailableSpace(), totalSize - offset);
			packageBuffer.writeBuffer(compressedData.data() + offset, bytesToWrite);
			if (!sendPackage(packageBuffer, client)) {
				return;
			}
			offset += bytesToWrite;
		}
	}

	void Server::broadcastBlockDiffs(const std::vector<BlockDiff>& blockDiffs, uint32_t clientId)
	{
		for (auto client : clients) {
			if (client->id != clientId) {
				std::lock_guard<std::mutex> guard(client->blockDiffsToSendMutex);
				for (const auto& blockDiff : blockDiffs) {
					client->blockDiffsToSend.push(blockDiff);
				}
			}
		}
	}

	void Server::applyBlockDiffs(const std::vector<BlockDiff>& blockDiffs)
	{
		std::lock_guard<std::mutex> guard(terrainMutex);
		for (const auto& blockDiff : blockDiffs) {
			int chunkX = getChunkIndex(blockDiff.x, Chunk::MaxWidth);
			int chunkZ = getChunkIndex(blockDiff.z, Chunk::MaxLength);

			uint32_t localX = getLocalBlockIndex(blockDiff.x, Chunk::MaxWidth);
			uint32_t localZ = getLocalBlockIndex(blockDiff.z, Chunk::MaxLength);

			uint64_t chunkKey = getChunkKey(chunkX, chunkZ);

			Chunk* chunk = nullptr;
			auto chunkEntry = chunks.find(chunkKey);
			if (chunkEntry == chunks.end()) {
				chunk = new Chunk;
				fillChunk(chunk, chunkX, chunkZ, seed);
				chunks[chunkKey] = chunk;
			}
			else {
				chunk = chunkEntry->second;
			}

			chunk->setBlock(localX, blockDiff.y, localZ, blockDiff.type);
		}
	}

	bool Server::sendPackage(PackageBuffer& packageBuffer, ClientInfo* client)
	{
		auto result = send(client->sock, (const char*)packageBuffer.getData(), packageBuffer.getOffset(), 0);
		if (result < 0) {
			client->isConnected = false;
			return false;
		}
		return true;
	}

	void Server::startMainLoop(Server* server)
	{
		server->mainLoop();
	}

	void Server::startHandlerThread(Server* server, ClientInfo* client)
	{
		server->clientHandler(client);
	}

	void Server::connectClient(SOCKET clientSock)
	{
		auto client = new ClientInfo;
		client->isConnected = true;
		client->sock = clientSock;
		client->id = clientIdCounter;
		client->thread = std::move(std::thread(startHandlerThread, this, client));
		clients.insert(client);
		printf("Client %d has connected.\n", client->id);
		clientIdCounter++;
	}

	void Server::cleanupCompletedHandlers()
	{
		for (auto it = clients.begin(); it != clients.end(); ) {
			auto client = *it;
			if (!client->isConnected) {
				client->thread.join();
				printf("Client %d has disconnected.\n", client->id);
				it = clients.erase(it);
				delete client;
			}
			else {
				++it;
			}
		}
	}

	void Server::stop()
	{
		isRunning = false;
		serverThread.join();
		for (auto chunkEntry : chunks) {
			delete chunkEntry.second;
		}
	}

	Server::~Server()
	{
		deinitSockets();
	}
}
