#include "TestManager.h"

#define START_CONSOLE() {AllocConsole();  freopen("CONOUT$", "w", stdout); freopen("CONIN$", "r", stdin);}
#define STOP_CONSOLE()  {FreeConsole();}

TestManager::TestManager()
{

}

TestManager::~TestManager()
{
}

void TestManager::DisconnectClient(int cl)
{
	closesocket(player_[cl].clientSocket);
	player_[cl].connect = false;
	printf("Client [ %d ] Disconnected!\n", cl);
}

void TestManager::ProcessPacket(int cl, unsigned char packet[])
{
	switch (packet[1]) {
	case SC_WALK_MOVE:
	case SC_ROLL_MOVE:
	{
		sc_packet_move* movePacket = reinterpret_cast<sc_packet_move *>(packet);
		if (movePacket->id < NPC_ID_START)
		{
			if (INVALID_ID == player_[cl].id) player_[cl].id = cl;
			if (cl == movePacket->id) {
				player_[cl].pos.x = movePacket->posX;
				player_[cl].pos.y = movePacket->posY;
				player_[cl].pos.z = movePacket->posZ;
			}
		}
		else
		{
			int npcIndex = cl - NPC_ID_START;
			if (INVALID_ID == npc_[npcIndex].id) npc_[npcIndex].id = npcIndex;
			if (npcIndex == movePacket->id - NPC_ID_START) {
				npc_[npcIndex].pos.x = movePacket->posX;
				npc_[npcIndex].pos.y = movePacket->posY;
				npc_[npcIndex].pos.z = movePacket->posZ;
			}
		}
	} break;
	case SC_PUT_PLAYER: 
	{
		sc_packet_put_player* putPacket = reinterpret_cast<sc_packet_put_player *>(packet);
		if (INVALID_ID == player_[cl].id) player_[cl].id = cl;
		if (cl == putPacket->id) {
			player_[cl].connect = true;
			player_[cl].pos.x = putPacket->posX;
			player_[cl].pos.y = putPacket->posY;
			player_[cl].pos.z = putPacket->posZ;
		}
	} break;
	case SC_PUT_MONSTER: 
	{
		sc_packet_put_monster* putPacket = reinterpret_cast<sc_packet_put_monster *>(packet); 
		int npcIndex = cl - NPC_ID_START;
		if (INVALID_ID == npc_[npcIndex].id) npc_[npcIndex].id = npcIndex;
		if (npcIndex == putPacket->id - NPC_ID_START) {
			npc_[npcIndex].connect = true;
			npc_[npcIndex].pos.x = putPacket->posX;
			npc_[npcIndex].pos.y = putPacket->posY;
			npc_[npcIndex].pos.z = putPacket->posZ;
		}
	} break;
	case SC_REMOVE_OBJECT: 
	{
		sc_packet_remove_object* removePacket = reinterpret_cast<sc_packet_remove_object *>(packet);
		if (removePacket->id < NPC_ID_START)
		{
			if (INVALID_ID == player_[cl].id) player_[cl].id = cl;
			if (cl == removePacket->id) 
				player_[cl].connect = false;
		}
		else
		{
			int npcIndex = cl - NPC_ID_START;
			if (INVALID_ID == npc_[npcIndex].id) npc_[npcIndex].id = npcIndex;
			if (npcIndex == removePacket->id - NPC_ID_START)
				npc_[npcIndex].connect = false;
		}
	} break;
	case SC_CHAT: break;
	case SC_ROTATE: break;
	case SC_STATE: break;
	case SC_HP: break;
	case SC_DAMAGE: break;
	case SC_ULTIMATE_WARRIOR: break;
	case SC_ULTIMATE_WIZARD: break;
	case SC_ULTIMATE_OFF: break;
	case SC_LEVEL_UP: break;
	case SC_EXP: break;
	case SC_WIZARD_HEAL: break;
	case SC_PLAY_ENDING: break;
	default: std::cout << "Unknown Packet Type from Server : " << cl << std::endl;
		while (true);
	}
}

void TestManager::WorkerThread()
{
	while (true) {
		DWORD ioSize;
		unsigned long long cl;
		OverlappedEx *over;
		BOOL ret = GetQueuedCompletionStatus(hIOCP_, &ioSize, &cl,
			reinterpret_cast<LPWSAOVERLAPPED *>(&over), INFINITE);
		// std::cout << "GQCS :";
		if (FALSE == ret) {
			int err_no = WSAGetLastError();
			if (64 == err_no) DisconnectClient(cl);
			else ErrorDisplay("GQCS : ", WSAGetLastError());
		}
		if (0 == ioSize) {
			DisconnectClient(cl);
			continue;
		}
		if (OP_RECV == over->eventType) {
			//std::cout << "RECV from Client :" << cl;	// For Debug
			//std::cout << "  IO_SIZE : " << ioSize << std::endl;
			unsigned char *buf = player_[cl].recvOver.iocpBuf;
			unsigned pSize = player_[cl].currPacketSize;
			unsigned preSize = player_[cl].prevPacketData;
			while (ioSize > 0) {
				if (0 == pSize) pSize = buf[0];
				if (ioSize + preSize >= pSize) {
					// 지금 패킷 완성 가능
					unsigned char packet[MAX_PACKET_SIZE];
					memcpy(packet, player_[cl].packetBuf, preSize);
					memcpy(packet + preSize, buf, pSize - preSize);
					ProcessPacket(static_cast<int>(cl), packet);
					ioSize -= pSize - preSize;
					buf += pSize - preSize;
					pSize = 0; preSize = 0;
				}
				else {
					memcpy(player_[cl].packetBuf + preSize, buf, ioSize);
					preSize += ioSize;
					ioSize = 0;
				}
			}
			player_[cl].currPacketSize = pSize;
			player_[cl].prevPacketData = preSize;
			DWORD recv_flag = 0;
			WSARecv(player_[cl].clientSocket,
				&player_[cl].recvOver.wsaBuf, 1,
				NULL, &recv_flag, &player_[cl].recvOver.over, NULL);
		}
		else if (OP_SEND == over->eventType) {
			if (ioSize != over->wsaBuf.len) {
				std::cout << "Send Incomplete Error!\n";
				closesocket(player_[cl].clientSocket);
				player_[cl].connect = false;
			}
			delete over;
		}
		else if (OP_DO_MOVE == over->eventType) {
			// Not Implemented Yet
			delete over;
		}
		else {
			std::cout << "Unknown GQCS event!\n";
			while (true);
		}
	}
}

void TestManager::ConnectPlayer()
{
	if (numOfPlayers_ == NUM_OF_PLAYER) return;
	if (std::chrono::high_resolution_clock::now() < lastConnectTime_ + std::chrono::milliseconds(100)) return;

	player_[numOfPlayers_].clientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int Result = WSAConnect(player_[numOfPlayers_].clientSocket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);
	if (0 != Result) {
		int err_no = WSAGetLastError();
		ErrorDisplay("Error in Connect:", err_no);
	}

	player_[numOfPlayers_].currPacketSize = 0;
	player_[numOfPlayers_].prevPacketData = 0;
	ZeroMemory(&player_[numOfPlayers_].recvOver, sizeof(player_[numOfPlayers_].recvOver));
	player_[numOfPlayers_].recvOver.eventType = OP_RECV;
	player_[numOfPlayers_].recvOver.wsaBuf.buf =
		reinterpret_cast<CHAR *>(player_[numOfPlayers_].recvOver.iocpBuf);
	player_[numOfPlayers_].recvOver.wsaBuf.len = sizeof(player_[numOfPlayers_].recvOver.iocpBuf);

	DWORD recv_flag = 0;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(player_[numOfPlayers_].clientSocket), hIOCP_, numOfPlayers_, 0);
	WSARecv(player_[numOfPlayers_].clientSocket, &player_[numOfPlayers_].recvOver.wsaBuf, 1,
		NULL, &recv_flag, &player_[numOfPlayers_].recvOver.over, NULL);
	player_[numOfPlayers_].connect = true;

	if (MULTI_TEST == userInput_)
	{
		std::random_device rd;
		std::default_random_engine dre(rd());
		std::uniform_int_distribution<> uidWidth(-MAP_WIDTH, MAP_WIDTH);
		std::uniform_int_distribution<> uidHeight(-MAP_HEIGHT, MAP_HEIGHT);

		player_[numOfPlayers_].pos.x = uidWidth(dre);
		player_[numOfPlayers_].pos.y = 0;
		player_[numOfPlayers_].pos.z = uidHeight(dre);
		cs_packet_multi_test p;
		p.type = CS_MULTI_TEST;
		p.size = sizeof(p);
		p.x = player_[numOfPlayers_].pos.x;
		p.z = player_[numOfPlayers_].pos.z;
		SendPacket(numOfPlayers_, &p);
	}
	else
	{
		cs_packet_hotspot_test p;
		p.type = CS_HOTSPOT_TEST;
		p.size = sizeof(p);
		SendPacket(numOfPlayers_, &p);
	}
	player_[numOfPlayers_].lastMoveTime = std::chrono::high_resolution_clock::now();
	numOfPlayers_++;
}

void TestManager::SendPacket(int cl, void *packet)
{
	int psize = reinterpret_cast<unsigned char *>(packet)[0];
	int ptype = reinterpret_cast<unsigned char *>(packet)[1];

	OverlappedEx *over = new OverlappedEx;
	over->eventType = OP_SEND;
	memcpy(over->iocpBuf, packet, psize);
	ZeroMemory(&over->over, sizeof(over->over));
	over->wsaBuf.buf = reinterpret_cast<CHAR *>(over->iocpBuf);
	over->wsaBuf.len = psize;
	int ret = WSASend(player_[cl].clientSocket, &over->wsaBuf, 1, NULL, 0,
		&over->over, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			ErrorDisplay("Error in SendPacket:", err_no);
	}
	//std::cout << "Send Packet [" << ptype << "] To Client : " << cl << std::endl;
}

void TestManager::TestThread()
{
	cs_packet_dir movePacket;

	while (true) {
		Sleep(10);
		ConnectPlayer();

		for (int i = 0; i < numOfPlayers_; ++i) {
			if (false == player_[i].connect) continue;
			if (player_[i].lastMoveTime + std::chrono::seconds(1) > std::chrono::high_resolution_clock::now()) continue;
			player_[i].lastMoveTime = std::chrono::high_resolution_clock::now();

			movePacket.type = 0;
			switch (rand() % 4)
			{
			case 0: movePacket.type |= CS_DIR_FORWARD; break;
			case 1: movePacket.type |= CS_DIR_BACKWARD; break;
			case 2: movePacket.type |= CS_DIR_LEFT; break;
			case 3: movePacket.type |= CS_DIR_RIGHT; break;
			}
			movePacket.type |= CS_TEST_MOVE;
			movePacket.size = sizeof(movePacket);
			SendPacket(i, &movePacket);
		}
	}
}

void TestManager::JoinThreads()
{
	testThread_.join();
	for (auto pth : workerThreads_) {
		pth->join();
		delete pth;
	}
}

void TestManager::GetPointCloud(int *size, float **points)
{
	for (int i = 0; i < numOfPlayers_; ++i) {
		point_cloud[i * 2] = player_[i].pos.x;
		point_cloud[i * 2 + 1] = player_[i].pos.z;
	}
	*size = numOfPlayers_;
	*points = point_cloud;
}

void TestManager::StartTest()
{
	START_CONSOLE();

	while (true)
	{
		system("cls");
		printf("Select Test Mode\n");
		printf("1. Hotspot Test\n");
		printf("2. Concurrent Access Test\n");
		printf("Input : ");
		std::cin >> userInput_;
		if (userInput_ == HOTSPOT_TEST) break;
		if (userInput_ == MULTI_TEST) break;
	}

	for (int i = 0; i < NUM_OF_PLAYER; ++i) {
		player_[i].connect = false;
		player_[i].id = INVALID_ID;
	}
	for (int i = 0; i < NUM_OF_NPC_TOTAL; ++i) {
		npc_[i].connect = false;
		npc_[i].id = INVALID_ID;
	}

	numOfPlayers_ = 0;
	lastConnectTime_ = std::chrono::high_resolution_clock::now();

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	hIOCP_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);

	for (int i = 0; i < 6; ++i)
		workerThreads_.push_back(new std::thread{ [&]() { TestManager::WorkerThread(); } });

	testThread_ = std::thread{ [&]() { TestManager::TestThread(); } };
}

void TestManager::ErrorDisplay(char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg);
	std::wcout << L"에러" << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
}

