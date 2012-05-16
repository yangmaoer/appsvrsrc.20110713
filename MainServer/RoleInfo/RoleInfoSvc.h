//ҵ���� server   RoleInfo ���

#ifndef ROLEINFOSVC_H
#define ROLEINFOSVC_H

#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"
#include "ArchvPK.h"
#include "Role.h"

class MainSvc;
class NewTimer;

class RoleInfoSvc
{
public:
	RoleInfoSvc(void* service, ConnectionPool * cp);
	~RoleInfoSvc();

 	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);

	int AdjustCoordinateX( UInt32 X, UInt32 originX );

	int AdjustCoordinateY( UInt32 Y, UInt32 originY );



	static void HandleEvent1( void * obj, void * arg, int argLen);
	//------------S_C ����----------------------------------------
	void NotifySKill(UInt32 roleID,List<ArchvSkill>& sk);
	//֪ͨ���ܱ��
	void Notifyinfo(UInt32 roleID);

	void Notifyinfo(UInt32 roleID,RolePtr& role);
	//֪ͨ����������Ա��

	//[306]֪ͨ��ǰ�ľ���
	void NotifyExp(UInt32 roleID);

	//------------C_S ����-----------------------------------------
	//[MsgType:0301]��ѯ��ɫ����fromDB (���ʺϵ�½��δ�����ͼʹ��;��Ϊ��̨�Ǵ����ݿ���ȡ����)
	void	ProcessGetRoleInfoFromDB(Session& session,Packet& packet);

	//msgtype 302 ��ѯ��ɫ��Ҫ����
	void ProcessGetRoleBriefInfo(Session& session,Packet& packet);


	//msgtype 303���ܲ�ѯ
	void ProcessGetRoleSkill(Session& session,Packet& packet);

	//msgtype 304 ���ܼӵ�
	void ProcessRoleSkillAdd(Session& session,Packet& packet);

	void RoleExpAddToDB(UInt32 roleID,int num);

	//[MsgType:0305] ��ѯ��ɫ����from���� (�ʺϵ�½�����ѽ����ͼʹ��;��̨�Ǵӻ�����ȡ����)
	void	ProcessGetRoleInfoFromCache(Session& session,Packet& packet);
	
	Byte GetIfIsAdult(UInt32 roleID);
	//------------��ҵ���� -----------------------------------------
	void NotifyCtAdult(UInt32 roleID,Byte Type);

	void NotifyRolePoint(UInt32 roleID,UInt32 point); 
		
    void GetSkill( UInt32 RoleID, List<ArchvSkill>& SK);

private:
	MainSvc * _mainSvc;
	IniFile _file;
	ConnectionPool *_cp;

	std::list<NewTimer*> _timerList;
	
};


#endif

