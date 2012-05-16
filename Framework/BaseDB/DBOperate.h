//DB�� select ��update ����
//�������߳���

#ifndef DBOPERATE_h
#define DBOPERATE_h

#include "mysql.h"
#include "OurDef.h"

class DBOperate
{
public:
	DBOperate();
	~DBOperate();


	void FreeResult();


	void SetHandle( MYSQL* input);
	


	int QuerySQL( const char * ptrSql );


	int ExceSQL( const char * ptrSql );
	

	int Operate( const char * ptrSql, bool hasResults = false );


	UInt32 RowNum();


	UInt32 FieldNum();


	Int64 AffectRows();
	

	UInt32 LastInsertID();


	int GetIntField(int nField, int nNullValue=0);
	const char* GetStringField(int nField, const char* szNullValue="");
  double GetFloatField(int nField, double fNullValue=0.0);


	bool HasRowData();


	void NextRow();

private:
	//msyql ���Ӿ��
	MYSQL* _mysql;
	
	//�����
	MYSQL_RES*  _results;

	//������¼
	MYSQL_ROW  _record;

	//��¼��
	UInt32 _rowNum;

	//�ֶ���
	UInt32 _fieldNum;

	//Ӱ���¼��
	ULONG _affectRows;
	
};


#endif


