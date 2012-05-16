#include <stdlib.h>
#include <string.h>
#include "mysql.h"
#include "DBOperate.h"
#include "Log.h"

DBOperate::DBOperate()
:_mysql(NULL)
,_results(NULL)
,_rowNum(0)
,_fieldNum(0)
,_affectRows(0)
{
}

DBOperate::~DBOperate()
{
	FreeResult();
}

//���þ��
void DBOperate::SetHandle(MYSQL * input)
{
	_mysql = input ;
}

//�ͷ����ݿ���Դ
//	ͬһ DBOperate ��������� select
//	��һ��select ֮ǰӦ��ʾ���ø� FreeResult()�����ͷŽ����
void DBOperate::FreeResult()
{
	if( _results)
	{
		mysql_free_result(_results);
		_results = NULL;
	}
}

//���ݿ� select
//param ptrSql sql���
//return  0 �ɹ�   1 ���ݿ��¼������  ��������ֵ ʧ��
int DBOperate::QuerySQL( const char * ptrSql )
{
	return Operate( ptrSql, true );
}

//���ݿ� updaet,insert
//param ptrSql sql���
//return  0 �ɹ�  ���� ʧ�� 
int DBOperate::ExceSQL( const char * ptrSql )
{
	return Operate( ptrSql, false );
}


//���ݿ����
//param ptrSql sql���
//param hasResults		�Ƿ��н����
//return  0 �ɹ�   1 ���ݿ��¼������  ��������ֵ ʧ��

int DBOperate::Operate( const char * ptrSql, bool hasResults )
{
	//�ͷ���ǰִ�еĽ����
	FreeResult();

	if( NULL == _mysql )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"invalid mysql connection!!!! DBOperate::_mysql is NULL,ptrSql[%s] ", ptrSql);
		return -1;
	}

	if( mysql_real_query( _mysql, ptrSql, strlen(ptrSql) ) )
		return -1;

	if( hasResults )
	{//select ��䴦��

		//�洢�����
		_results = mysql_store_result(_mysql);
		if( NULL == _results )
			return -1;

		//��¼��
		_rowNum = mysql_num_rows(_results);

		//��һ�м�¼
		_record = mysql_fetch_row(_results);

		//����
		_fieldNum = mysql_num_fields(_results);

		//��¼�����ڣ����� 1
		if( _rowNum == 0 )
			return 1;
	}
	else
	{//update,insert ��䴦��
		_affectRows = mysql_affected_rows( _mysql );
		if( -1 == (LONG)_affectRows )
			return -1;
	}


	return 0;
}


//��ȡ��¼��
UInt32 DBOperate::RowNum()
{
	return _rowNum;
}

//��ȡ�ֶ���
UInt32 DBOperate::FieldNum()
{
	return _fieldNum;
}

//��ȡӰ���¼��
Int64 DBOperate::AffectRows()
{
	return _affectRows;
}

//mysql�� last insert id, ���� auto increasement��
UInt32 DBOperate::LastInsertID()
{
	return mysql_insert_id(_mysql);
}




//0...n-1��
int DBOperate::GetIntField(int nField, int nNullValue)
{
	if ( NULL == _results ||
			 (nField + 1 > _fieldNum ) ||
			  NULL == _record ||_record[nField]==NULL)
  	return nNullValue;

	return atoi(_record[nField]);
}


const char* DBOperate::GetStringField(int nField, const char* szNullValue)
{
	if ( NULL == _results ||
			 (nField + 1 > _fieldNum ) ||
			  NULL == _record )
		return szNullValue;

	return _record[nField];
}


double DBOperate::GetFloatField(int nField, double fNullValue)
{
	if ( NULL == _results ||
		 (nField + 1 > _fieldNum ) ||
		  NULL == _record )
	return fNullValue;

	return atof(_record[nField]);
}


//�Ƿ��������ݼ�¼ 
//return true ��  false ��
bool DBOperate::HasRowData()
{
	if( NULL == _record )
		return false;

	return true;
}

//��һ��
void DBOperate::NextRow()
{
	if( NULL == _results )
		return;
		
	_record = mysql_fetch_row(_results);
}


