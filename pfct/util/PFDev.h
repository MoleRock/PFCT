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
	*		Model_BusBarSection		:	ĸ��
	**********************************************/
	class Elcbus{
		//��Ա����
		Elcbus();

		//��Ա����
	public:
		PFID			uid;				//ĸ�߱�ʶ
		PFStr			name;			//ĸ������

		PFStr			tpnd;			//���˽ڵ�����
		PFDouble		nomKV;			//��ѹ��׼ֵ
		PFDouble		voltKV;			//��ѹ�ȼ�ֵ

		PFDouble		vol;		//��ѹ����ֵ
		PFDouble		ang;			//��ǻ�����

	protected:
		PFULong			node;			//�豸���ӵ��
		PFID			stUID;			//��վ��ʶ
		PFID			bvUID;			//
		PFID			vlUID;			//��ѹ�ȼ���ʶ

		PFDouble        volHigh;		//��ѹ����
		PFDouble		volLow;		//��ѹ����
	private:
	};

	/**********************************************
	*		Model_ACLine		:	����������
	**********************************************/
	class ACLine{
		//Fields

		//Functions

	};

	/**********************************************
	*		Model_Energy		:	������
	**********************************************/
	class Energy{
	//��Ա����
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
	//��Ա����
	public:
		PFStr		descr;		//��������
		PFDouble	p;			//�й�
		PFDouble	q;			//�޹�
		PFDouble	p_max;		//�й�����
		PFDouble	p_min;		//�й�����
		PFDouble	q_max;		//�޹�����
		PFDouble	q_min;		//�޹�����
	protected:		
		PFStr       tpnd;		//���ӵ����˽ڵ�
		PFULong     node;		//���ӵ��
	private:
	};

	/**********************************************
	*		Model_Generator		:	������
	**********************************************/
	class Generator : public Energy{
	//��Ա����
	public:
		Generator();

	protected:
	private:
	
	//��Ա����
	public:
		PFDouble	vol;		//��������˵�ѹ
		PFDouble	ang;			//������������
		PFDouble	volHighLimit;		//��������˵�ѹ����
		PFDouble    volLowLimit;		//��������˵�ѹ����
		PFDouble    volSet;	//��������˵�ѹ�趨ֵ
	protected:
	private:
	};

	/**********************************************
	*		Model_Consumer		:	������
	**********************************************/
	class Consumer : public Energy {
		//Functions
	public:
		Consumer();

		//Fields
	public:

	};

	/**********************************************
	*		Model_TapType		:	�ɵ��ֽ�ͷ������
	**********************************************/
	class TapType{
	//��Ա����
	public:
		TapType();
		~TapType();
	protected:
	private:
	//��Ա����
	public:
		PFUInt			tap;		//Ŀǰ�ĵ�λ
		PFUInt          tapNom;		//�ֽ�ͷ���λ
		PFUInt			tapHigh;	//�ֽ�ͷ�ߵ�
		PFUInt			tapLow;		//�ֽ�ͷ�͵�
		PFDouble		step;		//����
	protected:
	private:

	};

	/**********************************************
	*		Model_ShuntCompensator			:	������
	**********************************************/
	class ShuntCompensator{
	public:
		ShuntCompensator();
		~ShuntCompensator();
	public:
		PFStr		name;		
		PFDouble    b;		//�Եص���

		PFStr       tpnd;		//���˽ڵ�����
		PFULong     node;		//���ӵ��

		PFDouble	q;
		PFDouble    qnom;
	protected:
		
	};







}





#endif