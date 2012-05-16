// ��ɫ��
#ifndef TEAM_H
#define TEAM_H

#include "OurDef.h"
#include "Creature.h"
#include  "ArchvRole.h"
//��ɫ�����ɼ����Ե�

class Team
{
public:
		Team();
		~Team();

public:
		list<TeamRole> GetMemberRoleID();
		UInt32 GetTeamID();
		UInt32 GetLeaderRoleID();

		
		void AddToTeam(UInt32 roleID);//�����Ա
		void ChangToBeLeader(UInt32 roleID);//�����ӳ�
		void OutTheTeam(UInt32);//�뿪����
		void changeRoleStues(UInt32 roleID,Byte type);
		
			
		void TeamID(UInt32 input);
		void LeaderRoleID(UInt32 input);
		
		
	//-------------------------
private:
		UInt32		_teamID;
		UInt32		_leaderRoleID;
		list<TeamRole>	_listMember;

};


#endif

