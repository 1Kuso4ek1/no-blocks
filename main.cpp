#include <Engine.h>
#include <random>
#include <PerlinNoise.hpp>
#include <Physics.h>

std::random_device r;
std::default_random_engine eng(r());

long double Random(long double min, long double max)
{
	return std::uniform_real_distribution<long double>(min, max)(eng);
}

void Deform(Model& chunk, float offsetX, float offsetY, sf::Image& map, double strenght)
{
    for(int i = 0; i < chunk.numVerts * 3; i += 3)
    {
        chunk.vertexArray[i + 1] = (((float)map.getPixel(std::abs(chunk.vertexArray[i] + offsetX), std::abs(chunk.vertexArray[i + 2] + offsetY)).r) / 255) * strenght;
    }
}

sf::Image CreateWorld(int imagesize = 512)
{
    sf::Image ret;
    ret.create(imagesize, imagesize);

    double freq = Random(0.1, 64), fx = ret.getSize().x / freq, fy = ret.getSize().y / freq;
    uint32_t oct = (uint32_t)Random(1, 16), seed = (uint32_t)Random(0, 2147483648);

    siv::PerlinNoise perlin(seed);

    for(int y = 0; y < ret.getSize().y; y++)
        for(int x = 0; x < ret.getSize().x; x++)
        {
            double col = perlin.accumulatedOctaveNoise2D_0_1(x / fx, y / fy, oct) * 255;
            ret.setPixel(x, y, sf::Color(col, col, col));
        }

    return ret;
}

int main()
{
    sf::Image map = CreateWorld();

    Engine engine;
    
    engine.GetSettings().depthBits = 24;
    engine.CreateWindow(1280, 720, "no-blocks");
    engine.Init(45, 1280, 720, 0.1, 5000);
    
    engine.GetWindow().setMouseCursorVisible(false);
    engine.GetWindow().setVerticalSyncEnabled(true);
    engine.GetWindow().setFramerateLimit(120);

    glewInit();

    engine.EventLoop([&](sf::Event& event) { if(event.type == sf::Event::Closed) engine.Close(); });
    
    Light l(GL_LIGHT0, 30, 30, 30);
    Camera cam(0, 10, 0, 0.1);

    Model chunk("data/models/chunk.obj", "data/textures/grass.jpg", 0, 0, 0);

    bool day = true;

    sf::Clock frametime, globaltime;

    l.SetParameters({ 0.05 }, GL_LINEAR_ATTENUATION);
    l.SetParameters({ 2, 2, 2, 1 }, GL_AMBIENT);

    GLuint skybox[6] = {
        LoadTexture("data/skybox/skybox_front.bmp"),
        LoadTexture("data/skybox/skybox_back.bmp"),
        LoadTexture("data/skybox/skybox_left.bmp"),
        LoadTexture("data/skybox/skybox_right.bmp"),
        LoadTexture("data/skybox/skybox_bottom.bmp"),
        LoadTexture("data/skybox/skybox_top.bmp")
    };

    engine.Loop([&]() 
    {
        float time = frametime.restart().asMilliseconds() / 20;
        time = time > 5 ? 5 : time;

        cam.Move(time);
        cam.Mouse(engine.GetWindow());
        cam.y += 2;
        cam.Look();
        cam.y -= 2;

        float gtime = globaltime.getElapsedTime().asSeconds();
        if(!day) l.SetParameters({ 0 + gtime / 65, 0 + gtime / 70, 0 + gtime / 70, 1 }, GL_AMBIENT);
        else l.SetParameters({ 3 - gtime / 70, 3 - gtime / 65, 3 - gtime / 65, 1 }, GL_AMBIENT);
        
        if(l.GetParameters(GL_AMBIENT)[0] >= 3) { day = true; globaltime.restart(); }
        else if(l.GetParameters(GL_AMBIENT)[0] <= 0) { day = false; globaltime.restart(); }

        auto amb = l.GetParameters(GL_AMBIENT);
        std::for_each(amb.begin(), amb.end() - 1, [](float& a) { a /= 4; });
        l.SetParameters(amb, GL_DIFFUSE);
        l.SetPosition(cam.x - 30, 30, cam.z - 30);

        l.Update();

        cam.y -= 0.1 * time;
        for(int i = -4 + cam.z / 10; i < 4 + cam.z / 10; i++)
            for(int j = -4 + cam.x / 10; j < 4  + cam.x / 10; j++)
            {
                Deform(chunk, j * 10, i * 10, map, 7);
                chunk.SetPosition(j * 10, 0, i * 10);
                chunk.Draw();

                float col = collision(cam.x, cam.y, cam.z, chunk).second.y;
                
                if(col != 0) cam.y += (col / 3) + (0.09 * time);
            }
                        
        l.SetParameters({ 0 }, GL_LINEAR_ATTENUATION);
        l.SetParameters({ 0.0, 0.0, 0.0, 1 }, GL_DIFFUSE);
        Shape::Draw(skybox, cam.x, cam.y, cam.z, 1000, 1000, 1000);
        l.SetParameters({ 0.05 }, GL_LINEAR_ATTENUATION);
    });

    engine.Launch();

    map.saveToFile("world.jpg");
}