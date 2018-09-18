#include "AcceptThread.h"
#include "ExceptionUtil.h"


AcceptThread::AcceptThread()
{
	std::wcout.imbue(std::locale("korean"));

	WSADATA   wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	std::cout << "Slash 서버 생성 완료" << std::endl;
	
	Thread::SetIocp(CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0));

	socket_ = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	ZeroMemory(&bindAddr_, sizeof(SOCKADDR_IN));
	bindAddr_.sin_family = AF_INET;
	bindAddr_.sin_port = htons(MY_SERVER_PORT);
	bindAddr_.sin_addr.s_addr = INADDR_ANY;

	::bind(socket_, reinterpret_cast<sockaddr *>(&bindAddr_), sizeof(SOCKADDR));
	listen(socket_, 1000);

	objectManager_ = GameObjectManager::GET_INSTANCE()->GetGameObjectManager();
}

AcceptThread::~AcceptThread()
{
}

void AcceptThread::Run()
{
	while (true) {
		SOCKADDR_IN clientAddress;
		ZeroMemory(&clientAddress, sizeof(SOCKADDR_IN));
		clientAddress.sin_family = AF_INET;
		clientAddress.sin_port = htons(MY_SERVER_PORT);
		clientAddress.sin_addr.s_addr = INADDR_ANY;
		int clientLength = sizeof(SOCKADDR_IN);

		auto newSocket = WSAAccept(socket_, reinterpret_cast<sockaddr *>(&clientAddress), &clientLength, NULL, NULL);
		std::cout << "새로운 클라이언트 접속\n";

		int newKey = -1;
		for (int i = 0; i < NUM_OF_PLAYER; ++i)
			if (false == (objectManager_->GetPlayer(i))->isActive_) {
				newKey = i;
				objectManager_->GetPlayer(i)->ID_ = newKey;
				break;
			}
		if (-1 == newKey) {
			std::cout << "MAX USER EXCEEDED!!!" << std::endl;
			continue;
		}
		GameObject* newPlayer = objectManager_->GetPlayer(newKey);
		auto pNewPlayer = dynamic_cast<Player*>(newPlayer);

		printf("%d번 플레이어 접속.\n", newKey);
		pNewPlayer->s_ = newSocket;

		ZeroMemory(&pNewPlayer->exover_.wsaOver, sizeof(WSAOVERLAPPED));

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(newSocket),Thread::GetIocp(), reinterpret_cast<ULONG_PTR>(newPlayer), 0);
		//g_clients[new_key].viewlist.clear();
		pNewPlayer->viewList_.clear();
		pNewPlayer->viewList_.insert(newPlayer);
		pNewPlayer->isActive_ = true;
		pNewPlayer->exp_ = 0;
		pNewPlayer->level_ = 1;
		pNewPlayer->hp_ = INIT_PLAYER_HP;

		unsigned long flag = 0;
		int ret = WSARecv(newSocket, &pNewPlayer->exover_.wsaBuf, 1,
			NULL, &flag, &pNewPlayer->exover_.wsaOver, NULL);
		if (0 != ret) {
			int errNum = WSAGetLastError();
			if (WSA_IO_PENDING != errNum)
				ExceptionUtil::ErrDisplay("Recv in AcceptThread", errNum);
		}
	}
}
