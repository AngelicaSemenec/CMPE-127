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

    void useLCD()
    {
        Status = 0;
        M_IO.SetLow();
        Write.SetHigh();
        ALE.SetHigh();
        for (int i = 0; i < 8; i ++)
        {
            ad[i].SetAsOutput();
            ad[i].SetHigh();
        }
        Set_Low();
        LCD_setup_4bit();

        LCD_prompt();

        ALE.SetLow();
        Write.SetHigh();
        return;
    }

    void LCD_prompt()
    {
        char text[32];
        //char c;
        //int i = 0;
        while (true)
        {
            printf("\nWrite to LCD or enter ':q' to quit: ");
            scanf("%s", text);
            printf("%s", text);
            if (text[0] == ':' and text[1] == 'q')
            {
                return;
            }
            clear_display();
            for (int i = 0; i < strlen(text); i ++)
            {   
                if(i == 16)
                {
                    move_cursor();
                }
                write_char(text[i]);
            }
        }
        return;
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
        printf("\nsym: %s, # %d ", &symbol, ascii);
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
        for (int i = 7; i >= 0; i --)
        {
            printf("%d", binary[i]);
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
        //ad[2].SetHigh();
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
        ad[4].SetHigh();
        ad[1].SetHigh();
        pulse_enable();
        Set_Low();
        ad[0].SetHigh();
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
        //Writes to memory until = is detected
        while(calc != -1)
        {
            //Return -1 when detects =
            //Returns 0 when no keypress detected
            //returns 1 when kp_detected
            calc = calc_kp(memory_location);
            if (calc == 1)
            {
                memory_location ++;
            }
            //Delay(100);
        }
        int val;
        //if math == 0 --> none
        //if math == 1 --> +
        //if math == 2 --> -
        int total = 0;
        char symb[32];
        int stack[32];
        int top = -1;

        int curr;
        int count;
        int math;

        for(curr = 0; curr < memory_location; curr++)
        {
            val = ReadfromMem(curr);
            printf("\n val: %d", val);
            symb[curr] = char(val);
            printf("\n symb[curr]: %c", symb[curr]);
            if (symb[curr] != '+' && symb[curr] != '-')
            {
                top ++;
                stack[top] = val;
            }
            else if (symb[curr] == '+')
            {
                //If no previous operation, set new operation to +
                if(math == 0)
                {
                    math = 1;
                }
                //If prior operation was addition, add previous number, keep +
                else if (math == 1)
                {
                    count = 0;
                    while(top >= 0)
                    {
                        total = total + (stack[top] * int(pow(10, count)));
                        top --;
                        count ++;
                    }
                }
                //If prior operation was subtraction, subtract previous number, make +
                else if (math == 2)
                {
                    count = 0;
                    while (top >= 0)
                    {
                        total = total - (stack[top] * int(pow(10, count)));
                        top --;
                        count ++;
                    }
                    math = 1;
                }
            }
            else if (symb[curr] == '-')
            {
                //If no prior operation, set as -
                if(math == 0)
                {
                    math = 2;
                }
                //If prior operation was addition, add previous number, keep +
                else if (math == 1)
                {
                    count = 0;
                    while(top >= 0)
                    {
                        total = total + (stack[top] * int(pow(10, count)));
                        top --;
                        count ++;
                    }
                    math = 2;
                }
                //If prior operation was subtraction, subtract previous number, make +
                else if (math == 2)
                {
                    count = 0;
                    while (top >= 0)
                    {
                        total = total - (stack[top] * int(pow(10, count)));
                        top --;
                        count ++;
                    }
                }
            }
        }
        if (math == 1)
        {
            count = 0;
            while(top >= 0)
            {
                total = total + (stack[top] * int(pow(10, count)));
                top --;
                count ++;
            }
        }
        else if (math == 2)
        {
            count = 0;
            while (top >= 0)
            {
                total = total - (stack[top] * int(pow(10, count)));
                top --;
                count ++;
            }
        }
        printf("%d\n", total);
        symb[curr] = '=';
        curr ++;
        char int_string[32];
        count = 0;
        int value = total;
        int x;
        while (value != 0)
        {
            value = total / int(pow(10, count));
            x = value % 10;
            int_string[count] = char(x);
            count ++;
        }

        while (count != 0)
        {
            top --;
            symb[curr] = int_string[count];
            curr ++;
        }
        symb[curr] = '\n';
        //symb[curr] should contain all of the text to print at this point
        //Sets up LCD signals
        LCD_call();

        curr = 0;
        clear_display();
        while (symb[curr] != '\n')
        {
            printf("%c", symb[curr]);
            write_char(symb[curr]);
            curr ++;
        }

        Write.SetLow();
        ALE.SetLow();
        return;
    }

    void LCD_call()
    {
        Status = 0;
        M_IO.SetLow();
        Write.SetHigh();
        ALE.SetHigh();
        for (int i = 0; i < 8; i ++)
        {
            ad[i].SetAsOutput();
            ad[i].SetHigh();
        }
        Set_Low();
        LCD_setup_4bit();
    }

    int calc_kp(int memory_location)
    {
        setupIO();
        //printf("\n IO");

        int address = 0;
        int state = 0;
        char c;
        for (int i = 0; i < 4; i ++)
        {
            setIOaddress(state);
            address = checkKeyPress(state);
            //printf("%d", address);
            if (address != 0)
            { 
                c = getChar(address);
                printf("%c", c);
                Delay(100);
                int sym = int(c);
                printf("\n sym: %d", sym);
                WritetoMem(sym, memory_location);
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

    void ISR_call(void)
    {
        int bitArr[8];
        switch (Status)
        {
            //Case 0: Current bits do not need to be saved
            case 0:
                break;
            //Case 1: Status is in SendAddress
            //Bit status should be saved if function has begun calculating sent address
            case 1:
            //Case 2: Status is in WriteData
            //Bit status should be saved if function has begun calculating data bits
            case 2:
                for (int i = 0; i < 8; i ++)
                {
                    if (ad[i].Read() == true)
                    {
                        bitArr[i] = 1;
                    }
                    else
                    {
                        bitArr[i] = 0;
                    }
                }
                break;
            //Case 3: Status is in ReadData
            //Current bit status does not need to be saved
            case 3:
                break;
        }

        //Write letter from keypad to console
        WritefromIO();

        switch (Status)
        {
            //Status does not need to be updated
            case 0:
                return;
            //Status is in SendAddress
            //Control signals reset
            case 1:
                ALE.SetHigh();
                M_IO.SetLow();
                Write.SetHigh();
                for (int i = 0; i < 8; i ++)
                {
                    ad[i].SetAsOutput();
                }
                for (int i = 0; i < 8; i++)
                {
                    if (bitArr[i] == 0)
                    {
                        ad[i].SetLow();
                    }
                    else
                    {
                        ad[i].SetHigh();
                    }
                    
                }
                return;
            //Status is in WriteData
            case 2: 
                ALE.SetLow();
                M_IO.SetHigh();
                Write.SetHigh();
                for (int i = 0; i < 8; i ++)
                {
                    ad[i].SetAsOutput();
                }
                for (int i = 0; i < 8; i++)
                {
                    if (bitArr[i] == 0)
                    {
                        ad[i].SetLow();
                    }
                    else
                    {
                        ad[i].SetHigh();
                    }
                    
                }
                return;
            //Status is in ReadData
            case 3:
                Write.SetLow();
                ALE.SetLow();
                M_IO.SetHigh();
                for (int i = 0; i < 8; i ++)
                {
                    ad[i].SetAsInput();
                }
                return;
        }
    }

    void WritefromIO(void)
    {
        setupIO();

        int address = 0;
        int state = 0;
        while (address == 0)
        {
            setIOaddress(state);
            address = checkKeyPress(state);
            if (address != 0)
            { 
                printChar(address);
                return;
            }
            state = nextState(state);
        }
        //printf("\n Error.");
        return;
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

    void printChar(int address)
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
            case 72: key = '#'; break;
            case 129: key = 'A'; break;
            case 130: key = 'B'; break;
            case 132: key = 'C'; break;
            case 136: key = 'D';    
        }
        printf("%c\n", key);
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



Bus obj;

void ISR()
{
    //printf("\n In ISR");
    obj.ISR_call();
}

int main(void)
{
    Gpio Int = Gpio(0, 11);
    Int.GetPin().SetMode(Pin::Mode::kPullUp);
    Int.AttachInterrupt(&ISR, GpioInterface::Edge::kEdgeRising);
    Int.EnableInterrupts();

    int op = -1;
    while(op != 0)
    {
      printf("\n Please enter the number of the function you'd like to perform:");
          printf("\n 0. Quit");
          printf("\n 1. Read data from memory");
          printf("\n 2. Write data to memory");
          printf("\n 3. Write from keypad");
          printf("\n 4. Use LCD");
          printf("\n 5. Do math");
          printf("\n Operation: ");
      scanf("%d", &op);
      if (op != -1 && op != 0)
      {
        if(op == 1)
        {
          int address = 0;
          printf("\n Address: ");
          scanf("%d", &address);
          int data = obj.ReadfromMem(address);
          printf("\n The data stored at address %d is %d", address, data);
        }
        else if(op == 2)
        {
          int address = 0;
          int data = 0;
          printf("Address: ");
          scanf("%d", &address);
          printf("Data: ");
          scanf("%d", &data);
          obj.WritetoMem(address, data);
          printf("\n The number %d has been written to address %d", data, address);
        }
        else if (op == 3)
        {
            obj.WritefromIO();
        }
        else if (op == 4)
        {
            Int.DisableInterrupts();
            obj.useLCD();
            Int.EnableInterrupts();
        }
        else if (op == 5)
        {
            Int.DisableInterrupts();
            obj.calculate();
            Int.EnableInterrupts();
        }
        else
        {
          printf("\n The operation entered is invalid.\n");
        }
        
      }
    }
    return 0;
  }

