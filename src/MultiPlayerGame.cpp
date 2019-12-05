#include "MultiPlayerGame.h"
#include "Sockets.h"

namespace yamc
{
	MultiPlayerGame::MultiPlayerGame(uint32_t visibleChunksRadius) :
		Game(visibleChunksRadius),
		worldDataManager(567421, "test")
	{

	}

	void MultiPlayerGame::init()
	{
		Game::init();

		if (initSockets() != 0) {
			printf("Cannot init sockets.");
			return;
		}

		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (IS_INVALID_SOCKET(sock)) {
			printf("Cannot create socket.");
			return;
		}

		struct sockaddr_in serverAddress;
		serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(8888);

		if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			printf("Cannot connect to server.");
			return;
		}

		char message[1024];
		int messageSize = recv(sock, message, 1024, 0);
		if (messageSize < 0) {
			printf("Cannot receive message.");
			return;
		}

		printf("%s\n", message);

		safelyCloseSocket(sock);
		deinitSockets();
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

	void MultiPlayerGame::destroy()
	{
		Game::destroy();

		auto chunks = terrain.getChunks();
		for (uint64_t key : terrain.getChunkKeysToSave()) {
			worldDataManager.saveChunk(key, chunks[key]);
		}
	}
}