/*********************************************************************
*	NAME		:		PFModel.h
*	CONTENT		:		Definition of the BUS-BRANCH model in the PFCT
*	COPYRIGHT	:		ROCK(rockzheng1127@126.com)
*	DATE		:		2014/06/05 -- 2014//
*	DIRECTION	:		
**********************************************************************/

#ifndef _PFMODEL_H
#define _PFMODEL_H
#include <iostream>
#include <vector>
#include <fstream>

#include "PFCore.h"
#include "PFElem.h"
namespace PFModel
{
	/**********************************************
	*		Model_AREA			:	区域计算类
	**********************************************/
	class AREA{
	public:
		AREA();
		//AREA(const AREA&);
	public:
		PFStr		name;		//区域名称
		PFUInt		gene_num;	//机组数目
		PFUInt		load_num;	//负荷数目

		PowerInfo   load;		//区域总负荷信息
		PowerInfo   gene;		//区域总功率出力信息

	};
	/**********************************************
	*		Model_BUS			:	节点计算类
	**********************************************/
	class BUS{
		//友元函数
		friend std::ostream& operator<<(std::ostream& dc, const BUS& bus);

	//成员函数
	public:	
		BUS();
		BUS(const BUS& );
		//~BUS();

		inline PFVoid		Backup(){
			this->btype_bak   = this->btype;
			this->voltage.Backup();
			this->load.Backup();
			this->gene.Backup();
			this->para.Backup();
		}
		
		inline PFVoid BackupBusType() {
			this->btype_bak = this->btype;
		}

		inline PF_BUS_TYPE GetOriBusType() const {
			return this->btype_bak;
		}

		inline PFVoid       UpdateAreaPD(){
			this->pArea->load.p += this->load.p;
		}

		inline PFVoid       UpdateAreaQD(){
			this->pArea->load.q += this->load.q;
		}

		inline PFVoid       UpdateAreaPG(){
			this->pArea->gene.p += this->gene.p;
		}

		inline PFVoid       UpdateAreaQG(){
			this->pArea->gene.q += this->gene.q;
		}

		PFVoid                Clear();							//所有成员变量清空

		PFCore::PFDoubleVec	  ComputeDPQElement();				//计算功率失配量

		PFDouble			  ComputeDQCal();					//计算无功功率失配量

		PFCore::PFDoubleVec   ComputeDPQCal();					//计算功率计算是配量，不包含

		PFCore::PFTripletDVec ComputeJacoElement();				//计算雅可比阵节点相关元素
	private:

	//成员变量
	public:
		Identification  IDN;			//名称标识

		PFUInt          nArea;			//所属分区索引
		PF_BUS_TYPE		btype;			//节点电压类型

		VoltageInfo     voltage;		//节点电压信息
		PowerInfo		load;			//节点负荷信息
		PowerInfo		gene;			//节点机组信息
		Parameter		para;			//自导纳信息
		//离散控制量
		Discrete		shunt;			//并补控制
	protected:
		AREA*           pArea;			//所在分区指针
	private:
		PF_BUS_TYPE		btype_bak;		
	};
	/**********************************************
	*		Model_BRANCH		:	支路计算类
	**********************************************/
	class BRANCH{
	//成员函数
	public:
		BRANCH();
		~BRANCH();

		inline PFVoid SetPFromBus(BUS* const pbus){
			this->pFromBus = pbus;
		}

		inline PFVoid SetPEndBus(BUS* const pbus){
			this->pEndBus = pbus;
		}

		inline PFVoid SetFromBusTPND(PFStr tpnd){
			this->fromTpnd = tpnd;
		}

		inline PFVoid SetEndBusTPND(PFStr tpnd){
			this->endTpnd = tpnd;
		}

		inline PFStr  GetFromBusTPND()const{
			return this->fromTpnd;
		}

		inline PFStr  GetEndBusTPND()const{
			return this->endTpnd;
		}

		inline PFVoid Backup(){
			this->pqij.Backup();
			this->pqji.Backup();
			this->para.Backup();
			this->tap.Backup();
		}

		PFVoid							Clear();				//成员变量清空
		
		PFVoid							ComputeYGBElement();	//该支路计算节点导纳阵相关元素
		
		PFCore::PFTripletDVec			ComputeJacoElement();	//计算雅可比阵元素
		
		PFCore::PFDoubleVec				ComputeDPQElement();	//计算功率失配量

		PFCore::PFDoubleVec				ComputeDQCal();		//计算无功失配量
	protected:
	private:
	//成员变量
	public:
		Identification		IDN;			//名称标识

		PF_BRANCH_TYPE		btype;			//支路类型
		PFInt				inn;			//首端节点序号
		PFInt				jnn;			//末端节点序号

		//节点导纳矩阵非对角元部分元素
		Parameter			para;			//支路参数和互导纳信息
		//支路潮流功率
		PowerInfo			pqij;
		PowerInfo			pqji;
		//支路控制信息
		Discrete			tap;			//线路变比信息
	protected:
		BUS*				pFromBus;		//首端节点指针
		BUS*				pEndBus;		//末端节点指针
		PFStr               fromTpnd;		//首端拓扑节点名称
		PFStr				endTpnd;		//末端拓扑节点名称
	private:
	};

	typedef std::vector<BUS>				PFBusVec;
	typedef std::vector<BUS>::iterator		iter_PFBus;
	typedef std::vector<BRANCH>				PFBranchVec;
	typedef std::vector<BRANCH>::iterator	iter_PFBranch;
	typedef std::vector<AREA>				PFAreaVec;
	typedef std::vector<AREA>::iterator		iter_PFArea;

	class PFSys{
	//成员函数
	public:
		PFSys();
		~PFSys();
		//静态成员函数
		static PFVoid SetPFSysBaseMVA(PFDouble);
		static PFDouble GetPFSysBaseMVA();
		//普通成员函数
		/******************************************************************************
		*		初始化系统数据
		******************************************************************************/
		PFUInt InitSysDataByFile(std::string& filepath, PF_DATA_FILE_FORMAT format);		//初始化系统数据
		PFVoid InitSysDataByDB(PF_DATA_DB_FORMAT format);									//初始化系统数据
		PFVoid OutputSysData2File(std::string& filepath, PF_DATA_FILE_FORMAT format);		//输出系统数据至文件
		PFVoid OutputSysData2DB(PF_DATA_DB_FORMAT format);									//输出系统书库至数据库

		PFVoid FormAssociation();															//形成系统的连接关系
		PFVoid SolvePowerInjection();														//形成系统功率汇入汇出
		PFVoid SolveYGBMatrix();															//求解节点导纳矩阵，并存储到相应单元
		PFUInt GetSysSize();																//求解系统规模
		PFVoid BackupVoltage();																//备份当前系统电压信息
		PFVoid RewindVoltage();																//恢复系统电压备份值
		PFVoid BackupPower();
		PFVoid RewindPower();
		PFVoid BackupBusType();
		PFVoid RewindBusType();
		PFVoid BackupBusCase();
		PFVoid RewindBusCase();

		//TEST
		PFVoid OutputYGBMatrix2DOS();														//输出节点导纳矩阵
		PFVoid OutputBusVoltageInfo2Dos();													//输出节点电压信息
		PFVoid OutputBusPowerInfo2Dos();													//输出节点功率信息
		PFVoid OutputVoltageInfo2File(const std::string file);									//输出电压信息至文件			

	protected:
		PFVoid _GetSysDataByFile_IEEE(std::string& filepath);								//读取IEEE标准数据
		PFVoid _OutputSysData2File_IEEE(std::string& filepath);								//输出IEEE标准数据
		PFVoid _GetSysDataByFile_BPA(std::string& filepath);								//读取BPA标准数据
		PFVoid _OutputSysData2File_BPA(std::string& filepath);								//输出BPA标准数据
		PFVoid _GetSysDataByFile_XML(std::string& filepath);								//读取xml格式数据
		PFVoid _OutputSysData2File_XML(std::string& filepath);								//输出xml标准数据
		PFVoid _GetSysDataByDB_MYSQL();														//读取MySql数据库中数据
	private:
		PFVoid _GetSysDataByFile_IEEE_BUS(std::ifstream& fin);								//读取IEEE数据中BUS段
		PFVoid _GetSysDataByFile_IEEE_BRANCH(std::ifstream& fin);							//读取IEEE数据中BRANCH段

	//成员变量
	public:
		PFUInt			PFSysSize;	//系统规模

		PFAreaVec		PFSysArea;	//区域
		PFBusVec		PFSysBus;	//计算节点
		PFBranchVec		PFSysBranch;//计算支路
	protected:
	private:
		PFBusVec        PFSysBusCase;//计算节点备份 -- 用于XX潮流计算前基态备份
		static PFDouble SysBaseMVA;	//系统功率基准值
	};
}

#endif