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
	this->pSysData = NULL;					//系统数据指针设为空
	this->PFCBTypeSwitch = false;
	this->PFCBTypeSwitch = PF_BUSTYPE_SWITCH_LOGIC;
	this->PFCResult = PF_RESULT_DIVERGE;
}

//复制构造函数
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
// Qualifier: 系统数据指针
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
// Qualifier: 系统数据引用
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
// Qualifier: 由于采用组合的构造方法，将系统数据和计算矩阵|方法隔离相分隔开，则计算类中不涉及系统数据类的内存释放。
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
	PFCore::PFTripletDVec triplist, tmplist;//生成矩阵的基础元素
	//遍历支路增加雅可比阵元素
	for (iter_PFBranch iter = pSysData->PFSysBranch.begin();iter != pSysData->PFSysBranch.end();++iter){
		tmplist = iter->ComputeJacoElement();							//计算支路构成的雅可比阵元素
		triplist.insert(triplist.end(), tmplist.begin(), tmplist.end());//加入构建列
	}
	//遍历节点增加雅可比阵元素
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		tmplist = iter->ComputeJacoElement();
		triplist.insert(triplist.end(), tmplist.begin(), tmplist.end());
	}
	//形成雅可比阵
	this->PFCJaco.setFromTriplets(triplist.begin(), triplist.end());

	//修正雅可比阵
	_ModifyJaco();

	return;
}
//修正雅可比阵
PFVoid PFC::_ModifyJaco(){
	//节点类型修正矩阵
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
	//首先鉴定是否需要节点类型转换
	if (!PFCBTypeSwitch){
		//不需要节点类型转换
		_MakeDPQVectorByBranch();
		_MakeDPQVectorByBus();
		_ModifyDPQByBusType(0);		//根据节点类型修正功率失配量
	} else {
		//需要节点类型转换
		switch(PFCBtypeMethod){
		case PF_BUSTYPE_SWITCH_LOGIC:	//节点类型转换逻辑
			_MakeDQVector();			//生成QGCal向量，判断是否越限
			_SwitchBType();				//执行节点类型转换逻辑

			_MakeDPQVectorByBranch();	//计算支路功率失配
			_MakeDPQVectorByBus();		//计算节点功率失配
			_ModifyDPQByBusType(0);		//根据节点类型修正功率失配量
			break;
		case PF_BUSTYPE_SWITCH_COMPLEMENTARY:	//互补约束转换
			_MakeDPQVectorByBranch();			//计算支路失配
			_MakeDPQCalByBus();					//计算节点功率失配，不包含机组出力
			//TODO: 执行互补约束光滑方程计算
			_SwitchBType();
			//TODO: 增加节点功率出力，构建完备功率失配量
			_MakeDPQCalByGene();
			_ModifyDPQByBusType(1);		//根据节点类型修正功率失配量
			break;
		}
	}

	/************************************************************************/
	/* 常规操作                                                             */
	/************************************************************************/
	//if (PFCBTypeSwitch && 
	//	PFCBtypeMethod == PF_BUSTYPE_SWITCH_LOGIC){
	//	_MakeDQVector();
	//	_SwitchBType();
	//}
	////转换完成之后，重新计算功率失配量
	//_MakeDPQVectorByBranch();
	//_MakeDPQVectorByBus();
	return;
}

//构造无功功率失配量
PFVoid PFC::_MakeDQVector(){
	PFInt inn, jnn;
	PFDoubleVec tmplist;
	PFDouble qbus;
	const PFInt csize = this->pSysData->GetSysSize();

	dPFCPQ.setZero();//功率失配量清零
	assert(dPFCPQ.size() == csize * 2);//确保向量维数一致
	//遍历支路
	for (iter_PFBranch iter = pSysData->PFSysBranch.begin();iter != pSysData->PFSysBranch.end();++iter){
		inn = iter->inn;
		jnn = iter->jnn;
		assert(inn < csize && jnn < csize);
		tmplist = iter->ComputeDQCal();
		assert(tmplist.size() == 2);
		dPFCPQ[inn * 2 + 1]     += tmplist[0];
		dPFCPQ[jnn * 2 + 1]     += tmplist[1];
	}
	//遍历节点
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		inn = iter->IDN.index;
		qbus = iter->ComputeDQCal();
		dPFCPQ[inn * 2 + 1] += qbus;
	}
	return;
}

//构建功率失配计算量，不包含机组出力和负荷
PFVoid PFC::_MakeDPQCalByBus() {
	PFInt inn;
	PFDoubleVec tmplist;
	//遍历节点
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		inn = iter->IDN.index;
		tmplist = iter->ComputeDPQCal();
		assert(tmplist.size() == 2);
		dPFCPQ[inn * 2]     += tmplist[0];
		dPFCPQ[inn * 2 + 1] += tmplist[1];

		//在互补约束中这个Qcal的值是需要用到的，在这里就采用gene.q来记录
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

	dPFCPQ.setZero();//功率失配量清零
	assert(dPFCPQ.size() == csize * 2);//确保向量维数一致
	//遍历支路
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
	//遍历节点
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		inn = iter->IDN.index;
		tmplist = iter->ComputeDPQElement();
		assert(tmplist.size() == 2);
		dPFCPQ[inn * 2]     = tmplist[0] - dPFCPQ[inn * 2];
		dPFCPQ[inn * 2 + 1] = tmplist[1] - dPFCPQ[inn * 2 + 1];
	}
	return;
}
//在功率失配量的基础上增加机组出力
PFVoid PFC::_MakeDPQCalByGene() {
	PFInt inn;
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		inn = iter->IDN.index;
		if (iter->btype != PF_BUS_TYPE_PV){
			//常规节点类型
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
//			  mode = 0: 修正VA节点(加大数)，PV节点(DQ加大数)
//			  mode = 1: 修正VA节点(加大数)，PV节点(互补约束参数修正)
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

//互补约束中修正雅可比阵dQG 行元素
PFVoid PFC::_ModifyJacoQGElem() {
	PFUInt inn;
	PFDouble vol, vset, qmax, qmin, qcal;
	PFDouble fi, dfq, dfv, sii, mii;
	PFDouble a1, a2, a3;
	PF_BUS_TYPE btype;
	const PFUInt csyssize = pSysData->GetSysSize();
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		btype = iter->btype;			//获取节点计算类型
		if (btype != PF_BUS_TYPE_PV){	//非PV节点则退出继续
			continue;
		}

		//获取相关参数
		inn = iter->IDN.index;
		assert(inn < csyssize);
		vol = iter->voltage.vol;
		vset = iter->voltage.GetVolSet();
		qmax = iter->gene.qHighLimit;
		qmin = iter->gene.qLowLimit;
		qcal = iter->gene.q;
		//计算所需的校正因子 Sii 和 Mii
		a1 = qmax - qcal;
		a2 = - LAMBDA * (vol - vset);
		fi = sqrt(a1 * a1 + a2 * a2) - a1 - a2;
		dfq = - a1 / (sqrt(a1 * a1 + a2 * a2)) + 1;
		a3 = qcal - qmin;
		sii = (a3 + fi * dfq) / (sqrt(a3 * a3 + fi * fi)) - 1 - dfq;

		dfv = (- LAMBDA * a2) / (sqrt(a1 * a1 + a2 * a2)) + LAMBDA;
		mii = ((fi * dfv) / (sqrt(a3 * a3 + fi * fi)) - dfv) * vol;

		//TODO:修正雅可比阵相关行
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
// Qualifier: 潮流计算模板函数，构造各种方法求解线性方程组
//************************************
PFVoid PFC::Solve(){
	PFDouble opml(0.0);
	Eigen::SparseLU<PFCore::PFSMatrixXD, PFCore::PFCOLAMDOrdering> solver;
	PFCIter = 0;
	//0. 初始化计算矩阵及向量大小
	this->_InitPFCalMatrix();

	do {
		//1. 求解功率失配量
		cout << "QG DISPATCH..." << endl;
		_MakeDPQVector();
		//2. 如果达到收敛条件，退出
		cout << "MAXDISPATCH\t" << PFCIter << "\t" << dPFCPQ.cwiseAbs().maxCoeff() << endl;
		if (dPFCPQ.cwiseAbs().maxCoeff() <  PFCEpsm){
			this->PFCResult = PF_RESULT_CONVERG;
			return;
		}
		//3. 计算雅可比阵
		_MakeJacoMatrix();
		//4. 计算电压偏差
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
		//5. 检测是否出现NAN
		if(dPFCVA.hasNaN()){
			this->PFCResult = PF_RESULT_DIVERGE_NAN;
			return;
		}
		//6. 更新状态量
		cout << "VOL DISPATCH..." << endl;
		opml = _CalOptimalMultiplier();
		dPFCVA *= opml;
		_UpdateSysState();
		//7. 迭代次数增加
		++PFCIter;
	} while (PFCIter <= PFCMaxIter);
	//迭代次数越限
	if (PFCIter > PFCMaxIter){
		this->PFCResult = PF_RESULT_DIVERGE_OVER_ITER;
		return;
	}
	this->PFCResult = PF_RESULT_DIVERGE;
	return;
}

PFVoid PFC::_SwitchBType() {
	switch(this->PFCBtypeMethod){
	//节点类型转换逻辑
	case PF_BUSTYPE_SWITCH_LOGIC:
		_ExecuteSwitchLogic();
		break;
	//互补约束转换
	case PF_BUSTYPE_SWITCH_COMPLEMENTARY:	
		_ExecuteSwitchComplementary();
		break;

	default:
		cout << "Error: Execute Unknown BusType Switch Method!!!" << endl;
	}
	return;
}

//执行节点类型转换逻辑
PFVoid PFC::_ExecuteSwitchLogic() {
	PFInt index;
	PF_BUS_TYPE btype0, btype;
	PFDouble qg, qd, qcal, qghigh, qglow, vset;
	PF_PQ_VIOLATION violation;
	//遍历节点查找发电机节点
	for (iter_PFBus iter = this->pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		btype0 = iter->GetOriBusType();
		//初始类型非PV类型，则不考虑
		if (btype0 != PF_BUS_TYPE_PV){
			continue;
		}
		//该节点为发电机节点，则获取相关计算量
		index = iter->IDN.index;	//计算节点索引
		btype = iter->btype;		//当前节点类型
		qg = iter->gene.q;
		qd = iter->load.q;
		qghigh = iter->gene.qHighLimit;
		qglow = iter->gene.qLowLimit;
		vset = iter->voltage.GetVolSet();
		qcal = dPFCPQ[index * 2 + 1];
		//开始判断类型转换
		//判断当前节点类型
		//当前节点没有发生类型转换
		if (btype == PF_BUS_TYPE_PV){
			violation = _CheckPQViolation(qcal, qghigh, qglow);	//检测是否越限
			switch(violation){
			case PF_PQ_VIOLATION_NORM:
				break;
			case PF_PQ_VIOLATION_HIGH:
				//越上限 --> 转为PQ节点，发电机无功出力定为上限
				iter->btype = PF_BUS_TYPE_PQ;
				iter->gene.q = iter->gene.qHighLimit;
				cout << setw(20) << "PV --> PQ" << setw(10) << "QCal" << setw(10) << "QMax" << endl;
				cout << setw(20) << iter->IDN.index << setw(10) << qcal << setw(10) << qghigh << endl;
				break;
			case PF_PQ_VIOLATION_LOW:
				//越下限 --> 转为PQ节点，发电机无功出力定为下限
				iter->btype = PF_BUS_TYPE_PQ;
				iter->gene.q = iter->gene.qLowLimit;
				cout << setw(20) << "PV --> PQ" << setw(10) << "QCal" << setw(10) << "QMin" << endl;
				cout << setw(20) << iter->IDN.index << setw(10) << qcal << setw(10) << qglow << endl;
				break;
			}
		} else if (btype == PF_BUS_TYPE_PQ){
			//当前节点已经发生转换，为PQ节点
			violation = _CheckPQViolation(qcal, qghigh, qglow);	//检测是否越限
			switch(violation) {
			case PF_PQ_VIOLATION_NORM:
				//不越限 --> 恢复成为PV节点, 节点电压定为设定值
				iter->btype = PF_BUS_TYPE_PV;
				iter->gene.q = qcal;		//这里计算也没有用处
				cout << setw(20) << "PQ --> PV" << setw(10) << "Vol" << setw(10) << "VSet" << endl;
				cout << setw(20) << iter->IDN.index << setw(10) << iter->voltage.vol << setw(10) << vset << endl;
				iter->voltage.vol = vset;		//恢复发电机设定电压
				break;
			case PF_PQ_VIOLATION_HIGH:
				//越上限 -->	发电机无功出力定为上限
				iter->gene.q = iter->gene.qHighLimit;
				cout << setw(20) << "PQ --> PQ" << setw(10) << "QCal" << setw(10) << "QMax" << endl;
				cout << setw(20) << iter->IDN.index << setw(10) << qcal << setw(10) << qghigh << endl;
				break;
			case PF_PQ_VIOLATION_LOW:
				//越下限 -->	发电机无功出力定为下限
				iter->gene.q = iter->gene.qLowLimit;
				cout << setw(20) << "PQ --> PQ" << setw(10) << "QCal" << setw(10) << "QMin" << endl;
				cout << setw(20) << iter->IDN.index << setw(10) << qcal << setw(10) << qglow << endl;
				break;
			}
		}

	}
	return;
}

//执行互补约束转换逻辑
PFVoid PFC::_ExecuteSwitchComplementary() {
	PFUInt inn;
	PFDouble vol, vset, qmax, qmin, qcal, res;
	PFDouble a1, a2, a3, a4;
	PF_BUS_TYPE btype;
	const PFUInt sysSize = pSysData->GetSysSize();
	for (iter_PFBus iter = pSysData->PFSysBus.begin();iter != pSysData->PFSysBus.end();++iter){
		btype = iter->btype;
		//非PV节点，则退出继续
		if (btype != PF_BUS_TYPE_PV){
			continue;
		}
		//发电机PV节点
		vol = iter->voltage.vol;
		vset = iter->voltage.GetVolSet();
		inn = iter->IDN.index;
		qmax = iter->gene.qHighLimit;
		qmin = iter->gene.qLowLimit;
		 
		assert(inn < sysSize);
		qcal = dPFCPQ[inn * 2 + 1];

		//计算互补函数值
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
		//给定互补约束函数失配值
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

//计算最优乘子 -- 1994 [CSEE] [王宪荣] 极坐标系准最优乘子病态潮流解法研究
const PFDouble  PFC::_CalOptimalMultiplier(){
	PFDouble opml = 0.0;
	PFDouble g0, g1, g2, g3, g;
	PFCore::PFDoubleVec vec;
	PFUInt retValue;
	//const PFDouble csize = pSysData->GetSysSize();
	//声明中间向量
	PFCore::PFVectorXD pfa = dPFCPQ;
	PFCore::PFVectorXD pfc;

	//备份当前电压信息
	pSysData->BackupVoltage();
	//更新电压信息
	_UpdateSysState();
	//计算电压更新后的功率失配量
	_MakeDPQVector();
	//恢复此前电压信息值
	pSysData->RewindVoltage();

	pfc = dPFCPQ;
	//计算gi量 -- 写错了，明天来改~~
	g0 = - pfa.cwiseAbs2().sum();
	//g3 = - 3 * g0;
	g3 = pfc.cwiseAbs2().sum() * 2;
	g = pfa.adjoint() * pfc;
	g1 = 2 * g - g0;
	g2 = - 3 * g;

	//求解方程
	retValue = _CalFormularByShengJin(g3, g2, g1, g0, vec);
	//根据根值确定最优乘子
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
	//求解最优乘子 -- 求解一元三次方程采用 盛金公式
	A = g2 * g2 - 3 * g3 * g1;
	B = g2 * g1 - 9 * g3 * g0;
	C = g1 * g1 - 3 * g2 * g0;
	Delta = B * B - 4 * A * C;

	if (A == 0.0 && B == 0.0){//盛金公式1 -- 三重实根
		x1 = x2 = x3 = - g1 / g2;
		ret = 1;
	} else if (Delta > 0){//盛金公式2 -- 一个实根和一对共轭虚根
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
	} else if (Delta == 0){//盛金公式3 -- 三个实根，其中一对两重根
		ret = 3;
		k = B / A;
		x1 = - g2 / g3 + k;
		x2 = x3 = - k / 2;
	} else if (Delta < 0){//盛金公式4 -- 三个不等实根
		ret = 4;
		t = (2 * A * g2 - 3 * g3 * B) / (sqrt(A * A * A) * 2);
		theta = acos(t);
		x1 = - (g2 + 2 * sqrt(A) * cos(theta / 3)) / (3 * g3);
		x2 = (- g2 + sqrt(A) * (cos(theta / 3) + sqrt((PFDouble)3.0) * sin(theta / 3))) / (3 * g3);
		x3 = (- g2 + sqrt(A) * (cos(theta / 3) - sqrt((PFDouble)3.0) * sin(theta / 3))) / (3 * g3);
	}
	//保存公式解
	result.clear();
	result.push_back(x1);
	result.push_back(x2);
	result.push_back(x3);

	return ret;
}
