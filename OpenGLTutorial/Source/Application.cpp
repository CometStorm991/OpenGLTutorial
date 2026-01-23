#include "Application.hpp"

Application::Application()
    : demo(std::make_unique<Instancing>())
{
}

void Application::prepare()
{
    demo->prepare();
}

void Application::run()
{
    demo->run();
}

bool Application::shouldEnd()
{
    return demo->shouldEnd();
}

void Application::terminate()
{
    demo->terminate();
}