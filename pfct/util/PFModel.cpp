#include "PFModel.h"

#include <sstream>
#include <exception>
#include <iomanip>
#include <assert.h>
#include <math.h>
#include <ios>
using namespace std;
using namespace PFModel;

//************************************
// Method:    BUS
// FullName:  PFModel::BUS::BUS
// Access:    public 
// Returns:   
// Qualifier:
//************************************
BUS::BUS(){
	this->btype = PF_BUS_TYPE_PQ;
	this->btype_bak = PF_BUS_TYPE_PQ;
	this->nArea = 0;
	this->pArea = NULL;
}
//************************************
// Method:    BUS
// FullName:  PFModel::BUS::BUS
// Access:    public 
// Returns:   
// Qualifier:
// Parameter: const BUS & bus
//************************************
BUS::BUS(const BUS& bus){
	this->IDN = bus.IDN;
	this->voltage = bus.voltage;
	this->para = bus.para;
	this->btype = bus.btype;
	this->btype_bak = bus.btype_bak;
	this->load = bus.load;
	this->gene = bus.gene;
	this->nArea   = bus.nArea;
	this->pArea   = bus.pArea;
	this->shunt   = bus.shunt;

}
//************************************
// Method:    operator<<
// FullName:  PFModel::operator<<
// Access:    public 
// Returns:   std::ostream&
// Qualifier:
// Parameter: std::ostream & dc
// Parameter: const BUS & bus
//************************************
std::ostream& PFModel::operator<<(std::ostream& dc, const BUS& bus){
	using namespace std;
	return dc << setiosflags(ios::left) << setw(30)<< bus.IDN.name;
}
//************************************
// Method:    Clear
// FullName:  PFModel::BUS::Clear
// Access:    public 
// Returns:   PFVoid
// Qualifier:
//************************************
PFVoid BUS::Clear(){
	this->IDN.Clear();
	this->voltage.Clear();
	this->load.Clear();
	this->gene.Clear();
	this->para.Clear();
	this->btype = PF_BUS_TYPE_PQ;
	this->btype_bak = PF_BUS_TYPE_PQ;
	this->nArea = 0;
	this->pArea = NULL;
	this->shunt.Clear();

}
//************************************
// Method:    ComputeJacoElement
// FullName:  PFModel::BUS::ComputeJacoElement
// Access:    public 
// Returns:   PFCore::PFTripletDVec
// Qualifier:
//************************************
PFCore::PFTripletDVec BUS::ComputeJacoElement(){
	PFCore::PFTripletDVec result;
	PFDouble vi, gii, bii;
	
	PFInt index = this->IDN.index;

	vi  = this->voltage.vol;
	gii = this->para.g;
	bii = this->para.b;

	result.push_back(PFCore::PFTripletD(index * 2    , index * 2 + 1, - 2 * vi * vi * gii));
	result.push_back(PFCore::PFTripletD(index * 2 + 1, index * 2 + 1,   2 * vi * vi * bii));
	return result;
}
//************************************
// Method:    ComputeDPQElement
// FullName:  PFModel::BUS::ComputeDPQElement
// Access:    public 
// Returns:   PFCore::PFDoubleVec
// Qualifier:
//************************************
PFCore::PFDoubleVec	  BUS::ComputeDPQElement(){
	PFCore::PFDoubleVec result;
	PFDouble v2 = this->voltage.vol * this->voltage.vol;
	PFDouble pis = this->gene.p - this->load.p - v2 * this->para.g;
	PFDouble qis = this->gene.q - this->load.q + v2 * this->para.b;
	result.push_back(pis);
	result.push_back(qis);
	return result;
}
//计算无功功率失配量
PFDouble BUS::ComputeDQCal() {
	PFDouble v2 = this->voltage.vol * this->voltage.vol;
	PFDouble qis = this->load.q - v2 * this->para.b;
	return qis;
}

//计算功率计算是配量，不包含机组出力
PFCore::PFDoubleVec BUS::ComputeDPQCal(){
	PFCore::PFDoubleVec result;
	PFDouble v2 = this->voltage.vol * this->voltage.vol;
	PFDouble pis = this->load.p + v2 * this->para.g;
	PFDouble qis = this->load.q - v2 * this->para.b;
	result.push_back(pis);
	result.push_back(qis);
	return result;
}

/*****************************************************************************
*								BRANCH
*****************************************************************************/
//************************************
// Method:    BRANCH
// FullName:  PFModel::BRANCH::BRANCH
// Access:    public 
// Returns:   
// Qualifier:
//************************************
BRANCH::BRANCH(){
	this->btype = PF_BRANCH_NULL;
	this->inn = PF_BUS_INDEX_DEFAULT;
	this->jnn = PF_BUS_INDEX_DEFAULT;
	this->pFromBus = NULL;
	this->pEndBus  = NULL;
	this->tap.SetVal(1.0);
}
//************************************
// Method:    ~BRANCH
// FullName:  PFModel::BRANCH::~BRANCH
// Access:    public 
// Returns:   
// Qualifier:
//************************************
BRANCH::~BRANCH(){
	this->Clear();
}

//************************************
// Method:    Clear
// FullName:  PFModel::BRANCH::Clear
// Access:    public 
// Returns:   PFVoid
// Qualifier: Clear all the member in the class BRANCH
//************************************
PFVoid BRANCH::Clear(){
	this->IDN.Clear();
	this->btype = PF_BRANCH_NULL;
	this->pFromBus = NULL;
	this->pEndBus  = NULL;
	this->pqij.Clear();
	this->pqji.Clear();
	this->para.Clear();
	this->tap.SetVal(1.0);
}
//************************************
// Method:    ComputeYGB
// FullName:  PFModel::BRANCH::ComputeYGB
// Access:    public 
// Returns:   PFVoid
// Qualifier: Compute the relative Y|G|B element of this branch
//************************************
PFVoid BRANCH::ComputeYGBElement(){
	PFDouble g(0.0), b(0.0), gb(0.0), t(0.0);
	PFDouble a1, a2;
	this->para.Calculate();
	g = this->para.g;
	b = this->para.b;
	//支路类型分析
	if (this->btype == PF_BRANCH_TRAN)
	{
		t = this->tap.val;
		assert(t > 0.001);
		a1 = (1 - t) / (t * t);
		a2 = (t - 1) / t;
		this->para.g = -g / t;
		this->para.b = -b / t;
		this->pFromBus->para.g += (g / t + a1 * g);
		this->pFromBus->para.b += (b / t + a1 * b);
		this->pEndBus->para.g  += (g / t + a2 * g);
		this->pEndBus->para.b  += (b / t + a2 * b);
		this->pFromBus->para.b += this->para.GetBch();
		this->pEndBus->para.b  += this->para.GetBch();
	}
	else
	{
		this->para.g = -g;
		this->para.b = -b;
		this->pFromBus->para.g += g;
		this->pFromBus->para.b += b;
		this->pEndBus->para.g  += g;
		this->pEndBus->para.b  += b;
		this->pFromBus->para.b += this->para.GetBch();
		this->pEndBus->para.b  += this->para.GetBch();

	}
	return;
}
//************************************
// Method:    ComputeJacoElement
// FullName:  PFModel::BRANCH::ComputeJacoElement
// Access:    public 
// Returns:   std::vector<PFCore::PFTripletD>
// Qualifier: 计算雅可比阵元素
//************************************
PFCore::PFTripletDVec BRANCH::ComputeJacoElement(){
	PFDouble da, vi, vj, gij, bij;
	PFDouble gijsin, bijcos, gijcos, bijsin;
	PFDouble gjisin, bjicos, gjicos, bjisin;
	PFCore::PFTripletDVec result;
	
	gij = this->para.g;
	bij = this->para.b;
	vi = pFromBus->voltage.vol;
	vj = pEndBus->voltage.vol;
	da = pFromBus->voltage.ang - pEndBus->voltage.ang;
	//上三角元素
	gijsin = gij * sin(da) * vi * vj;
	bijcos = bij * cos(da) * vi * vj;
	gijcos = gij * cos(da) * vi * vj;
	bijsin = bij * sin(da) * vi * vj;

	result.push_back(PFCore::PFTripletD(inn * 2    , jnn * 2    , - gijsin + bijcos));//Hij = dPi/dAj = - Vi * Vj * [Gij * sin(Aij) - Bij * cos(Aij)]
	result.push_back(PFCore::PFTripletD(inn * 2    , jnn * 2 + 1, - gijcos - bijsin));//Nij = (dPi/dVj) * Vj = - Vi * Vj * [Gij * cos(Aij) + Bij * sin(Aij)]
	result.push_back(PFCore::PFTripletD(inn * 2 + 1, jnn * 2    ,   gijcos + bijsin));//Jij = dQi/dAj =   Vi * Vj * [Gij * cos(Aij) + Bij * sin(Aij)]
	result.push_back(PFCore::PFTripletD(inn * 2 + 1, jnn * 2 + 1, - gijsin + bijcos));//Lij = (dQi/dVj) * Vj = - Vi * Vj * [Gij * sin(Aij) - Bij * cos(Aij)]
	//首端对角元素
	result.push_back(PFCore::PFTripletD(inn * 2    , inn * 2    ,   gijsin - bijcos));//Hii = SUM{Vi * Vj * [Gij * sin(Aij) - Bij * cos(Aij)]} = SUM{- Hij} (i<>j&i^j)
	result.push_back(PFCore::PFTripletD(inn * 2    , inn * 2 + 1, - gijcos - bijsin));//Nii = SUM{- Vi * Vj * [Gij * cos(Aij) + Bij * sin(Aij)]} = SUM{Nij} (i<>j&i^j)
	result.push_back(PFCore::PFTripletD(inn * 2 + 1, inn * 2    , - gijcos - bijsin));//Jii = SUM{- Vi * Vj * [Gij * cos(Aij) + Bij * sin(Aij)]} = SUM{-Jij}(i<>j&i^j)
	result.push_back(PFCore::PFTripletD(inn * 2 + 1, inn * 2 + 1, - gijsin + bijcos));//Lii = SUM{- Vi * Vj * [Gij * sin(Aij) - Bij * cos(Aij)]} = SUM{Lij} (i<>j&i^j)
	//下三角元素
	gjisin = - gijsin;
	gjicos =   gijcos;
	bjisin = - bijsin;
	bjicos =   bijcos;

	result.push_back(PFCore::PFTripletD(jnn * 2    , inn * 2    , - gjisin + bjicos));
	result.push_back(PFCore::PFTripletD(jnn * 2    , inn * 2 + 1, - gjicos - bjisin));
	result.push_back(PFCore::PFTripletD(jnn * 2 + 1, inn * 2    ,   gjicos + bjisin));
	result.push_back(PFCore::PFTripletD(jnn * 2 + 1, inn * 2 + 1, - gjisin + bjicos));
	//末端对角元素
	result.push_back(PFCore::PFTripletD(jnn * 2    , jnn * 2    ,   gjisin - bjicos));
	result.push_back(PFCore::PFTripletD(jnn * 2    , jnn * 2 + 1, - gjicos - bjisin));
	result.push_back(PFCore::PFTripletD(jnn * 2 + 1, jnn * 2    , - gjicos - bjisin));
	result.push_back(PFCore::PFTripletD(jnn * 2 + 1, jnn * 2 + 1, - gjisin + bjicos));

	return result;
}
//************************************
// Method:    ComputeDPQElement
// FullName:  PFModel::BRANCH::ComputeDPQElement
// Access:    public 
// Returns:   PFCore::PFDoubleVec
// Qualifier:
//************************************
PFCore::PFDoubleVec	  BRANCH::ComputeDPQElement(){
	PFCore::PFDoubleVec result;
	PFDouble da, vi, vj, gij, bij;
	PFDouble pi, qi, pj, qj;

	da = pFromBus->voltage.ang - pEndBus->voltage.ang;
	vi = pFromBus->voltage.vol;
	vj = pEndBus->voltage.vol;
	gij = this->para.g;
	bij = this->para.b;

	pi =   vi * vj * (gij * cos(da) + bij * sin(da));
	qi =   vi * vj * (gij * sin(da) - bij * cos(da));
	pj =   vj * vi * (gij * cos(da) - bij * sin(da));
	qj = - vj * vi * (gij * sin(da) + bij * cos(da));
	
	result.push_back(pi);
	result.push_back(qi);
	result.push_back(pj);
	result.push_back(qj);

	return result;
}
//计算无功失配量
PFCore::PFDoubleVec	BRANCH::ComputeDQCal(){
	PFCore::PFDoubleVec result;
	PFDouble da, vi, vj, gij, bij;
	PFDouble qi, qj;

	da = pFromBus->voltage.ang - pEndBus->voltage.ang;
	vi = pFromBus->voltage.vol;
	vj = pEndBus->voltage.vol;
	gij = this->para.g;
	bij = this->para.b;

	qi =   vi * vj * (gij * sin(da) - bij * cos(da));
	qj = - vj * vi * (gij * sin(da) + bij * cos(da));

	result.push_back(qi);
	result.push_back(qj);

	return result;

}


//************************************
// Method:    AREA
// FullName:  PFModel::AREA::AREA
// Access:    public 
// Returns:   
// Qualifier:
//************************************
AREA::AREA(){
	this->name = "";
	this->gene_num = 0;
	this->load_num = 0;
}


/************************************************************************/
/*                              PFSys                                   */
/************************************************************************/

//************************************
// Method:    PFSys
// FullName:  PFModel::PFSys::PFSys
// Access:    public 
// Returns:   
// Qualifier:
//************************************
PFSys::PFSys(){
	this->PFSysSize = 0;
	this->PFSysBus.clear();
	this->PFSysBranch.clear();
	this->PFSysArea.clear();
}
//************************************
// Method:    ~PFSys
// FullName:  PFModel::PFSys::~PFSys
// Access:    public 
// Returns:   
// Qualifier:
//************************************
PFSys::~PFSys(){
	this->PFSysSize = 0;
}

PFDouble PFSys::SysBaseMVA = 100.0;

//************************************
// Method:    SetPFSysBaseMVA
// FullName:  PFModel::PFSys::SetPFSysBaseMVA
// Access:    public static 
// Returns:   PFVoid
// Qualifier:
// Parameter: PFDouble base
//************************************
PFVoid PFSys::SetPFSysBaseMVA(PFDouble base){
	PFSys::SysBaseMVA = base;
}
//************************************
// Method:    GetPFSysBaseMVA
// FullName:  PFModel::PFSys::GetPFSysBaseMVA
// Access:    public static 
// Returns:   PFDouble
// Qualifier:
//************************************
PFDouble PFSys::GetPFSysBaseMVA(){
	return PFSys::SysBaseMVA;
}
//************************************
// Method:    SolveYGBMatrix
// FullName:  PFModel::PFSys::SolveYGBMatrix
// Access:    public 
// Returns:   PFVoid
// Qualifier: Solve the element of Y|G|B, and save them into BUS&BRANCH
//************************************
PFVoid PFSys::SolveYGBMatrix(){
	try
	{
		//遍历支路形成支路节点导纳元素
		for (iter_PFBranch iter = this->PFSysBranch.begin();iter != this->PFSysBranch.end();++iter)
			iter->ComputeYGBElement();
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return;
}
//************************************
// Method:    GetSysSize
// FullName:  PFModel::PFSys::GetSysSize
// Access:    public 
// Returns:   PFUInt
// Qualifier: Calculate the size of system
//************************************
PFUInt PFSys::GetSysSize(){
	PFUInt size(0);
	for (iter_PFBus iter = PFSysBus.begin();iter != PFSysBus.end();++iter){
		//Condition
		++size;
	}
	return size;
}

//************************************
// Method:    PFSys::_GetSysDataByFile_IEEE
// FullName:  PFSys::_GetSysDataByFile_IEEE
// Access:    public 
// Returns:   PFVoid
// Qualifier: 读取IEEE标准数据
// Parameter: std::string & filepath
//************************************
PFVoid PFSys::_GetSysDataByFile_IEEE(std::string& filepath){
	ifstream fin;
	fin.open(filepath.c_str(), ios::in);//打开文件
	if (fin == NULL)//文件打开情况校验，如果没有打开则发出异常
	{
		throw exception("ERROR: Failure in Opening & Reading file in PFSys::_GetSysDataByFile_IEEE!");
	}
	//
	PFStr tmpstr;
	while(!fin.eof()){
		fin >> tmpstr;
		if (tmpstr == "BUS")
		{
			getline(fin, tmpstr);
			_GetSysDataByFile_IEEE_BUS(fin);
		}
		else if (tmpstr == "BRANCH")
		{
			getline(fin, tmpstr);
			_GetSysDataByFile_IEEE_BRANCH(fin);
		}
		else
			getline(fin, tmpstr);		
	}
	//
	fin.close();//关闭文件
	return;
}							
//************************************
// Method:    _GetSysDataByFile_IEEE_BUS
// FullName:  PFModel::PFSys::_GetSysDataByFile_IEEE_BUS
// Access:    private 
// Returns:   PFVoid
// Qualifier:
// Parameter: ifstream & fin
//************************************
PFVoid PFSys::_GetSysDataByFile_IEEE_BUS(ifstream& fin){
	PFStr	 tmpstr, tmpstr2;
	PFInt    tmpint;
	const string BUS_END("-999");
	BUS tmpbus;
	stringstream ss;
	getline(fin, tmpstr);
	while (tmpstr != BUS_END)
	{
		if (tmpstr.size() < 128)
		{
			throw exception("WARNING: Failure in Geting data in line string in FORMAT_IEEE BUS FOLLOWS!");
		}
		tmpbus.Clear();
		tmpbus.IDN.SetUID(atoi(tmpstr.substr(0, 4).c_str()));
		ss.clear();
		ss << tmpbus.IDN.GetUID();
		ss >> tmpstr2;
		tmpbus.IDN.SetTpnd(tmpstr2);
		tmpbus.IDN.name = tmpstr.substr(5, 12);
		//确定节点类型
		tmpint = atoi(tmpstr.substr(24, 2).c_str());
		switch(tmpint){
		case 0:
			tmpbus.btype = PF_BUS_TYPE_PQ;
			break;
		case 2:
			tmpbus.btype = PF_BUS_TYPE_PV;
			break;
		case 3:
			tmpbus.btype = PF_BUS_TYPE_VA;
			break;
		}
		tmpbus.BackupBusType();
		//get the pq 
		tmpbus.load.p = atof(tmpstr.substr(40, 9).c_str())  / SysBaseMVA;
		tmpbus.load.q = atof(tmpstr.substr(49, 10).c_str()) / SysBaseMVA;
		tmpbus.gene.p = atof(tmpstr.substr(59, 8).c_str())  / SysBaseMVA;
		tmpbus.gene.q = atof(tmpstr.substr(67, 8).c_str())  / SysBaseMVA;
		//nomKV
		tmpbus.voltage.SetVolBase(atof(tmpstr.substr(76, 7).c_str()));
		tmpbus.voltage.SetVolNom(tmpbus.voltage.GetVolNom());
		//initial voltage
		//tmpbus.voltage.vol = atof(tmpstr.substr(84, 6).c_str());
		tmpbus.voltage.vol = atof(tmpstr.substr(27, 6).c_str());
		tmpbus.voltage.SetVolSet(atof(tmpstr.substr(84, 6).c_str()));
		tmpbus.gene.qHighLimit = atof(tmpstr.substr(90, 8).c_str()) / SysBaseMVA;
		tmpbus.gene.qLowLimit  = atof(tmpstr.substr(98, 8).c_str()) / SysBaseMVA;
		// gii|bii
		tmpbus.para.g = atof(tmpstr.substr(106, 8).c_str());
		tmpbus.para.b = atof(tmpstr.substr(114, 8).c_str());

		tmpbus.IDN.index = PFSysBus.size();
		PFSysBus.push_back(tmpbus);
		//get the next line string
		getline(fin, tmpstr);
	}
	
	return;
}
//************************************
// Method:    _GetSysDataByFile_IEEE_BRANCH
// FullName:  PFModel::PFSys::_GetSysDataByFile_IEEE_BRANCH
// Access:    private 
// Returns:   PFVoid
// Qualifier: 读取IEEE数据中BRANCH段
// Parameter: std::ifstream & fin
//************************************
PFVoid PFSys::_GetSysDataByFile_IEEE_BRANCH(std::ifstream& fin){
	PFStr		tmpstr, tmpstr2;
	PFUInt		tmpuint;
	BRANCH		tmpbr;
	const PFStr BRANCH_END("-999");
	stringstream ss;
	getline(fin, tmpstr);
	while (tmpstr != BRANCH_END)
	{
		if (tmpstr.size() < 120)
			throw exception("WARNING: Failure in Geting Data from line string in FORMART_IEEE BRANCH FOLLOWS!");

		//tmpbr.SetFromBusTPND(tmpstr.substr(0, 4));
		//tmpbr.Set_FromBus_ID(atoi(tmpstr.substr(0, 4).c_str()));
		tmpuint = atoi(tmpstr.substr(0, 4).c_str());
		ss.clear();
		ss << tmpuint;
		ss >> tmpstr2;
		tmpbr.SetFromBusTPND(tmpstr2);

		//tmpbr.SetEndBusTPND(tmpstr.substr(5, 4));
		//tmpbr.Set_EndBus_ID(atoi(tmpstr.substr(5, 4).c_str()));
		tmpuint = atoi(tmpstr.substr(5, 4).c_str());
		ss.clear();
		ss << tmpuint;
		ss >> tmpstr2;
		tmpbr.SetEndBusTPND(tmpstr2);
		//确定支路类型
		tmpuint = atoi(tmpstr.substr(18,1).c_str());
		if (tmpuint == 0)
			tmpbr.btype = PF_BRANCH_LINE;
		else if (tmpuint == 2)
			tmpbr.btype = PF_BRANCH_TRAN;

		//支路参数
		tmpbr.para.SetR(atof(tmpstr.substr(19, 10).c_str()));
		tmpbr.para.SetX(atof(tmpstr.substr(29, 10).c_str()));
		tmpbr.para.SetBch(atof(tmpstr.substr(40, 9).c_str()) / 2);
		tmpbr.tap.SetVal(atof(tmpstr.substr(76, 6).c_str()));

		PFSysBranch.push_back(tmpbr);
		//获得下一行
		getline(fin, tmpstr);
	}
	return;
}
//************************************
// Method:    FormAssociation
// FullName:  PFModel::PFSys::FormAssociation
// Access:    public 
// Returns:   PFVoid
// Qualifier:
//************************************
PFVoid PFSys::FormAssociation(){
	PFStr itpnd, jtpnd;
	PFUInt iFound;
	//1. 支路和节点连接关系
	for (iter_PFBranch iter = PFSysBranch.begin();iter != PFSysBranch.end();++iter)
	{
		itpnd = iter->GetFromBusTPND();
		jtpnd = iter->GetEndBusTPND();
		iFound = 0;
		for (PFUInt i = 0;i < PFSysBus.size();++i)
		{
			//匹配首端拓扑节点
			if (PFSysBus[i].IDN.GetTpnd() == itpnd)
			{
				iter->inn = PFSysBus[i].IDN.index;
				iter->SetPFromBus(&PFSysBus[i]);
				++iFound;
			}
			if (PFSysBus[i].IDN.GetTpnd() == jtpnd)
			{
				iter->jnn = PFSysBus[i].IDN.index;
				iter->SetPEndBus(&PFSysBus[i]);
				++iFound;
			}
			if (iFound == 2) break;
		}
	}

	return;
}
//************************************
// Method:    InitSysDataByFile
// FullName:  PFModel::PFSys::InitSysDataByFile
// Access:    public 
// Returns:   PFUInt  0: Failue 1: Success
// Qualifier:
// Parameter: std::string & filepath
// Parameter: PF_DATA_FILE_FORMAT format
//************************************
PFUInt PFSys::InitSysDataByFile(std::string& filepath, PF_DATA_FILE_FORMAT format){
	try{
		switch(format){
		case PF_DATA_FILE_IEEE:
			_GetSysDataByFile_IEEE(filepath);
			break;
		case PF_DATA_FILE_BPA:
			//_GetSysDataByFile_BPA(filepath);
			break;
		}
		return 1;
	}
	catch (exception* e){
		cout << e->what() << endl;
		return 0;
	}
}

//备份当前系统电压信息
PFVoid PFSys::BackupVoltage(){
	for (iter_PFBus iter = PFSysBus.begin();iter != PFSysBus.end();++iter){
		iter->voltage.Backup();
	}
}

//恢复系统电压备份值
PFVoid PFSys::RewindVoltage() {
	for (iter_PFBus iter = PFSysBus.begin();iter != PFSysBus.end();++iter){
		iter->voltage.Rewind();
	}
}

//备份当前潮流信息，即发电机和负荷功率流状况
PFVoid PFSys::BackupPower(){
	for (iter_PFBus iter = PFSysBus.begin();iter != PFSysBus.end();++iter){
		iter->gene.Backup();
		iter->load.Backup();
	}
}

//恢复系统功率备份值
PFVoid PFSys::RewindPower(){
	for (iter_PFBus iter = PFSysBus.begin();iter != PFSysBus.end();++iter){
		iter->gene.Rewind();
		iter->load.Rewind();
	}
}

//备份当前节点类型
PFVoid PFSys::BackupBusType(){
	for (iter_PFBus iter = PFSysBus.begin();iter != PFSysBus.end();++iter){
		iter->BackupBusType();
	}
}

//恢复此前节点类型
PFVoid PFSys::RewindBusType(){
	for (iter_PFBus iter = PFSysBus.begin();iter != PFSysBus.end();++iter){
		iter->btype = iter->GetOriBusType();
	}
}

//备份基态计算节点状态
PFVoid PFSys::BackupBusCase(){
	this->PFSysBusCase = this->PFSysBus;
}

//恢复基态计算节点状态
PFVoid PFSys::RewindBusCase(){
	this->PFSysBus = this->PFSysBusCase;
}
/************************************************************************/
/*                TEST		FUNCTIONS                                   */
/************************************************************************/
//输出节点导纳矩阵
PFVoid PFSys::OutputYGBMatrix2DOS(){
	const int wid = 15;
	cout << "+" << setfill('-') << setw(wid) << "-" << "+" << setfill('-') << setw(wid) << "-" 
		 << "+" << setfill('-') << setw(wid) << "-" << "+" << setfill('-') << setw(wid) << "-" << "+" << endl;
	cout << "|" << setfill(' ') << setw(wid) << "INN" << "|" << setw(wid) << "JNN" 
		 << "|" << setw(wid) << "G"   << "|" << setw(wid) << "B" << "|" << endl;
	cout << "+" << setfill('-') << setw(wid) << "-" << "+" << setfill('-') << setw(wid) << "-" 
		 << "+" << setfill('-') << setw(wid) << "-" << "+" << setfill('-') << setw(wid) << "-" << "+" << endl;
	cout.fill(' ');
	cout.setf(ios::showpoint|ios::fixed);
	cout.precision(4);
	//输出自导纳数据
	for (iter_PFBus iter = PFSysBus.begin();iter != PFSysBus.end();++iter){
		cout << "|" << setw(wid) << iter->IDN.index + 1 << "|"  << setw(wid) << iter->IDN.index + 1
			 << "|" << setw(wid) << iter->para.g << "|" << setw(wid) << iter->para.b << "|" << endl;
	}
	//分割线
	cout << "+" << setfill('-') << setw(wid) << "-" << "+" << setfill('-') << setw(wid) << "-" 
		 << "+" << setfill('-') << setw(wid) << "-" << "+" << setfill('-') << setw(wid) << "-" << "+" << endl;
	cout.fill(' ');
	//输出支路互导纳
	for (iter_PFBranch iter = PFSysBranch.begin();iter != PFSysBranch.end();++iter){
		cout << "|" << setw(wid) << iter->inn + 1 << "|" << setw(wid) << iter->jnn + 1
			 << "|" << setw(wid) << iter->para.g << "|" << setw(wid) << iter->para.b << "|" << endl;
	}
	//分割线
	cout << "+" << setfill('-') << setw(wid) << "-" << "+" << setfill('-') << setw(wid) << "-" 
		 << "+" << setfill('-') << setw(wid) << "-" << "+" << setfill('-') << setw(wid) << "-" << "+" << endl;
	cout.fill(' ');

}

//输出节点电压信息
PFVoid PFSys::OutputBusVoltageInfo2Dos(){
	for (iter_PFBus iter = PFSysBus.begin();iter != PFSysBus.end();++iter){
		cout << setw(5) << iter->IDN.index << setw(15) << iter->voltage.vol << setw(15) << iter->voltage.ang
			 << setw(10) << iter->voltage.GetVolSet() << setw(5) << iter->btype << endl;
		//cout << iter->IDN.index << "\t" << iter->voltage.vol << "\t" << iter->voltage.GetVolSet() << "\t" << iter->btype << endl;
	}
	return;
}

//输出节点功率信息
PFVoid PFSys::OutputBusPowerInfo2Dos(){
	for (iter_PFBus iter = PFSysBus.begin();iter != PFSysBus.end();++iter){
		if (iter->GetOriBusType() != PF_BUS_TYPE_PV){
			continue;
		}
		cout << iter->IDN.index << "\t" << iter->gene.q << "\t" << iter->gene.qHighLimit << "\t" << iter->gene.qLowLimit << "\t" << iter->load.q << endl;
	}
	return;
}														

//输出电压信息至文件			
PFVoid PFSys::OutputVoltageInfo2File(const string file){
	try{
		ofstream fout(file, ios::out);
		if (fout == NULL){
			cout << "Warning: Failure in Open & Write " << file << endl;
			return;
		}
		fout << setw(5) << "index"
			 << setw(20) << "name"
			 << setw(15) << "vol"
			 << setw(15) << "ang"
			 << endl;
		for (iter_PFBus iter = PFSysBus.begin();iter != PFSysBus.end();++iter){
			fout << setw(5) << iter->IDN.index
				 << setw(20) << iter->IDN.name
				 << setw(15) << iter->voltage.vol
				 << setw(15) << iter->voltage.ang
				 << endl;
		}
		fout.close();
	} catch (exception e) {
		cout << e.what();
	}
	return;
}