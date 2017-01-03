#include "DiamondSquare.h"


DiamondSquare::DiamondSquare(int s, int r, int min, int max)
{
	min_val = min;
	max_val = max;

	map = new double *[s];
	
	for (int i = 0; i < s; i++)
	{
		map[i] = new double[s];
		for (int j = 0; j < s; j++)
			map[i][j] = 0.0;
	}

	size = s;
	range = r;
}

DiamondSquare::~DiamondSquare()
{
	for (int i = 0; i < size; i++)
	{
		delete map[i];
	}
	delete map;
}

double ** DiamondSquare::process()
{
	_on_start();

	for (int sideLength = size - 1; sideLength >= 2; sideLength /= 2, range /= 2)
	{
		int halfSide = sideLength / 2;

		squareStep(sideLength, halfSide);
		diamondStep(sideLength, halfSide);
	}

	boxBlurAlgo(map, 0.8);

	return map;
}

/**
* Performs the diamond step on the map.
*/
void DiamondSquare::diamondStep(int sideLength, int halfSide)
{
	for (int x = 0; x < size - 1; x += halfSide)
	{
		for (int y = (x + halfSide) % sideLength; y < size - 1; y += sideLength)
		{
			double avg = map[(x - halfSide + size - 1) % (size - 1)][y] +
				map[(x + halfSide) % (size - 1)][y] +
				map[x][(y + halfSide) % (size - 1)] +
				map[x][(y - halfSide + size - 1) % (size - 1)];
			avg /= 4.0 + dRand(-range, range);
			avg = normalize(avg);
			map[x][y] = avg;

			if (x == 0) map[size - 1][y] = avg;
			if (y == 0) map[x][size - 1] = avg;
		}
	}
}

/**
* Performs the square step on the map.
*/
void DiamondSquare::squareStep(int sideLength, int halfSide)
{
	for (int x = 0; x < size - 1; x += sideLength)
	{
		for (int y = 0; y < size - 1; y += sideLength)
		{
			double avg = map[x][y] + map[x + sideLength][y] + map[x][y + sideLength] + map[x + sideLength][y + sideLength];
			avg /= 4.0;
			map[x + halfSide][y + halfSide] = normalize(avg + dRand(-range, range));
		}
	}
}

double DiamondSquare::normalize(int value) {
	return round(std::max(std::min(value, 255), 0));
} 

void DiamondSquare::_on_start()
{
	// Defining the corners values :
	map[0][0] = map[0][size - 1] = map[size - 1][0] = map[size - 1][size - 1] = 100;
	// Initializing srand for random values :
	srand(time(NULL));
}

double DiamondSquare::dRand(double dMin, double dMax)
{
	double d = (double)rand() / RAND_MAX;
	return dMin + d * (dMax - dMin);
}


void DiamondSquare::boxBlurAlgo(double** map, double radius)
{
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			int val = 0;

			for (int iy = i - radius; iy < i + radius + 1; iy++)
			{
				for (int ix = j - radius; ix < j + radius + 1; ix++)
				{
					int x = std::min(size - 1, std::max(0, ix));
					int y = std::min(size - 1, std::max(0, iy));
					val += map[x][y];
				}
			}
			map[i][j] = val / ((radius + radius + 1)*(radius + radius + 1));
		}
	}
}