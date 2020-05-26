#include <project_config.hpp>

#include <cstdint>
#include <iterator>

#include <math.h>

#include "L2_HAL/displays/led/onboard_led.hpp"
#include "utility/log.hpp"
#include "utility/time.hpp"

int main(void)
{
      Gpio ad[] = {
        Gpio(2, 2),
        Gpio(2, 5),
        Gpio(2, 7),
        Gpio(2, 9),
        Gpio(0, 15),
        Gpio(0, 18),
        Gpio(0, 1),
        Gpio(0, 10)
    };

    Gpio Write = Gpio(0, 17);
    Gpio ALE = Gpio(0, 22);
    Gpio M_IO = Gpio(0, 0);

    for (int i = 0; i < 8; i ++)
    {
      ad[i].GetPin().SetAsOpenDrain();
    }

    int address = 0;

    Write.SetAsOutput();
    Write.SetHigh();

    ALE.SetAsOutput();
    ALE.SetHigh();

    while(address != -1)
    {
      printf("Please enter the address or type '-1' to quit: ");
      scanf("%d", &address);
      if (address != -1)
      {
        for (int i = 0; i < 8; i++)
        {
          ad[i].SetAsOutput();
          if((address & int(pow(2,i))) > 0)
            {
              ad[i].SetHigh();
            }
          else
            {
              ad[i].SetLow();
            }
        }
      }

    }

  return 0;
}

