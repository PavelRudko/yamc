#include "MultiPlayerGame.h"
#include "Sockets.h"
#include "Compression.h"

namespace yamc
{
	MultiPlayerGame::MultiPlayerGame(const std::string& serverAddressStr, int port, uint32_t visibleChunksRadius) :
		Game(visibleChunksRadius),
		serverAddressStr(serverAddressStr),
		port(port),
		packageBuffer(PackageBufferCapacity),
		sock(INVALID_SOCKET),
		isConnected(false),
		seed(0)
	{

	}

	void MultiPlayerGame::init()
	{
		Game::init();

		if (initSockets() != 0) {
			printf("Cannot init sockets.\n");
			return;
		}

		SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
		if (IS_INVALID_SOCKET(serverSock)) {
			printf("Cannot create socket.\n");
			return;
		}

		struct sockaddr_in serverAddress;
		serverAddress.sin_addr.s_addr = inet_addr(serverAddressStr.c_str());
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(port);

		if (connect(serverSock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			printf("Cannot connect to server.\n");
			safelyCloseSocket(serverSock);
			return;
		}

		sock = serverSock;

		if (!requestConnection()) {
			printf("Cannot connect to server.\n");
			return;
		}
	}

	void MultiPlayerGame::update(const glm::vec3& playerPosition, float dt)
	{
		Game::update(playerPosition, dt);
	}

	void MultiPlayerGame::backgroundUpdate()
	{
		if (!isConnected) {
			return;
		}

		std::lock_guard<std::mutex> guard(connectionMutex);

		Game::backgroundUpdate();

		updateBlockDiffs();
	}

	void MultiPlayerGame::setBlock(int x, int y, int z, uint32_t type)
	{
		Game::setBlock(x, y, z, type);
		blockDiffsToSend.push({ x, y, z, type });
	}

	bool MultiPlayerGame::requestConnection()
	{
		packageBuffer.rewind();
		packageBuffer.writeByte((uint8_t)RequestCodes::Connect);

		if (!sendPackage(packageBuffer)) {
			return false;
		}

		if (!receivePackage(packageBuffer)) {
			return false;
		}
		packageBuffer.readByte();
		seed = packageBuffer.readInt32();
		isConnected = true;

		printf("Connected to server.\nWorld seed: %d.\n", seed);

		return true;
	}

	void MultiPlayerGame::updateBlockDiffs()
	{
		{
			std::lock_guard<std::mutex> guard(blockDiffsToSendMutex);

			packageBuffer.rewind();
			packageBuffer.writeByte((uint8_t)RequestCodes::UpdateBlockDiffs);
			writeBlockDiffsToPackage(packageBuffer, blockDiffsToSend);
		}

		if (!sendPackage(packageBuffer)) {
			return;
		}

		if (!receivePackage(packageBuffer)) {
			return;
		}

		uint8_t responseCode = packageBuffer.readByte();
		std::vector<BlockDiff> blockDiffs;
		readBlockDiffsFromPackage(packageBuffer, blockDiffs);
		if (blockDiffs.size() > 0) {
			std::lock_guard<std::mutex> guard(terrainMutex);
			for (const auto& blockDiff : blockDiffs) {
				terrain.setBlock(blockDiff.x, blockDiff.y, blockDiff.z, blockDiff.type);
			}
		}
	}

	Chunk* MultiPlayerGame::loadChunk(uint64_t key)
	{
		auto chunk = new Chunk();

		packageBuffer.rewind();
		packageBuffer.writeByte((uint8_t)RequestCodes::LoadChunk);
		packageBuffer.writeUint64(key);

		if (!sendPackage(packageBuffer)) {
			return chunk;
		}

		if (!receivePackage(packageBuffer)) {
			return chunk;
		}

		uint8_t responseCode = packageBuffer.readByte();
		if (responseCode == (uint8_t)ResponseCodes::ChunkDataStart) {
			size_t totalSize = packageBuffer.readUint32();
			printf("Received %d bytes of data.\n", (int)totalSize);
			std::vector<uint8_t> compressedData(totalSize);

			size_t offset = 0;
			size_t bytesToRead = (std::min)(packageBuffer.getAvailableSpace(), totalSize);
			packageBuffer.readBuffer(compressedData.data(), bytesToRead);
			offset += bytesToRead;

			while (totalSize - offset > 0) {
				if (!receivePackage(packageBuffer)) {
					return chunk;
				}

				responseCode = packageBuffer.readByte();
				bytesToRead = (std::min)(packageBuffer.getAvailableSpace(), totalSize - offset);
				packageBuffer.readBuffer(compressedData.data() + offset, bytesToRead);
				offset += bytesToRead;
			}

			decompressData(compressedData.data(), compressedData.size(), chunk->getData(), Chunk::MaxHeight * Chunk::MaxLength * Chunk::MaxWidth * sizeof(uint32_t));
		}
		else if (responseCode == (uint8_t)ResponseCodes::ChunkIsUnchanged) {
			auto chunkOffset = getChunkOffset(key);
			fillChunk(chunk, chunkOffset[0], chunkOffset[1], seed);
		}
		
		return chunk;
	}

	void MultiPlayerGame::saveChunk(uint64_t key, Chunk* chunk)
	{
	}

	void MultiPlayerGame::loadSurroundingChunks(const glm::vec3& playerPosition)
	{
		std::lock_guard<std::mutex> guard(connectionMutex);
		Game::loadSurroundingChunks(playerPosition);
	}

	bool MultiPlayerGame::sendPackage(PackageBuffer& packageBuffer)
	{
		auto result = send(sock, (char*)packageBuffer.getData(), packageBuffer.getCapacity(), 0);
		if (result < 0) {
			if (isConnected) {
				safelyCloseSocket(sock);
				sock = INVALID_SOCKET;
				printf("Connection lost.\n");
				isConnected = false;
			}
			return false;
		}
		return true;
	}

	
	bool MultiPlayerGame::receivePackage(PackageBuffer& packageBuffer)
	{
		packageBuffer.rewind();
		auto result = recv(sock, (char*)packageBuffer.getData(), packageBuffer.getCapacity(), 0);
		if (result < 0) {
			if (isConnected) {
				safelyCloseSocket(sock);
				sock = INVALID_SOCKET;
				printf("Connection lost.\n");
				isConnected = false;
			}
			return false;
		}
		return true;
	}

	void MultiPlayerGame::destroy()
	{
		Game::destroy();

		if (!IS_INVALID_SOCKET(sock)) {
			safelyCloseSocket(sock);
		}
		deinitSockets();
	}
	
}