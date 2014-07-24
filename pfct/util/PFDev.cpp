
#include "PFDev.h"

using namespace PFDev;

/**********************************************
*		Model_BusBarSection		:	Ä¸Ïß
**********************************************/
Elcbus::Elcbus(){
	this->uid = PF_ID_DEFAULT;
	this->name = PF_STR_DEFAULT;
	this->tpnd = PF_STR_DEFAULT;
	this->nomKV = PF_DOUBLE_DEFAULT;
	this->voltKV = PF_DOUBLE_DEFAULT;
	this->vol = PF_DOUBLE_DEFAULT;
	this->ang = PF_DOUBLE_DEFAULT;
	this->node = PF_ID_DEFAULT;
	this->stUID = PF_ID_DEFAULT;
	this->bvUID = PF_ID_DEFAULT;
	this->vlUID = PF_ID_DEFAULT;
	this->volHigh = PF_DOUBLE_DEFAULT;
	this->volLow  = PF_DOUBLE_DEFAULT;
}
