#pragma once
#include <WinSock2.h>
#include <winsock.h>
#include <Windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <array>
#include <random>
#include "Define.h"
#include "../../SlashServer/SlashServer/SingleTone.h"

struct PLAYER {
	DWORD id;
	DirectX::XMFLOAT3 pos;
	std::chrono::high_resolution_clock::time_point lastMoveTime;
	bool connect;
	SOCKET clientSocket;
	OverlappedEx recvOver;
	unsigned char packetBuf[MAX_PACKET_SIZE];
	int prevPacketData;
	int currPacketSize;
};

struct NPC {
	DWORD id;
	DirectX::XMFLOAT3 pos;
	bool connect;
};

class TestManager : public SingleTone<TestManager>
{
public:
	explicit TestManager();
	virtual ~TestManager();

public:
	void DisconnectClient(int cl);
	void ProcessPacket(int ci, unsigned char packet[]);
	void WorkerThread();
	void ConnectPlayer();
	void SendPacket(int cl, void *packet);
	void TestThread();
	void JoinThreads();
	void GetPointCloud(int * size, float ** points);
	void StartTest();
	void ErrorDisplay(char * msg, int err_no);

private:
	const static int INVALID_ID = -1;

private:
	HANDLE hIOCP_;
	int userInput_ = 0;
	std::chrono::high_resolution_clock::time_point lastConnectTime_;
	std::array<PLAYER, NUM_OF_PLAYER> player_;
	std::array<NPC, NUM_OF_NPC_TOTAL> npc_;
	std::atomic_int numOfPlayers_;
	std::vector <std::thread *> workerThreads_;
	std::thread testThread_;
	float point_cloud[NUM_OF_PLAYER * 2];
};