#include "MultiPlayerGame.h"
#include "Sockets.h"

namespace yamc
{
	MultiPlayerGame::MultiPlayerGame(uint32_t visibleChunksRadius) :
		Game(visibleChunksRadius),
		packageBuffer(PackageBufferCapacity),
		sock(INVALID_SOCKET)
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
		serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(8888);

		if (connect(serverSock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			printf("Cannot connect to server.\n");
			safelyCloseSocket(serverSock);
			return;
		}

		printf("Connected to server.\n");

		sock = serverSock;
	}

	void MultiPlayerGame::update(const glm::vec3& playerPosition, float dt)
	{
		Game::update(playerPosition, dt);
	}

	Chunk* MultiPlayerGame::loadChunk(uint64_t key)
	{
		auto chunk = new Chunk();
		auto offset = getChunkOffset(key);
		fillChunk(chunk, offset.x, offset.y, 85417);
		return chunk;
	}

	void MultiPlayerGame::saveChunk(uint64_t key, Chunk* chunk)
	{
	}

	void MultiPlayerGame::backgroundUpdate()
	{
		Game::backgroundUpdate();

		if (IS_INVALID_SOCKET(sock)) {
			return;
		}

		updateBlockDiffs();
	}

	void MultiPlayerGame::setBlock(int x, int y, int z, uint32_t type)
	{
		Game::setBlock(x, y, z, type);
		blockDiffsToSend.push({ x, y, z, type });
	}

	void MultiPlayerGame::updateBlockDiffs()
	{
		{
			std::lock_guard<std::mutex> guard(blockDiffsToSendMutex);

			packageBuffer.rewind();
			packageBuffer.writeByte((uint8_t)RequestCodes::UpdateBlockDiffs);
			writeBlockDiffsToPackage(packageBuffer, blockDiffsToSend);
		}

		auto result = send(sock, (const char*)packageBuffer.getData(), packageBuffer.getOffset(), 0);
		if (result < 0) {
			safelyCloseSocket(sock);
			sock = INVALID_SOCKET;
			printf("Connection lost.\n");
			return;
		}

		packageBuffer.rewind();
		result = recv(sock, (char*)packageBuffer.getData(), packageBuffer.getCapacity(), 0);
		if (result < 0) {
			safelyCloseSocket(sock);
			sock = INVALID_SOCKET;
			printf("Connection lost.\n");
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

	void MultiPlayerGame::destroy()
	{
		Game::destroy();

		if (!IS_INVALID_SOCKET(sock)) {
			safelyCloseSocket(sock);
		}
		deinitSockets();
	}
}