//S_S ��Ϣ client ��
//	�����ӣ���������-->������ȴ�Ӧ��-->�ر�����

#ifndef SSCLIENT_H
#define SSCLIENT_H

#include <string>
#include "OurDef.h"


using namespace std;

class DataBuffer;
class Packet;

class SSClient
{
public:
	SSClient( string inIp, UInt16 inPort );
	~SSClient();


	
	int SSRequest( Packet &packet);

private:
	int Connect();

	void Close();

	int SendMsg( Packet & packet);

	int RecvMsg( Packet & packet);

	
	int SockSend( void * sendBuff, int iSendLen );


	int SockRecv( void * recvBuff, int iRecvLen );


private:
	string _ip;
	UInt16 _port;
	int _sockFd;
};

#endif

