#ifndef DRIVER_HPP
#define DRIVER_HPP

class Driver {
public:
    Driver();
    ~Driver();

    virtual void activate();
    virtual int reset();
    virtual void deactivate();
};

class DriverManager {
public:
    Driver* drivers[265];
    int num_drivers;

public:
    DriverManager();
    void add_driver(Driver*);
    void activate_all();
};

#endif
