/*********************************************************************
*	NAME		:		PFModel.h
*	CONTENT		:		Definition of the BUS-BRANCH model in the PFCT
*	COPYRIGHT	:		ROCK(rockzheng1127@126.com)
*	DATE		:		2014/06/05 -- 2014//
*	DIRECTION	:		
**********************************************************************/

#ifndef _PFMODEL_H
#define _PFMODEL_H
#include <iostream>
#include <vector>
#include <fstream>

#include "PFCore.h"
#include "PFElem.h"
namespace PFModel
{
	/**********************************************
	*		Model_AREA			:	���������
	**********************************************/
	class AREA{
	public:
		AREA();
		//AREA(const AREA&);
	public:
		PFStr		name;		//��������
		PFUInt		gene_num;	//������Ŀ
		PFUInt		load_num;	//������Ŀ

		PowerInfo   load;		//�����ܸ�����Ϣ
		PowerInfo   gene;		//�����ܹ��ʳ�����Ϣ

	};
	/**********************************************
	*		Model_BUS			:	�ڵ������
	**********************************************/
	class BUS{
		//��Ԫ����
		friend std::ostream& operator<<(std::ostream& dc, const BUS& bus);

	//��Ա����
	public:	
		BUS();
		BUS(const BUS& );
		//~BUS();

		inline PFVoid		Backup(){
			this->btype_bak   = this->btype;
			this->voltage.Backup();
			this->load.Backup();
			this->gene.Backup();
			this->para.Backup();
		}
		
		inline PFVoid BackupBusType() {
			this->btype_bak = this->btype;
		}

		inline PF_BUS_TYPE GetOriBusType() const {
			return this->btype_bak;
		}

		inline PFVoid       UpdateAreaPD(){
			this->pArea->load.p += this->load.p;
		}

		inline PFVoid       UpdateAreaQD(){
			this->pArea->load.q += this->load.q;
		}

		inline PFVoid       UpdateAreaPG(){
			this->pArea->gene.p += this->gene.p;
		}

		inline PFVoid       UpdateAreaQG(){
			this->pArea->gene.q += this->gene.q;
		}

		PFVoid                Clear();							//���г�Ա�������

		PFCore::PFDoubleVec	  ComputeDPQElement();				//���㹦��ʧ����

		PFDouble			  ComputeDQCal();					//�����޹�����ʧ����

		PFCore::PFDoubleVec   ComputeDPQCal();					//���㹦�ʼ�����������������

		PFCore::PFTripletDVec ComputeJacoElement();				//�����ſɱ���ڵ����Ԫ��
	private:

	//��Ա����
	public:
		Identification  IDN;			//���Ʊ�ʶ

		PFUInt          nArea;			//������������
		PF_BUS_TYPE		btype;			//�ڵ��ѹ����

		VoltageInfo     voltage;		//�ڵ��ѹ��Ϣ
		PowerInfo		load;			//�ڵ㸺����Ϣ
		PowerInfo		gene;			//�ڵ������Ϣ
		Parameter		para;			//�Ե�����Ϣ
		//��ɢ������
		Discrete		shunt;			//��������
	protected:
		AREA*           pArea;			//���ڷ���ָ��
	private:
		PF_BUS_TYPE		btype_bak;		
	};
	/**********************************************
	*		Model_BRANCH		:	֧·������
	**********************************************/
	class BRANCH{
	//��Ա����
	public:
		BRANCH();
		~BRANCH();

		inline PFVoid SetPFromBus(BUS* const pbus){
			this->pFromBus = pbus;
		}

		inline PFVoid SetPEndBus(BUS* const pbus){
			this->pEndBus = pbus;
		}

		inline PFVoid SetFromBusTPND(PFStr tpnd){
			this->fromTpnd = tpnd;
		}

		inline PFVoid SetEndBusTPND(PFStr tpnd){
			this->endTpnd = tpnd;
		}

		inline PFStr  GetFromBusTPND()const{
			return this->fromTpnd;
		}

		inline PFStr  GetEndBusTPND()const{
			return this->endTpnd;
		}

		inline PFVoid Backup(){
			this->pqij.Backup();
			this->pqji.Backup();
			this->para.Backup();
			this->tap.Backup();
		}

		PFVoid							Clear();				//��Ա�������
		
		PFVoid							ComputeYGBElement();	//��֧·����ڵ㵼�������Ԫ��
		
		PFCore::PFTripletDVec			ComputeJacoElement();	//�����ſɱ���Ԫ��
		
		PFCore::PFDoubleVec				ComputeDPQElement();	//���㹦��ʧ����

		PFCore::PFDoubleVec				ComputeDQCal();		//�����޹�ʧ����
	protected:
	private:
	//��Ա����
	public:
		Identification		IDN;			//���Ʊ�ʶ

		PF_BRANCH_TYPE		btype;			//֧·����
		PFInt				inn;			//�׶˽ڵ����
		PFInt				jnn;			//ĩ�˽ڵ����

		//�ڵ㵼�ɾ���ǶԽ�Ԫ����Ԫ��
		Parameter			para;			//֧·�����ͻ�������Ϣ
		//֧·��������
		PowerInfo			pqij;
		PowerInfo			pqji;
		//֧·������Ϣ
		Discrete			tap;			//��·�����Ϣ
	protected:
		BUS*				pFromBus;		//�׶˽ڵ�ָ��
		BUS*				pEndBus;		//ĩ�˽ڵ�ָ��
		PFStr               fromTpnd;		//�׶����˽ڵ�����
		PFStr				endTpnd;		//ĩ�����˽ڵ�����
	private:
	};

	typedef std::vector<BUS>				PFBusVec;
	typedef std::vector<BUS>::iterator		iter_PFBus;
	typedef std::vector<BRANCH>				PFBranchVec;
	typedef std::vector<BRANCH>::iterator	iter_PFBranch;
	typedef std::vector<AREA>				PFAreaVec;
	typedef std::vector<AREA>::iterator		iter_PFArea;

	class PFSys{
	//��Ա����
	public:
		PFSys();
		~PFSys();
		//��̬��Ա����
		static PFVoid SetPFSysBaseMVA(PFDouble);
		static PFDouble GetPFSysBaseMVA();
		//��ͨ��Ա����
		/******************************************************************************
		*		��ʼ��ϵͳ����
		******************************************************************************/
		PFUInt InitSysDataByFile(std::string& filepath, PF_DATA_FILE_FORMAT format);		//��ʼ��ϵͳ����
		PFVoid InitSysDataByDB(PF_DATA_DB_FORMAT format);									//��ʼ��ϵͳ����
		PFVoid OutputSysData2File(std::string& filepath, PF_DATA_FILE_FORMAT format);		//���ϵͳ�������ļ�
		PFVoid OutputSysData2DB(PF_DATA_DB_FORMAT format);									//���ϵͳ��������ݿ�

		PFVoid FormAssociation();															//�γ�ϵͳ�����ӹ�ϵ
		PFVoid SolvePowerInjection();														//�γ�ϵͳ���ʻ�����
		PFVoid SolveYGBMatrix();															//���ڵ㵼�ɾ��󣬲��洢����Ӧ��Ԫ
		PFUInt GetSysSize();																//���ϵͳ��ģ
		PFVoid BackupVoltage();																//���ݵ�ǰϵͳ��ѹ��Ϣ
		PFVoid RewindVoltage();																//�ָ�ϵͳ��ѹ����ֵ
		PFVoid BackupPower();
		PFVoid RewindPower();
		PFVoid BackupBusType();
		PFVoid RewindBusType();
		PFVoid BackupBusCase();
		PFVoid RewindBusCase();

		//TEST
		PFVoid OutputYGBMatrix2DOS();														//����ڵ㵼�ɾ���
		PFVoid OutputBusVoltageInfo2Dos();													//����ڵ��ѹ��Ϣ
		PFVoid OutputBusPowerInfo2Dos();													//����ڵ㹦����Ϣ
		PFVoid OutputVoltageInfo2File(const std::string file);									//�����ѹ��Ϣ���ļ�			

	protected:
		PFVoid _GetSysDataByFile_IEEE(std::string& filepath);								//��ȡIEEE��׼����
		PFVoid _OutputSysData2File_IEEE(std::string& filepath);								//���IEEE��׼����
		PFVoid _GetSysDataByFile_BPA(std::string& filepath);								//��ȡBPA��׼����
		PFVoid _OutputSysData2File_BPA(std::string& filepath);								//���BPA��׼����
		PFVoid _GetSysDataByFile_XML(std::string& filepath);								//��ȡxml��ʽ����
		PFVoid _OutputSysData2File_XML(std::string& filepath);								//���xml��׼����
		PFVoid _GetSysDataByDB_MYSQL();														//��ȡMySql���ݿ�������
	private:
		PFVoid _GetSysDataByFile_IEEE_BUS(std::ifstream& fin);								//��ȡIEEE������BUS��
		PFVoid _GetSysDataByFile_IEEE_BRANCH(std::ifstream& fin);							//��ȡIEEE������BRANCH��

	//��Ա����
	public:
		PFUInt			PFSysSize;	//ϵͳ��ģ

		PFAreaVec		PFSysArea;	//����
		PFBusVec		PFSysBus;	//����ڵ�
		PFBranchVec		PFSysBranch;//����֧·
	protected:
	private:
		PFBusVec        PFSysBusCase;//����ڵ㱸�� -- ����XX��������ǰ��̬����
		static PFDouble SysBaseMVA;	//ϵͳ���ʻ�׼ֵ
	};
}

#endif