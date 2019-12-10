#include "MultiPlayerGame.h"
#include "Sockets.h"

namespace yamc
{
	MultiPlayerGame::MultiPlayerGame(uint32_t visibleChunksRadius) :
		Game(visibleChunksRadius),
		worldDataManager(567421, "test"),
		packageBuffer(PackageBufferCapacity),
		sock(INVALID_SOCKET)
	{

	}

	void MultiPlayerGame::init()
	{
		if (initSockets() != 0) {
			printf("Cannot init sockets.\n");
			return;
		}

		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (IS_INVALID_SOCKET(sock)) {
			printf("Cannot create socket.\n");
			return;
		}

		struct sockaddr_in serverAddress;
		serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(8888);

		if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			printf("Cannot connect to server.\n");
			return;
		}

		printf("Connected to server.\n");

		Game::init();
	}

	void MultiPlayerGame::update(const glm::vec3& playerPosition, float dt)
	{
		Game::update(playerPosition, dt);
	}

	Chunk* MultiPlayerGame::loadChunk(uint64_t key)
	{
		return worldDataManager.loadChunk(key);
	}

	void MultiPlayerGame::saveChunk(uint64_t key, Chunk* chunk)
	{
		worldDataManager.saveChunk(key, chunk);
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
		std::lock_guard<std::mutex> guard(blockDiffsToSendMutex);

		packageBuffer.rewind();
		packageBuffer.writeByte((uint8_t)RequestCodes::UpdateBlockDiffs);

		packageBuffer.writeByte(blockDiffsToSend.size());
		while (!blockDiffsToSend.empty()) {
			auto diff = blockDiffsToSend.front();
			blockDiffsToSend.pop();

			packageBuffer.writeInt32(diff.x);
			packageBuffer.writeInt32(diff.y);
			packageBuffer.writeInt32(diff.z);
			packageBuffer.writeUint32(diff.type);
		}

		auto result = send(sock, (const char*)packageBuffer.getData(), packageBuffer.getOffset(), 0);
		if (result < 0) {
			safelyCloseSocket(sock);
			sock = INVALID_SOCKET;
			printf("Connection lost.\n");
		}
	}

	void MultiPlayerGame::destroy()
	{
		Game::destroy();

		if (!IS_INVALID_SOCKET(sock)) {
			safelyCloseSocket(sock);
		}
		deinitSockets();

		auto chunks = terrain.getChunks();
		for (uint64_t key : terrain.getChunkKeysToSave()) {
			worldDataManager.saveChunk(key, chunks[key]);
		}
	}
}