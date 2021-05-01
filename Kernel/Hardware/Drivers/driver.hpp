#ifndef DRIVER_HPP
#define DRIVER_HPP

class Driver {
public:
    Driver();
    ~Driver();

    virtual void Activate();
    virtual int Reset();
    virtual void Deactivate();
};

class DriverManager {
public:
    Driver* drivers[265];
    int num_drivers;

public:
    DriverManager();
    void AddDriver(Driver*);
    void ActivateAll();
};

#endif
