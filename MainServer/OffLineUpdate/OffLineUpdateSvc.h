#ifndef OFFLINEUPDATESVC_H
#define OFFLINEUPDATESVC_H

#include "GWProxy.h"
#include "ConnectionPool.h"
#include "LuaState.h"
#include "IniFile.h"
#include "Serializer.h"
#include "ArchvBagItemCell.h"
#include "MainSvc.h"


class OffLineUpdateItem
{

public:
	//��Ա����
	UInt32 UpdateID;
	UInt32 RoleID;
	string BegDate;
	Byte   IsFinish;
	Byte   MoneyType;
	UInt32 LockMoney;
	UInt32 BegTime;
	UInt32 DesSec;
	UInt32 DesExp;
	UInt32 DesLevel;
	UInt32 DesNum;
	UInt32 BilledNum;
	double BilledMoney;
	UInt32 FinishedSec;
	UInt32 SpeedUpSec;
};



class OffLineUpdateSvc;

class OffLineUpdateSvc
{
public:
	//���캯��
	OffLineUpdateSvc(void *sever,ConnectionPool *cp);

	//��������
	~OffLineUpdateSvc();

	//�����
	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32 RetCode);

	void ProcessPacket(Session& session, Packet& packet);

	//�ͻ��˴���Ӧ��s
	void ClientErrorAck(Session& session, Packet& packet);


	//============================c_S Ack==================================

	//[MsgType:1601]�������ݲ�ѯ
	void ProcessUpdateDataQuery(Session& session, Packet& packet);

    //[MsgType:1602]��ʼ��������
	void ProcessBeginUpdate(Session& session, Packet& packet);

	 //[MsgType:1603]��Ҽ��ٹһ�
	void ProcessSpeedupUpdate(Session& session, Packet& packet);

	//[MsgType:1604]ֹͣ��������
	void ProcessStopUpdate(Session& session, Packet& packet);

	//[MsgType:1605] ʣ��һ���ʱ��
	 void ProcessLeftUpdateHour(Session& session, Packet& packet);

	 //[MsgType:16099]ǰ̨�����۷�
	void ProcessLostBilling(Session& session, Packet& packet);


	 //=======================s-c ack===================
	 void OnRoleLoginNotify(UInt32 roleID);

	 //[MsgType:1601]�һ�����
	 void NotifyStopUpdate(UInt32 roleID,UInt32 desExp, UInt32 desLevl,UInt32 finishSec,UInt32 desSec);

	 //[MsgType:1602]���ٹһ�10���ӵĿ۷�
	 void NotifySpeedUpCast(UInt32 roleID, UInt32 speedupCast);

public:

	//@brief	��ɫ���� ����۷�
	//@return 0 �ɹ�  ��0 ʧ��
	int AddBill( UInt32 roleID, UInt32& desExp, UInt32& desLevel, UInt32& finishSec, UInt32& desSec );

	//@brief �����þ���, Ŀ��ȼ�, ����
    //@return	0 �ɹ�  ��0 ʧ��
    int CalcCostAndGainsBySilver( UInt32 RoleID, Byte hangType, double& costSum, UInt32& expSum, UInt32& desLevel);

	int CalcCostPerMinite(UInt32 RoleID, UInt32 inputLevel, UInt32 inputExp, double& retCost, UInt32& retAddExp, UInt32& retFinalLevel, UInt32& retFinalExp,Byte vipLevl);


	//��ɫ�Ƿ�����ô���Ǯ, 1 ���ң�2 ��ҡ� ���� 0�У�1 û�У�-1ʧ��
	int IfHasSoMoney(UInt32 roleID, UInt32 money,Byte moneyType);

	//�۳���������Ǯ ,0 �ɹ���1 ʧ��
	int TakeoffAndLockMoney(UInt32 roleID, UInt32 money,Byte moneyType,const OffLineUpdateItem& offHang);

private:

	//����10���ӵĿ۷� ���� 0 �ɹ� ��0 ʧ��
	int SpeedTenMiniteCast(UInt32 roleID,UInt32 &speedTenMinute);

	//vip �һ��ӳ�
	int VipExtraExp(UInt32 roleID,Byte vipLevl,UInt32 &tmpExp);

private:
	 MainSvc * _mainSvc;
	//IniFile _file;
	ConnectionPool *_cp;
};


#endif
