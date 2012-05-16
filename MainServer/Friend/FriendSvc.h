#ifndef FRIENDSVC_H
#define FRIENDSVC_H
#include "GWProxy.h"
#include "ConnectionPool.h"
#include "LuaState.h"
#include "IniFile.h"
#include "Serializer.h"
#include "ArchvFriend.h"

#ifndef FRIEND_TYPE_NON_CHANGE
#define FRIEND_TYPE_NON_CHANGE 3001
#endif

#ifndef MAX_CHAT_CONTENT
#define MAX_CHAT_CONTENT 300
#endif

#ifndef MAX_FRIEND_NUM
#define MAX_FRIEND_NUM 40
#endif

class MainSvc;

class FriendSvc
{
public:
	//���캯��
	FriendSvc(void *sever,ConnectionPool *cp);

	//��������
	~FriendSvc();

	void OnProcessPacket(Session& session,Packet& packet);
	//�����
	void ClientErrorAck(Session& session, Packet& packet, UInt32 RetCode);

	void ProcessPacket(Session& session, Packet& packet);

	//�ͻ��˴���Ӧ��s
	void ClientErrorAck(Session& session, Packet& packet);

	//[MsgType:1201]��ѯ����
	void ProcessGetFriend(Session& session, Packet& packet);

	//[MsgType:1202]��Ӻ���
	void ProcessAddFriend(Session& session, Packet& packet);

	//[MsgType:1203]ɾ������
	void ProcessDeleteFriend(Session& session, Packet& packet);

	//[MsgType:1204]�޸ĺ�������
	void ProcessReviseFriendType(Session& session, Packet& packet);

	//[MsgType:1205]ͬ�����Ӻ���
	void ProcessAgreeFriend(Session& session, Packet& packet);

	//[MsgType:1206]����˽��
    void ProcessFriendPrivateChat(Session& session, Packet& packet);


	void OnFriendOnLine(UInt32 roleID);
	void OnFriendOffLine(UInt32 roleID);

	//============================ s - c ack ============================

	// [MsgType:1201] �����Ϊ����
	void NotifyFriendRole(UInt32 friendRoleID, string&roleName);

	// [MsgType:1202] ɾ�����ѳɹ�
	void NotifyDeleteFriend(UInt32 RoleID,UInt32 frinedID,Byte friendType);

	// [MsgType:1203] ��Ӻ��ѳɹ�
	void NotifyAddFriendSuccessd(UInt32 RoleID, ArchvFriendAddPro &friendItem);

	// [MsgType:1204]�޸ĺ�������
	void NotifyAlterFriendType(UInt32 RoleID, ArchvFriendAddPro &friendItem);

	// [MsgType:1205]����˽��
	void NotifyFriendPrivateChat(UInt32 friendRoleID,UInt32 roleID,string& name,string &chatContent);

	// [MsgType:1206]�������ߡ�����
	void NotifyFriendOnAndOffLine(UInt32 friendRoleID,UInt32 roleID,Byte friendState);


private:

	MainSvc * _mainSvc;
	IniFile _file;
	ConnectionPool *_cp;
};

#endif

