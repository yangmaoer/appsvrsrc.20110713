//ҵ���� server   �������

#ifndef BAGSVC_H
#define BAGSVC_H
//#define TOPCELL_NUM 25
#define TOPSTACK_NUM 99
#define ITEMTYPE_COM 3
#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"
#include "ArchvBagItemCell.h"
#include "ArchvBag.h"
#include "Role.h"
#include <map>

#ifndef NO_MUCH_MONEY
#define NO_MUCH_MONEY  8888
#endif

#ifndef NO_ITEM
#define NO_ITEM   8887
#endif

class MainSvc;
class NewTimer;

class BagSvc
{
public:
	BagSvc(void* service, ConnectionPool * cp);
	~BagSvc();
	void SetService( MainSvc * svc );

 	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);



	//===================��ҵ��====================================


	void ProcessGetItem(Session& session,Packet& packet);//msgtype 801 ��ѯ������Ʒ�б�

	void ProcessBuyItem(Session& session,Packet& packet);//�̵���Ʒ����  802

	void ProcessUseItem(Session & session,Packet & packet);//(����Ʒ����Ʒʹ��803

	void ProcessDropItem(Session & session,Packet & packet);//��Ʒ������804

	void ProcessSellItem(Session & session,Packet & packet);//��Ʒ���ۣ����̵꣩805

	void ProcessSortItem(Session & session,Packet & packet);//��Ʒ����899

	void ProcesschangeItem(Session & session,Packet & packet);//��Ʒλ�ý���806

    void ProcesschangeTopcell(Session & session,Packet & packet);//�ı䱳������807

	void ProcessputintoStoreBag(Session & session,Packet & packet);//�ѱ��������ŵ��ֿ�

	void ProcessMixEquip(Session & session,Packet & packet);//װ������809


	void ProcessUseItemFromItemID(Session & session,Packet & packet);//ʹ����Ʒ��ItemID 810

	void ProcessUseRune(Session & session,Packet & packet);//ʹ�÷��� 811

	void ProcessQueryBagItem(Session & session,Packet & packet); //����EntityID��ѯ������Ʒ��Ϣ812

	//===================��ҵ��=======================================


	//---------------------s-c�Ĺ㲥----------------------------------

	void NotifyHPandMP(UInt32 roleID);

	void NotifyBag(UInt32 roleID,List<ItemCell>& lic);

	void NotifyMoney(UInt32 roleID );

	void NotifyBuffUpdate(UInt32 roleID, UInt32 buffID, UInt32 buffTime);

	void NotifyTeamBuffUpdate(UInt32 teamRoleID, UInt32 roleID, UInt32 buffID, UInt32 buffTime);


	//--------------------��Ʒ��������----------------------------

	UInt16 GetItem(UInt32 roleID,UInt32 ItemID,List<ItemCell>& lic,UInt16 num=1);//�õ���Ʒ

	UInt16 DropItem(UInt32 RoleID,UInt32 ItemID,List<ItemCell>& lic);//ʧȥ��Ʒ,ÿ�ζ��ǵ���

	Int16 DropItems(UInt32 RoleID,List<ItemList>& items,List<ItemCell>& lic);//ʧȥ������Ʒ(һ��һ����ʧȥ)

	UInt32 UseRune(UInt32 roleID,UInt32 itemId,UInt32 cdTime);//ʹ�÷���
	UInt32 UseItemExcludeEquip(UInt32 roleID,UInt32 ItemType,UInt32 CdTime,UInt32 itemId,RolePtr& pRole);
	//=========================Ǯ���======================================

	UInt32 selectmoney(UInt32 RoleID,UInt32& BindMoney);//��ѯ��ɫ��Ǯ,

	UInt32 selectGold(UInt32 RoleID);//��ѯ���

	Int16 Getmoney(UInt32 RoleID,UInt32 num);//�õ���Ǯ,����1�ɹ�������0ʧ��

	Int16 GetBindMoney(UInt32 RoleID,UInt32 num);//�õ���Ǯ������1�ɹ�������0ʧ��

	Int16 GetBindAndMoney(UInt32 RoleID,UInt32 num1,UInt32 num2);//�õ���Ǯ�Ͳ��󶨵�

	Int16 GetGold(UInt32 RoleID,UInt32 num);//�õ���ң���ҳ�ֵ������1�ɹ�������0ʧ��

	Int16 DropGold(UInt32 RoleID,UInt32 num);//ʧȥ��ң�����1�Ź�������0��Ҳ���

	Int16 Dropmoney(UInt32 RoleID,UInt32 num);//ʧȥ��Ǯ,����1�ɹ�������0��Ǯ����

	Int16 DropMoneyOnly(UInt32 RoleID,UInt32 num);//û�а󶨵Ľ�Ǯ�Ķ�ʧ

	Int16 DropMoneyAndGold(UInt32 RoleID,UInt32 Money,UInt32 Gold);

	Int16 GetMoneyAndGold(UInt32 RoleID,UInt32 Money,UInt32 Gold);
	//=================================================================

	UInt16 GetToBag(UInt32 RoleID,UInt16 cell,DBBagItem& item);//��item�����Ĳ��������Ʒ

	UInt16 GetToBagItem(UInt32 RoleID,UInt32 ItemID,UInt32 EntityID,UInt32 num=1);//�����Ʒ

	UInt16 DeletefromBag(UInt32 RoleID,List<UInt16>& cell,List<DBBagItem>& lic);//�ӱ�����ȥ��������Ԫ�񣬵��ǲ�ɾ��

	UInt16 DeletefromBagitem(UInt32 RoleID,UInt16 cell,DBBagItem& item );//�ӱ�����ȥ�������Ƕ�ʵ����в�ɾ��,������Ʒ�ľ�����Ϣ

	Int16 TradeItem(UInt32 RoleID,List<UInt16>& cell,UInt32 ToRoleID,List<UInt16>& cell1 );//�����˽��ף����˲���

	Int16 DigTheItem(UInt32 EntityID,UInt32 ItemID,UInt32 holdID);//���

	Int16 AddstoneToItem(UInt32 EntityID,UInt32 holdID, UInt32 ItemID);//��Ƕ

	Int16 DleteItemForEmail(UInt32 RoleID,UInt16 CellIndex,ItemList& lis,UInt32& EntityID);

	Int16 GetItemFromEmail(UInt32 RoleID,ItemList& lis,UInt32 EntityID);


	//-----------------------CellIndex------------------
	UInt16 IfhascellIndex(UInt32 RoleID);//�ж��Ƿ���һ����Ԫ,�еĻ�������cellIndex,û�еĻ�������0

	UInt16 Ifhassomany(UInt32 RoleID,List<UInt16>& Cell,UInt16 num);//�ж��Ƿ��ж��cellIndex�������ص�Ԫ��λ�õ�


	//--------------------------------ItemUse-------------------------------------

	UInt32 CompiteUse(UInt32 RoleID,UInt32 itemID,List<ItemCell>& lic);//�ϳɾ����ʹ��

	Int16 UseGiftBag(UInt32 RoleID, UInt32 itemID,List<ItemList>& lis);//ʹ�����

	Int16 UseVIPItem(UInt32 RoleID,UInt32 itemID,Byte vip);//VIP����ʹ��

	Int32 UseItemToHpMp(UInt32 RoleID,UInt32 itemID,UInt32 CdTime,RolePtr& pRole);

	Int32 UseSilverCoinCard(UInt32 roleID);//���ҿ���ʹ��

	//---------------------------------------------------------------------

	Int32 GetEmtycellNum(UInt32 roleID);//��ȡ�յĵ�Ԫ������

	Int32 SelectIfhasItems(UInt32 RoleID,List<ItemList>& items,List<ItemCell>& lic);//��ѯ�Ƿ������Щ��Ʒ


	Int32 Updatebind(UInt32 RoleID,UInt32 EntityID);//���±�����λ�õİ�״̬

	Int32 dropItemIfCell(UInt32 RoleID,ItemCell & items );//���ڿ��Զѵ�����Ʒ���ж��������д���ĺ���

	//================================�����===============================

	Int32 RoleGetItem(UInt32 roleID,List<ItemList>& items);//����0�ɹ�������1ʧ��

	int RoleGetItem(UInt32 roleID,List<UInt32>& items);//����0�ɹ�������1ʧ��

	Int32 RoleDropItem(UInt32 roleID,List<ItemList>& items);//����0�ɹ�������1ʧ��


	//-------------------------------------------------------------------------------
	Int32 RoleSelectCell(UInt32 roleID,UInt16 cellIndex,ItemCell& lic);//��ѯ����Cell

	Int32 RoleSelectCellList(UInt32 roleID,List<UInt16>& cellIndex,List<ItemCell>& lic);//��ѯ���cell


	//-----------------------װ������-----------------------------

	UInt32 SelectMixEquipMoney(list<EquipItem>& Entity);

	Int32 UpdateMixEquip(list<EquipItem>& Entity);

	float GetTheNum(UInt32 euippos);

	UInt32 GetRoleCellNum(UInt32 roleID);//��õ�Ԫ������

	//-----------------------�ص�timer---------------------
	static void TimerDecToughness( void * obj, void * arg, int argLen );
	static void TimerDecStoneness( void * obj, void * arg, int argLen );
	static void TimerDecSaintness( void * obj, void * arg, int argLen );
	static void TimerDecIntelliness( void * obj, void * arg, int argLen );
	static void TimerDecCorrectness( void * obj, void * arg, int argLen );
	static void TimerDecSpeedness( void * obj, void * arg, int argLen );
	static void TimerDecFocusness( void * obj, void * arg, int argLen );
	static void TimerDecFlyness( void * obj, void * arg, int argLen );
	static void TimerDecAngryness( void * obj, void * arg, int argLen );
	static void TimerDecExtremeness( void * obj, void * arg, int argLen );
private:
	MainSvc * _mainSvc;
	ConnectionPool *_cp;

	enum RuneTimerType
	{
		ExpRuneTimer,
		ToughRuneTimer,         //���ͷ���
		StoneRuneTimer,   ////ʯ������
		SaintRuneTimer,   //��ʥ����
		IntelliRuneTimer, //��������
		CorrectRuneTimer, //��׼����
		SpeedRuneTimer,   //���ٷ���
		FocusRuneTimer,   //רע����
		FlyRuneTimer,     //�������
		AngryRuneTimer,   //�񱩷���
		ExtremeRuneTimer,  //���޷���

		RuneTimerNum,
	};
//	NewTimer*	_runeTimer[RuneTimerNum];
	std::map<UInt32, NewTimer**> _roleRuneTimerMap;  //todo:��Щ���Ƿŵ�Role�ȽϺ�

	void RoleUseToughRune(int roleId, int itemId);
	void RoleUseStoneRune(int roleId, int itemId);
	void RoleUseSaintRune(int roleId, int itemId);
	void RoleUseIntelliRune(int roleId, int itemId);
	void RoleUseCorrectRune(int roleId, int itemId);
	void RoleUseSpeedRuneRune(int roleId, int itemId);
	void RoleUseFocusRune(int roleId, int itemId);
	void RoleUseFlyRune(int roleId, int itemId);
	void RoleUseAngryRune(int roleId, int itemId);
	void RoleUseExtremeRune(int roleId, int itemId);
};


#endif

