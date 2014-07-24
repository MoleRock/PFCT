#include "CPFCal.h"

using namespace std;
using namespace PFCT;

PFUInt CPF::CPFMXPnt = 30;

//CPF���޲ι��캯��
CPF::CPF():PFC(){
	this->_InitDefault();
}
//���ƹ��캯��
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
//�������캯��
CPF::CPF(const PFC& pf):PFC(pf){
	this->_InitDefault();
}				
//ϵͳ���ݹ��캯��
CPF::CPF(PFModel::PFSys& psys):PFC(psys){
	this->_InitDefault();
}
//ϵͳ���ݹ��캯��
CPF::CPF(PFModel::PFSys* const psys):PFC(psys){
	this->_InitDefault();
}

//CPF����
PFVoid CPF::Solve(){
	//0. ���Ƚ��л�̬��������,���ݻ�̬����״��ȷ���Ƿ����������������
	PFC::Solve();
	if (this->GetPFCResult() != PF_RESULT_CONVERG){	//�����̬������ɢ�����޷����������������㣬�˳��������
		this->CPFResult = CPF_RESULT_FAILURE_PF_DIVERGE;
		return;
	}
	pSysData->BackupBusCase();//���浱ǰ��̬����״̬

	//1. ��̬����״�����ã�����Խ��������������㣬��ʼ�����������
	_InitPFCalMatrix();

	//2. ���㸺�ɷ�������ϵ������
	_MakeCPFPertCol();

	return;
}
//����CPF�����ſɱ���������
PFVoid CPF::_MakeCPFPertCol(){
	PFUInt inn;
	const PFUInt csize = pSysData->GetSysSize();//��ȡϵͳ��С
	const PFUInt ncol = csize * 2;
	PFDouble dppert, dqpert;
	CPFPertCol.clear();	//�������
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
//Ԥ�ⲽ����
PFBool CPF::_MakePredict(){
	Eigen::SparseLU<PFCore::PFSMatrixXD, PFCore::PFCOLAMDOrdering> solver;
	//0. ��ǰ�ڵ�״̬�ı���
	pSysData->BackupVoltage();
	pSysData->BackupBusType();
	//1. ���Ԥ�ⲽʧ��������
	_MakeCPFPredictVec();
	//2. ��������ſɱȾ���
	_MakeJacoMatrix();
	//3. ����Ԥ������
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
	//4. ����״̬����Ϣ
	pSysData->BackupVoltage();
	pSysData->BackupBusType();
	this->CPFLambdaBak = this->CPFLambda;

	//5. ���½ڵ��ѹ��Ϣ
	dPFCVA *= CPFStepLen;
	_UpdateSysState();

	return true;
}

//Ԥ�ⲽʧ��������
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
//����CPF�����ſɱ���������
PFVoid CPF::_MakeCPFPredictRow(){
	CPFPertRow.clear();
	const PFUInt csize = pSysData->GetSysSize();
	//��һ��Ԥ����㣬��������ȡ������������
	if (this->CPFNPoint == 0 || CPFEK == csize){
		CPFPertRow.push_back(PFCore::PFTripletD(csize * 2, csize * 2, 1));
	} else {
		assert(CPFEK < csize);
		CPFPertRow.push_back(PFCore::PFTripletD(csize * 2, CPFEK * 2 + 1));
	}
	return;
}	

//��������ſɱȾ�����д����
PFVoid CPF::_MakeJacoMatrix(){
	PFCore::PFTripletDVec triplist, tmplist;//���ɾ���Ļ���Ԫ��
	//0. ����֧·�����ſɱ���Ԫ��
	for (iter_PFBranch iter = pSysData->PFSysBranch.begin();iter != pSysData->PFSysBranch.end();++iter){
		tmplist = iter->ComputeJacoElement();							//����֧·���ɵ��ſɱ���Ԫ��
		triplist.insert(triplist.end(), tmplist.begin(), tmplist.end());//���빹����
	}
	//1. �����ڵ������ſɱ���Ԫ��
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		tmplist = iter->ComputeJacoElement();
		triplist.insert(triplist.end(), tmplist.begin(), tmplist.end());
	}
	//3. ������������Ԫ��
	triplist.insert(triplist.end(), CPFPertCol.begin(), CPFPertCol.end());
	triplist.insert(triplist.end(), CPFPertRow.begin(), CPFPertRow.end());
	
	//4. �γ��ſɱ���
	PFCJaco.setFromTriplets(triplist.begin(), triplist.end());

	//5. �����ſɱ���
	_ModifyJaco();

	return;

}		
//����ϵͳ״̬����
PFVoid CPF::_UpdateSysState(){
	//0. ���ø�����º���
	PFC::_UpdateSysState();
	//1. ���¸�����������
	PFUInt csize = pSysData->GetSysSize();
	CPFLambda -= dPFCVA.coeff(csize * 2);
}
//������������
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
