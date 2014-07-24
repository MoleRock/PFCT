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
	//����������
	class PFC {
	//��Ա����
	public:
		//���캯��
		PFC();
		PFC(const PFC& pf);								//���ƹ��캯��
		PFC(PFSys* const);								//ϵͳ����ָ��
		PFC(PFSys& SData);						//ϵͳ��������
		//��������
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

		virtual PFVoid	Solve();						//��������ģ�庯����������ַ���������Է�����

	protected:
		PFVoid			_MakeJacoMatrix();				//�����ſɱ���

		PFVoid          _MakeDPQVector();				//���칦��ʧ����

		PFVoid			_MakeDQVector();				//�����޹�����ʧ����

		PFVoid			_MakeDPQCalByBus();				//��������ʧ����������������������

		PFVoid			_MakeDPQCalByGene();			//�ڹ���ʧ�����Ļ��������ӻ������

		PFVoid			_UpdateSysState();				//����ϵͳ״̬����

		PFVoid			_ModifyJaco();					//�����ſɱ���

		PFVoid          _ModifyJacoByBusType(const int mode);//�ſɱ���Ԫ������

		PFVoid			_ModifyJacoQGElem();			//����Լ���������ſɱ���dQG ��Ԫ��

		PFVoid			_ModifyDPQByBusType(const int mode);//��������ʧ���� mode = 0:����ƽ��ڵ��PV�ڵ�; = 1: ֻ����ƽ��ڵ�

		PFVoid			_MakeDPQVectorByBranch();		//����ϵͳ֧·��ϵ����

		PFVoid			_MakeDPQVectorByBus();			//����ϵͳ�ڵ�ע�빦��

		PFVoid			_SwitchBType();					//ִ�нڵ�����ת��

		const PFDouble  _CalOptimalMultiplier();		//�������ų��� -- 1994 [CSEE] [������] ������ϵ׼���ų��Ӳ�̬�����ⷨ�о�

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

		PFVoid			_ExecuteSwitchLogic();			//ִ�нڵ�����ת���߼�
		
		PFVoid			_ExecuteSwitchComplementary();	//ִ�л���Լ��ת���߼�

		PF_PQ_VIOLATION _CheckPQViolation(PFDouble val, PFDouble valhigh, PFDouble vallow);
		
		PF_V_VIOLATION	_CheckVolViolation(PFDouble val, PFDouble valhigh, PFDouble vallow);
		
		//����ʢ��ʽ����һԪ���η��̣�g3,g2,g1,g0�ֱ�Ϊ����ϵ��
		PFUInt _CalFormularByShengJin(const PFDouble g3, const PFDouble g2, const PFDouble g1, const PFDouble g0, PFCore::PFDoubleVec& result);
	//��Ա����
	public:
		static PFDouble			PFCEpsm;				//������������
		static PFUInt			PFCMaxIter;				//��������������
	protected:
		PFCore::PFSMatrixXD		PFCJaco;				//�ſɱȾ���
		PFCore::PFVectorXD		dPFCVA;					//���������ѹʧ����
		PFCore::PFVectorXD		dPFCPQ;					//�������㹦��ʧ����

		PFSys*					pSysData;				//ϵͳ����ͷָ��
	private:
		PF_RESULT				PFCResult;				//��������״��
		PFUInt					PFCIter;				//���������������
		PF_VA_INIT				PFCInitType;			//������ʼ����ʽ

		PFBool					PFCBTypeSwitch;			//�Ƿ�ִ�нڵ�����ת��
		PF_BUSTYPE_SWITCH		PFCBtypeMethod;			//�ڵ�����ת������


	};//End of class PFC



}//End of namespace PFCal




#endif