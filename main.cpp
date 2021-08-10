#include <Engine.h>
#include <Physics.h>
#include <World.h>

void Deform(Model& chunk, float offsetX, float offsetY, World& world, double strenght)
{
    for(int i = 0; i < chunk.numVerts * 3; i += 3)
    {
        chunk.vertexArray[i + 1] = world.GetNoise(std::abs(chunk.vertexArray[i] + offsetX), std::abs(chunk.vertexArray[i + 2] + offsetY)) * strenght;
    }
}

int main()
{
    World world(512);

    Engine engine;
    
    engine.GetSettings().depthBits = 24;
    engine.CreateWindow(1280, 720, "no-blocks");
    engine.Init(45, 1280, 720, 0.1, 5000);
    
    engine.GetWindow().setMouseCursorVisible(false);
    engine.GetWindow().setVerticalSyncEnabled(true);
    engine.GetWindow().setFramerateLimit(60);

    glewInit();

    engine.EventLoop([&](sf::Event& event) { if(event.type == sf::Event::Closed) engine.Close(); });
    
    Light l(GL_LIGHT0, 30, 30, 30);
    Camera cam(world.Random(-512, 512), 10, world.Random(-512, 512), 0.1);

    Model chunk("data/models/chunk.obj", "data/textures/grass.jpg", 0, 0, 0);

    bool day = true;

    sf::Clock frametime, globaltime;

    l.SetParameters({ 0.05 }, GL_LINEAR_ATTENUATION);
    l.SetParameters({ 3, 3, 3, 1 }, GL_AMBIENT);

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
        float time = (float)frametime.restart().asMilliseconds() / 40;
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

        std::vector<float> amb = l.GetParameters(GL_AMBIENT);
        std::for_each(amb.begin(), amb.end() - 1, [](float& a) { a /= 4; });
        l.SetParameters(amb, GL_DIFFUSE);
        l.SetPosition(cam.x - 30, cam.y + 30, cam.z - 30);

        l.Update();

        cam.y -= 0.1;
        for(int i = -2 + cam.z / 10; i < 2 + cam.z / 10; i++)
            for(int j = -2 + cam.x / 10; j < 2  + cam.x / 10; j++)
            {
                Deform(chunk, j * 10, i * 10, world, 7);
                chunk.SetPosition(j * 10, 0, i * 10);
                chunk.Draw();
                cam.y += (collision(cam.x, cam.y, cam.z, chunk).second.y / 3);
            }
                        
        l.SetParameters({ 0 }, GL_LINEAR_ATTENUATION);
        l.SetParameters({ 0.0, 0.0, 0.0, 1 }, GL_DIFFUSE);
        Shape::Draw(skybox, cam.x, cam.y, cam.z, 1000, 1000, 1000);
        l.SetParameters({ 0.05 }, GL_LINEAR_ATTENUATION);
    });

    engine.Launch();
}