#include "Program.h"
#include <Arduino.h>

Program prg;
void setup()
{
  prg.setup();
}

void loop()
{
  prg.tick();
}