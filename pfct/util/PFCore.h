/*********************************************************************
*	NAME		:		PFCore.h
*	CONTENT		:		Redefinition of the matrix/vector in the PFCT
*	COPYRIGHT	:		ROCK(rockzheng1127@126.com)
*	DATE		:		2014/06/05 -- 2014//
*	DIRECTION	:		
**********************************************************************/

#ifndef _PFCore_H
#define _PFCore_H
#include <Eigen/Sparse>
#include <Eigen/Dense>
#include <Eigen/OrderingMethods>
#include "PFMenuDef.h"

namespace PFCore
{
	//������������ض���
	typedef Eigen::SparseMatrix<PFDouble> PFSMatrixXD;
	typedef Eigen::SparseVector<PFDouble> PFSVectorD;
	typedef Eigen::Triplet<PFDouble>	  PFTripletD;
	typedef std::vector<PFTripletD>       PFTripletDVec;
	typedef std::vector<PFDouble>		  PFDoubleVec;

	typedef Eigen::VectorXd				PFVectorXD;

	//��ⷽ���ض���

	//�ֽⷽ���ض���
	typedef Eigen::COLAMDOrdering<PFInt>    PFCOLAMDOrdering;
	typedef Eigen::AMDOrdering<PFInt>       PFAMDOrdering;
}

#endif	// end _PFCore_H