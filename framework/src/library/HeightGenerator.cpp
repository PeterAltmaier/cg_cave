#define _USE_MATH_DEFINES

#include "HeightGenerator.h"
#include <time.h>
#include <stdlib.h>
#include <math.h>


float HeightGenerator::getNoise(int x, int z)
{
    srand(x * 49632 + z * 32517 + this->seed);
    float r = ((float)rand()) / (float)RAND_MAX;
    r *= (float)(rand()) / (float)(RAND_MAX);
    r += (float)(rand()) / (float)(RAND_MAX);
    r += (float)(rand()) / (float)(RAND_MAX) /4.f;
    r*= (float)(rand()) / (float)(RAND_MAX);
    return r * 2.f -1;
}

float HeightGenerator::getSmoothNoise(int x, int z)
{
    float corners = (getNoise(x - 1, z - 1) + getNoise(x + 1, z - 1) + getNoise(x - 1, z + 1) + getNoise(x + 1, z + 1)) / 16.f;
    float sides = (getNoise(x - 1, z) + getNoise(x + 1, z) + getNoise(x, z - 1) + getNoise(x, z + 1)) / 8.f;
    float center = getNoise(x, z) / 4.f;
    return corners + sides + center;
}

float HeightGenerator::interpolate(float a, float b, float blend)
{
    double theta = blend * M_PI;
    float f = (1.f - cos(theta)) *0.5f;
    return a * (1.f -f) + b * f;
}

float HeightGenerator::getInterpolatedNoise(float x, float z)
{
    int intX = (int)x;
    int intZ = (int)z;
    float fracX = x - intX;
    float fracZ = z - intZ;

    float v1 = getSmoothNoise(intX, intZ);
    float v2 = getSmoothNoise(intX +1, intZ);
    float v3 = getSmoothNoise(intX, intZ +1);
    float v4 = getSmoothNoise(intX+1, intZ+1);
    float i1 = interpolate(v1, v2, fracX);
    float i2 = interpolate(v3, v4, fracX);
    return interpolate(i1, i2, fracZ);
}

HeightGenerator::HeightGenerator()
{
    srand(time(NULL));
    this->seed = rand()%1000000000;
    this->AMPLITUDE=8.f;
}

float HeightGenerator::generateHeight(int x, int z)
{
    float total = getInterpolatedNoise(x / 8.f, z / 8.f) * this->AMPLITUDE ;
    total += getInterpolatedNoise(x / 2.f, z / 2.f) * this->AMPLITUDE/3.f;
    //total += getInterpolatedNoise(x , z ) * this->AMPLITUDE / 9.f;
    return total;
}
