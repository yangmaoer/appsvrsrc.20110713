#ifndef APPGWSVC_H
#define APPGWSVC_H

#include <map>
#include <sys/epoll.h>

#include <queue>


#include "PkgHead.h"
#include "System.h"
#include "ConnectSession.h"
#include "PkgBuff.h"
#include "Mutex.h"
#include "Condition.h"

using namespace std;


typedef map<int,ConnectSession*> FD_CONNSS_MAP;
typedef map<int,int> ROLEID_FD_MAP;
typedef map<int,int> IP_COUNT_MAP;

class AppGwSvc
{
public:
	AppGwSvc();
	~AppGwSvc();
	
	int init();
	
	void run_once();
	void run();
	void HandleThdPopFd( int thdSeq );

private:
	int InsertConnSession(ConnectSession* s);
	void CloseMarkConnSession(ConnectSession* s);
	
	void RemoveConnSession();
	
	int RemoveRoleFdMap( u_int roleid );
	int CheckIpCount( const struct sockaddr_in &peer );
	int ListenInit( int &lsfd, char * ip, u_short port );	
	
	void HandleAccept( const int lsfd );
	void HandlePushFd( const int fd );
	int FindUserFd( u_int roleid );
	void ProcessC2S( ConnectSession* reqConnSs, const char * reqPkg );
	void ProcessS2C( ConnectSession* reqConnSs, const char * reqPkg );
	void ProcessS2G( ConnectSession* reqConnSs, const char * reqPkg );
	void HandleProcessPkg( int fd, char * ptrBuff );
	int HandleProcessInsidePkg( ConnectSession* s, char * pkg, int iLen );
	int HandleProcessOutsidePkg( ConnectSession* s, char * pkg, int iLen );
	ConnectSession* FindFdConnSession(int fd);
	void check_timeout(ConnectSession* s, int timeout);
	void NotifyClientClosed( ConnectSession *s );
	int InsertUserFdMap( u_int roleid, int fd );
	int UpdateUserFdMap( u_int roleid, int fd );
	ConnectSession* FindRoleConnSession( u_int roleid );
	int check_srv_scope( unsigned char ucSrvType, unsigned char ucSrvSeq );
	void MakeClientAckPkg( PkgBuff &buff, PkgHead & pkgHead, unsigned int uiRetCode );
	static void * thread_ProcMsg(void *arg);

	static void * thread_CloseConn(void *arg);

	static void * thread_ResetListenFd(void *input);

	void ResetListenFd();
	

	int init1();
	static void * thread_accept_isd_blk(void *input);
	static void * thread_accept_osd_blk(void *input);
	void HandleAcceptBlk( const int lsfd );
	int ListenInit1( int &lsfd, char * ip, u_short port );	

	int GetListenIsdFd();

	int GetListenOsdFd();

	void ResetFd( int fd);

	void CloseEpollFd( int fd );
	
private:
	
	//������
	mutable MutexLock		mutexData_;
	
	//�ɶ�fd������
	mutable MutexLock		mutexQ_;
	
	//�ɶ�fd���ж�����������
	Condition	condQ_;

private:
	//epoll ������
	int _epfd;
	
	//��������������
	int _listenInsideFd;
	
	//��������������
	int _listenOutsideFd;
	
	// ������fd<->���ӻỰ Map
	FD_CONNSS_MAP _fd_ConnSs_Map;
	
	// userid<->����fd  Map
	ROLEID_FD_MAP _roleID_Fd_Map;
	
	// Ip<->Count Map
	IP_COUNT_MAP _ip_Count_Map;
	
	// ������fd���� ��(srvType,srvSeq)<->����srvFd
	int _iSrvFd[MAX_SVRTYPE][MAX_SVRSEQ];
	
	//�ɶ�fd����
	queue<int>	_queFd;

	//���ӹرն���
	queue<long long> _queCloseConn;
	

};

class ThreadArg
{
public:
	int 	_iThreadSeq;
	AppGwSvc	* _service;
};

#endif


