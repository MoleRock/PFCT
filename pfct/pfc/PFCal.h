/*********************************************************************
*	NAME		:		PFCal.h
*	CONTENT		:		PF Calculation in the PFCT
*	COPYRIGHT	:		ROCK(rockzheng1127@126.com)
*	DATE		:		2014/06/05 -- 2014//
*	DIRECTION	:		
**********************************************************************/

#ifndef _PFCAL_H
#define _PFCAL_H

#include <exception>

#include "../util/PFCore.h"
#include "../util/PFModel.h"
using namespace PFCore;
using namespace PFModel;

namespace PFCT{
	//潮流计算类
	class PFC {
	//成员函数
	public:
		//构造函数
		PFC();
		PFC(const PFC& pf);								//复制构造函数
		PFC(PFSys* const);								//系统数据指针
		PFC(PFSys& SData);						//系统数据引用
		//析构函数
		~PFC();
		
		inline PFVoid   SetBTypeSwitchState(PFBool state) {
			this->PFCBTypeSwitch = state;
		}

		inline PFVoid   SetBTypeSwitchMethod(PF_BUSTYPE_SWITCH method) {
			this->PFCBtypeMethod = method;
		}

		inline PFVoid	SetPFInit(PF_VA_INIT init){
			this->PFCInitType = init;
		}

		inline const PFUInt   GetPFIter() const{
			return this->PFCIter;
		}

		inline const PF_RESULT GetPFCResult() const {
			return this->PFCResult;
		}

		virtual PFVoid	Solve();						//潮流计算模板函数，构造各种方法求解线性方程组

	protected:
		PFVoid			_MakeJacoMatrix();				//构造雅可比阵

		PFVoid          _MakeDPQVector();				//构造功率失配量

		PFVoid			_MakeDQVector();				//构造无功功率失配量

		PFVoid			_MakeDPQCalByBus();				//构建功率失配计算量，不包含机组出力

		PFVoid			_MakeDPQCalByGene();			//在功率失配量的基础上增加机组出力

		PFVoid			_UpdateSysState();				//更新系统状态变量

		PFVoid			_ModifyJaco();					//修正雅可比阵

		PFVoid          _ModifyJacoByBusType(const int mode);//雅可比阵元素修正

		PFVoid			_ModifyJacoQGElem();			//互补约束中修正雅可比阵dQG 行元素

		PFVoid			_ModifyDPQByBusType(const int mode);//修正功率失配量 mode = 0:修正平衡节点和PV节点; = 1: 只修正平衡节点

		PFVoid			_MakeDPQVectorByBranch();		//构造系统支路联系功率

		PFVoid			_MakeDPQVectorByBus();			//构造系统节点注入功率

		PFVoid			_SwitchBType();					//执行节点类型转换

		const PFDouble  _CalOptimalMultiplier();		//计算最优乘子 -- 1994 [CSEE] [王宪荣] 极坐标系准最优乘子病态潮流解法研究

	private:
		inline PFVoid   _InitPFCalMatrix(){
			PFUInt size = this->pSysData->GetSysSize();
			if(size <= 0)
				throw std::exception("WARNING: Failure in FUNC_InitPFCalMatrix cause the SysSize <= 0 !");
			//Resize the inner Matrix & Vector
			//this->PFCVA.resize(size * 2);
			this->dPFCVA.resize(size * 2);
			this->dPFCPQ.resize(size * 2);
			this->PFCJaco.resize(size * 2, size * 2);
		}

		PFVoid			_ExecuteSwitchLogic();			//执行节点类型转换逻辑
		
		PFVoid			_ExecuteSwitchComplementary();	//执行互补约束转换逻辑

		PF_PQ_VIOLATION _CheckPQViolation(PFDouble val, PFDouble valhigh, PFDouble vallow);
		
		PF_V_VIOLATION	_CheckVolViolation(PFDouble val, PFDouble valhigh, PFDouble vallow);
		
		//采用盛金公式计算一元三次方程，g3,g2,g1,g0分别为方程系数
		PFUInt _CalFormularByShengJin(const PFDouble g3, const PFDouble g2, const PFDouble g1, const PFDouble g0, PFCore::PFDoubleVec& result);
	//成员变量
	public:
		static PFDouble			PFCEpsm;				//潮流收敛精度
		static PFUInt			PFCMaxIter;				//潮流最大迭代次数
	protected:
		PFCore::PFSMatrixXD		PFCJaco;				//雅可比矩阵
		PFCore::PFVectorXD		dPFCVA;					//潮流计算电压失配量
		PFCore::PFVectorXD		dPFCPQ;					//潮流计算功率失配量

		PFSys*					pSysData;				//系统数据头指针
	private:
		PF_RESULT				PFCResult;				//潮流收敛状况
		PFUInt					PFCIter;				//潮流计算迭代次数
		PF_VA_INIT				PFCInitType;			//潮流初始化方式

		PFBool					PFCBTypeSwitch;			//是否执行节点类型转换
		PF_BUSTYPE_SWITCH		PFCBtypeMethod;			//节点类型转换方法


	};//End of class PFC



}//End of namespace PFCal




#endif