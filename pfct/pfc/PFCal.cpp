#include "PFCal.h"

#include <Eigen/SparseLU>
#include <iomanip>
#include <cmath>

using namespace std;
using namespace PFCT;

/************************************************************************/
/*		                      PFC                                       */
/************************************************************************/
PFDouble PFC::PFCEpsm = 1e-6;
PFUInt PFC::PFCMaxIter = 50;

//************************************
// Method:    PFC
// FullName:  PFCal::PFC::PFC
// Access:    public 
// Returns:   
// Qualifier:
//************************************
PFC::PFC():PFCIter(0), PFCInitType(PF_VA_INIT_FLAT){
	this->pSysData = NULL;					//ϵͳ����ָ����Ϊ��
	this->PFCBTypeSwitch = false;
	this->PFCBTypeSwitch = PF_BUSTYPE_SWITCH_LOGIC;
	this->PFCResult = PF_RESULT_DIVERGE;
}

//���ƹ��캯��
PFC::PFC(const PFC& pf){
	this->PFCIter = pf.PFCIter;
	this->PFCInitType = pf.PFCInitType;
	this->PFCBtypeMethod = pf.PFCBtypeMethod;
	this->PFCBTypeSwitch = pf.PFCBTypeSwitch;
	this->PFCJaco = pf.PFCJaco;
	this->dPFCVA = pf.dPFCVA;
	this->dPFCPQ = pf.dPFCPQ;
	this->pSysData = pf.pSysData;
	this->PFCResult = pf.PFCResult;
}

//************************************
// Method:    PFC
// FullName:  PFCal::PFC::PFC
// Access:    public 
// Returns:   
// Qualifier: ϵͳ����ָ��
// Parameter: PFSys * const
//************************************
PFC::PFC(PFSys* const pPFSys):PFCIter(0), PFCInitType(PF_VA_INIT_FLAT){
	this->pSysData = pPFSys;
	this->PFCBTypeSwitch = false;
	this->PFCBTypeSwitch = PF_BUSTYPE_SWITCH_LOGIC;
	this->PFCResult = PF_RESULT_DIVERGE;
}
//************************************
// Method:    PFC
// FullName:  PFCal::PFC::PFC
// Access:    public 
// Returns:   
// Qualifier: ϵͳ��������
// Parameter: PFSys & SData
//************************************
PFC::PFC(PFSys& SData):PFCIter(0), PFCInitType(PF_VA_INIT_FLAT){
	this->pSysData = &SData;
	this->PFCBTypeSwitch = false;
	this->PFCBTypeSwitch = PF_BUSTYPE_SWITCH_LOGIC;
	this->PFCResult = PF_RESULT_DIVERGE;
}
//************************************
// Method:    ~PFC
// FullName:  PFCal::PFC::~PFC
// Access:    public 
// Returns:   
// Qualifier: ���ڲ�����ϵĹ��췽������ϵͳ���ݺͼ������|����������ָ�������������в��漰ϵͳ��������ڴ��ͷš�
//************************************
PFC::~PFC(){
	//
}
//************************************
// Method:    _MakeJacoMatrix
// FullName:  PFCal::PFC::_MakeJacoMatrix
// Access:    protected 
// Returns:   PFVoid
// Qualifier: Form the Matrix Jaco
//************************************
PFVoid PFC::_MakeJacoMatrix(){
	PFCore::PFTripletDVec triplist, tmplist;//���ɾ���Ļ���Ԫ��
	//����֧·�����ſɱ���Ԫ��
	for (iter_PFBranch iter = pSysData->PFSysBranch.begin();iter != pSysData->PFSysBranch.end();++iter){
		tmplist = iter->ComputeJacoElement();							//����֧·���ɵ��ſɱ���Ԫ��
		triplist.insert(triplist.end(), tmplist.begin(), tmplist.end());//���빹����
	}
	//�����ڵ������ſɱ���Ԫ��
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		tmplist = iter->ComputeJacoElement();
		triplist.insert(triplist.end(), tmplist.begin(), tmplist.end());
	}
	//�γ��ſɱ���
	this->PFCJaco.setFromTriplets(triplist.begin(), triplist.end());

	//�����ſɱ���
	_ModifyJaco();

	return;
}
//�����ſɱ���
PFVoid PFC::_ModifyJaco(){
	//�ڵ�������������
	if (PFCBTypeSwitch && PFCBtypeMethod == PF_BUSTYPE_SWITCH_COMPLEMENTARY){
		_ModifyJacoByBusType(1);
	} else {
		_ModifyJacoByBusType(0);
	}
	return;
}


//************************************
// Method:    _MakeDPQVector
// FullName:  PFCal::PFC::_MakeDPQVector
// Access:    protected 
// Returns:   PFVoid
// Qualifier: Compute the dPQ vector
//************************************
PFVoid PFC::_MakeDPQVector(){
	//���ȼ����Ƿ���Ҫ�ڵ�����ת��
	if (!PFCBTypeSwitch){
		//����Ҫ�ڵ�����ת��
		_MakeDPQVectorByBranch();
		_MakeDPQVectorByBus();
		_ModifyDPQByBusType(0);		//���ݽڵ�������������ʧ����
	} else {
		//��Ҫ�ڵ�����ת��
		switch(PFCBtypeMethod){
		case PF_BUSTYPE_SWITCH_LOGIC:	//�ڵ�����ת���߼�
			_MakeDQVector();			//����QGCal�������ж��Ƿ�Խ��
			_SwitchBType();				//ִ�нڵ�����ת���߼�

			_MakeDPQVectorByBranch();	//����֧·����ʧ��
			_MakeDPQVectorByBus();		//����ڵ㹦��ʧ��
			_ModifyDPQByBusType(0);		//���ݽڵ�������������ʧ����
			break;
		case PF_BUSTYPE_SWITCH_COMPLEMENTARY:	//����Լ��ת��
			_MakeDPQVectorByBranch();			//����֧·ʧ��
			_MakeDPQCalByBus();					//����ڵ㹦��ʧ�䣬�������������
			//TODO: ִ�л���Լ���⻬���̼���
			_SwitchBType();
			//TODO: ���ӽڵ㹦�ʳ����������걸����ʧ����
			_MakeDPQCalByGene();
			_ModifyDPQByBusType(1);		//���ݽڵ�������������ʧ����
			break;
		}
	}

	/************************************************************************/
	/* �������                                                             */
	/************************************************************************/
	//if (PFCBTypeSwitch && 
	//	PFCBtypeMethod == PF_BUSTYPE_SWITCH_LOGIC){
	//	_MakeDQVector();
	//	_SwitchBType();
	//}
	////ת�����֮�����¼��㹦��ʧ����
	//_MakeDPQVectorByBranch();
	//_MakeDPQVectorByBus();
	return;
}

//�����޹�����ʧ����
PFVoid PFC::_MakeDQVector(){
	PFInt inn, jnn;
	PFDoubleVec tmplist;
	PFDouble qbus;
	const PFInt csize = this->pSysData->GetSysSize();

	dPFCPQ.setZero();//����ʧ��������
	assert(dPFCPQ.size() == csize * 2);//ȷ������ά��һ��
	//����֧·
	for (iter_PFBranch iter = pSysData->PFSysBranch.begin();iter != pSysData->PFSysBranch.end();++iter){
		inn = iter->inn;
		jnn = iter->jnn;
		assert(inn < csize && jnn < csize);
		tmplist = iter->ComputeDQCal();
		assert(tmplist.size() == 2);
		dPFCPQ[inn * 2 + 1]     += tmplist[0];
		dPFCPQ[jnn * 2 + 1]     += tmplist[1];
	}
	//�����ڵ�
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		inn = iter->IDN.index;
		qbus = iter->ComputeDQCal();
		dPFCPQ[inn * 2 + 1] += qbus;
	}
	return;
}

//��������ʧ�����������������������͸���
PFVoid PFC::_MakeDPQCalByBus() {
	PFInt inn;
	PFDoubleVec tmplist;
	//�����ڵ�
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		inn = iter->IDN.index;
		tmplist = iter->ComputeDPQCal();
		assert(tmplist.size() == 2);
		dPFCPQ[inn * 2]     += tmplist[0];
		dPFCPQ[inn * 2 + 1] += tmplist[1];

		//�ڻ���Լ�������Qcal��ֵ����Ҫ�õ��ģ�������Ͳ���gene.q����¼
		if (iter->btype == PF_BUS_TYPE_PV){
			iter->gene.q = dPFCPQ[inn * 2 + 1];
		}
	}
	return;
}				

PFVoid PFC::_MakeDPQVectorByBranch() {
	PFInt inn, jnn;
	PFDoubleVec tmplist;
	const PFInt csize = this->pSysData->GetSysSize();

	dPFCPQ.setZero();//����ʧ��������
	assert(dPFCPQ.size() == csize * 2);//ȷ������ά��һ��
	//����֧·
	for (iter_PFBranch iter = pSysData->PFSysBranch.begin();iter != pSysData->PFSysBranch.end();++iter){
		inn = iter->inn;
		jnn = iter->jnn;
		assert(inn < csize && jnn < csize);
		tmplist = iter->ComputeDPQElement();
		assert(tmplist.size() == 4);
		dPFCPQ[inn * 2]         += tmplist[0];
		dPFCPQ[inn * 2 + 1]     += tmplist[1];
		dPFCPQ[jnn * 2]         += tmplist[2];
		dPFCPQ[jnn * 2 + 1]     += tmplist[3];
	}
	return;
}

PFVoid PFC::_MakeDPQVectorByBus() {
	PFInt inn;
	PFDoubleVec tmplist;
	//�����ڵ�
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		inn = iter->IDN.index;
		tmplist = iter->ComputeDPQElement();
		assert(tmplist.size() == 2);
		dPFCPQ[inn * 2]     = tmplist[0] - dPFCPQ[inn * 2];
		dPFCPQ[inn * 2 + 1] = tmplist[1] - dPFCPQ[inn * 2 + 1];
	}
	return;
}
//�ڹ���ʧ�����Ļ��������ӻ������
PFVoid PFC::_MakeDPQCalByGene() {
	PFInt inn;
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		inn = iter->IDN.index;
		if (iter->btype != PF_BUS_TYPE_PV){
			//����ڵ�����
			dPFCPQ[inn * 2] = iter->gene.p - dPFCPQ[inn * 2];
			dPFCPQ[inn * 2 + 1] = iter->gene.q - dPFCPQ[inn * 2 + 1];
		} else {
			dPFCPQ[inn * 2] = iter->gene.p - dPFCPQ[inn * 2];
		}
	}
	return;
}			

//************************************
// Method:    _ModifyJacoByBusType
// FullName:  PFCal::PFC::_ModifyJacoByBusType
// Access:    protected 
// Returns:   PFVoid
// Qualifier:
// Parameter: const int mode
//			  mode = 0: ����VA�ڵ�(�Ӵ���)��PV�ڵ�(DQ�Ӵ���)
//			  mode = 1: ����VA�ڵ�(�Ӵ���)��PV�ڵ�(����Լ����������)
//************************************
PFVoid PFC::_ModifyJacoByBusType(const int mode){
	PFUInt		index;
	PF_BUS_TYPE type;

	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		index = iter->IDN.index;
		type = iter->btype;
		switch(type){
		case PF_BUS_TYPE_VA:
			PFCJaco.coeffRef(index * 2    , index * 2    ) = BIGNUM;
			PFCJaco.coeffRef(index * 2 + 1, index * 2 + 1) = BIGNUM;

			break;
		case PF_BUS_TYPE_PV:
			if (mode == 0){
				PFCJaco.coeffRef(index * 2 + 1, index * 2 + 1) = BIGNUM;
			}
			break;
		}
	}
	if (mode == 1){
		_ModifyJacoQGElem();
	}
	return;
}

//����Լ���������ſɱ���dQG ��Ԫ��
PFVoid PFC::_ModifyJacoQGElem() {
	PFUInt inn;
	PFDouble vol, vset, qmax, qmin, qcal;
	PFDouble fi, dfq, dfv, sii, mii;
	PFDouble a1, a2, a3;
	PF_BUS_TYPE btype;
	const PFUInt csyssize = pSysData->GetSysSize();
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		btype = iter->btype;			//��ȡ�ڵ��������
		if (btype != PF_BUS_TYPE_PV){	//��PV�ڵ����˳�����
			continue;
		}

		//��ȡ��ز���
		inn = iter->IDN.index;
		assert(inn < csyssize);
		vol = iter->voltage.vol;
		vset = iter->voltage.GetVolSet();
		qmax = iter->gene.qHighLimit;
		qmin = iter->gene.qLowLimit;
		qcal = iter->gene.q;
		//���������У������ Sii �� Mii
		a1 = qmax - qcal;
		a2 = - LAMBDA * (vol - vset);
		fi = sqrt(a1 * a1 + a2 * a2) - a1 - a2;
		dfq = - a1 / (sqrt(a1 * a1 + a2 * a2)) + 1;
		a3 = qcal - qmin;
		sii = (a3 + fi * dfq) / (sqrt(a3 * a3 + fi * fi)) - 1 - dfq;

		dfv = (- LAMBDA * a2) / (sqrt(a1 * a1 + a2 * a2)) + LAMBDA;
		mii = ((fi * dfv) / (sqrt(a3 * a3 + fi * fi)) - dfv) * vol;

		//TODO:�����ſɱ��������
		PFCJaco.middleRows(inn * 2 + 1, 1) *= -sii;
		PFCJaco.coeffRef(inn * 2 + 1, inn * 2 + 1) += mii;	

	}

	return;
}


//************************************
// Method:    _ModifyDPQByBusType
// FullName:  PFCal::PFC::_ModifyDPQByBusType
// Access:    protected 
// Returns:   PFVoid
// Qualifier:
// Parameter: const int mode 
//			  mode = 0 : modify VA & PV bus
//			  mode = 1 : modify VA bus ONLY
//************************************
PFVoid PFC::_ModifyDPQByBusType(const int mode){
	PFUInt		index;
	PF_BUS_TYPE type;
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		index = iter->IDN.index;
		type = iter->btype;
		switch(type){
		case PF_BUS_TYPE_VA:
			dPFCPQ.coeffRef(index * 2    ) = 0.0;
			dPFCPQ.coeffRef(index * 2 + 1) = 0.0;
			break;
		case PF_BUS_TYPE_PV:
			dPFCPQ.coeffRef(index * 2 + 1) *= mode;
			break;
		}
	}
	return;
}
//************************************
// Method:    _UpdateSysState
// FullName:  PFCal::PFC::_UpdateSysState
// Access:    protected 
// Returns:   PFVoid
// Qualifier:
//************************************
PFVoid PFC::_UpdateSysState(){
	PFInt index;
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		index = iter->IDN.index;
		//Condition
		if(iter->btype == PF_BUS_TYPE_VA)
			continue;
		else if(iter->btype == PF_BUS_TYPE_PV) {
			cout << setw(5) << iter->IDN.index << setw(15) << iter->voltage.GetVolSet() << setw(15) << iter->voltage.vol << setw(15) << dPFCVA[index * 2 + 1] * iter->voltage.vol;
			//cout << iter->IDN.index << "\t" << iter->voltage.vol << "\t" << dPFCVA[index * 2 + 1] * iter->voltage.vol;
			iter->voltage.ang   -= dPFCVA[index * 2    ];
			iter->voltage.vol	-= dPFCVA[index * 2 + 1] * iter->voltage.vol;
			cout << setw(15) << iter->voltage.vol << endl;
			//cout << "\t" << iter->voltage.vol << endl;
		} else {
			iter->voltage.ang   -= dPFCVA[index * 2    ];
			iter->voltage.vol   -= dPFCVA[index * 2 + 1] * iter->voltage.vol;
		}
		//END_Condition
	}
	return;
}
//************************************
// Method:    SolvePFC
// FullName:  PFCal::PFC::SolvePFC
// Access:    public 
// Returns:   PF_RESULT
// Qualifier: ��������ģ�庯����������ַ���������Է�����
//************************************
PFVoid PFC::Solve(){
	PFDouble opml(0.0);
	Eigen::SparseLU<PFCore::PFSMatrixXD, PFCore::PFCOLAMDOrdering> solver;
	PFCIter = 0;
	//0. ��ʼ���������������С
	this->_InitPFCalMatrix();

	do {
		//1. ��⹦��ʧ����
		cout << "QG DISPATCH..." << endl;
		_MakeDPQVector();
		//2. ����ﵽ�����������˳�
		cout << "MAXDISPATCH\t" << PFCIter << "\t" << dPFCPQ.cwiseAbs().maxCoeff() << endl;
		if (dPFCPQ.cwiseAbs().maxCoeff() <  PFCEpsm){
			this->PFCResult = PF_RESULT_CONVERG;
			return;
		}
		//3. �����ſɱ���
		_MakeJacoMatrix();
		//4. �����ѹƫ��
		solver.compute(PFCJaco);
		if (solver.info() != Eigen::Success){
			cout << "Solving<stage_1> Failed!" << endl;
			this->PFCResult = PF_RESULT_DIVERGE_FAILURE;
			return;
		}
		dPFCVA = solver.solve(dPFCPQ);
		if (solver.info() != Eigen::Success){
			cout << "Solving<stage_2> Failed!" << endl;
			this->PFCResult = PF_RESULT_DIVERGE_FAILURE;
			return;
		}
		//5. ����Ƿ����NAN
		if(dPFCVA.hasNaN()){
			this->PFCResult = PF_RESULT_DIVERGE_NAN;
			return;
		}
		//6. ����״̬��
		cout << "VOL DISPATCH..." << endl;
		opml = _CalOptimalMultiplier();
		dPFCVA *= opml;
		_UpdateSysState();
		//7. ������������
		++PFCIter;
	} while (PFCIter <= PFCMaxIter);
	//��������Խ��
	if (PFCIter > PFCMaxIter){
		this->PFCResult = PF_RESULT_DIVERGE_OVER_ITER;
		return;
	}
	this->PFCResult = PF_RESULT_DIVERGE;
	return;
}

PFVoid PFC::_SwitchBType() {
	switch(this->PFCBtypeMethod){
	//�ڵ�����ת���߼�
	case PF_BUSTYPE_SWITCH_LOGIC:
		_ExecuteSwitchLogic();
		break;
	//����Լ��ת��
	case PF_BUSTYPE_SWITCH_COMPLEMENTARY:	
		_ExecuteSwitchComplementary();
		break;

	default:
		cout << "Error: Execute Unknown BusType Switch Method!!!" << endl;
	}
	return;
}

//ִ�нڵ�����ת���߼�
PFVoid PFC::_ExecuteSwitchLogic() {
	PFInt index;
	PF_BUS_TYPE btype0, btype;
	PFDouble qg, qd, qcal, qghigh, qglow, vset;
	PF_PQ_VIOLATION violation;
	//�����ڵ���ҷ�����ڵ�
	for (iter_PFBus iter = this->pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		btype0 = iter->GetOriBusType();
		//��ʼ���ͷ�PV���ͣ��򲻿���
		if (btype0 != PF_BUS_TYPE_PV){
			continue;
		}
		//�ýڵ�Ϊ������ڵ㣬���ȡ��ؼ�����
		index = iter->IDN.index;	//����ڵ�����
		btype = iter->btype;		//��ǰ�ڵ�����
		qg = iter->gene.q;
		qd = iter->load.q;
		qghigh = iter->gene.qHighLimit;
		qglow = iter->gene.qLowLimit;
		vset = iter->voltage.GetVolSet();
		qcal = dPFCPQ[index * 2 + 1];
		//��ʼ�ж�����ת��
		//�жϵ�ǰ�ڵ�����
		//��ǰ�ڵ�û�з�������ת��
		if (btype == PF_BUS_TYPE_PV){
			violation = _CheckPQViolation(qcal, qghigh, qglow);	//����Ƿ�Խ��
			switch(violation){
			case PF_PQ_VIOLATION_NORM:
				break;
			case PF_PQ_VIOLATION_HIGH:
				//Խ���� --> תΪPQ�ڵ㣬������޹�������Ϊ����
				iter->btype = PF_BUS_TYPE_PQ;
				iter->gene.q = iter->gene.qHighLimit;
				cout << setw(20) << "PV --> PQ" << setw(10) << "QCal" << setw(10) << "QMax" << endl;
				cout << setw(20) << iter->IDN.index << setw(10) << qcal << setw(10) << qghigh << endl;
				break;
			case PF_PQ_VIOLATION_LOW:
				//Խ���� --> תΪPQ�ڵ㣬������޹�������Ϊ����
				iter->btype = PF_BUS_TYPE_PQ;
				iter->gene.q = iter->gene.qLowLimit;
				cout << setw(20) << "PV --> PQ" << setw(10) << "QCal" << setw(10) << "QMin" << endl;
				cout << setw(20) << iter->IDN.index << setw(10) << qcal << setw(10) << qglow << endl;
				break;
			}
		} else if (btype == PF_BUS_TYPE_PQ){
			//��ǰ�ڵ��Ѿ�����ת����ΪPQ�ڵ�
			violation = _CheckPQViolation(qcal, qghigh, qglow);	//����Ƿ�Խ��
			switch(violation) {
			case PF_PQ_VIOLATION_NORM:
				//��Խ�� --> �ָ���ΪPV�ڵ�, �ڵ��ѹ��Ϊ�趨ֵ
				iter->btype = PF_BUS_TYPE_PV;
				iter->gene.q = qcal;		//�������Ҳû���ô�
				cout << setw(20) << "PQ --> PV" << setw(10) << "Vol" << setw(10) << "VSet" << endl;
				cout << setw(20) << iter->IDN.index << setw(10) << iter->voltage.vol << setw(10) << vset << endl;
				iter->voltage.vol = vset;		//�ָ�������趨��ѹ
				break;
			case PF_PQ_VIOLATION_HIGH:
				//Խ���� -->	������޹�������Ϊ����
				iter->gene.q = iter->gene.qHighLimit;
				cout << setw(20) << "PQ --> PQ" << setw(10) << "QCal" << setw(10) << "QMax" << endl;
				cout << setw(20) << iter->IDN.index << setw(10) << qcal << setw(10) << qghigh << endl;
				break;
			case PF_PQ_VIOLATION_LOW:
				//Խ���� -->	������޹�������Ϊ����
				iter->gene.q = iter->gene.qLowLimit;
				cout << setw(20) << "PQ --> PQ" << setw(10) << "QCal" << setw(10) << "QMin" << endl;
				cout << setw(20) << iter->IDN.index << setw(10) << qcal << setw(10) << qglow << endl;
				break;
			}
		}

	}
	return;
}

//ִ�л���Լ��ת���߼�
PFVoid PFC::_ExecuteSwitchComplementary() {
	PFUInt inn;
	PFDouble vol, vset, qmax, qmin, qcal, res;
	PFDouble a1, a2, a3, a4;
	PF_BUS_TYPE btype;
	const PFUInt sysSize = pSysData->GetSysSize();
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		btype = iter->btype;
		//��PV�ڵ㣬���˳�����
		if (btype != PF_BUS_TYPE_PV){
			continue;
		}
		//�����PV�ڵ�
		vol = iter->voltage.vol;
		vset = iter->voltage.GetVolSet();
		inn = iter->IDN.index;
		qmax = iter->gene.qHighLimit;
		qmin = iter->gene.qLowLimit;
		 
		assert(inn < sysSize);
		qcal = dPFCPQ[inn * 2 + 1];

		//���㻥������ֵ
		/************************************************************************
		*      f[a,b] = sqrt(a^2 + b^2) - a - b
		*	   F[v,a] = f[(qcal - qmin), f[(qmax - qcal), - lamda * (vol - vset)]]
		************************************************************************/
		a1 = - LAMBDA * (vol - vset);
		a2 = qmax - qcal;
		a3 = sqrt(a1 * a1 + a2 * a2) - a1 - a2;
		a4 = qcal - qmin;
		res = sqrt(a4 * a4 + a3 * a3) - a3 - a4;
		cout << setw(5) << iter->IDN.index << setw(15) << qcal << setw(15) << qmax << setw(15) << qmin << endl;
		//cout << iter->IDN.index << "\t" << qcal << "\t" << qmax << "\t" << qmin << endl;
		//��������Լ������ʧ��ֵ
		dPFCPQ[inn * 2 + 1] = res;
	}
	
	return;
}

PF_PQ_VIOLATION PFC::_CheckPQViolation(PFDouble val, PFDouble valhigh, PFDouble vallow){
	assert(valhigh >= vallow);
	if (val > valhigh){
		return PF_PQ_VIOLATION_HIGH;
	} else if (val < vallow){
		return PF_PQ_VIOLATION_LOW;
	} else {
		return PF_PQ_VIOLATION_NORM;
	}
}

//�������ų��� -- 1994 [CSEE] [������] ������ϵ׼���ų��Ӳ�̬�����ⷨ�о�
const PFDouble  PFC::_CalOptimalMultiplier(){
	PFDouble opml = 0.0;
	PFDouble g0, g1, g2, g3, g;
	PFCore::PFDoubleVec vec;
	PFUInt retValue;
	//const PFDouble csize = pSysData->GetSysSize();
	//�����м�����
	PFCore::PFVectorXD pfa = dPFCPQ;
	PFCore::PFVectorXD pfc;

	//���ݵ�ǰ��ѹ��Ϣ
	pSysData->BackupVoltage();
	//���µ�ѹ��Ϣ
	_UpdateSysState();
	//�����ѹ���º�Ĺ���ʧ����
	_MakeDPQVector();
	//�ָ���ǰ��ѹ��Ϣֵ
	pSysData->RewindVoltage();

	pfc = dPFCPQ;
	//����gi�� -- д���ˣ���������~~
	g0 = - pfa.cwiseAbs2().sum();
	//g3 = - 3 * g0;
	g3 = pfc.cwiseAbs2().sum() * 2;
	g = pfa.adjoint() * pfc;
	g1 = 2 * g - g0;
	g2 = - 3 * g;

	//��ⷽ��
	retValue = _CalFormularByShengJin(g3, g2, g1, g0, vec);
	//���ݸ�ֵȷ�����ų���
	opml = vec[0];
	//switch(retValue){
	//case 1:
	//	opml = vec[0];
	//	break;
	//case 2:
	//	opml = vec[0];
	//	break;
	//case 3:
	//	opml = vec[0];
	//	break;
	//case 4:
	//	opml = 1.0;
	//	break;
	//}

	return opml;
}

PFUInt PFC::_CalFormularByShengJin(const PFDouble g3, const PFDouble g2, const PFDouble g1, const PFDouble g0, PFCore::PFDoubleVec& result){
	PFUInt ret;
	PFDouble A, B, C, Delta;
	PFDouble x1, x2, x3;
	PFDouble y1, y2, k, theta, t;
	PFDouble a1, a2, a3;
	//������ų��� -- ���һԪ���η��̲��� ʢ��ʽ
	A = g2 * g2 - 3 * g3 * g1;
	B = g2 * g1 - 9 * g3 * g0;
	C = g1 * g1 - 3 * g2 * g0;
	Delta = B * B - 4 * A * C;

	if (A == 0.0 && B == 0.0){//ʢ��ʽ1 -- ����ʵ��
		x1 = x2 = x3 = - g1 / g2;
		ret = 1;
	} else if (Delta > 0){//ʢ��ʽ2 -- һ��ʵ����һ�Թ������
		ret = 2;
		y1 = A * g2 + 3 * g3 * (- B + sqrt(Delta)) / 2;
		y2 = A * g2 + 3 * g3 * (- B - sqrt(Delta)) / 2;

		//a1 = pow(y1, 1.0/3);
		//a2 = pow(y2, 1.0/3);
		a1 = (y1 >= 0.0 ? pow(y1, 1.0/3) : - pow(-y1, 1.0/3));
		a2 = (y2 >= 0.0 ? pow(y2, 1.0/3) : - pow(-y2, 1.0/3));
		//a3 = pow(3.0, 1.0/3);
		a3 = sqrt(3.0);

		x1 = - (g2 + a1 + a2) / (3 * g3);
		x2 = (- 2 * g2 + a1 + a2) / (6 * g3);
		x3 = a3 * (a1 - a2) / (6 * g3);
	} else if (Delta == 0){//ʢ��ʽ3 -- ����ʵ��������һ�����ظ�
		ret = 3;
		k = B / A;
		x1 = - g2 / g3 + k;
		x2 = x3 = - k / 2;
	} else if (Delta < 0){//ʢ��ʽ4 -- ��������ʵ��
		ret = 4;
		t = (2 * A * g2 - 3 * g3 * B) / (sqrt(A * A * A) * 2);
		theta = acos(t);
		x1 = - (g2 + 2 * sqrt(A) * cos(theta / 3)) / (3 * g3);
		x2 = (- g2 + sqrt(A) * (cos(theta / 3) + sqrt((PFDouble)3.0) * sin(theta / 3))) / (3 * g3);
		x3 = (- g2 + sqrt(A) * (cos(theta / 3) - sqrt((PFDouble)3.0) * sin(theta / 3))) / (3 * g3);
	}
	//���湫ʽ��
	result.clear();
	result.push_back(x1);
	result.push_back(x2);
	result.push_back(x3);

	return ret;
}
