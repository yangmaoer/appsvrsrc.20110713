//�������������� ������

#ifndef SSCLIENTMANAGER_H
#define SSCLIENTMANAGER_H
#include <string>
#include <map>
#include "Mutex.h"
#include "OurDef.h"
#include "List.h"


using namespace std;


class SSClient;
class ArchvRolePKInfo;
class ArchvRoleInfo;

class ServerInfo
{
public:
	ServerInfo():svrType(0),svrSeq(0), port(0)
	{}
public:
	Byte svrType;
	Byte svrSeq;
	string ip;
	UInt16	port;
};

class SSClientManager
{
public:
	SSClientManager(char* cfg);
	~SSClientManager();

	//��ʼ��
	int Init();


	//------------svrType 1  main server-----------------------------------------
	int	ProcessGetRoleInfo(	UInt32 roleID, ArchvRoleInfo & ari) ;

	//------------svrType 2  PK server-----------------------------------------
	int	ProcessPkReq(	UInt32 mapID,	UInt16 X,	UInt16 Y, List<ArchvRolePKInfo> &lpk, UInt32 &pkID );

	
private:
	
	int GetConf(const char* cfg);

	SSClient * GetClientHandle( Byte svrType, Byte svrSeq );

private:
	//�߳���
	mutable MutexLock mutex_;
	
	//�������б�
	//	(�����±�+1)��ʾ svrType
	SSClient	* _arrClient[MAX_SVRTYPE][MAX_SVRSEQ];

	//�������� ���������
	int _maxSvrSeq[MAX_SVRTYPE];

	//����ĵ�ǰ ���������
	int _currSvrSeq[MAX_SVRTYPE];

	//�����ļ�
	char* _cfg;

	//��������Ϣ
	list<ServerInfo> _listServer;

};


#endif


