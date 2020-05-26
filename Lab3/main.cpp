//Libraries #include <project_config.hpp> 
 
#include <cstdint> 
#include <iterator> 
 
#include <math.h> 
 
#include "L2_HAL/displays/led/onboard_led.hpp" 
#include "utility/log.hpp" 
#include "utility/time.hpp" 
 
//Bus Class Declaration and Definitions 
class Bus {    
    public:    
    enum class ControlType    
    {        
        kMemory = 0,        
        kIO    
    }; 
 
//Bus Constructor    
Bus()    
{        
    Write.SetAsOutput();        
    ALE.SetAsOutput();        
    M_IO.SetAsOutput(); 
 
    ad[0].GetPin().SetAsOpenDrain();        
    ad[1].GetPin().SetAsOpenDrain();        
    ad[2].GetPin().SetAsOpenDrain();        
    ad[3].GetPin().SetAsOpenDrain();        
    ad[4].GetPin().SetAsOpenDrain();        
    ad[5].GetPin().SetAsOpenDrain();        
    ad[6].GetPin().SetAsOpenDrain();        
    ad[7].GetPin().SetAsOpenDrain();    
} 

//Write data from console to memory    
void WritetoMem(int address, int data)    
{        
    sendAddress(address);        
    writeData(data); 
 
    return;    
} 
 
//Read data from memory to console    
int ReadfromMem(int address)    
{        
    sendAddress(address);        
    int data = readData(); 
 
    return data;    
} 
 
//Write data from IO keypad to console    
void WritefromIO(void)    
{        ​
    //Sets up initial signals        
    setupIO(); 
 
    int address = 0;        
    int state = 0;     
       ​
    //Address 72 corresponds to ‘ # ‘        
    //If user enters ‘ # ‘, the program will return control to main        
    while (address != 72)        
    {            ​
        //Sets up the lower half of the address bits            
        setIOaddress(state);            ​
        //Checks for a key press and generates the address            
        address = checkKeyPress(state);            
        if (address != 0)            
        {   
            //Prints the character corresponding to the calculated address                
            printChar(address);            
        }            ​
        
        //Updates the state of the lower bits            
        state = nextState(state);        
    }        
    
    ​//Returns control to main        
    return;    
} 
 
//Sets up control signals and input/output     
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
 
//Sets up the lower four bits of the IO address    
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
} 
 
//Checks each of the upper 4 address bits    
int checkKeyPress(int state)    
{        
    Write.SetLow();        
    for (int i = 4; i < 8; i ++)        
    {            ​
        //If pressed, value will be high and true            
        if (ad[i].Read() == true)            
        {                
            ​//Returns the address of the button pressed                
            return (int(pow(2,state) + int(pow(2, i) ) ) );            
        }        
    }        ​
    //If no buttons pressed, returns 0        
    return 0;    
} 
 
//Prints the character corresponding to the address    
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
    printf("\n%c", key);    
} 
 
 
//Calculates the next state to set up the lower 4 bits of the IO address    
int nextState(int state)    
{        
    state ++;        
    if (state == 4)            
        state = 0;        
    return state;    
} 
 
//Sends address from console to the address latch    
void sendAddress(int address)    
{        
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

//Writes data from console to memory    
void writeData(int data)    
{        
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
 
 
//Reads data from console to memory    
int readData()    
{        
    ALE.SetLow();        
    Write.SetLow();        
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
 
//Sets up the pin positions with the corresponding signals    
private:    
    Gpio Write = Gpio(0, 17);    
    Gpio ALE = Gpio(0, 22);    
    Gpio M_IO = Gpio(0, 0);    
    Gpio Int = Gpio(0, 11); //Not used till Lab 4 
 
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
 
//Main function 
int main(void) 
{  
    Bus obj;  
    int op = -1; 
    //Creates user interface    
    while(op != 0)    
    {      
        printf("\n Please enter the number of the function you'd like to perform:");          
        printf("\n 0. Quit");          
        printf("\n 1. Read data from memory");          
        printf("\n 2. Write data to memory");          
        printf("\n 3. Write from keypad"); 
        printf("\n Operation: ");      
        scanf("%d", &op);      
        if (op != -1 && op != 0)      
        {        ​
            //Reads from memory with user input address through console        
            if(op == 1)        
            {          
                int address = 0;          
                printf("\n Address: ");          
                scanf("%d", &address);          
                int data = obj.ReadfromMem(address);          
                printf("\n The data stored at address %d is %d", address, data);        
            }        ​
            //Writes to memory with user input address and data through console        
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
            }        ​
            //Passes control to IO keypad        
            else if (op == 3)        
            {            
                obj.WritefromIO();        
            }        ​
            //Prints if operation is invalid        
            else        
            {          
                printf("\n The operation entered is invalid.\n");        
            }        
        }    
    }    
    return 0;  
}

