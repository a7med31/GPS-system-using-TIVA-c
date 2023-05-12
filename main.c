#include "tm4c123gh6pm.h"
#include "stdint.h"
#include "stdlib.h"
#include "fpu.h"
#include "string.h"

//FPU 
#define NVIC_CPAC               0xE000ED88  // Coprocessor Access Control
#define HWREG(x)                (*((volatile uint32_t *)(x)))

//functions definitions
void FPUEnable(void);
void PortF_Init(void);
void UART0_write(char c);
void printStr(char *str);
void parse(void);
char* substring(char *destination, const char *source, int beg, int n);

//global variables
char latitude[100], longitude[100], command[100];

int main()
{
    
}

void PortF_Init(void)
{
    SYSCTL_RCGCGPIO_R |= 0X20;                      //start clock
    while(!(SYSCTL_PRGPIO_R & 0x20));               //check clock status
    GPIO_PORTF_DEN_R |= 0x0E;                       //enable digital
    GPIO_PORTF_AMSEL_R &= ~0x0E;                    //disable analog
    GPIO_PORTF_AFSEL_R &= ~0x0E;                    //no alternative function
    GPIO_PORTF_PCTL_R &= ~0xFFF0;                   //disable PCTL
    GPIO_PORTF_DIR_R |= 0x0E;                       //3 output LEDs
    GPIO_PORTF_DATA_R &= 0X0E;                      //leds are off initially
}

void FPUEnable(void)
{
    //
    // Enable the coprocessors used by the floating-point unit.
    //
    HWREG(NVIC_CPAC) = ((HWREG(NVIC_CPAC) &
                         ~(NVIC_CPAC_CP10_M | NVIC_CPAC_CP11_M)) |
                        NVIC_CPAC_CP10_FULL | NVIC_CPAC_CP11_FULL);
}
void UART0_write(char c)
{
    while ((UART0_FR_R & UART_FR_TXFF) != 0);
    UART0_DR_R = c;
}

void printStr(char *str)
{
    uint8_t i = 0;
    while(str[i])
    {
        char c = str[i];
        UART0_write(c);
        i++;
    }
}

void parse(void)
{
    char check[100];
    int i, j; 
    flag = 0;

 //CHECK
for (i = 0; i < 5; i++){
        check[i] = command[i];
}
    check[5] = '\0';
    if ((strcmp(check, "GPRMC")) != 0)
    {
        flag = 1;
        // printStr("Error1");
        return;
    }
    if (command[16] != 'A')
		{
        // printStr("Error2");
        flag = 1;

        return;
    }
    if (command[17] != ',')
		{
        // printStr("Error3");
        flag = 1;

        return;
    }
    first = 1; //Moved the first flag into the parse funtion to always trigger after finding the right format the first time
    j = 0;
    for (i = 18; i < 28; i++)
    {
        latitude[j] = command[i]; //un seperated lat
        j += 1;
    }
    j = 0;
    for (i = 31; i < 42; i++)
    {
        longitude[j] = command[i];
        j += 1;
    }
}

char* substring(char *destination, const char *source, int beg, int n)
{
    while (n > 0)
    {
        *destination = *(source + beg);
 
        destination++;
        source++;
        n--;
    }
    *destination = '\0';
    return destination;
}
