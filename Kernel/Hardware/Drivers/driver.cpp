#include "driver.hpp"

Driver::Driver()
{
}

Driver::~Driver()
{
}

void Driver::activate()
{
}

int Driver::reset()
{
    return 0;
}

void Driver::deactivate()
{
}

DriverManager::DriverManager()
{
    num_drivers = 0;
}

void DriverManager::add_driver(Driver* drv)
{
    drivers[num_drivers] = drv;
    num_drivers++;
}

void DriverManager::activate_all()
{
    for (int i = 0; i < num_drivers; i++)
        drivers[i]->activate();
}
