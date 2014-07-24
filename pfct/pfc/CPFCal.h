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
	//��������������
	class CPF: public PFC{
		//Fields
	public:
		static PFUInt CPFMXPnt;				//CPF����������ֵ
	protected:
		//�ڲ�������������
		PFTripletDVec	CPFPertCol;			//�����ſɱ���������,���ʶ��������ӵ�������ϵ��		
		PFTripletDVec   CPFPertRow;			//�����ſɱ���������,�������������������ӵ�ϵ��

	private:
		CPF_RESULT		CPFResult;			//CPF���������
		PFUInt			CPFNPoint;			//CPF��������

		CPF_DEST		CPFDest;			//CPF����Ŀ��
		CPF_PARA		CPFPara;			//CPF����������
		CPF_PREDICT		CPFPredict;			//CPFԤ�ⷽ��

		PFDouble		CPFLoadMargin;		//��ǰ����������ʽ�¸���ԣ��

		PFDouble		CPFLambda;			//������������
		PFDouble		CPFStepLen;			//���㲽��

		PFUInt			CPFEK;				//������������

		//������Ϣ
		PFDouble		CPFLambdaBak;		//�����������ӱ���

		//Functions
	public:
		CPF();								//�޲������캯��
		CPF(const CPF& cpf);				//���ƹ��캯��
		CPF(const PFC& pf);					//�������캯��
		CPF(PFModel::PFSys& psys);			//ϵͳ���ݹ��캯��
		CPF(PFModel::PFSys* const psys);	//ϵͳ���ݹ��캯��

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

		PFVoid		    Solve();					//CPF����
	protected:
		PFVoid			_MakeCPFPertCol();			//����CPF�����ſɱ���������
		PFVoid			_MakeCPFPredictRow();		//Ԥ�ⲽ����CPF�����ſɱ���������
		PFBool			_MakePredict();				//Ԥ�ⲽ����
		PFBool			_MakeCorrect();				//У��������

		PFVoid			_MakeJacoMatrix();			//��������ſɱȾ�����д����
		PFVoid			_UpdateSysState();			//����ϵͳ״̬����
		PFVoid			_UpdateCPFEK();				//������������	
	private:
		//Ĭ�ϳ�ʼ��
		inline PFVoid _InitDefault(){
			this->CPFResult = CPF_RESULT_FAILURE;
			this->CPFNPoint = 0;

			this->CPFDest = CPF_DEST_PV;
			this->CPFPara = CPF_PARA_LOCAL;
			this->CPFPredict = CPF_PRE_TANGENT;

			this->CPFLoadMargin = 0.0;

			this->CPFLambda = 0.0;
			this->CPFStepLen = 1.0;

			this->CPFEK = 0;	//��ʼ����ʱ��Ϊ0

			this->CPFLambdaBak = 0.0;
		}
		//��ʼ��������������
		inline PFVoid _InitPFCalMatrix() {
			PFUInt size = this->pSysData->GetSysSize();
			if(size <= 0)
				throw std::exception("WARNING: Failure in FUNC_InitPFCalMatrix cause the SysSize <= 0 !");
			//Resize the inner Matrix & Vector
			this->dPFCVA.resize(size * 2 + 1);
			this->dPFCPQ.resize(size * 2 + 1);
			this->PFCJaco.resize(size * 2 + 1, size * 2 + 1);
		}
		//Ԥ�ⲽʧ��������
		PFVoid _MakeCPFPredictVec();
	};


}

#endif
