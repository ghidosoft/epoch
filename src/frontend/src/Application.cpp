#include "Application.h"

namespace epoch
{
    Application::Application(std::shared_ptr<Emulator> emulator) : m_emulator{ std::move(emulator) }
    {
    }

    Application::~Application()
    {
    }

    int Application::run()
    {
        return 0;
    }
}
