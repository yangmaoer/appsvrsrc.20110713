#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include "AppGwSvc.h"
#include "Log.h"
#include "DebugData.h"
#include "Option.h"

#define MULTITHREAD												//������ 

using namespace std;

void * AppGwSvc::thread_ProcMsg(void *arg)
{
	ThreadArg* thdArg = static_cast<ThreadArg*>(arg);
	
	thdArg->_service->HandleThdPopFd( thdArg->_iThreadSeq );
	
	return NULL;
}

void * AppGwSvc::thread_CloseConn(void *input)
{
	AppGwSvc * service = static_cast<AppGwSvc*>(input);
	
	while(1)
	{
		sleep(5);
		service->RemoveConnSession();
	}

	return NULL;
}


void * AppGwSvc::thread_ResetListenFd(void *input)
{
	AppGwSvc * service = static_cast<AppGwSvc*>(input);
	
	while(1)
	{
		sleep(8);
		service->ResetListenFd();
	}

	return NULL;
}




void * AppGwSvc::thread_accept_isd_blk(void *input)
{
	AppGwSvc * service = static_cast<AppGwSvc*>(input);
	service->HandleAcceptBlk( service->GetListenIsdFd() );

	return NULL;
}


void * AppGwSvc::thread_accept_osd_blk(void *input)
{
	AppGwSvc * service = static_cast<AppGwSvc*>(input);
	
	service->HandleAcceptBlk(service->GetListenOsdFd());

	return NULL;
}


AppGwSvc::~AppGwSvc()
{
	//�ر��������ӻỰ
	FD_CONNSS_MAP::iterator pos;
	for(pos=_fd_ConnSs_Map.begin(); pos!=_fd_ConnSs_Map.end(); ++pos)	
	{
		ConnectSession* s = pos->second;
		CloseMarkConnSession(s);
	}
	
}

AppGwSvc::AppGwSvc()
:mutexData_()
,mutexQ_()
,condQ_(mutexQ_)
,_epfd(0)
,_listenInsideFd(0)
, _listenOutsideFd(0)
{
	
	for( int i = 0; i < MAX_SVRTYPE; i++ )
		for( int j = 0; j < MAX_SVRSEQ; j++ )
			_iSrvFd[i][j] = -1;
}

void AppGwSvc::run_once()
{
	int iRet = init();
	//int iRet = init1();
	if( iRet )
	{
		LOG (LOG_ERROR, __FILE__, __LINE__, "init error!! ");
		exit(-1);
	}
	
	run();
	
	return;
}

int AppGwSvc::init1()
{
	//epoll��ʼ��
	_epfd = epoll_create(MAXEPOLLFDNUM);
	
	//����������ʼ��
	if( ListenInit1( _listenInsideFd, bind_port_in.ip, bind_port_in.port ) )
	{
		LOG( LOG_ERROR, __FILE__, __LINE__, " ListenInit error, bind_port_in.ip[%s], bind_port_in.port[%d]", bind_port_in.ip, bind_port_in.port );
		return -1;
	}
	
		
	//����������ʼ��
	if( ListenInit1( _listenOutsideFd, bind_port_out.ip, bind_port_out.port ) )
	{
		LOG( LOG_ERROR, __FILE__, __LINE__, " ListenInit error, bind_port_out.ip[%s], bind_port_out.port[%d]", bind_port_out.ip, bind_port_out.port );
		return -1;
	}
	
	//������̴߳���
	//��ʼ��ҵ�������߳�
	pthread_t tid;
	ThreadArg thdArg;

#ifdef MULTITHREAD
	LOG( LOG_ERROR,  __FILE__, __LINE__, "multi thread ..................." );
	for( int i = 0; i < ini.thread_num; i++ )
	{
		thdArg._service = this;
		thdArg._iThreadSeq = i+1;
		int iRet = pthread_create( &tid, NULL, thread_ProcMsg, &thdArg );
		if(iRet == 0)
			usleep(1000);
		else
			LOG(LOG_ERROR,	__FILE__, __LINE__, "pthread_create error ! iRet[%d],strerr[%s]", iRet, strerror(errno));
	}
#endif

	int iRet = 0;
	iRet = pthread_create( &tid, NULL, thread_CloseConn, this );
	if(iRet)
	{
		LOG(LOG_ERROR,	__FILE__, __LINE__, "pthread_create  thread_CloseConn error ! iRet[%d],strerr[%s]", iRet, strerror(errno));
		return -1;
	}

	iRet = pthread_create( &tid, NULL, thread_accept_isd_blk, this );
	if(iRet)
	{
		LOG(LOG_ERROR,	__FILE__, __LINE__, "pthread_create  thread_accept_isd_blk error ! iRet[%d],strerr[%s]", iRet, strerror(errno));
		return -1;
	}

	iRet = pthread_create( &tid, NULL, thread_accept_osd_blk, this );
	if(iRet)
	{
		LOG(LOG_ERROR,	__FILE__, __LINE__, "pthread_create  thread_accept_osd_blk error ! iRet[%d],strerr[%s]", iRet, strerror(errno));
		return -1;
	}
	
	
	
	
	return 0;
}


int AppGwSvc::GetListenIsdFd()
{
	return _listenInsideFd;
}

int AppGwSvc::GetListenOsdFd()
{
	return _listenOutsideFd;
}


int AppGwSvc::init()
{
	//epoll��ʼ��
	_epfd = epoll_create(MAXEPOLLFDNUM);
	
	//����������ʼ��
	if( ListenInit( _listenInsideFd, bind_port_in.ip, bind_port_in.port ) )
	{
		LOG( LOG_ERROR, __FILE__, __LINE__, " ListenInit error, bind_port_in.ip[%s], bind_port_in.port[%d]", bind_port_in.ip, bind_port_in.port );
		return -1;
	}
	
		
	//����������ʼ��
	if( ListenInit( _listenOutsideFd, bind_port_out.ip, bind_port_out.port ) )
	{
		LOG( LOG_ERROR, __FILE__, __LINE__, " ListenInit error, bind_port_out.ip[%s], bind_port_out.port[%d]", bind_port_out.ip, bind_port_out.port );
		return -1;
	}
	
	//������̴߳���
	//��ʼ��ҵ�������߳�
	pthread_t tid;
	ThreadArg thdArg;

#ifdef MULTITHREAD
	LOG( LOG_ERROR,  __FILE__, __LINE__, "multi thread ..................." );
	for( int i = 0; i < ini.thread_num; i++ )
	{
		thdArg._service = this;
		thdArg._iThreadSeq = i+1;
		int iRet = pthread_create( &tid, NULL, thread_ProcMsg, &thdArg );
		if(iRet == 0)
			usleep(1000);
		else
			LOG(LOG_ERROR,  __FILE__, __LINE__, "pthread_create error ! iRet[%d],strerr[%s]", iRet, strerror(errno));
	}
#endif

	int iRet = pthread_create( &tid, NULL, thread_CloseConn, this );
	if(iRet)
	{
		LOG(LOG_ERROR,  __FILE__, __LINE__, "pthread_create  thread_CloseConn error ! iRet[%d],strerr[%s]", iRet, strerror(errno));
		return -1;
	}

	iRet = pthread_create( &tid, NULL, thread_ResetListenFd, this );
	if(iRet)
	{
		LOG(LOG_ERROR,  __FILE__, __LINE__, "pthread_create  thread_ResetListenFd error ! iRet[%d],strerr[%s]", iRet, strerror(errno));
		return -1;
	}

	
	
	
	return 0;
}


int AppGwSvc::ListenInit1( int &lsfd, char * ip, u_short port )
{
	//��������������
	lsfd = open_tcp_port_blk( ip, port, ini.backlog);
	if( lsfd < 0 )
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, " open_tcp_port_nblk error, ip[%s], port[%d], backlog[%d]", ip, port, ini.backlog );
		return -1;
	}
	
	return 0; 
}


int AppGwSvc::ListenInit( int &lsfd, char * ip, u_short port )
{
	//����������������
	lsfd = open_tcp_port_nblk( ip, port, ini.backlog);
	if( lsfd < 0 )
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, " open_tcp_port_nblk error, ip[%s], port[%d], backlog[%d]", ip, port, ini.backlog );
		return -1;
	}
	
	//epoll �¼�ע��
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = lsfd;
	epoll_ctl( _epfd, EPOLL_CTL_ADD, lsfd, &ev );

	
		
	return 0;	
}


void AppGwSvc::run()
{
	struct epoll_event events[MAXEVENTS];
	
	while( !stopped)
	{
		int nfds = epoll_wait(_epfd, events, MAXEVENTS, -1);

		if( nfds <= 0)
		{
			LOG( LOG_ERROR, __FILE__, __LINE__, "epoll_wait error!!! nfds[%d],errno[%d] \n", nfds, errno );
		}
//LOG( LOG_VERBOSE, __FILE__, __LINE__, "epoll_wait##### nfds[%d] \n", nfds );
		for( int n = 0; n < nfds; n++ )
		{

			if( events[n].data.fd == _listenInsideFd ||
				events[n].data.fd == _listenOutsideFd )
			{
//LOG( LOG_VERBOSE, __FILE__, __LINE__, "nfds accept n[%d], fd[%d] \n", n, events[n].data.fd );
				HandleAccept( events[n].data.fd );
			}
			else
			{
			
#ifdef MULTITHREAD
//DEBUG_PRINTF2( "multi thread ...................event[%d] fd[%d]\n", n, events[n].data.fd );
				//������̴߳���
				HandlePushFd(events[n].data.fd);
#else
				//������̴���
				char * szBuff = new char[MAXSOCKBUFF+1];
DEBUG_PRINTF( "single process________________________\n" );
				HandleProcessPkg( events[n].data.fd, szBuff );
#endif
				//ResetFd(events[n].data.fd);

			}
		}
	}
}




//�����Ӵ���
//����
//
//����ֵ
//	��
void AppGwSvc::HandleAcceptBlk( const int lsfd )
{
	struct sockaddr_in peer;
	struct epoll_event ev;
	
	//��������
	while(1)
	{
		int connFd = accept_tcp_nblk( lsfd, &peer);

		if(connFd < 0)
		{
DEBUG_PRINTF2(" accept_tcp_nblk error, errno[%d], strerror[%s]", errno, strerror(errno) );
			LOG( LOG_ERROR, __FILE__, __LINE__, " accept_tcp_nblk error, errno[%d], strerror[%s]", errno, strerror(errno) );

			if( errno == EMFILE )
				sleep(1);
			
			continue;
		}
		
		//���������� IP ������
		if( lsfd == _listenOutsideFd )
		{
			if( CheckIpCount( peer ) )
			{
				close( connFd );
				LOG( LOG_ERROR, __FILE__, __LINE__, " CheckIpCount error, connFd[%d]!!", connFd );
				continue;
			}
		}
		
		//�µ����ӻỰ
		ConnectSession* ptrSs = NULL;
		long long llConId = CONNECTION_ID ( peer.sin_addr.s_addr, peer.sin_port, connFd );
		if( lsfd == _listenInsideFd )
			ptrSs = new ConnectSession( llConId, INSIDE_TCP_STREAM );
		else
			ptrSs = new ConnectSession( llConId, OUTSIDE_TCP_STREAM );
			
		if( ptrSs->BuffInitCheck() )
		{
			// ��ʱ���� close( connFd )�� ConnectSession ���ٵ�ʱ���Զ��ر�����
			delete ptrSs;
			LOG(LOG_ERROR, __FILE__, __LINE__, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!malloc session buffer failed");
			continue;
		}
		
		
		//ע�� epoll �¼�
		ev.events = EPOLLIN | EPOLLET;
		ev.data.fd = connFd;
		epoll_ctl(_epfd, EPOLL_CTL_ADD, connFd, &ev);
		
		//_fd_ConnSs_Map ���������ӻỰ
		InsertConnSession(ptrSs);
		
		LOG( LOG_VERBOSE, __FILE__, __LINE__, "+++++++++++++ accept new  connect:fd[%d],ip[%s],port[%d]", connFd, ptrSs->GetIP(), ptrSs->GetPort());
	}
		
	return;
}


//���ڽ�������ע�ᵽ epoll
//	listen�ں˶���������accept ����������������( errno=EMFILE )�Ĵ������ epoll �� ETģʽ��
//	�򲻻��ٴ����ü���fd�ı�Ե�¼��� ��ʹ �ͻ��˶Ͽ��˺ܶ����ӣ�ʹ��listen�ں˶��в�����������
//	epoll Ҳ�����ٴ����ü���fd �Ŀɶ��¼�
//
//	����취�ǣ����ʱ��ֻҪ epoll_ctl(MOD) һ�¸ü���fd����
//	���������� epoll_ctl(MOD) ���� fd
//����
//
//����ֵ
//	��
void AppGwSvc::ResetListenFd()
{
	ResetFd( _listenInsideFd );
	ResetFd( _listenOutsideFd );
}


void AppGwSvc::ResetFd( int fd)
{
	struct epoll_event ev;
	
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = fd;
	epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev);
}



//�����Ӵ���
//����
//
//����ֵ
//	��
void AppGwSvc::HandleAccept( const int lsfd )
{
	struct sockaddr_in peer;
	struct epoll_event ev;
	
	//��������
	while(1)
	{
		int connFd = accept_tcp_nblk( lsfd, &peer);

		if(connFd < 0)
		{
			//û�к�����tcp����, ���ߴﵽϵͳ����ļ���
			if( EAGAIN == errno || 
					EMFILE == errno)
				break;
		
			LOG( LOG_ERROR, __FILE__, __LINE__, " accept_tcp_nblk error" );
			continue;
		}
		
		//���������� IP ������
		if( lsfd == _listenOutsideFd )
		{
			if( CheckIpCount( peer ) )
			{
				close( connFd );
				LOG( LOG_ERROR, __FILE__, __LINE__, " CheckIpCount error, connFd[%d]!!", connFd );
				continue;
			}
		}
		
		//�µ����ӻỰ
		ConnectSession* ptrSs = NULL;
		long long llConId = CONNECTION_ID ( peer.sin_addr.s_addr, peer.sin_port, connFd );
		if( lsfd == _listenInsideFd )
			ptrSs = new ConnectSession( llConId, INSIDE_TCP_STREAM );
		else
			ptrSs = new ConnectSession( llConId, OUTSIDE_TCP_STREAM );
			
		if( ptrSs->BuffInitCheck() )
		{
			// ��ʱ���� close( connFd )�� ConnectSession ���ٵ�ʱ���Զ��ر�����
			delete ptrSs;
			LOG(LOG_ERROR, __FILE__, __LINE__, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!malloc session buffer failed");
			continue;
		}
		
		
		//ע�� epoll �¼�
		ev.events = EPOLLIN | EPOLLET;
		ev.data.fd = connFd;
		epoll_ctl(_epfd, EPOLL_CTL_ADD, connFd, &ev);
		
		//_fd_ConnSs_Map ���������ӻỰ
		InsertConnSession(ptrSs);
		
		LOG( LOG_VERBOSE, __FILE__, __LINE__, "+++++++++++++ accept new  connect:fd[%d],ip[%s],port[%d]",	connFd, ptrSs->GetIP(), ptrSs->GetPort());
	}
		
	return;
}


void AppGwSvc::HandlePushFd( const int fd )
{
	MutexLockGuard lock(mutexQ_);
	_queFd.push(fd);
	condQ_.notify();
}


void AppGwSvc::HandleThdPopFd( int thdSeq )
{
	char * szMsgBuffer = new char[MAXSOCKBUFF+1];
	if( szMsgBuffer == NULL )
	{
		LOG( LOG_ERROR, __FILE__, __LINE__,  "buff malloc error, thread exit!!!!!!!!!!!!!!!!!!!!!!!" );
		return;
	}
	
	memset( szMsgBuffer, 0, sizeof(szMsgBuffer) );
	
//	LOG( LOG_VERBOSE, __FILE__, __LINE__,  "HandleThdPopFd : pid[%d],thdSeq[%d]", getpid(), thdSeq );
	
	while(1)
	{
		int fd = 0;
		{
		    MutexLockGuard lock(mutexQ_);
		    while( _queFd.size() == 0 ){
			     condQ_.wait();
		    }

		    fd = _queFd.front();
		    _queFd.pop();
		}
		
		HandleProcessPkg( fd, szMsgBuffer );

		ResetFd(fd);
	}
}

void AppGwSvc::HandleProcessPkg( int fd, char * ptrBuff )
{
	int iPkgLen = 0 ;
	int iRet = 0;


DEBUG_PRINTF1( "HandleProcessPkg, ---fd[%d]", fd );


	//�������ӻỰ
	ConnectSession* ss = NULL;
	if( (ss = FindFdConnSession(fd)) == NULL )
	{
		CloseEpollFd(fd);
		LOG( LOG_ERROR, __FILE__, __LINE__, " HandleProcessPkg():FindFdConnSession error, fd[%d], not found!!", fd );
		return;
	}

	//�ж������Ƿ��ѹر�
	if( ss->GetCloseFlag() )
		return;
		
	//��������
	iRet = ss->RecvMsg();
	if( iRet == 0 )
		return;
	else if( iRet < 0 )
	{
		//���ӹر�
//		LOG( LOG_ERROR, __FILE__, __LINE__, " HandleProcessPkg():conn need to close, fd[%d], errno[%d], err msg[%s]!", fd, errno, strerror(errno) );
		CloseMarkConnSession(ss);
		return;
	}
	
	//��ȡ�����
	while( (iRet = ss->GetDataPack( ptrBuff, iPkgLen )) )
	{
		if( iRet == 0 )
			return;
		else if( iRet < 0 )
		{
			LOG( LOG_ERROR, __FILE__, __LINE__, " HandleProcessPkg():GetDataPack error, iRet[%d], remove connSession!!", iRet );
			CloseMarkConnSession(ss);
			return;
		}
			
		//���������
		if( ss->GetConnType() == INSIDE_TCP_STREAM )
		{
			PkgHead * pkg = (PkgHead*)ptrBuff;
			
			switch( pkg->ucDirection )
			{
				case DIRECT_C_S_RESP:
				case DIRECT_S_C_REQ:
					
					ProcessS2C( ss, ptrBuff );	
					break;
					
				case DIRECT_S_S_REQ:
					ProcessS2G( ss, ptrBuff );	
					break;
					
				default:
					LOG( LOG_ERROR, __FILE__, __LINE__, " bad pkg->ucDirection[%d], srvType[%d], connSs->ConnType[%d]", pkg->ucDirection, ss->GetSrvType(), ss->GetConnType() );
					break;
			}
			
		}
		else if ( ss->GetConnType() == OUTSIDE_TCP_STREAM )
		{
			PkgHead * pkg = (PkgHead*)ptrBuff;
			
			switch( pkg->ucDirection )
			{
				
				case DIRECT_C_S_REQ:
				case DIRECT_S_C_RESP:
					ProcessC2S( ss, ptrBuff);
					break;
					
				default:
					LOG( LOG_ERROR, __FILE__, __LINE__, " bad pkg->ucDirection[%d], srvType[%d], connSs->ConnType[%d]", pkg->ucDirection, ss->GetSrvType(), ss->GetConnType() );
					break;			
			}
		}
		else
		{
			LOG( LOG_ERROR, __FILE__, __LINE__, " bad connType[%d], remove connSession!!", ss->GetConnType() );
			CloseMarkConnSession(ss);
		}
	}
}

void AppGwSvc::ProcessS2G( ConnectSession* reqConnSs, const char * ptrReqPkg )
{
	PkgHead * ptrReqPkgHead = (PkgHead *)ptrReqPkg;
	
	PkgBuff	pkgbuff;
	PkgHead	ackPkgHead;
	
	memset( &ackPkgHead, 0, sizeof(ackPkgHead) );
	
	if( MSGTYPE_GW_SVRCONNECT == ptrReqPkgHead->usMsgType && 		
		DIRECT_S_S_REQ == ptrReqPkgHead->ucDirection )
	{
		//��������������������,���ע������
		unsigned int uiRetCode = 0;
		PKGBODY_SG_SVRCONNECT_REQ pkgBody;
		memset( &pkgBody, 0, sizeof(pkgBody));
		
		//��Ӧ�����ͷ	
		memcpy( &ackPkgHead, ptrReqPkgHead, sizeof(PkgHead) );
			
		//���������
		memcpy( &pkgBody, ptrReqPkg + sizeof(PkgHead), sizeof(PKGBODY_SG_SVRCONNECT_REQ) );
			
		//�ж���������� srvtype srvseq ��Χ
		if( check_srv_scope( pkgBody.ucSrvType, pkgBody.ucSrvSeq) )
		{
			LOG(LOG_ERROR, __FILE__, __LINE__, "S_G bad pkgBody  ucSrvType[%d]ucSrvSeq[%d]",	pkgBody.ucSrvType, pkgBody.ucSrvSeq);
			return;
		}
			
		//��ȡ srvfd
		int srvfd = _iSrvFd[pkgBody.ucSrvType -1 ][pkgBody.ucSrvSeq-1];
			
		//Ӧ������
		ackPkgHead.ucDirection = DIRECT_S_S_RESP;		
		
		if( srvfd > 0 )
			uiRetCode = ERR_SYSTEM_SVRLOGINED;		//�������ѵ�¼
		else
			uiRetCode = 0;
		
		//�齨���Ͱ�
		pkgbuff.Clear();
		pkgbuff.Append( &ackPkgHead, (int)sizeof(ackPkgHead) );
		pkgbuff.Append( &uiRetCode, (int)sizeof(uiRetCode) );
		pkgbuff.EncodeLength();
	
DEBUG_PRINTF( " S_G, socket_send to srv\n" );
	DEBUG_SHOWHEX( pkgbuff.GetBuff(), pkgbuff.GetSize(), 0, __FILE__, __LINE__ );
	
		int iSend = reqConnSs->SendMsg( pkgbuff.GetBuff(), pkgbuff.GetSize() );
	
		if( iSend < 0 )
		{
				CloseMarkConnSession( reqConnSs );
				LOG( LOG_ERROR, __FILE__, __LINE__, "S_G ,connSs send msg error, srvType[%d], CloseMarkConnSession!", reqConnSs->GetSrvType());
				return;
		}
		else if( pkgbuff.GetSize() == iSend )
		{
			//ͬһ srvtype��srvseq�Ƿ��Ѿ���¼
			if( srvfd > 0 )
			{
				LOG(LOG_ERROR, __FILE__, __LINE__, " process_s_g() ,srv is logined!! srvType[%d]ucSrvSeq[%d]",
					pkgBody.ucSrvType, pkgBody.ucSrvSeq );
				return;
			}
				
			//��¼����������
			_iSrvFd[pkgBody.ucSrvType -1 ][pkgBody.ucSrvSeq-1] = CONNECTION_FD(reqConnSs->GetConnId());
			reqConnSs->SetSrvType(pkgBody.ucSrvType);
			reqConnSs->SetSrvSeq(pkgBody.ucSrvSeq);
			reqConnSs->SetLoginStatus(1);
	
			LOG( LOG_VERBOSE, __FILE__, __LINE__, "S_G ,srvType[%d]SrvSeq[%d] [%lld] login success!!!!!", pkgBody.ucSrvType, pkgBody.ucSrvSeq, reqConnSs->GetConnId() );
				
			return;
		}
		else
		{
			LOG( LOG_ERROR, __FILE__, __LINE__, "S_G ,connId[%lld]ip[%s]port[%d] login fail!!!", reqConnSs->GetConnId(), reqConnSs->GetIP(), reqConnSs->GetPort());
			return;
		}
	}
	else if( MSGTYPE_GW_CLOSECLIENT == ptrReqPkgHead->usMsgType &&		
		DIRECT_S_S_REQ == ptrReqPkgHead->ucDirection )
	{
		//�ر�ָ���ͻ�������				
		ConnectSession * client_session = NULL;		
		client_session = FindRoleConnSession( ptrReqPkgHead->uiRoleID );
		
		if( NULL != client_session )
		{
			LOG( LOG_ERROR, __FILE__, __LINE__, "S_G, close the _client! CloseMarkConnSession, roleid[%d] ", ptrReqPkgHead->uiRoleID );
			CloseMarkConnSession( client_session );
			return;
		}
		
		LOG( LOG_ERROR, __FILE__, __LINE__, "S_G, close the _client error, roleid[%d] not found!", ptrReqPkgHead->uiRoleID );
		return;		
	}
	else
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "S_G, srvType[%d]SrvSeq[%d] unknow commandid[%d]!", ptrReqPkgHead->ucSrvType, ptrReqPkgHead->ucSrvSeq, ptrReqPkgHead->usMsgType);
		return;
	}
	
	
	
}


//�齨�ͻ��˶�Ӧ���
//����
//	buff						����
//	uiRetCode				�������
//����ֵ
//	��
void AppGwSvc::MakeClientAckPkg( PkgBuff &buff, PkgHead & pkgHead, unsigned int uiRetCode )
{
	//�齨���Ͱ�
	buff.Clear();
	buff.Append( &pkgHead, sizeof(pkgHead) );
	buff.Append( &uiRetCode, sizeof(uiRetCode) );
	buff.EncodeLength();
	
}

void AppGwSvc::ProcessC2S( ConnectSession* reqConnSs, const char * ptrReqPkg )
{
	
	int srvfd;
	PkgBuff	pkgbuff;
	
	PkgHead * ptrReqPkgHead = (PkgHead *)ptrReqPkg;		//���������ͷ
	PKGBODY_CS_ROLELOGIN_REQ_R * ptrReqPkgBody = NULL;				//������¼�������
	PkgHead tranPkgHead;											//������ͷ
	PKGBODY_CS_ROLELOGIN_REQ_L tranPkgBody;								//������¼�������
	
	//��ʼ��
	memset( &tranPkgHead, 0, sizeof(tranPkgHead) );
	memset( &tranPkgBody, 0, sizeof(tranPkgBody) );

	// srv��Χ���
	if( check_srv_scope( ptrReqPkgHead->ucSrvType, ptrReqPkgHead->ucSrvSeq) )
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "C_S ucSrvType[%d]ucSrvSeq[%d] error",	ptrReqPkgHead->ucSrvType, ptrReqPkgHead->ucSrvSeq);
		return;
	}
	
	//��ȡllSrvid
	srvfd = _iSrvFd[ptrReqPkgHead->ucSrvType-1][ptrReqPkgHead->ucSrvSeq-1];
	
	//srvfdУ��
	if(srvfd <= 0)
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "C_S, srvfd[%d] is invalid, srvType[%d],SrvSeq[%d]", srvfd, ptrReqPkgHead->ucSrvType, ptrReqPkgHead->ucSrvSeq );
		
		//��Ӧ������ؿͻ���
		PkgHead ackPkgHead;
		memcpy( &ackPkgHead, ptrReqPkgHead, sizeof(PkgHead) );
		ackPkgHead.ucDirection = DIRECT_C_S_RESP;
		
		MakeClientAckPkg( pkgbuff, ackPkgHead, ERR_SYSTEM_SVRNOTSTART );
		
		int iSend = reqConnSs->SendMsg( pkgbuff.GetBuff(), pkgbuff.GetSize() );
		if( iSend < 0 )
		{
			CloseMarkConnSession( reqConnSs );
			LOG( LOG_ERROR, __FILE__, __LINE__, "C_S ,reqConnSs send msg error, roleid[%d], CloseMarkConnSession!", reqConnSs->GetRoleID());
		}
		
DEBUG_PRINTF( "C_S --> srvfd is invalid, ack to client Pkg:\n" );
	DEBUG_SHOWHEX( pkgbuff.GetBuff(), pkgbuff.GetSize(), 0, __FILE__, __LINE__ );

		
		return;
	}
	
	ConnectSession* srvConnSs = FindFdConnSession( srvfd );
	if( srvConnSs == NULL )
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "C_S, srvConnSs not found, srvfd[%d]", srvfd );
		return;
	}
	
	
	//��ͷDirection У��
	if( DIRECT_C_S_REQ != ptrReqPkgHead->ucDirection )
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "C_S, direction isn't DIRECT_C_S_REQ " );
		return;
	}

DEBUG_PRINTF( "C_S --> Pkg head:\n" );
	DEBUG_SHOWHEX( (char*)ptrReqPkg, sizeof(PkgHead), 0, __FILE__, __LINE__ );

	
	//����������ת��
	if( SVRTYPE_MAIN == ptrReqPkgHead->ucSrvType &&
		MSGTYPE_CS_ROLELOGIN == ptrReqPkgHead->usMsgType )
	{// C_S online srv ��¼�����������ע��������
		
		ptrReqPkgBody = (PKGBODY_CS_ROLELOGIN_REQ_R *)( ptrReqPkg + sizeof(PkgHead) );
		
		//������ͷ������ userid �ֶ�
		memcpy( &tranPkgHead, ptrReqPkgHead, sizeof(PkgHead) );
		tranPkgHead.uiRoleID = ptrReqPkgBody->uiRoleID;

		//�������壬���� ClientId �ֶ�
		memcpy( &tranPkgBody, ptrReqPkgBody, sizeof(PKGBODY_CS_ROLELOGIN_REQ_R) );
		tranPkgBody.llClientId = reqConnSs->GetConnId();

		//�齨���Ͱ�
		pkgbuff.Clear();
		pkgbuff.Append( &tranPkgHead, sizeof(tranPkgHead) );
		pkgbuff.Append( &tranPkgBody, sizeof(tranPkgBody) );
		pkgbuff.EncodeLength();
	}
	else
	{// �������ף����������ͷ
		
		//�ͻ����Ƿ��ѵ�¼
		if( 0 == reqConnSs->GetLoginStatus() )
		{
			LOG(LOG_ERROR, __FILE__, __LINE__, "client isn't logined!" );
			
			//��Ӧ������ؿͻ���
			PkgHead ackPkgHead;
			memcpy( &ackPkgHead, ptrReqPkgHead, sizeof(PkgHead) );
			ackPkgHead.ucDirection = DIRECT_C_S_RESP;
			ackPkgHead.uiRoleID = 0;		//�Կͻ������� roleID
			
			MakeClientAckPkg( pkgbuff, ackPkgHead, ERR_SYSTEM_ROLENOTLOGIN );
			
			int iSend = reqConnSs->SendMsg( pkgbuff.GetBuff(), pkgbuff.GetSize() );
			if( iSend < 0 )
			{
				CloseMarkConnSession( reqConnSs );
				LOG( LOG_ERROR, __FILE__, __LINE__, "C_S ,reqConnSs send msg error, roleid[%d], CloseMarkConnSession!", reqConnSs->GetRoleID());
			}
			
			return;
		}
		
		//������ͷ������ userid �ֶ�
		memcpy( &tranPkgHead, ptrReqPkgHead, sizeof(PkgHead) );
		tranPkgHead.uiRoleID = reqConnSs->GetRoleID();

		//�齨���Ͱ�
		pkgbuff.Clear();
		pkgbuff.Append( &tranPkgHead, sizeof(tranPkgHead) );
		pkgbuff.Append( ptrReqPkg+sizeof(PkgHead), ptrReqPkgHead->usPkgLen - sizeof(PkgHead));
		pkgbuff.EncodeLength();
		
	}

DEBUG_PRINTF( "C_S --> pkg, send to srv\n" );
	DEBUG_SHOWHEX( pkgbuff.GetBuff(), pkgbuff.GetSize(), 0, __FILE__, __LINE__ );

	int iSend = srvConnSs->SendMsg( pkgbuff.GetBuff(), pkgbuff.GetSize() );
	if( iSend < 0 )
	{
		//���srv �����ӻỰ
		CloseMarkConnSession( srvConnSs );
		LOG( LOG_ERROR, __FILE__, __LINE__, "C_S ,srvConnSs send msg error,srvType[%d], CloseMarkConnSession!", srvConnSs->GetSrvType());
		
		//��Ӧ������ؿͻ���
		PkgHead ackPkgHead;
		memcpy( &ackPkgHead, ptrReqPkgHead, sizeof(PkgHead) );
		ackPkgHead.ucDirection = DIRECT_C_S_RESP;
		
		MakeClientAckPkg( pkgbuff, ackPkgHead, ERR_SYSTEM_SVRACCESS );
		
		iSend = reqConnSs->SendMsg( pkgbuff.GetBuff(), pkgbuff.GetSize() );
		if( iSend < 0 )
		{
			CloseMarkConnSession( reqConnSs );
			LOG( LOG_ERROR, __FILE__, __LINE__, "C_S ,reqConnSs send msg error,roleid[%d], CloseMarkConnSession!", reqConnSs->GetRoleID());
		}
		
		return;
	}
	
	return;
}


void AppGwSvc::ProcessS2C( ConnectSession* reqConnSs, const char * ptrReqPkg )
{
	
	PkgBuff	pkgbuff;
	PkgHead * ptrReqPkgHead = (PkgHead *)ptrReqPkg;		//���������ͷ
	PKGBODY_CS_ROLELOGIN_RESP_L * ptrReqPkgBody_RspnLogin = NULL;				//������¼Ӧ�����
	PkgHead tranPkgHead;															//������ͷ
	unsigned int uiRetCode;												//������¼Ӧ�����
	
	ConnectSession* s_client = NULL;
	ConnectSession* s_logined = NULL;
	
	//��ʼ��
	
	memset( &tranPkgHead, 0, sizeof(tranPkgHead) );
	memset( &uiRetCode, 0, sizeof(uiRetCode) );


	//�ж� srvtype srvseq ��Χ
	if( check_srv_scope( ptrReqPkgHead->ucSrvType, ptrReqPkgHead->ucSrvSeq) )
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, " S_C, ucSrvType[%d],ucSrvSeq[%d] or error",	ptrReqPkgHead->ucSrvType, ptrReqPkgHead->ucSrvSeq );
		return;
	}
	
	//����������ת��
	
	if( DIRECT_C_S_RESP == ptrReqPkgHead->ucDirection &&
		SVRTYPE_MAIN == ptrReqPkgHead->ucSrvType &&
		MSGTYPE_CS_ROLELOGIN == ptrReqPkgHead->usMsgType )
	{// C_S online srv ��¼����Ӧ�����ע��������
		
		ptrReqPkgBody_RspnLogin = (PKGBODY_CS_ROLELOGIN_RESP_L*) ( ptrReqPkg + sizeof(PkgHead) );
		
		//������ͷ
		memcpy( &tranPkgHead, ptrReqPkgHead, sizeof(PkgHead) );
		tranPkgHead.uiRoleID = 0;			//�Կͻ������� roleID

		//Ŀ��ͻ������ӻỰ����
		s_client = FindFdConnSession( CONNECTION_FD(ptrReqPkgBody_RspnLogin->llClientId) );
		if ( s_client == NULL)			
		{
			LOG(LOG_ERROR, __FILE__, __LINE__, "S_C fd [%d]not exists ", CONNECTION_FD(ptrReqPkgBody_RspnLogin->llClientId) );
			return;
		}
		
		//�齨����Ӧ�����
		uiRetCode = ptrReqPkgBody_RspnLogin->uiRetCode;
		
		if( 0 == ptrReqPkgBody_RspnLogin->uiRetCode )
		{
			//��֮ǰ�Ѿ����û��Ѿ���¼���Ҳ��Ǳ�����session����Ͽ�֮ǰ�ѵ�¼������session���û���
			s_logined = FindRoleConnSession( ptrReqPkgBody_RspnLogin->uiRoleID );

			if( NULL != s_logined  &&
				s_logined->GetLoginStatus() &&
				s_logined != s_client )
			{
				UpdateUserFdMap( ptrReqPkgBody_RspnLogin->uiRoleID, CONNECTION_FD(ptrReqPkgBody_RspnLogin->llClientId) );
				s_logined->SetIsSameUser(1);		//���ñ�־������CloseMarkConnSession �� online srv �ظ���ͬһUserID ��logout����
				CloseMarkConnSession( s_logined );
				LOG( LOG_ERROR, __FILE__, __LINE__, " repeat login, CloseMarkConnSession previous client, roleid[%d]!!", s_logined->GetRoleID() );
			}
			else
				InsertUserFdMap( ptrReqPkgBody_RspnLogin->uiRoleID, CONNECTION_FD(ptrReqPkgBody_RspnLogin->llClientId) );
			
			//��¼�ɹ�, ��Ӧ�ͻ��� session ��Ϣ����
			s_client->SetSrvType( 0 );
			s_client->SetLoginStatus(1);
			s_client->SetRoleID( ptrReqPkgBody_RspnLogin->uiRoleID );
			
		}
		else
		{
			LOG( LOG_ERROR, __FILE__, __LINE__, "roleid[%d], login fail, retcode[%d] ", ptrReqPkgBody_RspnLogin->uiRoleID, ptrReqPkgBody_RspnLogin->uiRetCode );
		}
		
		//�齨���Ͱ�
		pkgbuff.Clear();
		pkgbuff.Append( &tranPkgHead, sizeof(tranPkgHead) );
		pkgbuff.Append( &uiRetCode, sizeof(uiRetCode) );
		pkgbuff.Append( &(ptrReqPkgBody_RspnLogin->uiRoleID), sizeof(ptrReqPkgBody_RspnLogin->uiRoleID) );		
		pkgbuff.EncodeLength();
	}
	else
	{// �������ף����������ͷ
		
		//������ͷ
		memcpy( &tranPkgHead, ptrReqPkgHead, sizeof(PkgHead) );
		tranPkgHead.uiRoleID = 0;		//�Կͻ������� roleID
		
		//�ͻ��� session ����
		if ( (s_client = FindRoleConnSession( ptrReqPkgHead->uiRoleID ) ) == NULL )
		{
			LOG(LOG_ERROR, __FILE__, __LINE__, "roleid[%d], connSession not found ", ptrReqPkgHead->uiRoleID );
			return;
		}
		
		//�齨���Ͱ�
		pkgbuff.Clear();
		pkgbuff.Append( &tranPkgHead, sizeof(tranPkgHead) );
		pkgbuff.Append( ptrReqPkg+sizeof(PkgHead), ptrReqPkgHead->usPkgLen - sizeof(PkgHead));
		pkgbuff.EncodeLength();
	}
	

DEBUG_PRINTF( "S_C --> pkg, send to srv\n" );
	DEBUG_SHOWHEX( pkgbuff.GetBuff(), pkgbuff.GetSize(), 0, __FILE__, __LINE__ );	
	
	int iSend = s_client->SendMsg( pkgbuff.GetBuff(), pkgbuff.GetSize() );
	if( -1 == iSend )
	{// SendMsg ���� -2��������ÿ���, ��Ϊ ���� -2 ˵��֮ǰ�Ѿ� CloseMarkConnSession ����
		//���client�����ӻỰ
		CloseMarkConnSession( s_client );
		LOG( LOG_ERROR, __FILE__, __LINE__, "S_C ,clientConnSs send msg error,CloseMarkConnSession!! roleid[%d], errno[%d]", s_client->GetRoleID(), errno);
		return;
	}

	return;
}






//���� IP ������У��
//����
//	peer		socket ��ַ����
//����ֵ
//	0		�ɹ�
//	��0	ʧ��
int AppGwSvc::CheckIpCount( const struct sockaddr_in &peer )
{
	IP_COUNT_MAP::iterator pos;
	MutexLockGuard lock(mutexData_);
	char szTmp[64];
	
	pos = _ip_Count_Map.find(peer.sin_addr.s_addr);
	int connect_count = 0;
	if(pos != _ip_Count_Map.end())
	{
		connect_count =(int)pos->second;
		if(connect_count > ini.ip_connect_limit)
		{
			LOG(LOG_ERROR, __FILE__, __LINE__, "CheckIpCount error, over count limit , ip[%s],port[%d]", inet_ntop( AF_INET, &peer.sin_addr, szTmp, sizeof(szTmp)), ntohs(peer.sin_port));
			return -1;
		}
	}
	connect_count++;
	_ip_Count_Map[peer.sin_addr.s_addr] = connect_count;
	
	return 0;
}

int AppGwSvc::InsertConnSession(ConnectSession* s)
{
	MutexLockGuard lock(mutexData_);
	
	int key = CONNECTION_FD(s->GetConnId());
	_fd_ConnSs_Map.insert(make_pair(key, s));
	
//	LOG( LOG_VERBOSE, __FILE__, __LINE__, "insert into _fd_ConnSs_Map,fd[%d],cConnType=[%d], _fd_ConnSs_Map.size[%d] ", key, s->GetConnType(), _fd_ConnSs_Map.size() );
	
	
	return 0;
}

void AppGwSvc::RemoveConnSession()
{
	//���ӻỰ�رն��У��Ƿ���Ԫ��
	if( _queCloseConn.size() == 0 )
		return;

	MutexLockGuard lock(mutexData_);
	FD_CONNSS_MAP::iterator pos;
	in_addr adrtTmp;
	int iQueSize = _queCloseConn.size();

//	LOG( LOG_VERBOSE, __FILE__, __LINE__, "Remove ConnSession begin### _fd_ConnSs_Map[%d],_roleID_Fd_Map[%d] ", _fd_ConnSs_Map.size(),_roleID_Fd_Map.size());

	while( iQueSize-- > 0 )
	{
 
		long long llConId = _queCloseConn.front();
		_queCloseConn.pop();
	
		//�������ӻỰ
		int fd = CONNECTION_FD(llConId);
		pos = _fd_ConnSs_Map.find(fd);
		if( pos == _fd_ConnSs_Map.end())
		{
//			LOG(LOG_ERROR, __FILE__, __LINE__, "RemoveConnSession(), can't find session map, fd[%d], _fd_ConnSs_Map.size [%d] \n", fd, _fd_ConnSs_Map.size() );
			continue;
		}
		ConnectSession* s = pos->second;

		//ʱ����ж�
		if( time(NULL) - s->GetStamp() < GW_CONNCLOSe_INTERNAL )
		{
			_queCloseConn.push(llConId);
			continue;
		}

		//��������������������
		if( s->GetConnType() == INSIDE_TCP_STREAM )
		{//�������Ӵ���
			
			//״̬Ϊ��¼���Ž� _iSrvFd ��Ϊ -1
			//��Ϊ�ѵ�¼״̬������session��ucSrvType ����������ȷ��ֵ
			if( s->GetLoginStatus() )
			{
				_iSrvFd[s->GetSrvType()-1][s->GetSrvSeq()-1] = -1;
				LOG( LOG_ERROR, __FILE__, __LINE__, "Removing svrConnSession; set _iSrvFd to -1, svrType[%d],svrSeq[%d]", s->GetSrvType(), s->GetSrvSeq());
			}
			LOG( LOG_ERROR, __FILE__, __LINE__, "Removing svrConnSession;");
		}
		else if( s->GetConnType() == OUTSIDE_TCP_STREAM )
		{//�������Ӵ���
			
			//�Ƿ��ߵ��ľ�����
			if( 0 == s->GetIsSameUser() )
			{
				//֪ͨ�� OL srv
				NotifyClientClosed(s);
			
				//ɾ�� _roleID_Fd_Map ��¼�� �û����ظ��ľ����ӳ���
				RemoveRoleFdMap( s->GetRoleID());
			}
			
			//ɾ�� _ip_Count_Map
			IP_COUNT_MAP::iterator pos_ip;	
			pos_ip = _ip_Count_Map.find(CONNECTION_IP(s->GetConnId()));
			int connect_count = 0;
			if(pos_ip != _ip_Count_Map.end())
			{
				connect_count =(int)pos_ip->second;
				connect_count--;
				if(connect_count <= 0)
				{
					_ip_Count_Map.erase(pos_ip);
				}
				else
				{
					_ip_Count_Map[CONNECTION_IP(s->GetConnId())] = connect_count;
				}
				
			}
			else
			{
				LOG(LOG_ERROR, __FILE__, __LINE__, "_client connID[%lld], ip[%s] not find in map",	s->GetConnId(), s->GetIP());
			}
			
		}
		else
		{
			LOG(LOG_ERROR, __FILE__, __LINE__, "RemoveConnSession unknow conntype[%d]!!!!!!!!!!", s->GetConnType() );
		}
		
		//delete �Ự���Ӷ���
//		LOG( LOG_VERBOSE, __FILE__, __LINE__, "----Remove ConnSession success!! fd[%d],ip[%s],port[%d],Conntype[%d], roleid[%d], srvType[%d] ",	s->GetFd(), s->GetIP(), s->GetPort(), s->GetConnType(), s->GetRoleID(), s->GetSrvType());
		delete s;
		
		//_fd_ConnSs Map ɾ�����ӻỰ
		_fd_ConnSs_Map.erase(pos);
		adrtTmp.s_addr = CONNECTION_IP(llConId);
	}

//	LOG( LOG_VERBOSE, __FILE__, __LINE__, "---Remove ConnSession end__ _fd_ConnSs_Map[%d],_roleID_Fd_Map[%d] ", _fd_ConnSs_Map.size(),_roleID_Fd_Map.size());

	return;
}


void AppGwSvc::CloseEpollFd( int fd )
{
	MutexLockGuard lock(mutexData_);

	close(fd);

	//ע�� epoll �¼�
	struct epoll_event ev;
	ev.data.fd = fd;
	epoll_ctl( _epfd, EPOLL_CTL_DEL, fd, &ev );
	
}

void AppGwSvc::CloseMarkConnSession( ConnectSession* s )
{
	MutexLockGuard lock(mutexData_);
	in_addr adrtTmp;

	//���ʱ����������ùرձ�־
	s->SetStamp();
	s->CloseConn();

	//�����ӹرն���
	_queCloseConn.push(s->GetConnId());
	
	//ע�� epoll �¼�
	struct epoll_event ev;
	int fd = CONNECTION_FD(s->GetConnId());
	ev.data.fd = fd;
	epoll_ctl( _epfd, EPOLL_CTL_DEL, fd, &ev );

	long long llConId = s->GetConnId();
	adrtTmp.s_addr = CONNECTION_IP(llConId);
//	LOG( LOG_VERBOSE, __FILE__, __LINE__, "CloseMarkConnSession  success!! fd[%d],ip[%s],port[%d], SessionConnType[%d],roleid[%d],srvType[%d] ",
//		s->GetFd(), s->GetIP(), s->GetPort(), s->GetConnType(), s->GetRoleID(), s->GetSrvType());

	return;
}

ConnectSession* AppGwSvc::FindFdConnSession(int fd)
{	
	FD_CONNSS_MAP::iterator pos;
	MutexLockGuard lock(mutexData_);
	
	pos = _fd_ConnSs_Map.find(fd);
	if(pos == _fd_ConnSs_Map.end())
		return NULL;

	return pos->second;
}

void AppGwSvc::check_timeout(ConnectSession* s, int timeout)
{
	if(s->GetConnType() != OUTSIDE_TCP_STREAM )
		return;

	int difftime = time(NULL) - s->GetStamp();
	if(difftime > timeout || difftime < 0)
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "check_timeout,CloseMarkConnSession: srvType[%d] _llConnId[%lld] closed", s->GetSrvType(),s->GetConnId());

		CloseMarkConnSession( s );
		return;
	}
	
	return;
}

void AppGwSvc::NotifyClientClosed( ConnectSession* s )
{
	PkgHead reqPkgHead;								//������ͷ
	PkgBuff	pkgbuff;
	int srvfd;
	
	//��ʼ��
	memset( &reqPkgHead, 0, sizeof(reqPkgHead) );
	
	//��ȡ Srvfd
	srvfd = _iSrvFd[SVRTYPE_MAIN-1][SVRSEQ_MAIN-1];

	if(srvfd == -1)
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "NotifyClientClosed, online srv srvfd[%d] is invalid", srvfd );
		return;
	}
	
	//���� online �����ӻỰ
	FD_CONNSS_MAP::iterator it;
	it = _fd_ConnSs_Map.find(srvfd);
	if(it == _fd_ConnSs_Map.end())
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "NotifyClientClosed, online connSession not found " );
		return;
	}

	ConnectSession * connSsOl = it->second;
	
	//����Ǳ��ߵ��ľ����ӣ��򲻷� logout ��Ϣ�� online srv 
	if( s->GetIsSameUser() )
		return;
		
	//���û�����ֱ�ӷ���
	if( 0 == s->GetRoleID() )
		return;
	
	//������ͷ������ userid �ֶ�
	reqPkgHead.ucSrvType = SVRTYPE_MAIN;
	reqPkgHead.ucSrvSeq = 1;
	reqPkgHead.usMsgType = MSGTYPE_CS_ROLELOGOUT;
	reqPkgHead.uiUniqID = 111;
	reqPkgHead.ucDirection = DIRECT_C_S_REQ;
	reqPkgHead.uiRoleID = s->GetRoleID();
	

	//�齨���Ͱ�
	pkgbuff.Clear();
	pkgbuff.Append( &reqPkgHead, (int)sizeof(reqPkgHead) );
	pkgbuff.EncodeLength();
	
DEBUG_PRINTF( " NotifyClientClosed, socket_send to online srv\n" );
	DEBUG_SHOWHEX( pkgbuff.GetBuff(), pkgbuff.GetSize(), 0, __FILE__, __LINE__ );

	//����ת�� srv
	int iSend = connSsOl->SendMsg( pkgbuff.GetBuff(), pkgbuff.GetSize() );
	if( iSend < 0 )
	{
		LOG( LOG_ERROR, __FILE__, __LINE__, "NotifyClientClosed  SendMsg error--- ");
		return;
	}
	
	//�ɹ�����
	LOG( LOG_VERBOSE, __FILE__, __LINE__, "NotifyClientClosed() Success!!!!!roleid[%d]connSs closed, ", s->GetRoleID() );

	return;
}


int AppGwSvc::UpdateUserFdMap( u_int roleid, int fd )
{
	MutexLockGuard lock(mutexData_);
	ROLEID_FD_MAP::iterator pos;
	
	pos = _roleID_Fd_Map.find(roleid);
	if(pos != _roleID_Fd_Map.end())
	{
		pos->second = fd;
	}
//	LOG( LOG_VERBOSE, __FILE__, __LINE__, "UpdateUserFdMap ,roldid[%d], fd[%d]", roleid, fd );
	
	return 0;
}


int AppGwSvc::InsertUserFdMap( u_int roleid, int fd )
{
	MutexLockGuard lock(mutexData_);
	
	_roleID_Fd_Map.insert(make_pair( roleid, fd ) );
//	LOG( LOG_VERBOSE, __FILE__, __LINE__, "InsertUserFdMap,roleid[%d], fd[%d]", roleid, fd );
	
	return 0;
}
	
int AppGwSvc::RemoveRoleFdMap( u_int roleid )
{
	ROLEID_FD_MAP::iterator pos;
		
	//���û�����ֱ�ӷ���
	if( 0 == roleid)
		return 0;
	
	if((pos = _roleID_Fd_Map.find(roleid)) == _roleID_Fd_Map.end())
	{
		LOG( LOG_ERROR, __FILE__, __LINE__, "remove _roleID_Fd_Map,can't find roleid [%d]\n", roleid);
		return -1;
	}

	_roleID_Fd_Map.erase(pos);

	return 0;
}
	
int AppGwSvc::FindUserFd( u_int roleid )
{
	MutexLockGuard lock(mutexData_);
	ROLEID_FD_MAP::iterator pos;

	pos = _roleID_Fd_Map.find(roleid);
	if(pos == _roleID_Fd_Map.end())
		return 0;

	return pos->second;
}


ConnectSession* AppGwSvc::FindRoleConnSession( u_int roleid )
{
	
	//�ͻ��� fd ����
	int fd = 0;
	fd = FindUserFd( roleid );
	if( 0 == fd )
		return NULL;

	//�ͻ��� session ����
	return(FindFdConnSession( fd ));
}

int AppGwSvc::check_srv_scope( unsigned char ucSrvType, unsigned char ucSrvSeq )
{
	//���������͡���Ų���Ϊ0
	if( 0 == ucSrvType || 0 == ucSrvSeq )
		return -1;

	return 0;
}
	

