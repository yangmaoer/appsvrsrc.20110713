// �ʺ���
#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "OurDef.h"



class Account
{

public:
	Account();
	Account(UInt32 li,UInt32 li2,Byte li3,UInt32 li4);
	~Account();


public:
	
	UInt32 AccountLastloginTime();
	UInt32 AccountToploginTime();
	UInt32 LoginTime();
	Byte IsAdult();
	
	
	//��������
	void IsAdult(Byte input);

	
private:
	
	UInt32 _accountlastloginTime;  //�ʺ��ϴε���ʱ��
	UInt32 _accounttopTime;        //�ʺŵ�������ʱ�䣬�����Ե����3Сʱ
	Byte _isAdult;//�Ƿ��ǳ���
	UInt32 _logintime;

	
};


#endif

