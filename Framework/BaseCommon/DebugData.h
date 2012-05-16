#ifndef DEBUGDATA_H
#define	DEBUGDATA_H

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

class DebugData{
	public:
		
		/*******************************************************	
		Ŀ�ģ���16���Ƹ�ʽ��ʾĳ���ڴ������
		������
			void *	input					�ڴ�ε���ʼ��ַ
			int	iSize					��ʾ����
			int 	iShowTail 				�Ƿ���ʾĩβ���ı��ַ� 0����ʾ  1��ʾ
			const char * PROGRAMNAME		Դ������������ʱ�� __FILE__ ����
			int		LINE					�ڵڼ��е���ShowHex�� ����ʱ�� __LINE__ ����
		����ֵ��
			��
		˵����
			�ú�������ʾ��ʽ����Ultra Edit��ʾ16�������ݸ�ʽ
			��ÿһ���ֽ���һ����λ��16����������ʾ:
			00000000h: 00 00 00 00 00 00 00 00 00 00 04 4d 00 00 00 00 ; M
			00000001h: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ; 
			00000002h: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ; 
			00000003h: 00 00 00 00 00 00 00 00 00 00 00 00 ; 
			
			����ShowHex���������ڵ��Գ����Ŀ�ģ�����б�Ҫ������ShowHex�ĺ������ڵ�
			Դ�������͵��õ�������ʾ����	
		�����÷���
			int iTmp;
			ShowHex( &iTmp, sizeof(int), 0, __FILE__, __LINE__ );
		*********************************************************/
		static void ShowHex( void * input, int iSize ,int iShowTail, const char * PROGRAMNAME, int LINE)
		{
			
			const int LINECHAR = 16;		/*ÿ����ʾ�ֽ�����Ĭ��Ϊ16*/
			int		iLine = 0;				/*����*/
			int		iCount = 0;				/*ÿ����ʾ�ۼ�*/
			char	cHead[1024];			/*��ͷ*/
			char	cHex[1024];				/*16�����ַ�*/
			char	cTail[1024];			/*�ı��ַ�*/
			char	cTmp[8];
			int		i = 0, iExit = 0;
			
			unsigned char * ptrChar = ( unsigned char * )input;
			
			printf( "\n File[%s] Line[%d] ShowHex, Bytes[%d]\n", PROGRAMNAME, LINE, iSize );
			printf( "-------------------------------\n" );
			if( iSize == 0 )
				return;
			
			while(1)
			{
				/* �˳���� iExitΪ1 ����LINECHAR * iLine ���õ���iSize ��ʱ���˳�ѭ��*/
				if( iExit == 1 || ( LINECHAR * iLine >= iSize) )
					break;
					
				iCount = 0;		
				strcpy( cHex, ":" );
				strcpy( cTail, " ; " );
				
				for( i = 0; i < LINECHAR; i++ )
				{
					/*�ж��Ƿ񳬳���ӡ��Χ*/
					if( ( ++iCount + LINECHAR * iLine ) > iSize)
					{
						iExit = 1;
						break;
					}
					
					/*���ַ���ʾ*/
					sprintf( cTmp, " %02x", *ptrChar );
					strcat( cHex, cTmp );			
					sprintf( cTmp, "%c", *ptrChar );
					strcat( cTail, cTmp );			
					ptrChar ++;			
				}
				
				sprintf( cHead,"%08xh",iLine );
				strcat( cHead, cHex );
				
				if( iShowTail )
					strcat( cHead, cTail);		
				printf( "%s-\n", cHead );
					
				iLine ++;
			};
			
			printf("--\n--\n--\n");
		}
		
		/*
		*	˵�����������õĴ�ӡ����
		*
		*	�÷���
		*		debug_printf( __FILE__, __LINE__, "int[%d] string [%s], iTmp, szTmp );
		*/
		static void Printf( const char * filename, int line, const char * fmt, ... )
		{
			char szBuff[1024];
			int nSize = 0;
			memset( szBuff, 0 ,sizeof(szBuff) );
			
			sprintf( szBuff, "\n[%s] Line.[%d]-->", filename, line );
			nSize = strlen( szBuff );
			
			va_list ap;
			va_start( ap, fmt );
			
			
			vsnprintf( szBuff+nSize, sizeof( szBuff )-nSize-1, fmt, ap ); 
			strcat( szBuff, "\n" );
			
			fprintf( stderr, "%s", szBuff );
			
			va_end(ap);	
			return;
		}

};


#ifdef DEBUG
//=========================if def DEBUG=================================================

//--------debug_showhex--------
#define DEBUG_SHOWHEX(input, size, showtail, file, line)	DebugData::ShowHex( input, size, showtail, file, line)
	
//--------debug_printf--------
#define DEBUG_PRINTF(args)  DebugData::Printf( __FILE__, __LINE__, args )

#define DEBUG_PRINTF1(args, args1)  \
	DebugData::Printf( __FILE__, __LINE__, args, args1 )
	
#define DEBUG_PRINTF2(args, args1, arg2 ) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2 )
	
#define DEBUG_PRINTF3(args, args1, arg2, arg3) \
	DebugData::Printf( __FILE__, __LINE__, args, args1 , arg2, arg3)
	
#define DEBUG_PRINTF4(args, args1, arg2, arg3, arg4) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2, arg3, arg4 )
	
#define DEBUG_PRINTF5(args, args1, arg2, arg3, arg4, arg5) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2, arg3, arg4, arg5 )
	
#define DEBUG_PRINTF6(args, args1, arg2, arg3, arg4, arg5, arg6) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2, arg3, arg4, arg5, arg6 )

#define DEBUG_PRINTF7(args, args1, arg2, arg3, arg4, arg5, arg6, arg7) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2, arg3, arg4, arg5, arg6, arg7 )
	
#define DEBUG_PRINTF8(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 )
	
#define DEBUG_PRINTF9(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)

#define DEBUG_PRINTF10(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 )
	
#define DEBUG_PRINTF11(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11 )

#define DEBUG_PRINTF12(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12 )

#define DEBUG_PRINTF13(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13 )

#define DEBUG_PRINTF14(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14 )

#define DEBUG_PRINTF15(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15) \
	DebugData::Printf( __FILE__, __LINE__, args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15 )

	

	
#else
//=========================if not def DEBUG=================================================
//--------debug_showhex--------
#define DEBUG_SHOWHEX(input, size, showtail, file, line)	NULL;

//--------debug_printf--------
#define DEBUG_PRINTF(args)  NULL;

#define DEBUG_PRINTF1(args, args1)  \
	NULL;
	
#define DEBUG_PRINTF2(args, args1, arg2 ) \
	NULL;
	
#define DEBUG_PRINTF3(args, args1, arg2, arg3) \
	NULL;
	
#define DEBUG_PRINTF4(args, args1, arg2, arg3, arg4) \
	NULL;
	
#define DEBUG_PRINTF5(args, args1, arg2, arg3, arg4, arg5) \
	NULL;
	
#define DEBUG_PRINTF6(args, args1, arg2, arg3, arg4, arg5, arg6) \
	NULL;

#define DEBUG_PRINTF7(args, args1, arg2, arg3, arg4, arg5, arg6, arg7) \
	NULL;
	
#define DEBUG_PRINTF8(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
	NULL;
	
#define DEBUG_PRINTF9(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
	NULL;

#define DEBUG_PRINTF10(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
	NULL;
	
#define DEBUG_PRINTF11(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) \
	NULL;

#define DEBUG_PRINTF12(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) \
	NULL;

#define DEBUG_PRINTF13(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13) \
	NULL;

#define DEBUG_PRINTF14(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14) \
	NULL;

#define DEBUG_PRINTF15(args, args1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15) \
	NULL;


#endif	//#ifdef DEBUG

#endif //#ifndef DEBUGDATA_H
