#pragma once
#include "USM.hpp"

class SCV : public USM
{
public: 
	SCV(Unit unit);
	~SCV() {};

	virtual void onFrame();
};