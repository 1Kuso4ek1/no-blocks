#include <random>
#include <PerlinNoise.hpp>

class World
{
public:
    World(uint32_t size);
    World(uint32_t seed, uint32_t size);

    ~World();

    float GetNoise(float x, float y);
    float GetNoise(float x, float y, float z);

    long double Random(long double min, long double max);
private:
    siv::PerlinNoise* noise;

    std::random_device r;
    std::default_random_engine eng;

    double freq, fx, fy, fz;
    uint32_t oct, seed, size;
};