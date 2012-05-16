
#ifndef TRADESVC_H
#define TRADESVC_H

#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"
#include "ArchvTrade.h"

#ifndef   BIND_ITEM
#define  BIND_ITEM 1399
#endif



class MainSvc;
 
class TradeSvc
{
public:
	TradeSvc(void* service, ConnectionPool * cp);
	~TradeSvc();
	//void SetService( MainSvc * svc );

 	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);



	//===================��ҵ��====================================
	
	
	 
	//������[MsgType:1301]
	void ProcessRequestTrade(Session& session,Packet& packet);
	//S-C���Է���ĳ���˸��㷢�����뽻��
	//�Է��յ�S-C ������ѡ��
	//a.���ܣ�b,�ܾ�

	//�������˷���Ľ���[MsgType:1302]
	//�ش��s-c�𰸣�ĳ���˽������������, �������˶����ͣ���ʾ������������
	void ProcessAnswerTrade(Session& session,Packet& packet);
	
    //��Ʒ��Ϣ����[MsgType:1303]
	void ProcessItemPast(Session& session,Packet& packet);//��Ʒ��Ϣת��
	//������CELLIndex

    //��Ǯ��Ϣ����[MsgType:1305]
	void ProcessMoney(Session& session,Packet& packet);//��Ǯ

	//������Ʒ[MsgType:1304]
	void ProcessLockTrade(Session& session,Packet& packet);//������Ʒ��������Ʒ��ʱ�������һ����Ʒ��Ϣ��ȫ����Ʒ����Ϣ
	//���͸��Է�����̨���ݴ洢�����Ҽ�¼״̬,��ʱ��̨�����ݴ洢

	//�������[MsgType:1306]
	void ProcessTrade(Session& session,Packet& packet);//����OK

	// ȡ������ [MsgType:1307]
	void ProcessCancelTrade(Session& session,Packet& packet);

	
	//==============================  S-C Ack ===============================================
	void NotifyRequest(UInt32 roleID,string Name,UInt32 toRoleID);
	void NotifyRequestResult(UInt32 roleID,string Name,UInt32 ToroleID,Byte type);	

	void NotifyToRoleBagItem(UInt32 roleID,UInt32 ID,ItemCell lic,Byte type);//����ɫ������Ʒ��Ϣ��ֻ��һ����Ϣ����
		
	void NotifyToRoleMoney(UInt32 roleID,UInt32 ID,UInt32 Money,UInt32 Gold);
	void NotifyLockTrade(UInt32 roleID,UInt32 toRoleID);
	void NotifyCancelTrade(UInt32 toRoleID,UInt32 roleID,string roleName,Byte result);

	void NotifyTradeOk(UInt32 toRoleID,Byte TradeResult,UInt32 RoleID,string RoleName);//S-C���OK�����


	//==================================
	//����Ľ�Ǯ����
	int TradeMoney(TradeItem& lic1,TradeItem& lic2);

	//��ɫ�Ƿ��ڽ����� ����ֵ 0 Ϊ�ڽ��ף�1 ���ڽ��ף� -1 ʧ��
	int IsOnTrade(UInt32 roleID);

	//��ɫ�Ƿ������˽��� ����ֵ 0 δ������1 �Ѿ�����, -1 ʧ��
	int IsLockTrade(UInt32 roleID);

	//��ɫ���׽����󣬽�����Ϣ���óɳ�ʼ��״̬
	//����ֵ 0 �ɹ����� 0 ʧ��
	int InitTradeItem(UInt32 roleID);

	//test
	void Test(TradeItem trade,UInt32 roleID);
	

private:
	MainSvc * _mainSvc;
	IniFile _file;
	ConnectionPool *_cp;

};


#endif


