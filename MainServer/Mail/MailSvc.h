#ifndef MAILSVC_H
#define MAILSVC_H

#include "GWProxy.h"
#include "ConnectionPool.h"
#include "LuaState.h"
#include "IniFile.h"
#include "Serializer.h"
#include "ArchvMail.h"
#include "ArchvBagItemCell.h"



#ifndef MAIL_MAX_NUM
#define MAIL_MAX_NUM 101   //�ʼ�����
#endif

#ifndef PER_MAIL_COST 
#define PER_MAIL_COST 100  //ÿ���ʼ��۳��Ľ�Ǯ
#endif

#ifndef MAX_MAIL_CONTENT
#define MAX_MAIL_CONTENT 600    //�ʼ���������ַ���
#endif

#ifndef MAIL_SAVE_TIME
#define MAIL_SAVE_TIME 100000000       //�ʼ�����ʱ��
#endif


#ifndef ERROR_LACK_MONTY
#define ERROR_LACK_MONTY 2001
#endif

#ifndef ERROR_MAIL_FULL 
#define ERROR_MAIL_FULL 2002
#endif

#ifndef ERROR_ITEM_NOT_FIND
#define ERROR_ITEM_NOT_FIND 2003
#endif

class MainSvc;

class MailSvc
{
public:
	//���캯��	
	MailSvc(void *sever,ConnectionPool *cp);
	
	//��������
	~MailSvc();

	void OnProcessPacket(Session& session,Packet& packet);
	//�����
	void ClientErrorAck(Session& session, Packet& packet, UInt32 RetCode);
	
	void ProcessPacket(Session& session, Packet& packet);

	//�ͻ��˴���Ӧ��s
	void ClientErrorAck(Session& session, Packet& packet);

	//[MsgType:1501]�ʼ���ѯ
	void ProcessGetRoleMails(Session& session, Packet& packet);

    //[MsgType:1502]�����ʼ�
	void ProcessSendRoleMails(Session& session, Packet& packet);

     //[MsgType:1503]ɾ���ʼ�
	void ProcessDeleteRoleMails(Session& session, Packet& packet);

	 //[MsgType:1504]�����Ƶ�����
	void ProcessAttachToBag(Session& session, Packet& packet);
	 
	//ϵͳ�ʼ�����Ǯ���� 1 ����(�ǰ�ͭ��)��2 ���(Ԫ��)
	//���� 0 �ɹ����� 0 ʧ��
	int OnSendSystemMail(ArchvSystemMailItem sysMail);
	 //===================== s - c  ack =================================
	 void NotifyNewMail(UInt32 roleID,ArchvMailQueryItem &newMail);

private:
	int InitMoneyAttach(UInt32 &mailid, UInt32 &roleid);
	int InitItemAttach(UInt32 &mailid, UInt32 &roleid);


private:
	
	MainSvc * _mainSvc;
	//IniFile _file;
	ConnectionPool *_cp;
};


#endif

