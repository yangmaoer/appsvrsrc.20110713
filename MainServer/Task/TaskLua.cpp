#include "TaskLua.h"
#include "TaskSvc.h"
#include "Log.h"
#include "CoreData.h"

namespace TaskLua
{

//@brief ע��C API for lua
void RegisterTaskCAPI(lua_State* L)
{
	lua_register(L,"GetRoleTaskStatus",GetRoleTaskStatus);
	lua_register(L,"GetRoleProID",GetRoleProID);
	
	lua_register(L,"AddRoleItem",AddRoleItem);
	lua_register(L,"AddRoleMoney",AddRoleMoney);
	lua_register(L,"AddRoleBindMoney",AddRoleBindMoney);
	
	lua_register(L,"AddRoleExp",AddRoleExp);
	lua_register(L,"DeleteItem",DeleteItem);
	
	lua_register(L,"AddRoleTaskKillMonster",AddRoleTaskKillMonster);
	lua_register(L,"AddRoleTaskHoldItem",AddRoleTaskHoldItem);
	lua_register(L,"AddRoleTaskUseItem", AddRoleTaskUseItem );
	lua_register(L,"AddRoleTaskDress", AddRoleTaskDress);
	lua_register(L,"AddRoleTaskCallPet", AddRoleTaskCallPet);
	lua_register(L,"AddRoleTaskLearnSkill", AddRoleTaskLearnSkill);
	lua_register(L,"AddRoleTaskComposeEquip", AddRoleTaskComposeEquip);
	lua_register(L,"AddRoleTaskAddFriend", AddRoleTaskAddFriend);
	lua_register(L,"AddRoleTaskAddTeam", AddRoleTaskAddTeam);


	
	
	
 	lua_settop(L,0);
}



//@brief ��ȡlua �ű�ִ�еķ���ֵ 
//		��������һ����������ֵ�Ľű�
//@return lua �ķ���ֵ
int GetLuaRetCode(lua_State* L)
{
	return lua_tointeger(L,-1);
}



//@brief ��ѯָ����ɫָ�������״̬��Ϣ
//@lua ���ø�ʽ: GetRoleTaskStatus( taskID ) 
//@param-Lua	taskID	����ID
//@return-Lua	status 1 δ�� 2δ��� 3 ����� 4 �ѽ���
int GetRoleTaskStatus(lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID;
	UInt32 taskID;
	Byte status = 0;
 	int iRet = 0;

	//���
	GET_TASKSVC(L,taskSvc);
	GET_ROLEID(L, roleID);	
	taskID = lua_tointeger(L,1);

 	iRet = taskSvc->GetRoleTaskStatus( roleID, taskID, status );
 	if(iRet)
 	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"GetRoleTaskStatus error!!!!");
		return 0;
	}
	
	lua_pushinteger(L,status);
 
	return 1;
}

//@brief ��ѯ��ɫ��ְҵ
//@lua ���ø�ʽ: GetRoleProID() 
//@return-Lua	proID	ְҵID
int GetRoleProID(lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID;
	UInt32 taskID;
	UInt32 proID = 0;
 	int iRet = 0;

	//���
	GET_TASKSVC(L,taskSvc);
	GET_ROLEID(L, roleID);	



 	iRet = taskSvc->GetRoleProID( roleID, proID);
 	if(iRet)
 	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"GetRoleProID error!!!!");
		return 0;
	}

	
	lua_pushinteger(L,proID);
 
	return 1;
}




int AddRoleMoney(lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	UInt32 num=0;
	int iRet=0;
	List<ItemCell> lic;
	
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	num=lua_tointeger(L,1);

	iRet=taskSvc->AddRoleMoney(roleID,num);
	if(iRet==0)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRoleMoney error!!!!");
		return 0;
	}
	return 0;
}





int AddRoleBindMoney(lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	UInt32 num=0;
	int iRet=0;
	List<ItemCell> lic;
	
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	num=lua_tointeger(L,1);

	iRet=taskSvc->AddRoleBindMoney(roleID,num);
	if(iRet==0)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRoleBindMoney error!!!!");
		return 0;
	}
	return 0;
}


int AddRoleExp(lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	UInt32 exp=0;
	int iRet=0;
	List<ItemCell> lic;
	
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	exp=lua_tointeger(L,1);

	iRet=taskSvc->AddRoleExp(roleID,exp);
	if(iRet==-1)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRoleExp error!!!!");
		return 0;
	}
	//s-c�Ļ�þ����֪ͨ����ʱû��
	return 0;
}
int AddRoleItem(lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	int iRet=0;
	UInt32 ItemID=0;
	UInt32 num=0;
	
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	ItemID=lua_tointeger(L,1);
	num=lua_tointeger(L,2);

	iRet=taskSvc->AddRoleItem(roleID,ItemID,num);
	if(iRet==-1)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"there is no room to get item error!!!!");
		return 0;
	}
	if(iRet==1)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"get item error!!!!");
		return 0;
		
	}
	
	return 0;
}

int DeleteItem(lua_State* L)
{
		TaskSvc * taskSvc;
		UInt32 roleID = 0;
		int iRet=0;
		UInt32 ItemID=0;
		UInt32 num=0;
		
		GET_TASKSVC( L,taskSvc );
		GET_ROLEID( L, roleID );
		ItemID=lua_tointeger(L,1);
		num=lua_tointeger(L,2);

		iRet=taskSvc->DeleteItem(roleID,ItemID,num);
		if(iRet==-1)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"there is not so many  item error!!!!");
			return 0;
		}
		if(iRet==1)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"get item error!!!!");
			return 0;
			
		}
		
		return 0;
}


//@brief ��������ϸ��--ɱ��
//@lua ���ø�ʽ: AddRoleTaskKillMonster( goalID, goalNum ) 
//@param-Lua	goalID	Ŀ��ID
//@param-Lua	goalNum		Ŀ������
//@return-Lua	��
int AddRoleTaskKillMonster(lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	UInt32 taskID = 0;

	//���
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	GET_TASKID( L, taskID );
	UInt32 goalID = lua_tointeger(L,1);
	UInt32 goalNum = lua_tointeger(L,2);
	Byte	 goalType = 1;										//ɱ��

 	int iRet = taskSvc->AddRoleTaskDetail( roleID, taskID, goalType, goalID, goalNum);
 	if(iRet)
 	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRoleTaskDetail error!roleID[%d], taskID[%d], goalType[%d], goalID[%d], goalNum[%d]", roleID, taskID, goalType, goalID, goalNum	);
		return 0;
	}
	
	return 0;
}


//@brief ��������ϸ��--��Ʒ��ȡ  , �� ��Ʒ����
//@lua ���ø�ʽ: AddRoleTaskHoldItem( goalID, goalNum ) 
//@param-Lua	goalID	Ŀ��ID
//@param-Lua	goalNum		Ŀ������
//@return-Lua	��
int AddRoleTaskHoldItem(lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	UInt32 taskID = 0;

	//���
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	GET_TASKID( L, taskID );
	UInt32 goalID = lua_tointeger(L,1);
	UInt32 goalNum = lua_tointeger(L,2);
	Byte	 goalType = 2;										//��Ʒ��ȡ,��Ʒ����

 	int iRet = taskSvc->AddRoleTaskDetail( roleID, taskID, goalType, goalID, goalNum);
 	if(iRet)
 	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRoleTaskDetail error!roleID[%d], taskID[%d], goalType[%d], goalID[%d], goalNum[%d]", roleID, taskID, goalType, goalID, goalNum	);
		return 0;
	}
	
	return 0;
}


//@brief ��������ϸ��--ʹ����Ʒ
//@lua ���ø�ʽ: AddRoleTaskUseItem( goalID ) 
//@param-Lua	goalID	Ŀ��ID
//@return-Lua	��
int AddRoleTaskUseItem(lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	UInt32 taskID = 0;

	//���
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	GET_TASKID( L, taskID );
	UInt32 goalID = lua_tointeger(L,1);
	UInt32 goalNum = 1;
	Byte	 goalType = 3;										//ʹ����Ʒ

 	int iRet = taskSvc->AddRoleTaskDetail( roleID, taskID, goalType, goalID, goalNum);
 	if(iRet)
 	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRoleTaskDetail error!roleID[%d], taskID[%d], goalType[%d], goalID[%d], goalNum[%d]", roleID, taskID, goalType, goalID, goalNum	);
		return 0;
	}
	
	return 0;
}




//@brief ��������ϸ��--��װ��
//@lua ���ø�ʽ: AddRoleTaskDress( goalID ) 
//@param-Lua	goalID	Ŀ��ID
//@return-Lua	��
int AddRoleTaskDress(lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	UInt32 taskID = 0;

	//���
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	GET_TASKID( L, taskID );
	UInt32 goalID = lua_tointeger(L,1);
	UInt32 goalNum = 1;
	Byte	 goalType = 4;										//��װ��

 	int iRet = taskSvc->AddRoleTaskDetail( roleID, taskID, goalType, goalID, goalNum);
 	if(iRet)
 	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRoleTaskDetail error!roleID[%d], taskID[%d], goalType[%d], goalID[%d], goalNum[%d]", roleID, taskID, goalType, goalID, goalNum	);
		return 0;
	}
	
	return 0;
}







//@brief ��������ϸ��--��װ��
//@lua ���ø�ʽ: AddRoleTaskCallPet( goalID ) 
//@param-Lua	goalID	Ŀ��ID
//@return-Lua	��
int AddRoleTaskCallPet(lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	UInt32 taskID = 0;

	//���
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	GET_TASKID( L, taskID );
	UInt32 goalID = lua_tointeger(L,1);
	UInt32 goalNum = 1;
	Byte	 goalType = 5;										//�ٻ�����

 	int iRet = taskSvc->AddRoleTaskDetail( roleID, taskID, goalType, goalID, goalNum);
 	if(iRet)
 	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRoleTaskDetail error!roleID[%d], taskID[%d], goalType[%d], goalID[%d], goalNum[%d]", roleID, taskID, goalType, goalID, goalNum	);
		return 0;
	}
	
	return 0;
}



//@brief ��������ϸ��--ѧϰ����
//@lua ���ø�ʽ: AddRoleTaskLearnSkill () 
//@return-Lua	��
int AddRoleTaskLearnSkill (lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	UInt32 taskID = 0;

	//���
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	GET_TASKID( L, taskID );
	UInt32 goalID = 0;
	UInt32 goalNum = 1;
	Byte	 goalType = 6;										//ѧϰ����

 	int iRet = taskSvc->AddRoleTaskDetail( roleID, taskID, goalType, goalID, goalNum);
 	if(iRet)
 	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRoleTaskDetail error!roleID[%d], taskID[%d], goalType[%d], goalID[%d], goalNum[%d]", roleID, taskID, goalType, goalID, goalNum	);
		return 0;
	}
	
	return 0;
}





//@brief ��������ϸ��--װ���ϳ�
//@lua ���ø�ʽ: AddRoleTaskComposeEquip () 
//@return-Lua	��
int AddRoleTaskComposeEquip (lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	UInt32 taskID = 0;

	//���
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	GET_TASKID( L, taskID );
	UInt32 goalID = 0;
	UInt32 goalNum = 1;
	Byte	 goalType = 7;										//װ���ϳ�

 	int iRet = taskSvc->AddRoleTaskDetail( roleID, taskID, goalType, goalID, goalNum);
 	if(iRet)
 	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRoleTaskDetail error!roleID[%d], taskID[%d], goalType[%d], goalID[%d], goalNum[%d]", roleID, taskID, goalType, goalID, goalNum	);
		return 0;
	}
	
	return 0;
}


//@brief ��������ϸ��--��Ӻ���
//@lua ���ø�ʽ: AddRoleTaskAddFriend () 
//@return-Lua	��
int AddRoleTaskAddFriend (lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	UInt32 taskID = 0;

	//���
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	GET_TASKID( L, taskID );
	UInt32 goalID = 0;
	UInt32 goalNum = 1;
	Byte	 goalType = 8;										//��Ӻ���

 	int iRet = taskSvc->AddRoleTaskDetail( roleID, taskID, goalType, goalID, goalNum);
 	if(iRet)
 	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRoleTaskDetail error!roleID[%d], taskID[%d], goalType[%d], goalID[%d], goalNum[%d]", roleID, taskID, goalType, goalID, goalNum	);
		return 0;
	}
	
	return 0;
}



//@brief ��������ϸ��--��Ӳ���
//@lua ���ø�ʽ: AddRoleTaskAddTeam () 
//@return-Lua	��
int AddRoleTaskAddTeam (lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID = 0;
	UInt32 taskID = 0;

	//���
	GET_TASKSVC( L,taskSvc );
	GET_ROLEID( L, roleID );
	GET_TASKID( L, taskID );
	UInt32 goalID = 0;
	UInt32 goalNum = 1;
	Byte	 goalType = 9;										//��Ӳ���

 	int iRet = taskSvc->AddRoleTaskDetail( roleID, taskID, goalType, goalID, goalNum);
 	if(iRet)
 	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRoleTaskDetail error!roleID[%d], taskID[%d], goalType[%d], goalID[%d], goalNum[%d]", roleID, taskID, goalType, goalID, goalNum	);
		return 0;
	}
	
	return 0;
}



}



