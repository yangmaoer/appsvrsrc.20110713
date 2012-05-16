#include "Pet.h"
#include <time.h>
#include "ConnectionPool.h"
#include "DBOperate.h"
#include "Log.h"

Pet::Pet()
{
	Flag(4);					// �������� 4����	
}
Pet::~Pet()
{
}

int Pet::InitPetCache(UInt32 petID, ConnectionPool * cp )
{
	ID(petID);
	_cp = cp;

	//��DB�������Ľ�ɫ����
	int iRet = DB2Cache();
	if(iRet!=0)
	return iRet;
	iRet=DB2CacheBonus();
	return iRet;	
}


int Pet::DB2Cache(UInt32 petID, ConnectionPool * cp)
{
  ID(petID);
	_cp = cp;
	
	int iRet = DB2Cache();
	if(iRet != 0)
	  return iRet;
	
}

int Pet::DB2CacheBonus(UInt32 petID, ConnectionPool * cp)
{
  ID(petID);
	_cp = cp;

	int iRet = DB2CacheBonus();
	if(iRet != 0)
	  return iRet;
}

int Pet::DB2Cache()
{
	DEBUG_PRINTF( "InitCache ---------------->" );
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
	//��ȡ��ɫ��Ϣ
	sprintf( szSql, "select	 PetID ,PetType,PetKind ,PetName   ,Level      ,Exp ,MaxExp,   \
							   AddPoint   ,Strength \
							   ,Intelligence   ,Agility        ,MoveSpeed      ,HP             ,MP          \
							   ,MaxHP          ,MaxMP          ,HPRegen        ,MPRegen        ,AttackPowerHigh \
							   ,AttackPowerLow    ,AttackScope       ,AttackSpeed       ,BulletSpeed       ,Defence        \
							   ,MDefence          ,CritRate          ,HitRate           ,DodgeRate             \
							from Pet \
							where PetID = %d;", ID() );
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
		return -1;
	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}

	while(dbo.HasRowData())
	{
		//_ID = dbo.GetIntField(0);
		_petType=dbo.GetIntField(1);
		_petkind=dbo.GetIntField(2) ;
		Name( dbo.GetStringField(3) );
		Level( dbo.GetIntField(4) );
		Exp( dbo.GetIntField(5) );
		MaxExp(dbo.GetIntField(6));
		AddPoint( dbo.GetIntField(7) );
		Strength( dbo.GetIntField(8) );
		Intelligence( dbo.GetIntField(9) );
		Agility( dbo.GetIntField(10) );
		MoveSpeed( dbo.GetIntField(11) );
		Hp( dbo.GetIntField(12) );
		Mp( dbo.GetIntField(13) );
		MaxHp( dbo.GetIntField(14) );
		MaxMp( dbo.GetIntField(15) );
		HpRegen( dbo.GetIntField(16) );
		MpRegen( dbo.GetIntField(17) );
		AttackPowerHigh( dbo.GetIntField(18) );
		AttackPowerLow( dbo.GetIntField(19) );
		AttackScope( dbo.GetIntField(20) );
		AttackSpeed( dbo.GetIntField(21) );
		BulletSpeed( dbo.GetIntField(22) );
		Defence( dbo.GetIntField(23) );
		MDefence( dbo.GetIntField(24) );
		CritRate( dbo.GetIntField(25) );
		HitRate( dbo.GetIntField(26) );
		DodgeRate( dbo.GetIntField(27) );
		
		dbo.NextRow();
	}
	return 0;
}
int Pet::DB2CacheBonus()
{
	char szSql[1024];
	Connection conNew;
	DBOperate dboNew;
	int iRet = 0;

	//��ȡDB����
	conNew = _cp->GetConnection();
	dboNew.SetHandle(conNew.GetHandle());
	sprintf( szSql, "select	 PetID     ,Strength   ,Intelligence   ,Agility      ,MovSpeed ,MaxHP,MaxMP     \
						   ,HPRegen      ,MPRegen    ,AttackPowerHigh      ,AttackPowerLow   ,AttackSpeed \
						   ,Defence   ,MDefence        ,CritRate      ,HitRate             ,DodgeRae          \
						from PetBonus \
						where PetID = %d ", ID() );
	iRet=dboNew.QuerySQL(szSql);
	if( 1 == iRet )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
		return -1;
	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(conNew.GetHandle()), szSql);
		return -1;
 	}
	while(dboNew.HasRowData())
	{
		StrengthBonus(dboNew.GetIntField(1));
		IntelligenceBonus(dboNew.GetIntField(2));
		AgilityBonus(dboNew.GetIntField(3));
		MovSpeedBonus(dboNew.GetIntField(4));
		MaxHpBonus(dboNew.GetIntField(5));
		MaxMpBonus(dboNew.GetIntField(6));
		HpRegenBonus(dboNew.GetIntField(7));
		MpRegenBonus(dboNew.GetIntField(8));
		AttackPowerHighBonus(dboNew.GetIntField(9));
		AttackPowerLowBonus(dboNew.GetIntField(10));
		AttackSpeedBonus(dboNew.GetIntField(11));
		DefenceBonus(dboNew.GetIntField(12));
		MDefenceBonus(dboNew.GetIntField(13));
		CritRateBonus(dboNew.GetIntField(14));
		HitRateBonus(dboNew.GetIntField(15));
		DodgeRateBonus(dboNew.GetIntField(16));
			
		dboNew.NextRow();
	}
	return 0;
}
int Pet::Cache2DB()
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	UInt32 critRate = 0;
	UInt32 dodgeRate = 0;
	UInt32 moveSpeed = 300;
	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//��ɫ��
	sprintf(szSql, "update Pet \
		set Level            = %d ,Exp              = %d,MaxExp=%d \
		  ,AddPoint         = %d ,Strength         = %d \
		 ,Intelligence     = %d ,Agility          = %d ,MoveSpeed        = %d ,HP               = %d ,MP               = %d \
		 ,MaxHP            = %d ,MaxMP            = %d ,HPRegen          = %d ,MPRegen          = %d ,AttackPowerHigh  = %d \
		 ,AttackPowerLow   = %d ,AttackScope      = %d ,AttackSpeed      = %d ,BulletSpeed      = %d ,Defence          = %d \
		 ,MDefence         = %d ,CritRate         = %d ,HitRate          = %d ,DodgeRate        = %d  \
		where PetID = %d ",
		Level(),
		Exp(),
		MaxExp(),
		AddPoint(),
		Strength(),
		Intelligence(),
		Agility(),
		MoveSpeed(),
		Hp(),
		Mp(),
		MaxHp(),
		MaxMp(),
		HpRegen(),
		MpRegen(),
		AttackPowerHigh(),
		AttackPowerLow(),
		AttackScope(),
		AttackSpeed(),
		BulletSpeed(),
		Defence(),
		MDefence(),
		CritRate(),
		HitRate(),
		DodgeRate(),
		ID() );

	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}

	//��ɫ�������ĵ�ͼ������
 	return Cache2Bonus();
}

int Pet::Cache2Bonus()
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
	sprintf(szSql, "update PetBonus set PetID  =  %d,Strength  =  %d,  Intelligence  =  %d,  Agility = %d, \
					MovSpeed = %d , MaxHP =  %d  ,MaxMP  =%d ,HPRegen=  %d,MPRegen =%d , \
					AttackPowerHigh = %d, 	AttackPowerLow =%d ,AttackSpeed =  %d ,Defence=  %d, \
					MDefence = %d ,CritRate = %d ,HitRate=%d ,DodgeRae=%d where PetID=%d;",
			ID(),
			StrengthBonus(),
			IntelligenceBonus(),
			AgilityBonus(),
			MovSpeedBonus(),
			MaxHpBonus(),
			MaxMpBonus(),
			HpRegenBonus(),
			MpRegenBonus(),
			AttackPowerHighBonus(),
			AttackPowerLowBonus(),
			AttackSpeedBonus(),
			DefenceBonus(),
			MDefenceBonus(),
			CritRateBonus(),
			HitRateBonus(),
			DodgeRateBonus(),
			ID() );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}
 	return 0;
}


//��������
void Pet::PetAddStrength(Int32 input)
{
		AddStrength(input);
		AddMaxHp(input*5);//����5��HP
		AddAttackPowerHigh(2*input);
		AddAttackPowerLow(2*input);
	
}
void Pet::PetAddAgility(Int32 input)
{
		AddAgility(input);
		AddDefence(input*5);//һ����������5�㻤��
		AddAttackPowerHigh(2*input);
		AddAttackPowerLow(2*input);

}

void Pet::PetAddAgilityBonus(Int32 input)
{
	AddAgilityBonus(input);
	AddDefenceBonus(input*5);//һ����������5�㻤��
	AddAttackPowerHighBonus(2*input);
	AddAttackPowerLowBonus(2*input);

}

//��������
void Pet::PetAddIntelligence(Int32 input)
{
	AddIntelligence(input);
	AddMaxMp(input*5);//��������5��MP
	AddAttackPowerHigh(2*input);
	AddAttackPowerLow(2*input);
}

void Pet::PetAddIntelligenceBonus(Int32 input)
{
	AddIntelligenceBonus(input);
	AddMaxMpBonus(input*5);//��������5��MP
	AddAttackPowerHighBonus(2*input);
	AddAttackPowerLowBonus(2*input);
}

void Pet::PetAddStrengthBonus(Int32 input)
{
	AddStrengthBonus(input);
	AddMaxHpBonus(input*5);
	AddAttackPowerHighBonus(2*input);
	AddAttackPowerLowBonus(2*input);
}

// �����������Եı仯
void Pet::Pet_ADDLev()
{

	Connection con;
	DBOperate dbo;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
	int iRet;
	char szSql[1024];
	sprintf(szSql, "select PetKind from Pet where PetID=%d;", ID());
	iRet = dbo.QuerySQL(szSql);
	if (iRet !=0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found " );
	}

	UInt32 id = dbo.GetIntField(0);
	//���������Լ����Ա仯
	AddLevel(1);
	PetAddStrength(Strength_ADDD(id));
	PetAddIntelligence(Intelligence_ADDD(id));
	PetAddAgility(Agility_ADDD(id));
	
}


//�ж��Ƿ����������� 1 ������ 0 û�������� -1 ʧ��
Int16 Pet::IfTheExpToMax()
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
	UInt32 exp = Exp()/3;
	UInt32 maxExp =MaxExp()/3;
	if(exp>maxExp)
	{
		//�ﵽ��������������ȥ���ֵ
		//Max����ֵ�趨Ϊ��һ�������ֵ
		sprintf(szSql, "select MaxExp from LevelDesc where Level=%d",Level()+1);
		iRet=dbo.QuerySQL(szSql);
		if( 1 == iRet )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
			return -1;
		}
		if( iRet < 0 )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			return -1;
 		}
 		AddAddPoint(AddAddPoint_Pet);
 		Exp(exp-maxExp);
 		while(dbo.HasRowData())
		{
			
			MaxExp(dbo.GetIntField(0));
			//��¼����һ����¼
			dbo.NextRow();
		}
		
		Pet_ADDLev();   //���������Լ����Ա仯
		return 1;
	}
	else
	{
		return 0;
	}
}

//����1����������-1�쳣������0û������
Int32 Pet::PetExpAdd(Int32 input)
{
	AddExp(input);
	return IfTheExpToMax();
}


