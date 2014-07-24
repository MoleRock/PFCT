/*********************************************************************
*	NAME		:		CPFCal.h
*	CONTENT		:		CPF Calculation in the PFCT
*	COPYRIGHT	:		ROCK(rockzheng1127@126.com)
*	DATE		:		2014/07/21 -- 2014//
*	DIRECTION	:		
**********************************************************************/
#ifndef _CPFCAL_H
#define _CPFCAL_H

#include "PFCal.h"

namespace PFCT{
	//连续潮流计算类
	class CPF: public PFC{
		//Fields
	public:
		static PFUInt CPFMXPnt;				//CPF计算点数最大值
	protected:
		//内部计算矩阵和向量
		PFTripletDVec	CPFPertCol;			//增广雅可比阵最右列,功率对增长因子的灵敏度系数		
		PFTripletDVec   CPFPertRow;			//增广雅可比阵最下行,参数化方法对增长因子的系数

	private:
		CPF_RESULT		CPFResult;			//CPF计算结果情况
		PFUInt			CPFNPoint;			//CPF计算点序号

		CPF_DEST		CPFDest;			//CPF计算目标
		CPF_PARA		CPFPara;			//CPF参数化方法
		CPF_PREDICT		CPFPredict;			//CPF预测方法

		PFDouble		CPFLoadMargin;		//当前负荷增长方式下负荷裕度

		PFDouble		CPFLambda;			//负荷增长因子
		PFDouble		CPFStepLen;			//计算步长

		PFUInt			CPFEK;				//连续参数索引

		//备份信息
		PFDouble		CPFLambdaBak;		//负荷增长因子备份

		//Functions
	public:
		CPF();								//无参数构造函数
		CPF(const CPF& cpf);				//复制构造函数
		CPF(const PFC& pf);					//潮流构造函数
		CPF(PFModel::PFSys& psys);			//系统数据构造函数
		CPF(PFModel::PFSys* const psys);	//系统数据构造函数

		//Getter And Setter Functions
		inline PFVoid SetDest(const CPF_DEST dest){
			this->CPFDest = dest;
		}

		inline const CPF_DEST GetDest() const {
			return this->CPFDest;
		}

		inline PFVoid SetPara(const CPF_PARA para){
			this->CPFPara = para;
		}

		inline const CPF_PARA GetPara() const {
			return this->CPFPara;
		}

		inline PFVoid SetPredict(CPF_PREDICT pred) {
			this->CPFPredict = pred;
		}

		inline const CPF_PREDICT GetPredict() const {
			return this->CPFPredict;
		}

		inline const PFDouble GetLoadMargin() const {
			return this->CPFLoadMargin;
		}

		inline const PFDouble GetCPFLambda() const {
			return this->CPFLambda;
		}

		inline const PFDouble GetStepLen() const {
			return this->CPFStepLen;
		}

		inline const CPF_RESULT GetCPFResult() const {
			return this->CPFResult;
		}

		PFVoid		    Solve();					//CPF计算
	protected:
		PFVoid			_MakeCPFPertCol();			//计算CPF增广雅可比阵最右列
		PFVoid			_MakeCPFPredictRow();		//预测步计算CPF增广雅可比阵最下行
		PFBool			_MakePredict();				//预测步计算
		PFBool			_MakeCorrect();				//校正步计算

		PFVoid			_MakeJacoMatrix();			//求解增广雅可比矩阵，重写函数
		PFVoid			_UpdateSysState();			//更新系统状态变量
		PFVoid			_UpdateCPFEK();				//更新连续变量	
	private:
		//默认初始化
		inline PFVoid _InitDefault(){
			this->CPFResult = CPF_RESULT_FAILURE;
			this->CPFNPoint = 0;

			this->CPFDest = CPF_DEST_PV;
			this->CPFPara = CPF_PARA_LOCAL;
			this->CPFPredict = CPF_PRE_TANGENT;

			this->CPFLoadMargin = 0.0;

			this->CPFLambda = 0.0;
			this->CPFStepLen = 1.0;

			this->CPFEK = 0;	//初始化的时候定为0

			this->CPFLambdaBak = 0.0;
		}
		//初始化计算矩阵和向量
		inline PFVoid _InitPFCalMatrix() {
			PFUInt size = this->pSysData->GetSysSize();
			if(size <= 0)
				throw std::exception("WARNING: Failure in FUNC_InitPFCalMatrix cause the SysSize <= 0 !");
			//Resize the inner Matrix & Vector
			this->dPFCVA.resize(size * 2 + 1);
			this->dPFCPQ.resize(size * 2 + 1);
			this->PFCJaco.resize(size * 2 + 1, size * 2 + 1);
		}
		//预测步失配量向量
		PFVoid _MakeCPFPredictVec();
	};


}

#endif
