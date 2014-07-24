/*********************************************************************
*	NAME		:		PFMenuDef.h
*	CONTENT		:		Menu Definition in the PFCT
*	COPYRIGHT	:		ROCK(rockzheng1127@126.com)
*	DATE		:		2014/06/05 -- 2014//
*	DIRECTION	:		
**********************************************************************/

#ifndef _PFMENUDEF_H
#define _PFMENUDEF_H
#include <string>

typedef double PFDouble;
typedef int	   PFInt;
typedef unsigned int PFUInt;
typedef long   PFLong;
typedef unsigned long PFULong;
typedef std::string PFStr;
typedef void   PFVoid;
typedef unsigned long PFID;
typedef bool   PFBool;

//���ó�������
#define PI 3.1415926
#define BIGNUM 1e6
#define LAMBDA 10.0

/* type of bus */
typedef enum{
	PF_BUS_TYPE_PQ,						//PQ�ڵ�
	PF_BUS_TYPE_PV,						//PV�ڵ�
	PF_BUS_TYPE_VA						//ƽ��ڵ�
}PF_BUS_TYPE;

/* type of branch */
typedef enum{
	PF_BRANCH_LINE,
	PF_BRANCH_TRAN,
	PF_BRANCH_SERIES,
	PF_BRANCH_NULL
}PF_BRANCH_TYPE;

//���Ƶ�������
typedef enum{
	PF_CTRL_NULL = 0,
	PF_CTRL_UP = 1,
	PF_CTRL_DOWN = -1
}PF_CTRL_DIRECTION;

//��ѹԽ�����
typedef enum{
	PF_V_VIOLATION_HIGH,
	PF_V_VIOLATION_LOW,
	PF_V_VIOLATION_NORM
}PF_V_VIOLATION;
//����Խ�����
typedef enum{
	PF_PQ_VIOLATION_HIGH,
	PF_PQ_VIOLATION_LOW,
	PF_PQ_VIOLATION_NORM
}PF_PQ_VIOLATION;

//������ʼֵѡ��
typedef enum{
	PF_VA_INIT_FLAT  = 0,											//ƽ����
	PF_VA_INIT_SYS   = 1											//���ݵ�ѹ��������
}PF_VA_INIT;
//��������״��
typedef enum{
	PF_RESULT_CONVERG			 = 0,							//��������
	PF_RESULT_DIVERGE			 = 1,							//������ɢ
	PF_RESULT_DIVERGE_OVER_ITER	 = 2,							//���������������޷�ɢ
	PF_RESULT_DIVERGE_NAN		 = 3,							//������������г���NAN��ɢ
	PF_RESULT_DIVERGE_OVER_LIMIT = 4,							//�������������е�ѹ����Խ�޷�ɢ
	PF_RESULT_DIVERGE_FAILURE	 = 5							//�ſɱ���ֽ�ʧ��
}PF_RESULT;
//CPF�����������
typedef enum{
	CPF_RESULT_SUCCESS				= 0,							//����ɹ�
	CPF_RESULT_FAILURE				= 1,							//����ʧ��
	CPF_RESULT_FAILURE_PF_DIVERGE	= 2,							//��̬������ɢ
	CPF_RESULT_FAILURE_OVERITER		= 3,							//У���������������޷�ɢ
	CPF_RESULT_FAILURE_NAN			= 4,							//����������������г���NAN��ɢ
	CPF_RESULT_FAILURE_OVERLIMIT	= 5,							//�������������е�ѹ����Խ�޷�ɢ
	CPF_RESULT_FAILURE_MATDEC		= 6								//�ſɱ���ֽ�ʧ��
}CPF_RESULT;
//PFModel�����Զ���
#define PF_BUS_INDEX_DEFAULT -1
#define PF_BRANCH_INDEX_DEFAULT -1
//PFIDĬ��ֵ
#define PF_ID_DEFAULT 0
//PFĬ��ֵ
#define PF_STR_DEFAULT ""
#define PF_DOUBLE_DEFAULT 0.0



//PFSys������Դ
typedef enum{
	PF_DATA_FILE,
	PF_DATA_DB
}PF_DATA_SOURCE;
//PF_DATA_FILE�������ݸ�ʽ
typedef enum{
	PF_DATA_FILE_IEEE,
	PF_DATA_FILE_BPA,
	PF_DATA_FILE_XML
}PF_DATA_FILE_FORMAT;
//PF_DATA_DB�������ݿ�����
typedef enum{
	PF_DATA_DB_MYSQL,
	PF_DATA_DB_ORACLE
}PF_DATA_DB_FORMAT;

//PFCal�ж���
typedef enum {
	PF_BUSTYPE_SWITCH_LOGIC,				//�ڵ�����ת���߼�
	PF_BUSTYPE_SWITCH_COMPLEMENTARY			//����Լ��ת��
}PF_BUSTYPE_SWITCH;

//CPFCal�ж���
//����������
typedef enum {
	CPF_PARA_LOCAL,							//�ֲ�������
	CPF_PARA_ARC,							//����������
	CPF_PARA_GEOMETRIC						//���β�����
}CPF_PARA;
//CPF����Ŀ��
typedef enum{
	CPF_DEST_PV,							//����PV����
	CPF_DEST_BP								//���ֲ����Ϣ
}CPF_DEST;
//CPFԤ�ⷽ��
typedef enum{
	CPF_PRE_TANGENT,						//���߷�Ԥ��
	CPF_PRE_INTERPOLATION					//��ֵ��Ԥ��
}CPF_PREDICT;

#endif