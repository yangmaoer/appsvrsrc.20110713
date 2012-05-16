#ifndef SERVICE_SESSION_H
#define SERVICE_SESSION_H

#include <list>
#include <queue>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <pthread.h>
#include "Packet.h"


class Session
{
public:
	Session();

	Session(const Session& session);

	~Session();

	DataBuffer* GetInBuffer(bool lock=false);

	DataBuffer* GetOutBuffer(bool lock=false);

	void SetInBuffer(DataBuffer* buffer);

	void SetOutBuffer(DataBuffer* buffer);

	void ReleaseInBuffer();

	void ReleaseOutBuffer();

	int Send(DataBuffer* buffer,bool lock=true);

	Packet* Call(Packet* packet);

	int Open();

	void Shutdown();

	void* Service();

	void Service(void* service);

private:
	int WaitSend();

	int SendProc(DataBuffer* buffer);

public:
	sockaddr_in LocalAddress;
	sockaddr_in RemoteAddress;

	int Handle;
	int EventIndex;

	bool Connected;
	unsigned short PacketLength;
	
private:

	//�� session ������Ϣ���Ļ���
	DataBuffer* _inBuffer;

	//�� session ������Ϣ���Ļ��� 
	DataBuffer* _outBuffer;

	//_inBuffer ���߳���
	pthread_mutex_t _inMutex;

	//_outBuffer ���߳���
	pthread_mutex_t _outMutex;

	// �� session ������Ϣ���߳���
	pthread_mutex_t _sendMutex;
	pollfd _pfd;

	void* _service;
};

//��Ϣ������
//	���߳̽���Ϣ������У������̳߳شӶ���ͷȡ��Ϣ�������� 
typedef std::queue<std::pair<Session*,Packet*> > PacketQueue;

#endif //SERVICE_SESSION_H

