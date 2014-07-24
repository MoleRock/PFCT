/*********************************************************************
*	NAME		:		PFElem.h
*	CONTENT		:		Definition of the Base Element class in the PFCT
*	COPYRIGHT	:		ROCK(rockzheng1127@126.com)
*	DATE		:		2014/07/12 -- 2014//
*	DIRECTION	:		该文件主要定义些基础的基类
**********************************************************************/

#include "PFMenuDef.h"
#include "PFCore.h"

#include <ostream>

namespace PFModel {
	/********************************************
	*		标识定义
	*********************************************/
	class Identification {
		//Fields
	public:
		PFInt	index;		//索引标识
		PFStr	name;		//名称标识
	protected:
		PFID	uid;		//id标识
		PFStr	tpnd;		//拓扑标识

		//Functions
	public:
		inline Identification() {
			index = PF_BUS_INDEX_DEFAULT;
			uid = PF_ID_DEFAULT;
			name = PF_STR_DEFAULT;
			tpnd = PF_STR_DEFAULT;
		}

		inline PFVoid SetUID(PFID id) {
			this->uid = id;
		}

		inline const PFID GetUID() const {
			return this->uid;
		}

		inline PFVoid SetTpnd(PFStr tpnd) {
			this->tpnd = tpnd;
		}

		inline const PFStr GetTpnd() const {
			return this->tpnd;
		}

		inline PFVoid Clear() {
			this->index = PF_BUS_INDEX_DEFAULT;
			this->name = PF_STR_DEFAULT;
			this->tpnd = PF_STR_DEFAULT;
			this->uid = PF_ID_DEFAULT;
		}

		const PFStr toString() const;

		inline Identification& operator=(const Identification& iden){
			this->index = iden.index;
			this->uid = iden.uid;
			this->name = iden.name;
			this->tpnd = iden.tpnd;
			return *this;
		}

		inline friend std::ostream& operator<<(std::ostream& dc, const Identification& iden){
			return dc << iden.toString();
		}
	};

	/********************************************
	*		电压信息类
	*********************************************/
	class VoltageInfo {
		//Fields
	public:
		PFDouble vol;
		PFDouble ang;
		PFDouble volHighLimit;
		PFDouble volLowLimit;

	protected:
		PFDouble nomKV;		//电压基准值
		PFDouble voltKV;	//电压等级值
		PFDouble volSet;	//设定电压值
	private:
		PFDouble volBak;	//电压幅值备份
		PFDouble angBak;	//电压相角备份

		//Functions
	public:
		inline VoltageInfo() {
			vol = 0.0;
			ang = 0.0;
			volLowLimit = 0.0;
			volLowLimit = 0.0;
			nomKV = 0.0;
			voltKV = 0.0;
			volBak = 0.0;
			angBak = 0.0;
			volSet = 0.0;
		}

		inline PFVoid Clear() {
			this->vol = this->ang = 0.0;
			this->volHighLimit = this->volLowLimit = 0.0;
			this->nomKV = this->voltKV = 0.0;
			this->volBak = this->angBak = 0.0;
			this->volSet = 0.0;
		}

		inline PFVoid SetVolSet(const PFDouble vset) {
			this->volSet = vset;
		}

		inline const PFDouble GetVolSet() const {
			return this->volSet;
		}

		inline PFVoid UpdateVol() {
			this->vol = this->volSet;
		}

		inline PFVoid SetVolBase(const PFDouble vb) {
			this->nomKV = vb;
		}

		inline const PFDouble GetVolBase() const {
			return this->nomKV;
		}

		inline PFVoid SetVolNom(const PFDouble vn) {
			this->voltKV = vn;
		}

		inline const PFDouble GetVolNom() const {
			return this->voltKV;
		}

		inline const PFDouble GetVolReal () const {
			return this->vol * this->nomKV;
		}

		inline PFVoid Backup() {
			this->volBak = this->vol;
			this->angBak = this->ang;
		}

		inline PFVoid BackupVol() {
			this->volBak = this->vol;
		}

		inline PFVoid BackupAng() {
			this->angBak = this->ang;
		}

		inline PFVoid RewindVol() {
			this->vol = this->volBak;
		}

		inline PFVoid RewindAng() {
			this->ang = this->angBak;
		}

		inline PFVoid Rewind() {
			RewindVol();
			RewindAng();
		}

		inline const PFDouble GetVolBak() const {
			return this->volBak;
		}

		inline const PFDouble GetAngBak() const {
			return this->angBak;
		}

		const PFStr toString() const;

		inline VoltageInfo& operator= (const VoltageInfo& vi) {
			this->vol = vi.vol;
			this->ang = vi.ang;
			this->volHighLimit = vi.volHighLimit;
			this->volLowLimit = vi.volLowLimit;
			this->nomKV = vi.nomKV;
			this->voltKV = vi.voltKV;
			this->volBak = vi.volBak;
			this->angBak = vi.angBak;
			this->volSet = vi.volSet;
			return *this;
		}

		//Friends Functions
		inline friend std::ostream& operator<< (std::ostream& dc, const VoltageInfo& vi) {
			return dc << vi.toString();
		}

	};

	/********************************************
	*		功率信息类
	*********************************************/
	class PowerInfo {
		//Fields
	public:
		PFDouble p;
		PFDouble q;
		PFDouble pHighLimit;
		PFDouble pLowLimit;
		PFDouble qHighLimit;
		PFDouble qLowLimit;	

	protected:
		static PFDouble SBase;
		PFDouble pertP;			//有功功率增减单位量
		PFDouble pertQ;			//无功功率增减单位量
	private:
		PFDouble pBak;
		PFDouble qBak;

		//Functions
	public:
		PowerInfo() {
			p = 0.0;
			q = 0.0;
			pHighLimit = 0.0;
			qHighLimit = 0.0;
			pLowLimit = 0.0;
			qLowLimit = 0.0;
			pertP = pertQ = 0.0;
			pBak = 0.0;
			qBak = 0.0;
		}

		inline PFVoid Clear() {
			this->p = this->q = 0.0;
			this->pHighLimit = this->pLowLimit = 0.0;
			this->qHighLimit = this->qLowLimit = 0.0;
			this->pBak = this->qBak = 0.0;
			this->pertP = this->pertQ = 0.0;
		}

		inline PFVoid UpdateByPertP(PFDouble lamda) {
			this->p = this->pBak + this->pertP * lamda;
		}

		inline PFVoid UpdateByPertQ(PFDouble lamda) {
			this->q = this->qBak + this->pertQ * lamda;
		}

		inline PFVoid UpdateByPert(PFDouble lamda) {
			UpdateByPertP(lamda);
			UpdateByPertQ(lamda);
		}

		inline PFVoid SetPert(PFDouble pert) {
			this->pertP = this->pertQ = pert;
		}

		inline PFVoid SetPert(PFDouble pertp, PFDouble pertq) {
			this->pertP = pertp;
			this->pertQ = pertq;
		}

		inline PFVoid SetPertP(PFDouble pert) {
			this->pertP = pert;
		}

		inline PFVoid SetPertQ(PFDouble pert) {
			this->pertQ = pert;
		}

		inline const PFDouble GetPertP() const {
			return this->pertP;
		}

		inline const PFDouble GetPertQ() const {
			return this->pertQ;
		}

		inline const PFDouble GetPReal() const {
			return this->p * PowerInfo::SBase;
		}

		inline const PFDouble GetQReal() const {
			return this->q * PowerInfo::SBase;
		}

		inline PFVoid BackupP() {
			this->pBak = this->p;
		}

		inline PFVoid BackupQ() {
			this->qBak = this->q;
		}

		inline PFVoid Backup() {
			BackupP();
			BackupQ();
		}

		inline PFVoid RewindP() {
			this->p = this->pBak;
		}

		inline PFVoid RewindQ() {
			this->q = this->qBak;
		}

		inline PFVoid Rewind() {
			RewindP();
			RewindQ();
		}

		inline const PFDouble GetPBak() {
			return this->pBak;
		}

		inline const PFDouble GetQBak() {
			return this->qBak;
		}

		const PFStr toString() const;

		static PFVoid SetSBase(PFDouble sb);
		
		static PFDouble GetSBase();

		inline PowerInfo& operator= (const PowerInfo& pi) {
			this->p = pi.p;
			this->q = pi.q;
			this->pHighLimit = pi.pHighLimit;
			this->pLowLimit = pi.pLowLimit;
			this->qHighLimit = pi.qHighLimit;
			this->qLowLimit = pi.qLowLimit;
			this->pBak = pi.pBak;
			this->qBak = pi.qBak;
			return *this;
		}

		inline PowerInfo& operator+= (const PowerInfo& pi) {
			this->p += pi.p;
			this->q += pi.q;
			this->pHighLimit += pi.pHighLimit;
			this->pLowLimit += pi.pLowLimit;
			this->qHighLimit += pi.qHighLimit;
			this->qLowLimit += pi.qLowLimit;
			this->pertP += pi.pertP;
			this->pertQ += pi.pertQ;
			this->pBak += pi.pBak;
			this->qBak += pi.qBak;
			return *this;
		}

		inline PowerInfo operator+(const PowerInfo& pi) {
			PowerInfo result;
			result.p = this->p + pi.p;
			result.q = this->q + pi.q;
			return result;
		}

		inline friend std::ostream& operator<< (std::ostream& dc, const PowerInfo& pi){
			return dc << pi.toString();
		}
	};

	/********************************************
	*		参数类
	*********************************************/
	class Parameter {
		//Fields
	public:
		PFDouble g;
		PFDouble b;
	private:
		PFDouble r;
		PFDouble x;
		PFDouble bch;	//单端并联电纳
	private:
		PFDouble gBak;
		PFDouble bBak;

		//Functions
	public:
		Parameter() {
			g = b = 0.0;
			r = x = 0.0;
			bch = 0.0;
			gBak = bBak = 0.0;
		}

		Parameter(PFDouble r, PFDouble x, PFDouble bch = 0.0) {
			this->r = r;
			this->x = x;
			this->bch = bch;
		}

		inline PFVoid Clear() {
			this->g = this->b = 0.0;
			this->r = this->x = 0.0;
			this->bch = 0.0;
			this->gBak = this->bBak = 0.0;
		}

		inline const PFBool IsComplete() const {
			return !(r == 0.0 && x == 0.0);
		}

		inline PFVoid Calculate() {
			if (!this->IsComplete()){
				throw std::exception("Error: Divided By ZERO in Function_Parameter.calculate!\n");
			}
			PFDouble rx = r * r + x * x;
			g = r / rx;
			b = - x / rx;
		}

		inline PFVoid SetR(PFDouble r) {
			this->r = r;
		}

		inline const PFDouble GetR() const {
			return this->r;
		}

		inline PFVoid SetX(PFDouble x) {
			this->x = x;
		}

		inline const PFDouble GetX() const {
			return this->x;
		}

		inline PFVoid SetBch(PFDouble bch) {
			this->bch = bch;
		}

		inline const PFDouble GetBch() const {
			return this->bch;
		}

		inline PFVoid BackupG() {
			this->gBak = this->g;
		}

		inline PFVoid BackupB() {
			this->bBak = this->b;
		}

		inline PFVoid Backup() {
			BackupG();
			BackupB();
		}

		inline PFVoid RewindG() {
			this->g = this->gBak;
		}

		inline PFVoid RewindB() {
			this->b = this->bBak;
		}

		inline PFVoid Rewind() {
			RewindG();
			RewindB();
		}

		inline const PFDouble GetGBak() const {
			return this->gBak;
		}

		inline const PFDouble GetBBak() const {
			return this->bBak;
		}

		const PFStr toString() const;

		inline Parameter& operator= (const Parameter& para) {
			this->g = para.g;
			this->b = para.b;
			this->r = para.r;
			this->x = para.x;
			this->bch = para.bch;
			this->gBak = para.gBak;
			this->bBak = para.bBak;
			return *this;
		}		

		inline friend std::ostream& operator<< (std::ostream& dc, const Parameter& para) {
			return dc << para.toString();
		}
	};
	
	/********************************************
	*		离散变量类
	*********************************************/
	class Discrete {
		//Fields
	public:
		PFDouble	val;		//当前值
		PFDouble	valHigh;	//离散上限
		PFDouble	valLow;		//离散下限
	protected:
		PF_CTRL_DIRECTION ctrl;	//控制方向
	private:
		PFDouble	valBak;		//当前值备份
		//Functions
	public:
		inline Discrete() {
			val = valHigh = valLow = PF_DOUBLE_DEFAULT;
			ctrl = PF_CTRL_NULL;
			valBak = PF_DOUBLE_DEFAULT;
		}

		inline Discrete(PFDouble val, PFDouble valhigh, PFDouble vallow) {
			this->val = val;
			this->valHigh = valhigh;
			this->valLow = vallow;
			this->valBak = val;
			this->ctrl = PF_CTRL_NULL;
		}

		inline Discrete(PFDouble val) {
			this->val = this->valHigh = this->valLow = this->valBak = val;
			this->ctrl = PF_CTRL_NULL;
		}

		inline PFVoid Clear() {
			this->val = this->valHigh = this->valLow = this->valBak = 0.0;
			this->ctrl = PF_CTRL_NULL;
		}

		inline PFVoid SetVal(const PFDouble val) {
			this->val = this->valHigh = this->valLow = this->valBak = val;
			this->ctrl = PF_CTRL_NULL;
		}

		inline PFVoid SetCtrl(PF_CTRL_DIRECTION ctrl) {
			this->ctrl = ctrl;
		}

		inline const PF_CTRL_DIRECTION GetCtrl() const {
			return this->ctrl;
		}

		inline const PFDouble GetValBak() const {
			return this->valBak;
		}

		inline PFVoid Backup() {
			this->valBak = this->val;
		}

		inline PFVoid Rewind() {
			this->val = this->valBak;
		}

		const PFStr toString() const;

		inline Discrete& operator=(const Discrete& dis) {
			this->val = dis.val;
			this->valHigh = dis.valHigh;
			this->valLow  = dis.valLow;
			this->ctrl    = dis.ctrl;
			this->valBak  = dis.valBak;
			return *this;
		}

		inline friend std::ostream& operator<<(std::ostream& dc, const Discrete& dis) {
			return dc << dis.toString();
		}

	};

}