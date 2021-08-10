#include <World.h>

World::World(uint32_t size) : eng(r()), freq(Random(0.1, 64)), oct((uint32_t)Random(1, 16)), seed((uint32_t)Random(0, 2147483648)), size(size)
{
    fx = fy = fz = size / freq;
    noise = new siv::PerlinNoise(seed);
}

World::World(uint32_t seed, uint32_t size) : eng(r()), seed(seed), freq(Random(0.1, 64)), oct((uint32_t)Random(1, 16)), size(size)
{
    fx = fy = fz = size / freq;
    noise = new siv::PerlinNoise(seed);
}

World::~World()
{
    delete noise;
}

float World::GetNoise(float x, float y)
{
    return noise->accumulatedOctaveNoise2D_0_1(x / fx, y / fy, oct);
}

float World::GetNoise(float x, float y, float z)
{
    return noise->accumulatedOctaveNoise3D_0_1(x / fx, y / fy, z / fz, oct);
}

long double World::Random(long double min, long double max)
{
    return std::uniform_real_distribution<long double>(min, max)(eng);
}