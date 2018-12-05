#pragma once
#include <BWAPI.h>
#include <BWTA.h>

using namespace BWAPI;
using namespace BWTA;

class USM
{
public:
	USM(Unit unit) { this->mUnit = unit; };
	~USM() {};

	virtual void onFrame() = 0;
private:
	Unit mUnit;
};