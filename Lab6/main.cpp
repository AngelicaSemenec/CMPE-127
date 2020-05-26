#include <project_config.hpp>

#include <cstdint>
#include <iterator>
#include <stdlib.h>
#include <string>

#include <math.h>

#include "L2_HAL/displays/led/onboard_led.hpp"
#include "utility/log.hpp"
#include "utility/time.hpp"

class Bus
{
    public:
    enum class ControlType
    {
        kMemory = 0,
        kIO
    };
    Bus()
    {
        Write.SetAsOutput();
        ALE.SetAsOutput();
        M_IO.SetAsOutput();

        Status = 0;

        ad[0].GetPin().SetAsOpenDrain();
        ad[1].GetPin().SetAsOpenDrain();
        ad[2].GetPin().SetAsOpenDrain();
        ad[3].GetPin().SetAsOpenDrain();
        ad[4].GetPin().SetAsOpenDrain();
        ad[5].GetPin().SetAsOpenDrain();
        ad[6].GetPin().SetAsOpenDrain();
        ad[7].GetPin().SetAsOpenDrain();
        
    }

    void WritetoMem(int address, int data)
    {
        sendAddress(address);
        Status = 0;
        writeData(data);
        Status = 0;

        return;
    }

    int ReadfromMem(int address)
    {
        sendAddress(address);
        Status = 0;
        int data = readData();
        Status = 0;

        return data;
    }

    void move_cursor()
    {
        ad[4].SetLow();
        Set_Low();
        ad[0].SetHigh();
        ad[1].SetHigh();
        pulse_enable();
        Set_Low();
        pulse_enable();
        ad[4].SetHigh();
    }

    void clear_display()
    {
        Set_Low();
        ad[4].SetLow();
        pulse_enable();
        ad[3].SetHigh();
        pulse_enable();
        return;
    }

    void write_char(char symbol)
    {
        int ascii = int(symbol);
        int binary[8];
        ad[4].SetHigh();
        for (int i = 0; i < 8; i ++)
        {
            if((ascii & int(pow(2,i))) > 0)
            {
                binary[i] = 1;
            }
            else
            {
                binary[i] = 0;
            }
        }
        //Set upper 4 bits
        int count = 3;
        for(int i = 4; i < 8; i ++)
        {
            if(binary[i] == 0)
            {
                ad[count].SetLow();
            }
            else
            {
                ad[count].SetHigh();
            }
            count --;
        }
        pulse_enable();
        //Set lower 4 bits
        count = 3;
        for(int i = 0; i < 4; i ++)
        {
            if(binary[i] == 0)
            {
                ad[count].SetLow();
            }
            else
            {
                ad[count].SetHigh();
            }
            count --;
        }
        pulse_enable();
        inc_cursor();
    }

    void inc_cursor()
    {
        Set_Low();
        ad[4].SetLow();
        pulse_enable();
        ad[1].SetHigh();
        ad[2].SetHigh();
        pulse_enable();
    }

    void Set_Low()
    {
        ad[0].SetLow();
        ad[1].SetLow();
        ad[2].SetLow();
        ad[3].SetLow();
    }

    void pulse_enable()
    {
        Delay(1);
        ALE.SetLow();
        Delay(1);
        ALE.SetHigh();
        Delay(1);
    }

    void calculate()
    {
        int calc = 0;
        int memory_location = 0;

        while(calc != -1)
        {
            calc = calc_kp(memory_location);
            if (calc == 1)
            {
                memory_location ++;
            }
        }

        int total = 0;
        int numbers[32];
        char symbol[32];
        int num_top = -1;
        int operation = 1;
        int location_count = 0;
        int data;

        while (location_count <= memory_location)
        {
            if (location_count != memory_location)
            {
                data = ReadfromMem(location_count);
                symbol[location_count] = num_to_char(data);
            }
            else
            {
                symbol[location_count] = '=';
            }
            

            //not + or -
            if (data != 43 && data != 45 && location_count != memory_location)
            {
                num_top ++;
                numbers[num_top] = data;
            }
            else if (data == 43 || data == 45 || location_count == memory_location)
            {
                int index = num_top;
                int temp_val = 0;
                for(int i = 0; i <= index; i++)
                {
                    temp_val = temp_val + (numbers[num_top] * int(pow(10, i)));
                    num_top --;
                }
                if (operation == 1)
                {
                    total = total + temp_val;
                }
                else if (operation == 2)
                {
                    total = total - temp_val;
                }
                if (data == 43)
                {
                    operation = 1;
                }
                else if (data == 45)
                {
                    operation = 2;
                }
            }
            location_count ++;
        }

        printf("%d\n", total);

        if (total < 0)
        {
            symbol[location_count] = '-';
            location_count ++;
            total = total * -1;
        }

        char num[32];
        int count = -1;
        int value = total;
        int x;
        while (value != 0)
        {
            count ++;
            value = total / int(pow(10, count));
            x = value % 10;
            num[count] = num_to_char(x);
        }

        while (count != 0)
        {
            count --;
            symbol[location_count] = num[count];
            location_count ++;
        }

        LCD_call();

        count = 0;

        clear_display();
        for(int i = 0; i < 2; i ++)
        {
            while (count != location_count)
            {
                if (count == 16)
                {
                    move_cursor();
                }
                write_char(symbol[count]);
                count ++;
            }
        }

        Write.SetLow();
        ALE.SetLow();
        return;
    }

    int calc_kp(int memory_location)
    {
        setupIO();

        int address = 0;
        int state = 0;
        char c;
        for (int i = 0; i < 4; i ++)
        {
            setIOaddress(state);
            address = checkKeyPress(state);
            if (address != 0)
            { 
                c = getChar(address);
                printf("%c", c);
                Delay(100);
                int num = char_to_num(c);
                Write.SetLow();
                Delay(1);
                WritetoMem(memory_location, num);
                if (c == '=')
                {
                    return -1;
                }
                else
                {
                    return 1;
                }
            }
            state = nextState(state);
        }
        return 0;
    }

    char num_to_char(int num)
    {
        switch(num)
        {
            case 0: return '0';
            case 1: return '1';
            case 2: return '2';
            case 3: return '3';
            case 4: return '4';
            case 5: return '5';
            case 6: return '6';
            case 7: return '7';
            case 8: return '8';
            case 9: return '9';
            case 43: return '+';
            case 45: return '-';
        }
        return -1;
    }

    int char_to_num(char c)
    {
        switch(c)
        {
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case '+': return 43;
            case '-': return 45;
        }
        return -1;
    }

    char getChar(int address)
    {
        char key;
        switch (address)
        {
            case 17: key = '1'; break;
            case 18: key = '4'; break;
            case 20: key = '7'; break;
            case 24: key = '*'; break;
            case 33: key = '2'; break;
            case 34: key = '5'; break;
            case 36: key = '8'; break;
            case 40: key = '0'; break;
            case 65: key = '3'; break;
            case 66: key = '6'; break;
            case 68: key = '9'; break;
            //#
            case 72: key = '='; break;
            //A
            case 129: key = '+'; break;
            //B
            case 130: key = '-'; break;
            case 132: key = 'C'; break;
            case 136: key = 'D';    
        }
       return key;
    }

    void setupIO(void)
    {
        ALE.SetLow();
        M_IO.SetLow();
        Write.SetHigh();

        ad[0].SetAsOutput();
        ad[1].SetAsOutput();
        ad[2].SetAsOutput();
        ad[3].SetAsOutput();

        ad[4].SetAsInput();
        ad[5].SetAsInput();
        ad[6].SetAsInput();
        ad[7].SetAsInput();
    }

    void setIOaddress(int state)
    {
        Write.SetHigh();

        switch (state)
        {
            case 0:
                ad[0].SetHigh();
                ad[1].SetLow();
                ad[2].SetLow();
                ad[3].SetLow();
                break;
            case 1:
                ad[0].SetLow();
                ad[1].SetHigh();
                ad[2].SetLow();
                ad[3].SetLow();
                break;
            case 2:
                ad[0].SetLow();
                ad[1].SetLow();
                ad[2].SetHigh();
                ad[3].SetLow();
                break;
            case 3:
                ad[0].SetLow();
                ad[1].SetLow();
                ad[2].SetLow();
                ad[3].SetHigh();
                break;
        }
        Delay(1);
    }

    int checkKeyPress(int state)
    {
        Write.SetLow();
        Delay(1);
        for (int i = 4; i < 8; i ++)
        {
            if (ad[i].Read() == true)
            {
                return (int(pow(2,state) + int(pow(2, i) ) ) );
            }
        }
        return 0;
    }

    int nextState(int state)
    {
        state ++;
        if (state == 4)
            state = 0;
        return state;
    }

    void sendAddress(int address)
    {
        Status = 1;

        ALE.SetHigh();
        M_IO.SetLow();
        Write.SetHigh();

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
        return;
    }

    void writeData(int data)
    {
        Status = 2;

        ALE.SetLow();
        M_IO.SetHigh();
        Write.SetHigh();

        for (int i = 0; i < 8; i++)
        {
            ad[i].SetAsOutput();
            if((data & int(pow(2,i))) > 0)
            {
                ad[i].SetHigh();
            }
            else
            {
                ad[i].SetLow();
            }
        }
        return;
    }

    int readData()
    {
        Status = 3;

        Write.SetLow();
        ALE.SetLow();
        M_IO.SetHigh(); 

        int data = 0;

        for (int i = 0; i < 8; i ++)
        {
            ad[i].SetAsInput();
            if(ad[i].Read() == true)
            {
                data = data + int(pow(2,i));
            }
        }

        return data;
    }
    void LCD_setup_4bit()
    {
        power_on();
        configure_4bit();
    }

    void power_on()
    {
        
        Delay(15);
        ad[4].SetLow();
        Delay(1);
        ALE.SetLow();
        Delay(5);
        ALE.SetHigh();
    }

        void configure_4bit()
    {
        ad[2].SetHigh();
        pulse_enable();
        pulse_enable();
        Set_Low();
        ad[0].SetHigh();
        pulse_enable();
        Set_Low();
        pulse_enable();
        ad[0].SetHigh();
        ad[1].SetHigh();
        ad[2].SetHigh();
        pulse_enable();
        Set_Low();
        pulse_enable();
        ad[1].SetHigh();
        ad[2].SetHigh();
        pulse_enable();

        char qm[] = "QuickMaths";
        for(int i = 0; i < 10; i ++)
        {
            write_char(qm[i]);
        }

        Write.SetLow();
        ALE.SetLow();
        return;
    }

    void LCD_call()
    {
        M_IO.SetLow();
        Write.SetHigh();
        ALE.SetHigh();
        for (int i = 0; i < 8; i ++)
        {
            ad[i].SetAsOutput();
            ad[i].SetHigh();
        }
        Delay(1);
        Set_Low();
    }

    private:
    Gpio Write = Gpio(0, 17);
    Gpio ALE = Gpio(0, 22);
    Gpio M_IO = Gpio(0, 0);
    //Gpio Int = Gpio(0, 11);

    int Status;

    Gpio ad[8] = {
      Gpio(2, 2),
      Gpio(2, 5),
      Gpio(2, 7),
      Gpio(2, 9),
      Gpio(0, 15),
      Gpio(0, 18),
      Gpio(0, 1),
      Gpio(0, 10)
    };
};

int main(void)
{
    Bus obj;
    int op = -1;

    obj.LCD_call();
    obj.LCD_setup_4bit();

    while(op != 0)
    {
        printf("\n Please enter the number of the function you'd like to perform:");
            printf("\n 0. Quit");
            printf("\n 1. Quick maths");
            printf("\n 2. Read from Memory\n");
        scanf("%d", &op);
        if (op == 1)
        {
            obj.calculate();
        }
        else if (op == 2)
        {
            int address;
            int data;
            printf("\n Enter address: ");
            scanf("%d", &address);
            data = obj.ReadfromMem(address);
            printf("\n The value stored at %d is %d", address, data);
        }
        else
        {
          printf("\n The operation entered is invalid.\n");
        }
    
    }
    return 0;
  }