#include "CPFCal.h"

using namespace std;
using namespace PFCT;

PFUInt CPF::CPFMXPnt = 30;

//CPF类无参构造函数
CPF::CPF():PFC(){
	this->_InitDefault();
}
//复制构造函数
CPF::CPF(const CPF& cpf):PFC(cpf) {
	this->CPFPertCol = cpf.CPFPertCol;
	this->CPFPertRow = cpf.CPFPertRow;

	this->CPFResult = cpf.CPFResult;
	this->CPFNPoint = cpf.CPFNPoint;

	this->CPFDest = cpf.CPFDest;
	this->CPFPara = cpf.CPFPara;
	this->CPFPredict = cpf.CPFPredict;
	
	this->CPFLoadMargin = cpf.CPFLoadMargin;

	this->CPFLambda = cpf.CPFLambda;
	this->CPFStepLen = cpf.CPFStepLen;

	this->CPFEK = cpf.CPFEK;

	this->CPFLambdaBak = cpf.CPFLambdaBak;
}
//潮流构造函数
CPF::CPF(const PFC& pf):PFC(pf){
	this->_InitDefault();
}				
//系统数据构造函数
CPF::CPF(PFModel::PFSys& psys):PFC(psys){
	this->_InitDefault();
}
//系统数据构造函数
CPF::CPF(PFModel::PFSys* const psys):PFC(psys){
	this->_InitDefault();
}

//CPF计算
PFVoid CPF::Solve(){
	//0. 首先进行基态潮流计算,根据基态潮流状况确定是否继续连续潮流计算
	PFC::Solve();
	if (this->GetPFCResult() != PF_RESULT_CONVERG){	//如果基态潮流发散，则无法继续连续潮流计算，退出计算过程
		this->CPFResult = CPF_RESULT_FAILURE_PF_DIVERGE;
		return;
	}
	pSysData->BackupBusCase();//保存当前基态潮流状态

	//1. 基态潮流状况良好，则可以进行连续潮流计算，初始化矩阵和向量
	_InitPFCalMatrix();

	//2. 计算负荷发电增长系数矩阵
	_MakeCPFPertCol();

	return;
}
//计算CPF增广雅可比阵最右列
PFVoid CPF::_MakeCPFPertCol(){
	PFUInt inn;
	const PFUInt csize = pSysData->GetSysSize();//获取系统大小
	const PFUInt ncol = csize * 2;
	PFDouble dppert, dqpert;
	CPFPertCol.clear();	//清空数据
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		inn = iter->IDN.index;
		assert(inn < csize);
		dppert = iter->gene.GetPertP() - iter->load.GetPertP();
		dqpert = - iter->load.GetPertQ();
		CPFPertCol.push_back(PFCore::PFTripletD(inn * 2, ncol, dppert));
		CPFPertCol.push_back(PFCore::PFTripletD(inn * 2 + 1, ncol, dqpert));
	}
	return;
}		
//预测步计算
PFBool CPF::_MakePredict(){
	Eigen::SparseLU<PFCore::PFSMatrixXD, PFCore::PFCOLAMDOrdering> solver;
	//0. 当前节点状态的备份
	pSysData->BackupVoltage();
	pSysData->BackupBusType();
	//1. 求解预测步失配量向量
	_MakeCPFPredictVec();
	//2. 求解增广雅可比矩阵
	_MakeJacoMatrix();
	//3. 计算预测向量
	solver.compute(PFCJaco);
	if (solver.info() != Eigen::Success){
		cout << "Solving<stage_1> Failed!" << endl;
		this->CPFResult = CPF_RESULT_FAILURE_MATDEC;
		return false;
	}
	dPFCVA = solver.solve(dPFCPQ);
	if (solver.info() != Eigen::Success){
		cout << "Solving<stage_2> Failed!" << endl;
		this->CPFResult = CPF_RESULT_FAILURE_MATDEC;
		return false;
	}
	if(dPFCVA.hasNaN()){
		this->CPFResult = CPF_RESULT_FAILURE_NAN;
		return false;
	}
	//4. 备份状态量信息
	pSysData->BackupVoltage();
	pSysData->BackupBusType();
	this->CPFLambdaBak = this->CPFLambda;

	//5. 更新节点电压信息
	dPFCVA *= CPFStepLen;
	_UpdateSysState();

	return true;
}

//预测步失配量向量
PFVoid CPF::_MakeCPFPredictVec(){
	const PFUInt csize = pSysData->GetSysSize();
	PFDouble dlambda = dPFCVA.coeff(csize * 2);
	dPFCPQ.setZero();
	if (dlambda > 0.0){
		dPFCPQ.coeffRef(csize * 2) = 1.0;
	} else if (dlambda <= 0.0){
		dPFCPQ.coeffRef(csize * 2) = -1.0;
	}
	return;
}
//计算CPF增广雅可比阵最下行
PFVoid CPF::_MakeCPFPredictRow(){
	CPFPertRow.clear();
	const PFUInt csize = pSysData->GetSysSize();
	//第一次预测计算，连续参数取负荷增长参数
	if (this->CPFNPoint == 0 || CPFEK == csize){
		CPFPertRow.push_back(PFCore::PFTripletD(csize * 2, csize * 2, 1));
	} else {
		assert(CPFEK < csize);
		CPFPertRow.push_back(PFCore::PFTripletD(csize * 2, CPFEK * 2 + 1));
	}
	return;
}	

//求解增广雅可比矩阵，重写函数
PFVoid CPF::_MakeJacoMatrix(){
	PFCore::PFTripletDVec triplist, tmplist;//生成矩阵的基础元素
	//0. 遍历支路增加雅可比阵元素
	for (iter_PFBranch iter = pSysData->PFSysBranch.begin();iter != pSysData->PFSysBranch.end();++iter){
		tmplist = iter->ComputeJacoElement();							//计算支路构成的雅可比阵元素
		triplist.insert(triplist.end(), tmplist.begin(), tmplist.end());//加入构建列
	}
	//1. 遍历节点增加雅可比阵元素
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		tmplist = iter->ComputeJacoElement();
		triplist.insert(triplist.end(), tmplist.begin(), tmplist.end());
	}
	//3. 增加增广行列元素
	triplist.insert(triplist.end(), CPFPertCol.begin(), CPFPertCol.end());
	triplist.insert(triplist.end(), CPFPertRow.begin(), CPFPertRow.end());
	
	//4. 形成雅可比阵
	PFCJaco.setFromTriplets(triplist.begin(), triplist.end());

	//5. 修正雅可比阵
	_ModifyJaco();

	return;

}		
//更新系统状态变量
PFVoid CPF::_UpdateSysState(){
	//0. 调用父类更新函数
	PFC::_UpdateSysState();
	//1. 更新负荷增长因子
	PFUInt csize = pSysData->GetSysSize();
	CPFLambda -= dPFCVA.coeff(csize * 2);
}
//更新连续变量
PFVoid CPF::_UpdateCPFEK(){
	const PFUInt csize = pSysData->GetSysSize();
	PFUInt inn, ek = 0
	PFDouble dek = 0.0, mxek = 0.0;
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		inn = iter->IDN.index;
		dek = fabs(dPFCVA.coeff(inn * 2 + 1) / iter->voltage.vol);
		if (dek > mxek){
			ek = inn;
			mxek = dek;
		}
	}
	if (fabs(dPFCVA.coeff(csize * 2) / CPFLambda > mxek)){
		CPFEK = csize;
	} else {
		CPFEK = ek;
	}
	return;
}
