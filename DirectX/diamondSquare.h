#ifndef _DIAMONSQUARE_H_
#define _DIAMONSQUARE_H_

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <algorithm>
//#include <unistd.h>


class DiamondSquare
{
private:
	double random_range;
	double min_val;
	double max_val;

	double ** map;
	int size;

	int range;

public:
	DiamondSquare(int s, int r, int min, int max);
	~DiamondSquare();

	double** process();
	void _on_start();
	void diamondStep(int, int);
	void squareStep(int, int);
	double normalize(int value);
	double dRand(double dMin, double dMax);
	void boxBlurAlgo(double** map, double radius);
};


#endif