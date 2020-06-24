#pragma once
class HeightGenerator{
    private:
        float AMPLITUDE;
        int seed;
        float* roughness;
        float* amp_fit;
        size_t arr_size;
        float getNoise(int x, int z);
        float getSmoothNoise(int x, int z);
        float interpolate(float a, float b, float blend);
        float getInterpolatedNoise(float x, float z);
        
    public:
        HeightGenerator(float fAMPLITUDE, float* roughness, float* amp_fit, size_t arr_size);
        float generateHeight(int x, int z);

};
