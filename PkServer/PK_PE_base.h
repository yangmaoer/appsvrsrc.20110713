#ifndef PK_PE_BASE_H
#define PK_PE_BASE_H

#include <list>
#include <sys/time.h>

using namespace std;

#define 	MAP_SIZE_X 			50
#define 	MAP_SIZE_Y 			29
#define 	MAP_MAX_LINE 			1000	//A*Ѱ·���ޣ�10Ϊһ������
#define	MAP_IMPACT_AREA		30		//A*�����ײ�ķ�Χ
#define	MAX_ROLE_COUNT		30		//���֧�ֵĽ�ɫ

#define 	TIME_WAITING_READY	60
#define	TIME_OFFLINE			1200

#define	LEN_PKG_HEAD 			15
#define	DIRECT_C_S_REQ		1
#define	DIRECT_C_S_RESP		2
#define	DIRECT_S_C_REQ		3
#define	DIRECT_SC_S_REQ		5
#define	DIRECT_SC_S_RESP		6
#define	SRVTYPE_GE				1
#define	SRVTYPE_PK				2
#define	MSGTYPE_C_S_READY		101
#define	MSGTYPE_C_S_ORDER		102
#define	MSGTYPE_S_S_BEGIN		101
#define	MSGTYPE_S_C_READY		101
#define	MSGTYPE_S_C_BEGIN		102
#define	MSGTYPE_S_C_MOVE			103
#define	MSGTYPE_S_C_ATTACK	 	104
#define	MSGTYPE_S_C_DEAD			107
#define	MSGTYPE_S_C_STOP			108
#define   MSGTYPE_S_C_MULTIHURT	109
#define	MSGTYPE_S_C_SKILL			118
#define	MSGTYPE_S_C_SKILL_BUFFER		119
#define	MSGTYPE_S_C_SKILL_NOLOAD		120
#define   MSGTYPE_S_C_BUFFER_RUN	121
#define   MSGTYPE_S_C_BUFFER_STOP	122
#define   MSGTYPE_S_C_FIGHTEND		127
#define   MSGTYPE_S_C_LOCATAINOFFSET		128
#define	MSGTYPE_S_C_MPCHANGE			129
#define	MSGTYPE_S_C_HPCHANGE			130

#define	MSGTYPE_S_C_STOPSKILL		110

#define	MSGTYPE_S_C_SKILL_NEWROLE	126
#define	MSGTYPE_S_S_CONNECT_GW		101
#define	MSGTYPE_S_S_END_GAME			201

#define 	ORDER_TYPE_NO			0
#define 	ORDER_TYPE_ATTACK_E	1
#define 	ORDER_TYPE_ATTACK_P	2
#define 	ORDER_TYPE_MOVE_E	3
#define 	ORDER_TYPE_MOVE_P	4
#define   ORDER_TYPE_SKILL		5
#define 	ORDER_TYPE_HOLD		6
#define 	ORDER_TYPE_STOP		7
#define	ORDER_TYPE_CASTSKILL	8


#define	ORIGIN_MAIN_ROLE		1		//����
#define	ORIGIN_PET				2		//����
#define	ORIGIN_CALL			3		//�ٻ�
#define	ORIGIN_CLONED			4		//����

#define	OPPOSITION_1 			1
#define	OPPOSITION_2 			2
#define	OPPOSITION_NEUTRAL 	3
#define	OPPOSITION_HOSTILITY 	4

#define	ROLE_STATUS_LIVE		1
#define	ROLE_STATUS_DEAD		2
#define	ROLE_STATUS_OFFLINE	3

#define	SKILL_TYPE_YueShi				201
#define	SKILL_TYPE_HuoXueShu			202
#define	SKILL_TYPE_YaoShu				203
#define	SKILL_TYPE_JianRenFengBao		204
#define	SKILL_TYPE_YouLingLang			205
#define	SKILL_TYPE_JingXiang			206
#define	SKILL_TYPE_FengBaoZhiChui		207
#define 	SKILL_TYPE_TaoTaiZhiRen		208
#define 	SKILL_TYPE_WuGuangZhiDun		209
#define 	SKILL_TYPE_FanJiLuoXuan		210
#define 	SKILL_TYPE_DiCi					211
#define 	SKILL_TYPE_DuSheShouWei		212
#define 	SKILL_TYPE_PaoXiao				213
#define 	SKILL_TYPE_XueXing				214
#define 	SKILL_TYPE_QuSan				215
#define 	SKILL_TYPE_LianSuoShanDian 	216
#define 	SKILL_TYPE_YouBu			 	217
#define 	SKILL_TYPE_ZhiLiaoShouWei	 	218
#define 	SKILL_TYPE_LiuXingHuoYU	 	219
#define 	SKILL_TYPE_ZhanZhengJianTa	220
#define 	SKILL_TYPE_NuHou			 	221
#define 	SKILL_TYPE_ChongFeng		 	222
#define 	SKILL_TYPE_ZuiJiuYunWu		 	223
#define 	SKILL_TYPE_WuDiZhan		 	224
#define 	SKILL_TYPE_ZuZhou			 	225
#define 	SKILL_TYPE_NengLiangLiuShi	 	226
#define 	SKILL_TYPE_QiangLiYiJi		 	227
#define 	SKILL_TYPE_ShuFuZhiJian	 	228
#define 	SKILL_TYPE_WeiKunZhiJian	 	229
#define 	SKILL_TYPE_SheShouTianFu	 	230
#define 	SKILL_TYPE_BenTeng			 	231
#define 	SKILL_TYPE_FenShen			 	232
#define 	SKILL_TYPE_FeiXueZhiMao	 	233
#define 	SKILL_TYPE_XiSheng			 	234
#define 	SKILL_TYPE_ChenMo	 			235
#define	SKILL_TYPE_FuHuo				236

#define	BUFFER_TYPE_BYSKILL			3000			//ͨ��������ȷ��
#define 	BUFFER_TYPE_CALL				3001			//�ٻ�����������һ��
#define 	BUFFER_TYPE_XUANYUN			3002			//ѣ�Σ��ƶ�����ħ������ֹ
#define 	BUFFER_TYPE_MOVESPEED		3003			//Ӱ���ƶ��ٶȣ����ǿ����ƶ�
#define 	BUFFER_TYPE_NOMOVE			3004			//��ֹ�ƶ�
#define 	BUFFER_TYPE_HP				3006			//Ӱ��HP
#define 	BUFFER_TYPE_MP				3007			//Ӱ��MP
#define	BUFFER_TYPE_ARMOR			3008			//Ӱ���������
#define	BUFFER_TYPE_CASTSKILL			3009			//����ʩ��
#define	BUFFER_TYPE_ATTACKPOWER		3010			//Ӱ�칥����
#define	BUFFER_TYPE_MAXHP			3011			//Ӱ�����HPֵ
#define	BUFFER_TYPE_MAXMP			3012			//Ӱ�����MPֵ
#define	BUFFER_TYPE_NOSKILL			3013			//��ֹʹ�ü���


#define   BUFFER_EFFECT_TYPE_ABS		1		//����ֵ
#define   BUFFER_EFFECT_TYPE_RELATIVE	2		//���ֵ



#define 	DIRECT_NORTH			1
#define 	DIRECT_NORTH_EAST		2
#define 	DIRECT_EAST			3
#define 	DIRECT_EAST_SOUTH		4
#define 	DIRECT_SOUTH			5
#define 	DIRECT_SOUTH_WEST		6
#define 	DIRECT_WEST			7
#define 	DIRECT_WEST_NORTH		8

#define 	LENGTH_STRAIGHT			1000
#define 	LENGTH_DIAGONAL			1400
#define 	MOD_STRAIGHT_DIAGONAL	0.7
#define 	MOD_DIAGONAL_STRAIGHT	1.4

#define 	PK_STATE_INIT			1
#define 	PK_STATE_READY		2
#define 	PK_STATE_PKING			3
#define 	PK_STATE_OVER			4

#define   RoleType_Player			1
#define   RoleType_Monster		2
#define   RoleType_NPC			3
#define   RoleType_Pet			4
#define   RoleType_Call			5


#define   ATTACK_EFFECT_MISS	2
#define   ATTACK_EFFECT_DOUBLE	3
#define   ATTACK_EFFECT_NORMAL 1

#define	HPCHANGE_SOURCE_ATK		1
#define	HPCHANGE_SOURCE_SKILL	2


#define	BULLET_TYPE_ATK_CLOSE		1		//�ӵ�����-����������
#define	BULLET_TYPE_ATK_REMOTE	2		//�ӵ�����-Զ��������

#define	ATTACKRESULT_OK_ATTACK			0		//�����ɹ�
#define	ATTACKRESULT_NO_ATTACK			1		//�޷�����
#define	ATTACKRESULT_CD_LIMIT				2		//CDʱ������
#define	ATTACKRESULT_TARGETI_NVALID		3		//Ŀ����Ч
#define	ATTACKRESULT_NO_MOVE			4		//��Ҫ�ƶ�����ֹ�ƶ�
#define	ATTACKRESULT_ASTART_FAIL			5		//Ѱ·ʧ��
#define	ATTACKRESULT_MOVE_FAIL			6		//�ƶ�ʧ��
#define	ATTACKRESULT_MOVEING				7		//�ƶ���

#define	MOVERESULT_OK_MOVE				0		//�ƶ��ɹ�
#define	MOVERESULT_NO_MOVE				1		//��ֹ�ƶ�
#define	MOVERESULT_ERROR					2		//·�����ȴ���

#define	UPDATABUFFER_ADD					1
#define	UPDATABUFFER_DEL					-1




typedef struct _thread_time
{
	struct timeval tv_start;
	unsigned int	 fragment_now;
}thread_time;

typedef struct _PkgHead
{
	unsigned short	usPkgLen;
	unsigned char		ucDirection;
	unsigned char		ucSrvType;
	unsigned char		ucSrvSeq;
	unsigned short	usMsgType;
	unsigned int		uiUniqID;
	unsigned int		uiRoleID;
	void packet(char *sPkgOutput);
	void unpacket(char *sPkgInput);
}PkgHead;

typedef  char DPOINTER[MAP_SIZE_X][MAP_SIZE_Y];

typedef struct _MapDesc
{
	unsigned short		Width;
	unsigned short		Height;
	char*			Map;
}MapDesc;

struct Pos
{
	short int X;
	short int Y;
	void Offset(const Pos &A) //���Aƫ��
	{
		X += A.X;
		Y += A.Y;
	}
	
	bool operator==(const Pos& rhs){
		return (X==rhs.X && Y ==rhs.Y);
	}

};

//����A*�㷨�Ľṹ
typedef struct _PosA
{
	Pos 		Node;
	Pos 		NodeParent;
	int 		F;
	int		G;
}PosA;

class PhysicalAttr
{
public:
	short int 	AttackArea;			//������Χ
	unsigned short int 	AttackCDTime;	//����CD
	short int 	AttackIngTime;		//��������ʱ��,����Ƶ����ָ��ɫ����һ�Σ�����Ҫ�ȴ���ʱ�䣬��λΪ0.1��
	short int	AttackBulletMod;		//�ӵ�ʱ��ϵ��
	short int 	AttackHit;			//������
	short int 	DefenseMiss;		//������
	short int 	DefenseArmor;		//����

	short int 	AttackPowerHign_Base;	//����������
	short int 	AttackPowerLow_Base;	//����������
	short int  AttackPowerAdd;			//�������ӳ�
	short int 	AttackCDTime_Base;		//����CD
	short int 	AttackHit_Base;			//������
	short int 	DefenseMiss_Base;		//������
	short int  AttackCrit_Base;			//��������
	short int 	AttackCrit;				//��ǰ����
	short int 	DefenseArmor_Base;		//����,1000�㻤�׻�ֵ���Լ65%���˺���10:1%; 20:2%;30:3%;100:10%;200:18%;300:26%;400:33%;500:40%

	short int HurtAdd;					//�˺��ӳ�

	short int ExemptCounter;			//���߼�����Ϊ0�����������˺�

	
	unsigned int 	PreAttackTime;		//��һ�ι���ʱ��	
	short int LimitCounter;				//��ֹ���������������Ϊ0�����ʾ���Թ���
	char		PreTimeStatus;

	PhysicalAttr()
	{
		ExemptCounter = 0;
		PreAttackTime = 0;
		LimitCounter = 0;
		HurtAdd = 0;
		AttackPowerAdd = 0;
	}

	
};



class SkillAttr
{
public:
	char		SkillLevel;
	char		DoingTime;
	char		Area;
	short int	MP;
	unsigned short int	SkillCD;
	unsigned int LastUseTime;
	SkillAttr()
	{
		SkillLevel = 0;
		DoingTime = 0;
		Area = 0;
		MP = 0;
		SkillCD = 0;
		LastUseTime = 0;
	}
};

/*
	ÿһ�񻮷�Ϊ1000*1000��������ӣ�
	���Ǽ����û����ƶ�ϵ��SpeedModΪ400��ʱ�䴰��Ϊ0.1�룬
	��˼��˵��ÿ0.1���ƶ�0.4�����ӣ�
	��������λ��Ҫ�����û����ƶ�����
	*/
class MoveAttr
{
public:
	short int			SpeedMod;		//�����ƶ�ϵ��	
	list<Pos> 		MoveLine;		//�ƶ�·������һ������Զ�ǵ�ǰ��
	Pos				CurrentPos;		//��ǰ��
	Pos				CurrentPosMini;
	char				Direct;			//����1:������2:������3:���ϣ�4:���ϣ�5:���ϣ�6:���ϣ�7:������8:����
	short int			SpeedMod_Base;	//�����ƶ�ϵ��
	short int			LimitCounter;		//��ֹ�ƶ���������0��ʾ�����ƶ�

	MoveAttr()
	{
		LimitCounter = 0;
	}
};

class MagicAttr
{
public:
	short int 			MagicArmor;			//ħ������
	short int 			MagicArmor_Base;	//ħ������
	map<short int, SkillAttr>	SkillMap;
	short int 			ExemptBadCounter;			//���߸���ħ��������0������
	short int 			ExemptAllCounter;			//��������ħ��������0������
	short int 			LimitCounter;			//��ֹʹ��ħ�������������Ϊ0���ʾ�����ͷ�ħ��
	MagicAttr()
	{
		LimitCounter = 0;
		ExemptBadCounter = 0;
		ExemptAllCounter = 0;
	}
};



 class Buff
{
public:

	short int 				Type;		//Buffer����
	short int 				Level;		//���ܵȼ�
	short int				ParentSkill;		//�����Buffer�ļ���
	unsigned int			BeginTime;		//��ʼʱ��
	unsigned int			EndTime;		//����ʱ��
	bool 				bCanBreak;		//�Ƿ���Ա����,true:���ԣ�false:������
	bool					bSendClient;		//�Ƿ�֪ͨ�ͻ���
	char					Effecttype;		//Ӱ�����Ե����ͣ�����ֵ�����ֵ����������BUFFER���Ͷ���
	int 					Parametar1;		//���Ӳ���1			
	int 					Parametar2;		//���Ӳ���2
	unsigned short int				RunCounter;		//������
	unsigned short int				RunTimeLimit;	//����ʱ��
	
	Buff( short int type,short int skillID,short int level,unsigned int beginTime,unsigned short int keepTime,bool bBreak,bool bClient )
	{
		Type = type;
		ParentSkill = skillID;
		BeginTime = beginTime;
		EndTime = BeginTime + keepTime;
		bCanBreak = bBreak;
		bSendClient = bClient;
		Parametar1 = 0;
		Parametar2 = 0;
		RunCounter = 0;
		RunTimeLimit = 0;
		Level = level;
		Effecttype = BUFFER_EFFECT_TYPE_ABS;
	}
	
	
};

class RoleOrder
{
public:
	short int 	Type;					// 0:no;1:attack E;2:attack P;3:move E;4:move P;5:hold;6:stop;128-65536:skill
	char		TargetType;				// 1:Pos; 2:RoleID;
	Pos 		TargetPos;
	int 		TargetRoleID;
	unsigned int	OrderTime;
	RoleOrder()
	{
		

	}
};

class HaterInfo
{
public:
	short int Distance;		//�����ϵӰ��
	short int Hp;			//Ѫ��Ӱ��
	short int Attack;		//��������Ӱ��
	short int Hit;			//�ܻ�����Ӱ��
	short int Hurt;		//��λʱ�����˺�Ӱ��
	HaterInfo()
	{
		Distance = 0;
		Hp = 0;
		Attack = 0;
		Hit = 0;
		Hurt = 0;
	}
	void clear()
	{
		Distance = 0;
		Hp = 0;
		Attack = 0;
		Hit = 0;
		Hurt = 0;
	}
	short int max()
	{
		return Distance + Hp + Attack + Hit;
	}
};
//PK״̬
class RoleState
{
public:
	int 				ControlID;		//������ID
	int 				PK_State;		//׼��״̬
	short int			Strength;		//����
	short int 			Agile;			//����
	short int 			Wisdom;		//����
	int				RoleID;			//��ɫID
	char 			RoleFlag;		//��ɫ��־  1 ��ɫ  2 ��  3 npc  4 ����
	int 				RoleType;		//��ɫ����
	char				Opposition;		// 1:����1��2:����2��3:������4:ȫ��ޣ�
	//char				Origin;		// 1:����2:���3:�ٻ���4:����
	char				Live;			// 1:�2:����3:����
	short int			Level;			//�ȼ�

	short int 			HP_MAX;
	short int 			Curr_HP_MAX;
	short int 			MP_MAX;
	short int 			Curr_MP_MAX;
	short int 			HP;
	short int 			MP;
	char				AttackType;		//  1:ָ����2:�Զ�
	RoleOrder 		CurrentOrder;	//�û��������ɫ�Ĳ���
	

	short int			Attack_Skill_WaitingTime;	//��ǰӲֱʱ��,�������������ͼ��ܷ�������
	MoveAttr			MoveAttrb;			//��ɫ��λ������
	PhysicalAttr 		PhysicalAttrib;		//��ɫ����״̬
	MagicAttr			MagicAttrb;

	multimap<short int, Buff> multimap_buff;

	map<int, HaterInfo>	HatreMap;
	void Init()
	{	
		Curr_HP_MAX = HP_MAX;
		Curr_MP_MAX = MP_MAX;
		MoveAttrb.SpeedMod = MoveAttrb.SpeedMod_Base;
		MoveAttrb.LimitCounter = 0;
		PhysicalAttrib.AttackCDTime = PhysicalAttrib.AttackCDTime_Base;
		PhysicalAttrib.AttackHit = PhysicalAttrib.AttackHit_Base;
		PhysicalAttrib.DefenseArmor  =PhysicalAttrib.DefenseArmor_Base;
		PhysicalAttrib.DefenseMiss = PhysicalAttrib.DefenseMiss_Base;
		PhysicalAttrib.AttackCrit = PhysicalAttrib.AttackCrit_Base;
		PhysicalAttrib.LimitCounter = 0;
		PhysicalAttrib.ExemptCounter = 0;
		MagicAttrb.MagicArmor = MagicAttrb.MagicArmor_Base;
		MagicAttrb.LimitCounter = 0;
		PhysicalAttrib.HurtAdd = 0;
		MagicAttrb.ExemptBadCounter = 0;
		MagicAttrb.ExemptAllCounter = 0;
		Attack_Skill_WaitingTime = 0;
		MoveAttrb.CurrentPosMini.X = 500;
		MoveAttrb.CurrentPosMini.Y = 500;
		PhysicalAttrib.PreAttackTime = 0;
		PhysicalAttrib.PreTimeStatus = 0;
		CurrentOrder.Type = ORDER_TYPE_NO;
		CurrentOrder.OrderTime = 0;
		Live = ROLE_STATUS_LIVE;
	}
	void Dead()
	{
		Live = ROLE_STATUS_DEAD;
		HP = 0;
		CurrentOrder.Type = ORDER_TYPE_NO;
		multimap_buff.clear();
		HatreMap.clear();
		MoveAttrb.MoveLine.clear();
	}
};





typedef struct _Bullet
{
	RoleState*		Role_Attack;			//������
	RoleState*		Role_Passive;		//�ܻ���
	unsigned short int	Start_Time;			//��ʼʱ��
	Pos				CurrentPos;			//��ǰ��
	char				Speed;				// �ӵ��ٶ�
	short int			BulletType;			// �ӵ����ͣ�1����������2Զ�̹���������Ϊ����
	char				Level;				//���ܵȼ�
}Bullet;

typedef struct _Skill_Order
{
	RoleState*		role;			//��ɫ
	unsigned short int	Start_Time;
	RoleOrder			order;
	char				SkillLevel;
}Skill_Order;


//����ֵ>0��ʾ·����ת�۵�����������ֵ��ʾʧ��
int AStar(Pos StartPos, Pos EndPos, list<Pos> *MoveLine, char (*pk_map)[MAP_SIZE_Y]);

double Pnt2SegmentDist(const Pos & A,const Pos & B,const Pos & C);
double Dist(const Pos & A, const Pos & B,bool bSqrt = true);
char getDirect( const Pos & startPos,const Pos & endPos );
short int getBHitTarget( const short int attackRoleHit,const short int targetRoleMiss );

#endif

