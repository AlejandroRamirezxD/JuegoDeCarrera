/*
  Archivo:  LAB0.c
  Autor:    Alejandro Ramirez Morales
  Creado:   19/enero/22
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#include <xc.h>
#include <stdint.h>

/*
 +----------------------------------------------------------------------------+
 |                             VARIABLES GLOBALES                             |
 +----------------------------------------------------------------------------+
 */
uint8_t estadoRB1 = 0;
uint8_t estadoRB2 = 0;

uint8_t turno = 0;
uint8_t banderaReset = 0;
uint8_t inicio = 0;
uint8_t segundos = 3;
/*
 +----------------------------------------------------------------------------+
 |                          PROTOTIPOS DE FUNCIONES                           |
 +----------------------------------------------------------------------------+
 */
void setup(void);
void resetTMR1(void);
void ceroDisplay(void);
void unoDisplay(void);
void dosDisplay(void);
void tresDisplay(void);
void subrutinaComparar(void);
/*
 +----------------------------------------------------------------------------+
 |                               INTERRUPCIONES                               |
 +----------------------------------------------------------------------------+
 */
void __interrupt() isr (void)
{
    // INTERRUPCION DEL TIMER 1
    if(PIR1bits.TMR1IF){
        // Reset del tmr1 al suceder un segundo
        resetTMR1();
        
        // Si la bandera de reset se activa, restar cada segundo
        // Recordar que junto con el set de la banderaReset, se configura el 
        // valor de segundos en 3 y resetea el tmr para tener un tiempo optimo.
        if (banderaReset){
            segundos--;
        }
    }
    
    // INTERRUPCION DEL PORTB   
    if(INTCONbits.RBIF){
        
        // El turno set indica que la interrupcion esta sucediendo
        turno = 1;
        
        // Cuando el boton de reset es presionado, se activa la banderaReset
        if(!PORTBbits.RB0 == 1){
            inicio = 0;             // Denegar el juego 
            segundos = 3;           // Set de segundos
            banderaReset = 1;       // Empezar conteo
            
            PORTA = 0;              // Limpiar PORTA
            PORTD = 0;              // Limpiar PORTD
            
            PORTE = 0b001;          // Activar led de conteo
            tresDisplay();     // Display en 3
            
            // Leds de victoria apagados
            PORTBbits.RB3 = 0;
            PORTBbits.RB4 = 0;
            
            // Reset del tmr1
            resetTMR1();
        }
        
        // Cuando la bandera de inicio se activa despues del conteo
        if (inicio){
            
            // Cuando los botones de incremento pueden actuar
            if(!PORTBbits.RB1 == 1 && estadoRB1 == 1){
                if(PORTA == 0){
                    PORTA = 1;
                }
                else{
                    PORTA = PORTA*2;
                }
            }

            if(!PORTBbits.RB2 == 1 && estadoRB2 == 1){
                if(PORTD == 0){
                    PORTD = 1;
                }
                else{
                    PORTD = PORTD*2;
                }
            }
        }
        
        INTCONbits.RBIF = 0;    // Limpiar banderta de PORTB
        turno = 0;              // Termina interrupcion del PORTB   
    }
}

/*
 +----------------------------------------------------------------------------+
 |                                   LOOP                                     |
 +----------------------------------------------------------------------------+
 */
void main(void) 
{
    setup(); // Se ejecuta funcion setup
    
    while(1)
    {
        /*
            Mientras la variable turno es igual a 0, la interrupcion del portb
            no se encuentra en proceso, por lo que si un push esta presionado
            (lo que implica que su respectivo puerto esta en tierra)
            mientras turno es 0, la variable estadoRBx pasa a 0, lo que bloquea
            su lectura cuando la interrupcion del portb se ejecute.
         */
        
        // Banderas para el RB1 (Jugador 1)
        if(PORTBbits.RB1 == 0 && turno == 0){
            estadoRB1 = 0; // No afecta al conteo
        }
        else if (PORTBbits.RB1 == 1 && turno == 0){
            estadoRB1 = 1; // Si afecta al conteo
        }
        
        // Bandera para el RB2 (Jugador 2)
        if(PORTBbits.RB2 == 0 && turno == 0){
            estadoRB2 = 0; // No afecta al conteo
        }
        else if (PORTBbits.RB2 == 1 && turno == 0){
            estadoRB2 = 1; // No afecta al conteo
        }
        
        //Segundos
        if (segundos <= 0){
            segundos = 3;       // Reset de segundos
            banderaReset = 0;   // Deja de contar
            inicio = 1;         // Empieza juego
            ceroDisplay(); // Display en 0
            PORTE = 0b000;      // Led Rojo
        }
       
        else if (segundos == 2){
            PORTE = 0b010;  // Led amarillo
            dosDisplay();
        }
        else if (segundos == 1){
            PORTE = 0b100;  // Led verde
            unoDisplay();
        }
        
        subrutinaComparar();
    }
}

/*
 +----------------------------------------------------------------------------+
 |                                  SETUP                                     |
 +----------------------------------------------------------------------------+
 */
void subrutinaComparar (void){
    if(PORTA == 128){
            inicio = 0;
            PORTBbits.RB3 = 1;
            PORTBbits.RB4 = 0;
            unoDisplay();
        }
        else if (PORTD == 128){
            inicio = 0;
            PORTBbits.RB3 = 0;
            PORTBbits.RB4 = 1;
            dosDisplay();
        }
    return;
}

void ceroDisplay (void){
    PORTC = 0b00111111;     // Display en 0
    return;
}

void unoDisplay (void){
    PORTC = 0b00000110;     // Display en 1
    return;
}

void dosDisplay (void){
    PORTC = 0b01011011;     // Display en 2
    return;
}

void tresDisplay (void){
    PORTC = 0b01001111;     // Display en 3     
    return;
}

void resetTMR1 (void){
    TMR1H = 133;             // preset for timer1 MSB register
    TMR1L = 238;             // preset for timer1 LSB register
    PIR1bits.TMR1IF = 0;        // Bandera TMR1 apagada
}

void setup(void)
{
    // Ports 
    ANSEL   =   0;              // Digital Ports
    ANSELH  =   0;
    
    TRISB   = 0b00111;
    
    TRISA   =   0;              // PORTA - salida
    TRISC   =   0;              // PORTC - salida
    TRISD   =   0;              // PORTD - salida
    TRISE   =   0;              // PORTE - salida
    
    PORTA   =   0;              // PORTA en 0
    PORTBbits.RB3 = 0;
    PORTBbits.RB4 = 0;
    PORTC   =   0b01000000; // Display en 0
    PORTD   =   0;              // PORTD en 0
    PORTE   =   0;              // PORTB en 0
    
    // Botones
    OPTION_REGbits.nRBPU = 0;   // Habilitar pull ups
    WPUBbits.WPUB0 = 1;         // RB0 con pull up
    WPUBbits.WPUB1 = 1;         // RB1 con pull up
    WPUBbits.WPUB2 = 1;         // RB2 con pull up
    
    // IOCB
    IOCBbits.IOCB0 = 1;
    IOCBbits.IOCB1 = 1;
    IOCBbits.IOCB2 = 1;
        
    // Reloj
    OSCCONbits.IRCF = 0b100;    // 1MHz
    OSCCONbits.SCS = 1;         // Activar reloj interno
    
    // Interrupciones
    INTCONbits.GIE  = 1;        // Interrupciones globales activadas
    INTCONbits.PEIE = 1;        // Interrupciones perifericas activadas
    INTCONbits.RBIE = 1;        // Interrupion del puerto b activada
    INTCONbits.RBIF = 0;        // Bandera interrupcion portb apagada
    PIE1bits.TMR1IE = 1;        // Interrupcion del TMR1
    PIR1bits.TMR1IF = 0;        // Bandera TMR1 apagada
    
    //Timer1 Registers Prescaler= 8 - TMR1 Preset = 34286 - Freq = 1.00 Hz - Period = 1.000000 seconds
    T1CONbits.T1CKPS1 = 1;   // bits 5-4  Prescaler Rate Select bits
    T1CONbits.T1CKPS0 = 1;   // bit 4
    T1CONbits.T1OSCEN = 0;   // bit 3 Timer1 Oscillator Enable Control bit 1 = on
    T1CONbits.T1SYNC = 1;    // bit 2 Timer1 External Clock Input Synchronization Control bit...1 = Do not synchronize external clock input
    T1CONbits.TMR1CS = 0;    // bit 1 Timer1 Clock Source Select bit...0 = Internal clock (FOSC/4)
    T1CONbits.TMR1ON = 1;    // bit 0 enables timer
        // Reset del tmr1
    resetTMR1();
    return;   
}

