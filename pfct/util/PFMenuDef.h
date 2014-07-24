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

//常用常数定义
#define PI 3.1415926
#define BIGNUM 1e6
#define LAMBDA 10.0

/* type of bus */
typedef enum{
	PF_BUS_TYPE_PQ,						//PQ节点
	PF_BUS_TYPE_PV,						//PV节点
	PF_BUS_TYPE_VA						//平衡节点
}PF_BUS_TYPE;

/* type of branch */
typedef enum{
	PF_BRANCH_LINE,
	PF_BRANCH_TRAN,
	PF_BRANCH_SERIES,
	PF_BRANCH_NULL
}PF_BRANCH_TYPE;

//控制调整方向
typedef enum{
	PF_CTRL_NULL = 0,
	PF_CTRL_UP = 1,
	PF_CTRL_DOWN = -1
}PF_CTRL_DIRECTION;

//电压越限情况
typedef enum{
	PF_V_VIOLATION_HIGH,
	PF_V_VIOLATION_LOW,
	PF_V_VIOLATION_NORM
}PF_V_VIOLATION;
//功率越限情况
typedef enum{
	PF_PQ_VIOLATION_HIGH,
	PF_PQ_VIOLATION_LOW,
	PF_PQ_VIOLATION_NORM
}PF_PQ_VIOLATION;

//潮流初始值选项
typedef enum{
	PF_VA_INIT_FLAT  = 0,											//平启动
	PF_VA_INIT_SYS   = 1											//根据电压参数启动
}PF_VA_INIT;
//潮流收敛状况
typedef enum{
	PF_RESULT_CONVERG			 = 0,							//潮流收敛
	PF_RESULT_DIVERGE			 = 1,							//潮流发散
	PF_RESULT_DIVERGE_OVER_ITER	 = 2,							//潮流迭代次数过限发散
	PF_RESULT_DIVERGE_NAN		 = 3,							//潮流计算过程中出现NAN发散
	PF_RESULT_DIVERGE_OVER_LIMIT = 4,							//潮流迭代过程中电压过度越限发散
	PF_RESULT_DIVERGE_FAILURE	 = 5							//雅可比阵分解失败
}PF_RESULT;
//CPF计算收敛情况
typedef enum{
	CPF_RESULT_SUCCESS				= 0,							//计算成功
	CPF_RESULT_FAILURE				= 1,							//计算失败
	CPF_RESULT_FAILURE_PF_DIVERGE	= 2,							//基态潮流发散
	CPF_RESULT_FAILURE_OVERITER		= 3,							//校正步迭代次数过限发散
	CPF_RESULT_FAILURE_NAN			= 4,							//连续潮流计算过程中出现NAN发散
	CPF_RESULT_FAILURE_OVERLIMIT	= 5,							//潮流迭代过程中电压过度越限发散
	CPF_RESULT_FAILURE_MATDEC		= 6								//雅可比阵分解失败
}CPF_RESULT;
//PFModel索引自定义
#define PF_BUS_INDEX_DEFAULT -1
#define PF_BRANCH_INDEX_DEFAULT -1
//PFID默认值
#define PF_ID_DEFAULT 0
//PF默认值
#define PF_STR_DEFAULT ""
#define PF_DOUBLE_DEFAULT 0.0



//PFSys数据来源
typedef enum{
	PF_DATA_FILE,
	PF_DATA_DB
}PF_DATA_SOURCE;
//PF_DATA_FILE具体数据格式
typedef enum{
	PF_DATA_FILE_IEEE,
	PF_DATA_FILE_BPA,
	PF_DATA_FILE_XML
}PF_DATA_FILE_FORMAT;
//PF_DATA_DB具体数据库类型
typedef enum{
	PF_DATA_DB_MYSQL,
	PF_DATA_DB_ORACLE
}PF_DATA_DB_FORMAT;

//PFCal中定义
typedef enum {
	PF_BUSTYPE_SWITCH_LOGIC,				//节点类型转换逻辑
	PF_BUSTYPE_SWITCH_COMPLEMENTARY			//互补约束转换
}PF_BUSTYPE_SWITCH;

//CPFCal中定义
//参数化方法
typedef enum {
	CPF_PARA_LOCAL,							//局部参数化
	CPF_PARA_ARC,							//弧长参数化
	CPF_PARA_GEOMETRIC						//几何参数化
}CPF_PARA;
//CPF计算目标
typedef enum{
	CPF_DEST_PV,							//绘制PV曲线
	CPF_DEST_BP								//求解分岔点信息
}CPF_DEST;
//CPF预测方法
typedef enum{
	CPF_PRE_TANGENT,						//切线法预测
	CPF_PRE_INTERPOLATION					//插值法预测
}CPF_PREDICT;

#endif