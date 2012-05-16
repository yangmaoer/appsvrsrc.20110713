#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <list>
#include <map>
#include "PK_PE_base.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <cstring>
using namespace std;

void PkgHead::packet(char *sPkgOutput)
{
	int nOutLen = 0;

	memcpy(sPkgOutput + nOutLen, &usPkgLen, 2);
	nOutLen += 2;

	memcpy(sPkgOutput + nOutLen, &ucDirection, 1);
	nOutLen += 1;

	memcpy(sPkgOutput + nOutLen, &ucSrvType, 1);
	nOutLen += 1;

	memcpy(sPkgOutput + nOutLen, &ucSrvSeq, 1);
	nOutLen += 1;

	memcpy(sPkgOutput + nOutLen, &usMsgType, 2);
	nOutLen += 2;

	memcpy(sPkgOutput + nOutLen, &uiUniqID, 4);
	nOutLen += 4;

	memcpy(sPkgOutput + nOutLen, &uiRoleID, 4);
	nOutLen += 4;
}

void PkgHead::unpacket(char *sPkgInput)
{
	int nInLen = 0;
	
	memcpy(&usPkgLen, sPkgInput+nInLen, 2);
	nInLen += 2;

	memcpy(&ucDirection, sPkgInput+nInLen, 1);
	nInLen += 1;

	memcpy(&ucSrvType, sPkgInput+nInLen, 1);
	nInLen += 1;

	memcpy(&ucSrvSeq, sPkgInput+nInLen, 1);
	nInLen += 1;

	memcpy(&usMsgType, sPkgInput+nInLen, 2);
	nInLen += 2;

	memcpy(&uiUniqID, sPkgInput+nInLen, 4);
	nInLen += 4;

	memcpy(&uiRoleID, sPkgInput+nInLen, 4);
	nInLen += 4;
}


char getDirect( const Pos & startPos,const Pos & endPos )
{
	if(startPos.X> endPos.X )
	{
		if(startPos.Y > endPos.Y)
			return DIRECT_EAST_SOUTH;
		else if(startPos.Y < endPos.Y)
			return DIRECT_NORTH_EAST;
		else
			return DIRECT_EAST;
	}
	else if(startPos.X < endPos.X)
	{
		if(startPos.Y > endPos.Y)
			return DIRECT_SOUTH_WEST;
		else if(startPos.Y < endPos.Y)
			return DIRECT_WEST_NORTH;
		else
			return DIRECT_WEST;
	}
	else
	{
		if(startPos.Y > endPos.Y)
			return DIRECT_SOUTH;
		else if(startPos.Y < endPos.Y)
			return DIRECT_NORTH;
		else
			return 0;
	}	
}
short int getBHitTarget(  const short int attackRoleHit,const short int targetRoleMiss )
{
	return ( short int )( attackRoleHit * ( 1 - targetRoleMiss / 100 ) );
}
double Dist(const Pos & A, const Pos & B,bool bSqrt)
{
	double x = A.X - B.X;
	double y = A.Y- B.Y;
	if( bSqrt )
		return sqrt(x*x+y*y);
	else
		return x*x+y*y;
}



double Pnt2SegmentDist(const Pos & A,const Pos & B,const Pos & C)
{
	Pos AB;
	AB.X = B.X - A.X; //����AB
	AB.Y = B.Y - A.Y;
	Pos AC;
	AC.X = C.X - A.X;
	AC.Y = C.Y - A.Y;
	double r = AB.X * AC.X + AB.Y * AC.Y;//AB��AC�ĵ�˻�
	r /= Dist(A,B,false);//AC��AB�ϵ�ͶӰ����AB������Dist(),������

	//��C��ͶӰ��AB��
	if(r < 0  || r > 1 )
		return 10000;

	//��C��ͶӰ��AB֮��
	Pos D = AB;
	D.X*= r;
	D.Y*= r;//��ΪAB�����������Կ������������õ�AC��AB�ϵ�ͶӰ������
	D.Offset(A);//��D�ľ�������
	return Dist(C, D);
}
