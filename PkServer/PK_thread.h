
#ifndef MORE_THREAD_H
#define MORE_THREAD_H

#include <pthread.h>
#include <list>
#include <map>
#include <errno.h>
#include "PK_PE_base.h"
#include "PK_main_thread.hpp"
#include <math.h>
#include "log.h"
#include <cstring>

using namespace std;

typedef struct _UserOrder
{
	int 			ControlID;		//������ID
	int			RoleID;			//��ɫID
	RoleOrder		Order;			//����
}UserOrder;

class Buff;

class PK_PE
{
public:
	PK_PE()
	{
		pthread_mutex_init(&lock_list, NULL);
		memset(pk_map_base, 0, MAP_SIZE_X*MAP_SIZE_Y);
		//memset(pk_map_role, 0, 4*MAP_SIZE_X*MAP_SIZE_Y);
		m_pMain_thread = NULL;
		PK_State = PK_STATE_INIT;
		
	}
	int Init(Main_thread *pMain_thread, char *input, int length);
	int process_c_s_ready(int RoleID);
	int process_c_s_order();
	
	int write_pipe(char event)
	{
		if (write (pipe_handles[1], &event, 1) <= 0)
		{
			LOG (LOG_ERROR, __FILE__, __LINE__, "wirte_pipe error![%d:%s]", errno, strerror(errno));
			return -1;
		}
		return 0;
	}
	void AddOrder(UserOrder stUserOrder)
	{
		stUserOrder.Order.OrderTime = current_time.fragment_now;
		pthread_mutex_lock(&lock_list);
		Orders.push_back(stUserOrder);
		pthread_mutex_unlock(&lock_list);
	}
	unsigned int getPK_ID()
	{
		return m_PK_ID;
	}
		
private:
	int 			RunTime();
	int 			RunRole(RoleState *pRoleState);
	static void* 	run_pking(void* object);
	
private:
	int 	process_c_s_attack(RoleState *pRoleState);
	//int 	process_c_s_order_skill(RoleState *pRoleState, RoleOrder target);
	int	process_c_s_skill(RoleState *pRoleState);
	
	int 	CalculateMoveP(RoleState *pRoleState);/*ֻҪ�ƶ������������OK, 0:�ƶ������У�1:��һ��Ŀ��㲻���ƶ�2:�Ѿ��ﵽĿ���*/	
	void 	CalculateAttack(RoleState &pRoleStateAttack, RoleState &pRoleStatePassive);
	void	CalculateBuff(RoleState *pRoleState);
	void	CalculateBullets();
	void CalculateSkill();
	void CalculateAIAttack( RoleState *pRoleStatePassive );
	void CalculateSkillHurt(RoleState & pRoleStatePassive,short int attackLevel, short int hurt_value,short int effect );
	void CalculateAttackHurt( RoleState &pRoleAttack,RoleState &pRoleTarget  );
	void CalculateMultiSkillHurt( map<int,short int> & target,const short int attackLevel,short int effect  );


	
	void skill_process_201(RoleState *pRoleState, RoleOrder role_order,char level);//��ʴ
	void skill_process_202(RoleState *pRoleState, RoleOrder role_order,char level);//��Ѫ��
	void skill_process_203(RoleState *pRoleState, RoleOrder role_order,char level);//����
	void skill_process_204(RoleState *pRoleState, RoleOrder role_order,char level);//���з籩
	void skill_process_204_buffer(RoleState *pRoleState, short int hurt_value);
	void skill_process_205(RoleState *pRoleState, RoleOrder role_order,char level);//������
	void skill_process_206(RoleState *pRoleState, RoleOrder role_order,char level);//����
	void skill_process_207(RoleState *pRoleState, RoleOrder role_order,char level);//�籩֮��
	void skill_process_207_effect(Pos target_pos,RoleState *pRoleState, char Level);//�籩֮��
	void skill_process_208( RoleState *pRoleState,RoleOrder role_order,char level );//��̭֮��
	void skill_process_209(RoleState * pRoleState, RoleOrder role_order,char level);	//�޹�֮��
	void skill_process_209_buffer(RoleState *pRoleState, Buff* p209Buffer);	
	void skill_process_210(RoleState *pRoleState, RoleOrder role_order,char level);	//��������
	void skill_process_210_buffer( RoleState *pRoleState, Buff* p210Buffer );//����������BUFFER����
	void skill_process_211(RoleState * pRoleState, RoleOrder role_order,char level);		//�ش�
	void skill_process_212(RoleState * pRoleState, RoleOrder role_order,char level);	//��������
	void skill_process_213(RoleState * pRoleState, RoleOrder role_order,char level);	//����
	void skill_process_214(RoleState * pRoleState, RoleOrder role_order,char level);	//Ѫ��
	void skill_process_215(RoleState * pRoleState, RoleOrder role_order,char level);	//��ɢ
	void skill_process_216(RoleState * pRoleState, RoleOrder role_order,char level);	//��������
	void skill_process_217( RoleState * pRoleState, RoleOrder role_order,char level );	//�ղ�
	void skill_process_217_Effect( Bullet & bullet );	//�ղ���Ч
	
		
	void skill_process_218( RoleState * pRoleState, RoleOrder role_order,char level );	//��������
	void skill_process_218_Buffer( RoleState * pRoleState, Buff* tmpBuff );	//��������buffer

	void skill_process_219( RoleState * pRoleState, RoleOrder role_order,char level );	//���ǻ���
	void skill_process_219_Effect( RoleState * pRoleState,Buff & tmpBuff );		//���ǻ�����Ч
	
	void skill_process_220( RoleState * pRoleState, RoleOrder role_order,char level );	//ս����̤
	void skill_process_221( RoleState * pRoleState, RoleOrder role_order,char level );	//ŭ�����ӻ���
	void skill_process_222( RoleState * pRoleState, RoleOrder role_order,char level );	//���   ����һ�����ϵĵ����˺������ƶ�
	void skill_process_223( RoleState * pRoleState, RoleOrder role_order,char level  );	//�������
	void skill_process_224( RoleState * pRoleState, RoleOrder role_order,char level  );	//�޵�ն
	void skill_process_223_Effect( Pos target_pos, char Opposition, char level );			//���������Ч
	void skill_process_224_Effect( RoleState * pRoleState,Buff & tmpBuff );		//�޵�ն��Ч
	void skill_process_225(RoleState * pRoleState, RoleOrder role_order, char level);	//����
	void skill_process_226(RoleState * pRoleState, RoleOrder role_order, char level);	//������ʧ
	void skill_process_227(RoleState * pRoleState, RoleOrder role_order, char level);	//ǿ��һ��
	void skill_process_228(RoleState * pRoleState, RoleOrder role_order, char level);	//����֮��
	void skill_process_229(RoleState * pRoleState, RoleOrder role_order, char level);	//Χ��֮��
	void skill_process_230(RoleState * pRoleState, RoleOrder role_order, char level);	//�����츳
	void skill_process_231(RoleState * pRoleState, RoleOrder role_order, char level);	//����
	void skill_process_232(RoleState * pRoleState, RoleOrder role_order, char level);	//����
	void skill_process_233(RoleState * pRoleState, RoleOrder role_order, char level);	//��Ѫ֮ì
	void skill_process_234(RoleState * pRoleState, RoleOrder role_order, char level);	//����
	void skill_process_235(RoleState * pRoleState, RoleOrder role_order, char level);	//��Ĭ	
	void skill_process_236(RoleState * pRoleState, RoleOrder role_order, char level);	//����	

	
	void breakLastSkill( RoleState & pTargetRole );//��ϳ����Լ���

	void UpdataBuffer(RoleState & pTargetRole,Buff & tmpBuff,char updataType );
	void RunBuffer( RoleState & pTargetRole,Buff & tmpBuff  );
	
	void BroadcastBegin();
	void BrodacastAttack(int role_id, int role_passive);
	void BrodacastMpChange( int role_id, short int mp,short int effectID );
	void BrodacastHpChange( int role_id, short int hp, char source,short int effectID );
	void BrodacastMultiHurt( map<int,short int> & target,short int effect );
	void BrodacastDead(int role_id);
	void BrodacastStop(int role_id, Pos current_pos, Pos current_pos_mini);
	void Brodacast_Skill(int role_id, RoleOrder & order);
	void Brodacast_Skill_Buffer(int role_id, short int type,short int parentSkillID,short int keepTime,int param1,int param2 );
	void Brodacast_Skill_Newrole(char *new_role);
	void Brodacast_Buffer_Run( int roleID,short int skillType,int beginTime );
	void Brodacast_Buffer_Stop( int roleID,short int bufferID,short int skillID,int beginTime );
	void Brodacast_Role_LocationOffset( int roleID,short int targetX,short int targetY,short int parentSkill );
	void onRoleDead( RoleState & pTargetRole );
	int CheckOver( const char opposition );
	int EndGame(char WinOpposition);
	int AStar(RoleState *role, Pos StartPos, Pos EndPos, list<Pos> *MoveLine, char *FirstDirect);//A*�㷨���Խ�ɫ��skill��ɵĵ�ͼӰ����м��
	void GetEmptyPos( short int currX,short int currY,char direct,Pos* result);
	
private:
	int 					PK_State;
	thread_time 			current_time;
	unsigned int			m_PK_ID;
	unsigned short int		map_id;
	
	map<int, RoleState> 	P_Roles;
	list<Bullet>			Bullets;
	list<Skill_Order>		skill_order_list;
	//int 					pk_map_role[MAP_SIZE_Y][MAP_SIZE_X];
	char					pk_map_base[MAP_SIZE_Y][MAP_SIZE_X];
	MapDesc 			MapDesc_base;
	
	//�˴��Ľӿ����û�����
	list<UserOrder> Orders;
	pthread_mutex_t lock_list;
	//�ѱ�����������ָ�봫�ݸ����̶߳����Ա����߳̿��Ե������ָ��ֱ�ӷ�����
	Main_thread *m_pMain_thread;
	int pipe_handles[2];
};

#endif

