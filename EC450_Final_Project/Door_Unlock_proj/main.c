#include <msp430.h>
#include "lcdLib.h"

#define keyport P1OUT       // Keypad Port
#define ROW1 (0x10 & P1IN)
#define ROW2 (0x20 & P1IN)
#define ROW3 (0x40 & P1IN)
#define ROW4 (0x80 & P1IN)

/** Globals **/
char password[4] = "0624";
unsigned short int no_press;
unsigned char ii,k,key=0;
unsigned char Key_Val[] = {' ','1','4','7','*','2','5','8','0','3','6','9','#'};
unsigned int count=0;
unsigned char Value=0;
char password_row_text_display[16] = "Password: ";

/** Function Declarations **/
unsigned char get_key(void);
unsigned char pressed_return(void);
void gpio_init(void);
void clear_password(void);

/** Main **/

void main(void)
{
    WDTCTL=(WDTPW + WDTTMSEL + WDTCNTCL + 0 + 1); // configure WDT
    IE1|=WDTIE;                                   // enable WDT interrupt

    lcdInit();// Initialize LCD
    // ('string', left offset, row)
    lcdSetText("Please Enter The", 0, 0);
    lcdSetText("Password: ", 0, 1);
    gpio_init();
    _bis_SR_register(GIE + LPM0_bits);   // enable interrupts and power down CPU
}

/** Function Dependant Globals **/

unsigned char last_value = 'l';
unsigned int short input_index = 10;
unsigned short int enter_counter = 0;
unsigned short int four_entered = 0;
unsigned short int did_press_enter = 0;

/** Watch Dog Timer **/

interrupt void WDT_interval_handler(){
    Value = pressed_return();

    // Indicate that a button has been pressed.
    if((last_value != Value) && (no_press == 0)){
        no_press = 1;
        if(input_index < 14){
            password_row_text_display[input_index++] = Value; //weird way of appending a c string
            lcdSetText(password_row_text_display, 0, 1); //('string', left offset, row)
        }
        else{
            did_press_enter = 1;
        }
        last_value = Value;
        // append LCD string
    }
    //In the case that two of the same button is clicked consecutively
    else if (no_press == 0){
        no_press = 1;
        if(input_index < 14){
            password_row_text_display[input_index++] = last_value; //weird way of appending a c string
            lcdSetText(password_row_text_display, 0, 1); //('string', left offset, row)
        }
        else{
            did_press_enter = 1;
        }
    }
    
    // AKA if (input_index == 14)
    if(did_press_enter){
        did_press_enter = 0;

      if((password_row_text_display[10] == password[0]) &&
         (password_row_text_display[11] == password[1]) &&
         (password_row_text_display[12] == password[2]) &&
         (password_row_text_display[13] == password[3])){
          // the inputted password was correct!
          lcdClear();
          lcdSetText("Success!", 0, 0);
          P1REN &= ~(0x08); //discard pull up resistor so that the signal is sent to the motor circuit
      }
      else{
          //regardless of success or failure you need to clear the LCD input
          clear_password();
      }
    }
}
ISR_VECTOR(WDT_interval_handler, ".int10")

/** LCD Extension **/

void clear_password(void){ // Only clears the component of the LCD responsible for displaying the password
    input_index = 10;
    int jj;
    for(jj = 10;jj < 14;jj++){
       password_row_text_display[jj] = ' ';
    }
    lcdSetText(password_row_text_display, 0, 1);
    four_entered = 0;
}

/** Configuration Initial Conditions **/

void gpio_init(void){
    /*Num Pad Config*/
    P1DIR = 0x07;                // Set P1.0 to 1.3 Output ,Set P1.4 to 2.7 Input
    P1REN = 0xF0;                // Set P1.0 to 1.7 Pull up Register enable
    P1OUT = 0x07;                // Set P1.0 to 1.7 Out Register.

    /*Sender Config*/
    P1DIR |= 0x08;   // make PIN3 output
    P1REN |= 0x08;  // make it initially output a low
}

/** Keypad Matrix Functions **/

unsigned char pressed_return(void){
    // waits for a button to be pressed
    while((count = get_key()) == 0){
        no_press = 1;            // need this in order to detect duplicate, consecutive button presses
    }; // once the button is pressed, the while will fail
    no_press = 0;
    return Key_Val[count];
}


 unsigned char get_key(void)
{
      kk = 1;                   // keep a count for where the button is pressed
      for(ii = 0; ii < 3; ii++) // iterate over the columns
      {
                keyport = ((0x01 << ii) ^ 0xff); // Scan for a Key by sending '0' on ROWS of COL_ii
                        if(!ROW1){               // when a key pressed numbers 1--16 will be returned
                                key = kk+0;
                                while(!ROW1);    // pauses at the given position until contact with button is released
                                return key;
                        }
                        else if(!ROW2){
                                key = kk+1;
                                while(!ROW2);
                                return key;
                        }
                        else if(!ROW3){
                                key = kk+2;
                                while(!ROW3);
                                return key;
                        }
                        else if(!ROW4){
                                key = kk+3;
                                while(!ROW4);
                                return key;
                        }
                kk += 4; // add 3 because you need scan next column
                keyport |= (0x01 << ii);
        }
        return 0; // otherwise return a '0'
}
