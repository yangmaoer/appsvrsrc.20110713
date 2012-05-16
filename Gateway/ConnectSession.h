#ifndef CONNECTSESSION_H
#define CONNECTSESSION_H

#include "Mutex.h"

#include "System.h"

#define SESSION_TRYSENDNUM  20				//��session����Է��ʹ���

class ConnectSession
{
public:
	ConnectSession(long long llInputConId, char cInputConnType );
	~ConnectSession();
	
	//��ȡ��������
	Byte GetConnType();
	long long GetConnId();
	u_int GetStamp();
	Byte GetLoginStatus();
	u_int GetRoleID();
	Byte GetIsSameUser();
	Byte GetSrvType();
	Byte GetSrvSeq();

	Byte GetCloseFlag();

	Byte GetClosedNum();
	
	int BuffInitCheck();
	char * GetIP();
	int GetPort();
	int GetFd();
	
	//������������
	void SetConnType( Byte ucInput );
	void SetConnId( long long llInput );
	void SetStamp();
	void SetLoginStatus( Byte ucInput );
	void SetRoleID( u_int input);
	void SetIsSameUser( Byte ucInput );
	void SetSrvType( Byte ucInput );
	void SetSrvSeq( Byte ucInput );
	void CloseConn();
	
	
	//��������
	int RecvMsg();
	
	//��������
	int SendMsg( const char *buffer, int buflen);
	
	//��ȡ�������ݰ�
	int GetDataPack( char * ptrData, int &iLen );
	
private:
	//��黺�����ݰ�
	int	CheckDataPack(char * ptrData, int &iLen);

	void CloseConnNoLock();
	
private:
	
	//���ݱ���������socket��������
	mutable MutexLock		mutexData_;
	
	//�������ݱ���,  ���ݽ��պͷ��ͷֿ�����, ����߲�������
//	mutable MutexLock		mutexSend_;

	//���Է��ʹ���
	//	���Է��ʹ�������һ����������Ͽ�����
	int _trySendNum;
	
private:
	
	//(������)�������� �� INSIDE_TCP_STREAM(���� TCP ����)    OUTSIDE_TCP_STREAM(���� TCP ����)
	Byte _connType;
	
	//(������)connect id ����idֵ,  llConId = ip(4B)+ port(2B)+ fd(2B)
	//_connID �� fd һֱ���治�ı䣬������Ϣ����;_fd �ڹر����Ӻ���Ϊ -1���������ӹر������ж�
	long long _connID;
	
	//(������)�������ݻ���
	char* _recvBuff;
	
	//(������)�������ݻ��泤��
	int _recvLen;
	
	//(������)ʱ���,		��¼session���һ�β�����ʱ��
	u_int _stamp;
	
	//(������)��¼״̬	������ʾsrv�ĵ�¼��������ʾ��ɫ�ĵ�¼			0��δ��¼  ��0���ѵ�¼
	Byte _loginStatus;
	
	//(����)��ɫID �������ӵ�key
	u_int _roleID;
	
	//(����)�Ƿ�ͬ����ɫ��¼, ��������ʹ��
	Byte _isSameUser;
	
	//(����)����������, �������� key1
	Byte _srvType;

	//(����)���������, �������� key2
	Byte _srvSeq;
	
	//���ӻỰ�رձ�־��0 ���ӻỰ���� 1 ���ӻỰ�����ر�
	Byte _closeFlag;

	//�����ùرյĴ���	0 ���;   1 һ��;  2 ���μ�����
	Byte _closedNum;
	

	//ip
	char _ip[16];

	//port
	int _port;

	//fd
	//_connID �� fd һֱ���治�ı䣬������Ϣ����;_fd �ڹر����Ӻ���Ϊ -1���������ӹر������ж�
	int _fd;
	
	
};

#endif

