/*********************************************************************
*	NAME		:		PFDev.h
*	CONTENT		:		Definition of the Device model in the PFCT
*	COPYRIGHT	:		ROCK(rockzheng1127@126.com)
*	DATE		:		2014/07/12 -- 2014//
*	DIRECTION	:		
**********************************************************************/

#ifndef _PFDEV_H
#define _PFDEV_H

#include "PFCore.h"
#include "PFMenuDef.h"

namespace PFDev {

	/**********************************************
	*		Model_BusBarSection		:	母线
	**********************************************/
	class Elcbus{
		//成员函数
		Elcbus();

		//成员变量
	public:
		PFID			uid;				//母线标识
		PFStr			name;			//母线名称

		PFStr			tpnd;			//拓扑节点名称
		PFDouble		nomKV;			//电压基准值
		PFDouble		voltKV;			//电压等级值

		PFDouble		vol;		//电压有名值
		PFDouble		ang;			//相角弧度制

	protected:
		PFULong			node;			//设备连接点号
		PFID			stUID;			//厂站标识
		PFID			bvUID;			//
		PFID			vlUID;			//电压等级标识

		PFDouble        volHigh;		//电压上限
		PFDouble		volLow;		//电压下限
	private:
	};

	/**********************************************
	*		Model_ACLine		:	功率馈入类
	**********************************************/
	class ACLine{
		//Fields

		//Functions

	};

	/**********************************************
	*		Model_Energy		:	功率类
	**********************************************/
	class Energy{
	//成员函数
	public:
		Energy();
		~Energy();

		inline PFStr  GetTpnd() const {
			return this->tpnd;
		}

		inline PFVoid SetTpnd(PFStr tpnd) {
			this->tpnd = tpnd;
		}

		inline PFVoid SetNode(PFULong node) {
			this->node = node;
		}

		inline PFULong GetNode() const {
			return this->node;
		}
	protected:
	private:
	//成员变量
	public:
		PFStr		descr;		//功率描述
		PFDouble	p;			//有功
		PFDouble	q;			//无功
		PFDouble	p_max;		//有功上限
		PFDouble	p_min;		//有功下限
		PFDouble	q_max;		//无功上限
		PFDouble	q_min;		//无功下限
	protected:		
		PFStr       tpnd;		//连接点拓扑节点
		PFULong     node;		//连接点号
	private:
	};

	/**********************************************
	*		Model_Generator		:	机组类
	**********************************************/
	class Generator : public Energy{
	//成员函数
	public:
		Generator();

	protected:
	private:
	
	//成员变量
	public:
		PFDouble	vol;		//发电机机端电压
		PFDouble	ang;			//发电机机端相角
		PFDouble	volHighLimit;		//发电机机端电压上限
		PFDouble    volLowLimit;		//发电机机端电压下限
		PFDouble    volSet;	//发电机机端电压设定值
	protected:
	private:
	};

	/**********************************************
	*		Model_Consumer		:	负荷类
	**********************************************/
	class Consumer : public Energy {
		//Functions
	public:
		Consumer();

		//Fields
	public:

	};

	/**********************************************
	*		Model_TapType		:	可调分接头计算类
	**********************************************/
	class TapType{
	//成员函数
	public:
		TapType();
		~TapType();
	protected:
	private:
	//成员变量
	public:
		PFUInt			tap;		//目前的档位
		PFUInt          tapNom;		//分接头额定档位
		PFUInt			tapHigh;	//分接头高档
		PFUInt			tapLow;		//分接头低档
		PFDouble		step;		//步长
	protected:
	private:

	};

	/**********************************************
	*		Model_ShuntCompensator			:	并补类
	**********************************************/
	class ShuntCompensator{
	public:
		ShuntCompensator();
		~ShuntCompensator();
	public:
		PFStr		name;		
		PFDouble    b;		//对地电纳

		PFStr       tpnd;		//拓扑节点名称
		PFULong     node;		//连接点号

		PFDouble	q;
		PFDouble    qnom;
	protected:
		
	};







}





#endif