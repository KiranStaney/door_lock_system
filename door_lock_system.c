#include <LPC17xx.h>
#include "lcd_fun.c"
#include <string.h>

#define ROW_MASK 0xF0
#define COL_MASK 0x0F
#define ENTER_KEY '*'

void delay_ms(unsigned int ms);
void keypad_init(void);
char keypad_scan(void);
void get_input(char *str, int hide);

void uart0_init(void);
void uart0_tx(char ch);
void uart0_string(char *str);

void delay_30sec(void);

char stored_user[] = "ABCA";
char stored_pass[] = "1234";

/* ================= MAIN ================= */

int main()
{
    char user[10];
    char pass[10];
    int attempts;

    lcd_config();
    keypad_init();
    uart0_init();

    uart0_string("\r\nDOOR LOCK SYSTEM READY\r\n");

    while(1)
    {
        lcd_cmd(0x01);
        lcd_str("Enter Username");

        uart0_string("\r\nEnter Username:\r\n");

        lcd_cmd(0xC0);
        get_input(user,0);

        uart0_string("Username Entered\r\n");

        if(strcmp(user, stored_user) != 0)
        {
            lcd_cmd(0x01);
            lcd_str("Wrong Username");

            uart0_string("Wrong Username\r\n");

            delay_ms(1500);
            continue;
        }

        attempts = 0;

        while(attempts < 3)
        {
            lcd_cmd(0x01);
            lcd_str("Enter Password");

            uart0_string("\r\nEnter Password:\r\n");

            lcd_cmd(0xC0);
            get_input(pass,1);

            if(strcmp(pass, stored_pass) == 0)
            {
                lcd_cmd(0x01);
                lcd_str("ACCESS GRANTED");

                uart0_string("ACCESS GRANTED\r\n");

                delay_ms(2000);
                break;
            }
            else
            {
                attempts++;

                lcd_cmd(0x01);
                lcd_str("Wrong Password");

                uart0_string("Wrong Password\r\n");

                delay_ms(1500);
            }
        }

        if(attempts == 3)
        {
            lcd_cmd(0x01);
            lcd_str("Locked 30 Sec");

            uart0_string("SYSTEM LOCKED 30 SEC\r\n");

            delay_30sec();
        }
    }
}

/* ================= DELAY ================= */

void delay_ms(unsigned int ms)
{
    unsigned int i,j;
    for(i=0;i<ms;i++)
        for(j=0;j<10000;j++);
}

void delay_30sec(void)
{
    delay_ms(30000);
}

/* ================= UART ================= */

void uart0_init(void)
{
    LPC_SC->PCONP|=(1<<3);				   //enable power
LPC_SC->PCLKSEL0&=~((1<<7)|(1<<6));	   //enable PCLK=CLK//4
//configure p0.2 as TX p0.3 as RX of UART0
LPC_PINCON->PINSEL0=(1<<4)|(1<<6);

/*configure UART0->8bit data,1stop bit,no parity,disable break transmissic enable DLAB  bit*/

LPC_UART0->LCR|=(1<<0)|(1<<1)|(1<<7);

/* To generate baudrate @9600bps*/
LPC_UART0->DLL=6;
LPC_UART0->DLM=0;

LPC_UART0->FDR=(12<<4)|(1<<0);				 //MUL=12,DIV=1

LPC_UART0->LCR&=~(1<<7);						   //disable DLAB bit
}

void uart0_tx(char ch)
{
    while(!(LPC_UART0->LSR & (1<<5)));
    LPC_UART0->THR = ch;
}

void uart0_string(char *str)
{
    while(*str)
        uart0_tx(*str++);
}

/* ================= KEYPAD ================= */

void keypad_init(void)
{
    LPC_GPIO2->FIODIR |= COL_MASK;
    LPC_GPIO2->FIODIR &= ~ROW_MASK;
    LPC_GPIO2->FIOSET = COL_MASK;
}

char keypad_scan(void)
{
    const char keymap[4][4] =
    {
        {'1','2','3','A'},
        {'4','5','6','B'},
        {'7','8','9','C'},
        {'*','0','#','D'}
    };

    int row,col;

    for(col=0; col<4; col++)
    {
        LPC_GPIO2->FIOSET = COL_MASK;
        LPC_GPIO2->FIOCLR = (1<<col);

        for(row=0; row<4; row++)
        {
            if(!(LPC_GPIO2->FIOPIN & (1<<(row+4))))
            {
                delay_ms(200);
                return keymap[row][col];
            }
        }
    }
    return 0;
}

/* ================= INPUT ================= */

void get_input(char *str, int hide)
{
    int i=0;
    char key;

    while(1)
    {
        key = keypad_scan();

        if(key)
        {
            if(key == ENTER_KEY)
            {
                str[i]='\0';
                return;
            }

            if(i<9)
            {
                str[i++] = key;

                uart0_tx(key);

                if(hide)
                    lcd_data('*');
                else
                    lcd_data(key);
            }
        }
    }
}
