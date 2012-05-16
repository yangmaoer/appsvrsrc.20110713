//ҵ���� server   �������

#ifndef SHOP_H
#define SHOP_H
//#define TOPCELL_NUM 25
#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"
#include "ArchvBagItemCell.h"

#ifndef NO_MUCH_GOLD
#define NO_MUCH_GOLD 1799
#endif

#ifndef NO_MUCH_GIFT
#define NO_MUCH_GIFT 1798
#endif


class MainSvc;

class ShopSvc
{
public:
	ShopSvc(void* service, ConnectionPool * cp);
	~ShopSvc();

 	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32 RetCode);



	//===================��ҵ��====================================
	void ProcessselectShopItem(Session& session,Packet& packet);
	void ProcessBuyItem(Session& session,Packet& packet);

	//===================��ҵ��=======================================

	int DropGift(UInt32 roleID,UInt32 num);

	int UpdateDBspecialItem(UInt32 ItemID,UInt32 num);
	//�۳������ĸ���
	int UpdateDBItemSellCount(UInt32 ItemID,Byte type,UInt32 num);


	//---------------------s-c�Ĺ㲥----------------------------------

	void NotifyGift(UInt32 roleID,UInt32 Gift);

	void NotifyspecialItemNum(List<ItemList>& lis );




private:
	MainSvc * _mainSvc;
	ConnectionPool *_cp;

};


#endif

