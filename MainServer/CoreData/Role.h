// ��ɫ��
#ifndef ROLE_H
#define ROLE_H

#include "OurDef.h"
#include "Creature.h"
#include "./Trade/ArchvTrade.h"
#include "ArchvRole.h"
#include <map>
#include <tr1/memory>

using namespace std;

//��ɫ�����ɼ����Ե�
#define AddAddPoint_Role 1 
#define CONNECTION_FD(x)	(int)(x&0xFFFF)
#define CONNECTION_IP(x)	(x >> 32)
#define CONNECTION_PORT(x)	(int)((x >> 16)&0xFFFF)
//ÿ�����ɳ�����
#define Strength_ADD(p) (p==1)?5:((p==2)?4:((p==3)?3:((p==4)?3:((p==5)?2:3))))
#define Intelligence_ADD(p)	(p==1)?3:((p==2)?3:((p==3)?5:((p==4)?3:((p==5)?3:4))))
#define Agility_ADD(p) (p==1)?2:((p==2)?3:((p==3)?2:((p==4)?4:((p==5)?5:3))))
//�츳��ɵ�
#define ADD_Skill1(p) (p==1)?5:((p==2)?5:((p==3)?5:((p==4)?5:((p==5)?5:5))))
#define ADD_Skill2(p) (p==1)?10:((p==2)?10:((p==3)?10:((p==4)?10:((p==5)?10:10))))
#define ADD_Skill3(p) (p==1)?15:((p==2)?15:((p==3)?15:((p==4)?15:((p==5)?15:15))))
#define ADD_Skill4(p) (p==1)?10:((p==2)?10:((p==3)?10:((p==4)?10:((p==5)?10:10))))
#define ADD_Skill5(p) (p==1)?5:((p==2)?5:((p==3)?5:((p==4)?5:((p==5)?5:5))))
#define ADD_Skill6(p) (p==1)?50:((p==2)?50:((p==3)?50:((p==4)?50:((p==5)?50:50))))
#define ADD_Skill7(p) (p==1)?10:((p==2)?10:((p==3)?10:((p==4)?10:((p==5)?10:10))))
#define ADD_Skill8(p) (p==1)?10:((p==2)?10:((p==3)?10:((p==4)?10:((p==5)?10:10))))
#define ADD_Skill9(p) (p==1)?5:((p==2)?5:((p==3)?5:((p==4)?5:((p==5)?5:5))))
#define ADD_Skill10(p) (p==1)?2:((p==2)?1:((p==3)?1:((p==4)?1:((p==5)?1:1))))
#define ADD_Skill11(p) (p==1)?15:((p==2)?15:((p==3)?15:((p==4)?15:((p==5)?15:15))))
#define ADD_Skill12(p) (p==1)?20:((p==2)?20:((p==3)?20:((p==4)?20:((p==5)?20:20))))
class ConnectionPool;

class Role
	:public Creature
{
public:
	Role();
	~Role();
public:
	//-------------------------
	int InitRoleCache(UInt32 roleID, UInt64 connID, ConnectionPool * cp );
	void SetConnectPool(ConnectionPool * cp );
	ConnectionPool *  Cp();
	int DB2Cache();
	int Cache2DB();
	int DB2CacheBonus();
	int Cache2Bonus();
	int UpdateaccountDB();
	void UpdateTotalOnlineSec();
	UInt32 HpMpfullflag;//Hp,Mp�Ƿ����ֵ�ı�־
	UInt32 HpCDflag;
	UInt32 MpCDflag;
	UInt32 HpMpflag;
	UInt32 LastHpMpTime;


public:
	//��ȡ����
	UInt32 MapID();
	UInt16 LastX();
	UInt16 LastY();
	UInt32 LoginTime();
	string Password();
	UInt32 ProID();
	UInt32 GuildID();
	UInt32 Glory();
	UInt32 Crime();
	UInt32 TotalOnlineSec();
	UInt32 LastloginTime();
	UInt32 TopCellNum();

	UInt32 TeamID();
	UInt32 LeaderRoleID();
	Byte TeamFlag();
	Byte ISVIP();//����vip�ȼ���0-6
	Byte VIP();//���ذ���0-6���Լ�11-16


	
	//��������
	void MapID(UInt32 input);
	void LastX(UInt16 input);
	void LastY(UInt16 input);
	void LoginTime(UInt32 input);
	void Password(const string& input );
	void ProID(UInt32 input);
	void GuildID(UInt32 input);
	void LastloginTime(UInt32 input);
	void TopCellNum(UInt32 input);
	void IsVIP(Byte input);
	
	void TeamID(UInt32 input);
	void LeaderRoleID(UInt32 input);
	void TeamFlag(Byte input);
	

	//���ԼӼ�
	void AddGlory(Int32 input);
	void AddCrime(Int32 input);
	//--------------------------------------------------------
	Int16 IfTheExpToMax();
	//�жϾ����Ƿ񵽴�����
	void Role_ADDLev();
	//�������������Ľ�ɫ���Ա仯
	void RoleAddStrength(Int32 input);
	void RoleAddIntelligence(Int32 input);
	void RoleAddAgility(Int32 input);
	void RoleAddStrengthBonus(Int32 input);
	void RoleAddIntelligenceBonus(Int32 input);
	void RoleAddAgilityBonus(Int32 input);
	void RoleAddSKill(Int32 skillID);
	void RoleAddtalentpoint(Int32 talentID);//����������츳�Ժ�����Ա仯
    void CdTimetoTheFalg(int kind,UInt32 CdTime);//CD
    void SetHpMpfullStatue();//״̬
    void HpMpadd(int time1);
	void RolePKset(Int32 Hp,Int32 Mp);
	Int32 RoleExpAdd(Int32 input);
	
	Byte IsOfflineUpdate();
	void IsOfflineUpdate(Byte input);

	//����
	TradeItem TradeInfo();
	void TradeInfo(TradeItem tradeItem);
	
	UInt32 EnterMapNum();
	void AddEnterMapNum();

	//���þ������
	void SetRoleRuneTime(UInt32 itemId);
	int GetRoleRuneTime(UInt32 itemId);

	void PopulateRoleRuneList(List<RoleRune>& lic) ;

	void UseRune(UInt32 itemId);
private:
	string _ip;								//�ͻ���ip
	UInt16 _port;							//�ͻ��˶˿�
	UInt32 _fd;								//�ͻ���fd
	
	UInt32 _mapID;						//��ɫ����map
	UInt16 _lastX;						//���x����
	UInt16 _lastY;						//���y����
	UInt32 _loginTime;				//��½ʱ��
	UInt32 _lastloginTime;			//�ϴ��˳�ʱ��
	UInt32 _topcellnum;      //������������

	UInt32 _teamID;          //����ID
	UInt32 _leaderRoleID;	//��ɫID
	Byte _teamFlag;         //��ӱ�־  0 û����� 1 ����Ա����� 2 ����Ա���ѹ�� 3 �ӳ�	

	string _password;						//��½����	
	UInt32 _proID;							//ְҵID
	UInt32 _guildID;						//�л�ID
	UInt32 _glory;							//����
	UInt32 _crime;							//���ֵ
	UInt32 _totalOnlineSec;		//������ʱ�� ��λ ��
	Byte  _isOfflineUpdate;		//�Ƿ������߹һ� 0 ��  1 ��
	TradeItem _trade;           //��ɫ������Ϣ
	UInt32 _enterMapNum;			//�����ͼ���ܴ���
	Byte _isvip;          //�Ƿ���VIP��0��(11��12��13��14��15��16��ǰ��VIP)Ŀǰ���ǣ�1-6��
	
	map<UInt32, UInt32> runeMap;// id + activetime

	int _expRuneTime; //���ʹ�þ�����ĵ�ʱ��

	int _toughRuneTime1; //���ͷ���1: 500
	int _toughRuneTime2; //���ͷ���2: 1000
	int _toughRuneTime3; //���ͷ���3: 1500
	int _toughRuneTime4; //���ͷ���4:  2000

	int _stoneRuneTime1;  //ʯ������
	int _stoneRuneTime2;  //ʯ������
	int _stoneRuneTime3;  //ʯ������
	int _stoneRuneTime4;  //ʯ������

	int _saintRuneTime1;  //��ʥ����
	int _saintRuneTime2;  //��ʥ����
	int _saintRuneTime3;  //��ʥ����
	int _saintRuneTime4;  //��ʥ����

	int _intelliRuneTime1;  //��������
	int _intelliRuneTime2;  //��������
	int _intelliRuneTime3;  //��������
	int _intelliRuneTime4;  //��������

	int _correctRuneTime1;  //��׼����
	int _correctRuneTime2;  //��׼����
	int _correctRuneTime3;  //��׼����
	int _correctRuneTime4;  //��׼����

	int _speedRuneTime1;  //���ٷ���1
	int _speedRuneTime2;  //���ٷ���2
	int _speedRuneTime3;  //���ٷ���3
	int _speedRuneTime4;  //���ٷ���4

	int _focusRuneTime1;  //רע����1
	int _focusRuneTime2;  //רע����2
	int _focusRuneTime3;  //רע����3
	int _focusRuneTime4;  //רע����4

	int _flyRuneTime1;  //�������1
	int _flyRuneTime2;  //�������2
	int _flyRuneTime3;  //�������3
	int _flyRuneTime4;  //�������4

	int _angryRuneTime1;  //�񱩷���1
	int _angryRuneTime2;  //�񱩷���2
	int _angryRuneTime3;  //�񱩷���3
	int _angryRuneTime4;  //�񱩷���4

	int _extremeRuneTime1;  //���޷���1
	int _extremeRuneTime2;  //���޷���2
	int _extremeRuneTime3;  //���޷���3
	int _extremeRuneTime4;  //���޷���4

	//���ݿ����ӳ�
	ConnectionPool* _cp;
	
};

typedef tr1::shared_ptr<Role> RolePtr;


#endif

