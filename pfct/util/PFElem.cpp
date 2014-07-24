#include "PFElem.h"

#include <sstream>

using namespace PFModel;

const PFStr VoltageInfo::toString() const  {
	std::string res;
	std::stringstream ss;
	res = "Vol\t: ";
	ss << this->vol << std::endl;
	res += ss.str();
	ss.str("");
	ss << this->ang << std::endl;
	res += "Ang\t: ";
	res += ss.str();
	return res;
}

PFDouble PowerInfo::SBase = 100.0;

PFVoid PowerInfo::SetSBase(PFDouble sb) {
	PowerInfo::SBase = sb;
}

PFDouble PowerInfo::GetSBase() {
	return PowerInfo::SBase;
}

const PFStr PowerInfo::toString() const  {
	std::string res;
	std::stringstream ss;
	res = "P\t: ";
	ss << this->p << std::endl;
	res += ss.str();
	ss.str("");
	res += "Q\t: ";
	ss << this->q << std::endl;
	res += ss.str();
	return res;
}

const PFStr Parameter::toString() const {
	std::string res;
	std::stringstream ss;
	res = "G\t: ";
	ss << this->g << std::endl;
	res += ss.str();
	
	ss.str("");
	res += "B\t: ";
	ss << this->b << std::endl;
	res += ss.str();
	return res;
}

const PFStr Identification::toString() const{
	PFStr res;
	std::stringstream ss;
	res = "Index\t: ";
	ss << this->index << std::endl;
	res += ss.str();
	res += "Name\t: ";
	ss.str("");
	ss << this->name << std::endl;
	res += ss.str();
	return res;
}

const PFStr Discrete::toString() const{
	PFStr res;
	std::stringstream ss;
	res = "Val\t: ";
	ss << this->val << std::endl;
	res += ss.str();

	ss.str("");
	res += "ValHigh\t: ";
	ss << this->valHigh << std::endl;
	res += ss.str();

	ss.str("");
	res += "ValLow\t: ";
	ss << this->valLow << std::endl;
	res += ss.str();

	return res;
}
