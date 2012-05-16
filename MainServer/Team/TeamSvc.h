
#ifndef TEAMSVC_H
#define TEAMSVC_H

#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"


class MainSvc;
 
class TeamSvc
{
public:
	TeamSvc(void* service, ConnectionPool * cp);
	~TeamSvc();
	void SetService( MainSvc * svc );

 	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);



	//===================��ҵ��====================================
	
	
	 
	//����������
	void ProcessRequestjoin(Session& session,Packet& packet);
	//S-C���Է���ĳ���˸��㷢�����뽻��

	//�ش���������Ϣ
	void ProcessAnswerRequest(Session& session,Packet& packet);
	//�ش��s-c�𰸣�ĳ���˽������������, �������˶����ͣ���ʾ������������
	void ProcessChangeCaptain(Session& session,Packet& packet);//����Ϊ�ӳ�

	void ProcessCapt_Member(Session& session,Packet& packet);//�߳�����

	void ProcessCallMember(Session& session,Packet& packet);//�ٻ���Ա

	void ProcessMember_OUt(Session& session,Packet& packet);//�˳�����

	void ProcessBackOrleave(Session& session,Packet& packet);//��ӻ������


	void NotifyReceive(UInt32 RoleID,string Name,List<UInt32>& it);

	void NotifyTeamChang(UInt32 RoleID,Byte changeType,List<UInt32>& it);

	void NotifyCaptionCall(List<UInt32>& it);
	
	void NotifyRefuse(UInt32 RoleID,UInt32 ID);//˭�ܾ�˭��RoleID��ָ˭ID���յ�����

	void NotifyTeamInfo(UInt32 RoleID,list<TeamRole>& l);
private:
	MainSvc * _mainSvc;
	IniFile _file;
	ConnectionPool *_cp;

};


#endif

