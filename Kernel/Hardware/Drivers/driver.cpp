#include "driver.hpp"

Driver::Driver()
{
}

Driver::~Driver()
{
}

void Driver::Activate()
{
}

int Driver::Reset()
{
    return 0;
}

void Driver::Deactivate()
{
}

DriverManager::DriverManager()
{
    num_drivers = 0;
}

void DriverManager::AddDriver(Driver* drv)
{
    drivers[num_drivers] = drv;
    num_drivers++;
}

void DriverManager::ActivateAll()
{
    for (int i = 0; i < num_drivers; i++)
        drivers[i]->Activate();
}
