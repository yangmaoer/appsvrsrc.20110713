//ҵ���� server   �������
#ifndef CHATSVC_H
#define CHATSVC_H

#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"


class MainSvc;
 
class ChatSvc
{
public:
	ChatSvc(void* service, ConnectionPool * cp);
	~ChatSvc();
	void SetService( MainSvc * svc );

 	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);



	//===================��ҵ��====================================
	
	void GetTeamRoleIDs(UInt32 teamID,UInt32 roleID,list<UInt32>& itor);//�������Լ�
	
	void ProcessChat(Session& session,Packet& packet);//msgtype 901 ����
	
    void NotifyChat(Byte Falg,UInt32 roleID,List<UInt32> lrid,string& Message);
	
	//===================��ҵ��=======================================

	
	//---------------------s-c�Ĺ㲥----------------------------------


private:
	MainSvc * _mainSvc;
	IniFile _file;
	ConnectionPool *_cp;

};


#endif

