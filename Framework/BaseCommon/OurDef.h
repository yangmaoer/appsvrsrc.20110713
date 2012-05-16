//���������ļ�ͷ
#ifndef OURDEF_H
#define OURDEF_H

/************************************
 *
 *       ����ר��
 *
 ************************************/
//epoll ���
#define MAXEPOLLFDNUM					5000									//epoll ��ע�����fd��
#define	MAXEVENTS							100										//ÿ��epoll�����ܴ�����¼���
#define MAXSOCKBUFF						65535									//socket ���ݻ���

//���������
#define MAX_SVRTYPE						255										//������������
#define MAX_SVRSEQ						255										//�����������

//
#define GW_CONNCLOSe_INTERNAL	1											//�������ӹر��߳� ��ʱ�ر����ӵ�ʱ����

//������ tcp ����
enum
{
	INSIDE_TCP_STREAM,					//���� TCP ����
	OUTSIDE_TCP_STREAM					//���� TCP ����
};



//------------���嶨��-------��ʼ--------------------------------------------
#pragma pack(1)
//S->G ��������������
struct PKGBODY_SG_SVRCONNECT_REQ
{
	unsigned char		ucSrvType;									//Ӧ�÷���������
	unsigned char		ucSrvSeq;										//Ӧ�÷��������
};

//C->S ��ɫ��¼ ��������
struct PKGBODY_CS_ROLELOGIN_REQ_R
{
	unsigned int		uiRoleID;											//��ɫID
	unsigned char		ucPasswd[40];								//��ɫ����
	
};

//C->S ��ɫ��¼ ��������
struct PKGBODY_CS_ROLELOGIN_REQ_L
{
	unsigned int		uiRoleID;											//��ɫID
	unsigned char		ucPasswd[40];								//��ɫ����
	long long				llClientId;									//�ͻ���id = ip+port+fd
};

//C->S ��ɫ��¼ ��������  Ӧ��
struct PKGBODY_CS_ROLELOGIN_RESP_L
{
	unsigned int		uiRetCode;									//����ֵ
	unsigned int		uiRoleID;											//��ɫID
	long long				llClientId;									//�ͻ���id = ip+port+fd
};
#pragma pack()

//------------���嶨��-------����--------------------------------------------


/************************************
 *
 *       ͨ����������
 *
 ************************************/
 //��������
typedef bool								Bool;
typedef char								SByte;
typedef unsigned char				Byte;
typedef short								Int16;
typedef unsigned short			UInt16;
typedef int									Int32;
typedef unsigned int				UInt32;
typedef long long						Int64;
typedef unsigned long long	UInt64;
typedef float								Single;
typedef double							Double;

typedef long					 			LONG;
typedef unsigned long 			ULONG;

//����������
#define SVRTYPE_MAIN			1									//main server
#define SVRTYPE_PK				2									//PK server

//���������
#define SVRSEQ_MAIN							1				//��ҵ����������

//��Ϣ���� msgtype
#define		MSGTYPE_GW_SVRCONNECT							101				//������Ϣ:������������
#define		MSGTYPE_GW_CLOSECLIENT						102				//������Ϣ:�ر�ָ���ͻ���

#define		MSGTYPE_CS_ROLELOGIN							101				//C_S��Ϣ:��ɫ��¼ login
#define		MSGTYPE_CS_ROLELOGOUT							104				//C_S��Ϣ:��ɫ�ǳ� logout


//������
#define	DIRECT_C_S_REQ		1
#define	DIRECT_C_S_RESP		2
#define	DIRECT_S_C_REQ		3
#define	DIRECT_S_C_RESP		4
#define	DIRECT_S_S_REQ		5
#define	DIRECT_S_S_RESP		6 
 

/************************************
 *
 *       Ӧ�÷�����
 *
 ************************************/

//ȫ�ֳ���
#define PACKET_HEADER_LENGTH 				15	//��ͷ����
#define MAX_EQUIPTYPE		8					//���װ������
#define MAX_MODULE			99				//ÿ���������У����Ӧ��ģ����
#define MAX_SESSION 		300				//S_S ����˲��������Ӹ�������

#define RNUM_PER_SEND 20					//ÿ�η��ص��û����������ڵ�ͼ�������û��ƶ���Ϣ��ѯ
#define MAXMAPNUM			255					//����ͼ����
#define MAXROLEDIRECT	8						//��ɫ�ƶ��������

#define PKSCREEN_XLENGTH	50			//pk��Ļx�᳤��
#define PKSCREEN_YLENGTH	29			//pk��Ļy�᳤��

#define POSACCURACY				7				//���꾫�ȣ���λ���� ����˵���ͻ��ˡ���������ǰ�������Χ
#define ROLE_MOVE_SPEED		541			//��ɫ�����ƶ��ٶ� , ��λ: 541 ΢�͸�/0.1��

#define LUASCRIPTRETURN_CANACCEPT	99		//lua�ű� return���ķ���ֵ, 99��ʾ����ɽ�



//��������
#define TASKTYPE_MAIN			1									//��������
#define TASKTYPE_BRANCH		2									//֧������
#define TASKTYPE_DAILY		3									//�ճ�����

//�������״̬
#define	TASKFINISHSTATUS_NOTFINISHED		0			//����δ���
#define	TASKFINISHSTATUS_FINISHED				1			//���������
#define	TASKFINISHSTATUS_DELIVERED			2			//�����ѽ���

//pk IDת��
#define 	CREATUREFLAG_ROLE     100000000			//�����־_��ɫ,��pk����ת����
#define   CREATUREFLAG_PET			400000000			//�����־_����,��pk����ת����


//Ӧ�ò�������

/************************************
 *
 *       ������
 *
 ************************************/
//ϵͳ������
#define ERR_SUCCESS										0			//�ɹ�              
#define ERR_SYSTEM_TIMEOUT						1			//��Ϣ��ʱ          
#define ERR_SYSTEM_LARGEMSG						2			//��Ϣ������������
#define ERR_SYSTEM_SVRNOTEXISTS				3			//Ӧ�÷��񲻴���    
#define ERR_SYSTEM_SVRNOTSTART				4			//Ӧ�÷���δ����    
#define ERR_SYSTEM_DBERROR 						5			//���ݿ����ʧ��    
#define ERR_SYSTEM_DBNORECORD					6			//���ݿ��¼������  
#define ERR_SYSTEM_SERERROR						7			//���л�����        
#define ERR_SYSTEM_SVRLOGINED					8			//�������ѵ�¼      
#define ERR_SYSTEM_SVRACCESS					9			//Ӧ�÷�����ʹ���(�޷��ɹ�������Ϣ)
#define ERR_SYSTEM_ROLENOTLOGIN				10		//��ɫδ��¼
#define ERR_SYSTEM_DATANOTEXISTS			11		//���ݲ�����
#define ERR_SYSTEM_PARAM							12		//��������
#define ERR_SYSTEM_DATAEXISTS					13		//�����Ѵ���


//Ӧ�ô�����
#define ERR_APP_ROLENOTEXISTS					1001	//��ɫ������
#define ERR_APP_ERRPASSWD							1002	//�������
#define ERR_APP_OP										1003	//�������
#define ERR_APP_DATA									1004	//ҵ�����ݴ���
#define ERR_APP_ROLENOTLOGIN					1005	//��ɫδ��½
#define ERR_APP_PLAYTIMEACCESS				1006	//��ɫ���ڷ�����������,�������¼
#define ERR_APP_ALREADYINMAP					1007	//��ɫ�Ѿ������ͼ

//��ɫ����
#define ERR_ROLE_LEVELNOTENOUGH				2001	//���𲻹�
#define ERR_ROLE_NOMONEY					2002	//��Ǯ����


#endif








