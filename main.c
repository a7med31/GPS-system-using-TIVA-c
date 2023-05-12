#include "tm4c123gh6pm.h"
#include "stdint.h"
#include "stdlib.h"
#include "fpu.h"
#include "string.h"
#include "math.h"

//FPU 
#define NVIC_CPAC               0xE000ED88  // Coprocessor Access Control
#define HWREG(x)                (*((volatile uint32_t *)(x)))

//functions definitions
void FPUEnable(void);
void PortF_Init(void);
void UART0_write(char c);
void printStr(char *str);
void parse(void);
char* substring(char *destination, const char *source, int beg, int n);
void UART0_Init(void);
void UART2_Init(void);
char UART2_read(void);
void getCommand(char*str);
void getCoordinates(void);
float delta(float p_lat, float p_long, float c_lat, float c_long);
float torad(int cor, float deg);
void printflo(float x);
void reverse(char* str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char* res, int afterpoint);

//global variables
char latitude[100], longitude[100], command[100];
int flag, len, first;
int lat_coordinate, long_coordinate;
float lat_deg, long_deg, c_lat, c_long, p_lat, p_long, f_lat, f_long, s_lat, s_long, total_distance, dtg, delta_view, lat_float, long_float;
int lat_coordinate, long_coordinate;
int flag, len, first;

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
    // Enable the coprocessors used by the floating-point unit.
    HWREG(NVIC_CPAC) = ((HWREG(NVIC_CPAC) &
                         ~(NVIC_CPAC_CP10_M | NVIC_CPAC_CP11_M)) |
                        NVIC_CPAC_CP10_FULL | NVIC_CPAC_CP11_FULL);
}

void UART0_Init(void)
{
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;                       //Enable UART clock
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;                       //Enable GPIO clock

    UART0_CTL_R &= ~UART_CTL_UARTEN;                               //Disable UART CTL
    
    UART0_IBRD_R = 104;                                            //Calculate BRD: (16*10^6)/(16*9600) = 104.16667
    UART0_FBRD_R = 11;                                             //int(0.16667*64 + 0.5) = 11

    UART0_LCRH_R = (UART_LCRH_WLEN_8 | UART_LCRH_FEN);             //Enable UART parameters such as data length & FIFO
    UART0_CTL_R |= (UART_CTL_UARTEN | UART_CTL_RXE | UART_CTL_TXE);//Enable UART CTL

    GPIO_PORTA_AFSEL_R |= 0x03;                                    //Enable UART Alternate Function for PA0 & PA1
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & ~0xFF) | (GPIO_PCTL_PA0_U0RX | GPIO_PCTL_PA1_U0TX);//PA0-->RX & PA1-->TX
    GPIO_PORTA_DEN_R |= 0x03;                                      //Digital Enable for 2pins        
}

void UART2_Init(void)
{
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R2;                       //enable UART clock
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3;                       //enable GPIO clock
	while(!(SYSCTL_PRUART_R&SYSCTL_PRUART_R2 ));                   //Wait for the Clock to start
	
    UART2_CTL_R &= ~UART_CTL_UARTEN;                               //disable UART CTL
    
    UART2_IBRD_R = 104;                                            //calculate BRD: (16*10^6)/(16*9600) = 104.16667
    UART2_FBRD_R = 11;      //0.16667*64 + 0.5

    UART2_LCRH_R = 0x70;  //enable uart parameters such as data length & FIFO
    UART2_CTL_R |= (UART_CTL_UARTEN | UART_CTL_RXE | UART_CTL_TXE);             //enable UART CTL

    GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTD_CR_R |= 0x080;
    GPIO_PORTD_AFSEL_R |= 0x0C0;
    GPIO_PORTD_PCTL_R |= (GPIO_PCTL_PD6_U2RX | GPIO_PCTL_PD7_U2TX);
    GPIO_PORTD_DEN_R |= 0x0C0;
}

char UART2_read(void)
{    while ((UART2_FR_R & UART_FR_RXFE) != 0);
    return UART2_DR_R & 0xFF;
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
void getCommand(char*str)
{
    char c;
    int i;
    for (i = 0; i < 100; i++)
    {
        c = UART2_read();      //Reading data from GPS Module
        if(c == '$')
        {
            len = i;
            break;
        }
        else
            str[i] = c; 
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
        return;
    }
    if (command[16] != 'A')
		{
        flag = 1;
        return;
    }
    if (command[17] != ',')
		{
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
void getCoordinates(void)
{
    char str[15];

    parse();
    lat_float = atof(latitude);
    long_float = atof(longitude);
  
    if (flag == 1)
        return;

    //coordinates
    substring(str, latitude, 0, 2);
    lat_coordinate = atoi(str);
    substring(str, longitude, 0, 3);
    long_coordinate = atoi(str);
    //degrees
    substring(str, latitude, 2, 8);
    lat_deg = atof(str);
    substring(str, longitude, 3, 8);
    long_deg = atof(str);
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
float torad(int cor, float deg) {
    float PI = 3.141592653589793;
    return ((cor + deg / 60) * (PI / 180));
}
float delta(float p_lat,float p_long ,float c_lat,float c_long) {
    double D;    
    float a = pow(sin((c_lat - p_lat) / 2), 2) + pow(sin((c_long - p_long) / 2), 2) * cos(c_lat) * cos(p_lat);
    float c = 2 * asin(sqrt(a));
    D = 6371 * c * 1000; 
    return D;
}
void printflo(float x) {
    char res[20];
    ftoa(x, res, 5);
    printStr(res);
}
void reverse(char* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}
void ftoa(float n, char* res, int afterpoint) //Check number of floating digits
{
    int ipart = (int)n;
    float fpart = n - (float)ipart;
    int i = intToStr(ipart, res, 0);
    if (afterpoint != 0) {
        res[i] = '.'; // add dot
        fpart = fpart * pow(10, afterpoint);
        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}
float input_latlong(float latlong){
    return torad((int)latlong, floor(100000*((latlong - (int)latlong) * 60))/100000);
}
