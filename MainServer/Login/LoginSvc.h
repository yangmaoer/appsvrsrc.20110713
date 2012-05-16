//ҵ���� server   Login ���

#ifndef LOGINSVC_H
#define LOGINSVC_H

#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"
#include "ArchvPK.h"

class MainSvc;
class ArchvRoute;

class LoginSvc
{
public:
	LoginSvc(void* service, ConnectionPool * cp);
	~LoginSvc();

 	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);

	int AdjustCoordinateX( UInt32 X, UInt32 originX );

	int AdjustCoordinateY( UInt32 Y, UInt32 originY );

	//------------S_C ����-----------------------------------------


	//------------C_S ����-----------------------------------------
	//msgtype 101 ��ɫ��½ 
	void ProcessRoleLogin(Session& session,Packet& packet);

	//msgtype 104 ��ɫ�ǳ�
	void ProcessRoleLogout(Session& session,Packet& packet);

	

	//msgtype 104 ��ɫ�ǳ�
	
	//------------��ҵ���� -----------------------------------------
	Byte GetIfIsAdult(UInt32 roleID,UInt32& TopTime,UInt32& LastloginTime);//����0δ�����ˣ�����1������

	//s-c
	void NotifyCtAdultLogin(UInt32 roleID,Byte IsAdult,Byte Flag);

private:
	MainSvc * _mainSvc;
	IniFile _file;
	ConnectionPool *_cp;
	
};


#endif

