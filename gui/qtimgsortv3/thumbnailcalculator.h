#pragma once
#include "thumbnail.h"
class thumbnailCalculator
{
public:
	thumbnailCalculator();
	~thumbnailCalculator(){;}
	static void format(thumbnail* p,unsigned size);
	static void calculateFeatureData(thumbnail* p);
};