//ҵ���� server   �������

#ifndef GUIDESVC_H
#define GUIDESVC_H

#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"
#include "ArchvBagItemCell.h"


class MainSvc;
 
class GuideSvc
{
public:
	GuideSvc(void* service, ConnectionPool * cp);
	~GuideSvc();
	void SetService( MainSvc * svc );

 	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);



	//===================��ҵ��====================================
	
	//[MsgType:1801]�������������Ľ���
	void ProcessSetGuideStep(Session& session,Packet& packet);

	//[MsgType:1802]������������
	void ProcessEndGuide(Session& session,Packet& packet);
	
	//===================��ҵ��=======================================

	
	//---------------------s-c�Ĺ㲥----------------------------------

	
	void NotifyGuideStep(UInt32 roleID);


private:
	MainSvc * _mainSvc;
	IniFile _file;
	ConnectionPool *_cp;

};


#endif

