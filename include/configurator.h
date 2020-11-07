#ifndef FC_CONFIGURATOR_H
#define FC_CONFIGURATOR_H

#include "infrared.h"

class Configurator : public IRControlled
{
public:
    Configurator();
    virtual ~Configurator();
    virtual void irmsgRecd(uint32_t code);
    void start();
    void stop();
    void poll();

private:
    unsigned long startedAt;
    bool running;
};

#endif // FC_CONFIGURATOR_H