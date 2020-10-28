/*=============================================================================
   CFPT - Projet : TP Communication ChargeActivePCMaster
   Auteur        : FM
   Date création : 19.10.2017
  =============================================================================
   Descriptif:    réception message sur UART avec détection de fin de réception
                  avec un timeout (TIMER0).
                  Les chaînes de caractères reçues sont affichées sur un
                  afficheur LCD connecté sur le port 2.
  =============================================================================
   Modification:
      16.10.2019 FM:    - Ajouter define des INTERRUPT_xxx
                        - Utiliser <reg51f380.h>
                        
===============================================================================
*/
#include <reg51f380.h>       // registres 51f38x
#include "Delay.h"          // unité gestion temporisation
#include "StringAZT.h"      // unité manipulation de chaîne de caractères
#include "LCD_DOG.h"
#include <string.h>

//-----------------------------------------------------------------------------
// Interrupt Priorities
//-----------------------------------------------------------------------------

#define INTERRUPT_INT0             0   // External Interrupt 0
#define INTERRUPT_TIMER0           1   // Timer0 Overflow
#define INTERRUPT_INT1             2   // External Interrupt 1
#define INTERRUPT_TIMER1           3   // Timer1 Overflow
#define INTERRUPT_UART0            4   // Serial Port 0
#define INTERRUPT_TIMER2           5   // Timer2 Overflow
#define INTERRUPT_SPI0             6   // Serial Peripheral Interface 0
#define INTERRUPT_SMBUS0           7   // SMBus0 Interface
#define INTERRUPT_USB0             8   // USB Interface
#define INTERRUPT_ADC0_WINDOW      9   // ADC0 Window Comparison
#define INTERRUPT_ADC0_EOC         10  // ADC0 End Of Conversion
#define INTERRUPT_PCA0             11  // PCA0 Peripheral
#define INTERRUPT_COMPARATOR0      12  // Comparator0
#define INTERRUPT_COMPARATOR1      13  // Comparator1
#define INTERRUPT_TIMER3           14  // Timer3 Overflow
#define INTERRUPT_VBUS_LEVEL       15  // VBUS level-triggered interrupt
#define INTERRUPT_UART1            16  // Serial Port 1
                                       // #17 Reserved
#define INTERRUPT_SMBUS1           18  // SMBus1 Interface
#define INTERRUPT_TIMER4           19  // Timer4 Overflow
#define INTERRUPT_TIMER5           20  // Timer5 Overflow


// ==== CONSTANTES ============================================================
#define TIMER0_VAL   60856 // Temporisation pour la détection de fin de trame
                           // timeout de 1,56 ms pour un débit de 9600 baud (10b) 
                           // timer0_clk = syst_clk / 4 = 3 MHz)
                           // precharge = 65536 - ( 1560 us / (1/3MHz)) = 60856
                           
#define BAUD_CST     0x64  // Valeur pour un Baudrate de 9600bps
                           // Pour plus d'info voir tableau dans la fonction 
                           // "UartInit ()" qui se trouve ci-dessous dans le code

#define PORT_LCD     2     // port utilisé par l'affichage LCD
#define LIGNE0       0
#define LIGNE1       1
#define COLONNE0     0

#define MAX_TEXTE    17    // grandeur de la chaîne pour une ligne d'affichage  

// ==== FONCTIONS PROTOTYPES===================================================
void ClockInit ();         // init. clock système
void PortInit ();          // init. config des ports
void Timer01Init ();       // init. des timers 0 et 1
void UartInit ();          // init. UART 
void MessageDecoder(unsigned int voltageValue, unsigned int currentValue,
                    unsigned int currentTargetValue);
unsigned int CalculCheckSum(unsigned char * valStr, unsigned char nbOfCharToUse,
                            unsigned char nbOfCharToSkip);
void DataAcquisition(unsigned int *pVoltageValue, unsigned int *pCurrentValue,
                     unsigned int *pCurrentTargetValue);

// ==== Variables globales ====================================================
unsigned char gUART_Tx[MAX_TEXTE] = "";
unsigned char gUART_Rx[MAX_TEXTE] = "";
unsigned char gRxIndex = 0;
unsigned char gTxIndex = 0;
unsigned char gNbrByte;
bit gFlagReception = 0;

// ==== MAIN ==================================================================
void main () 
{  
   unsigned int voltageValue = 0; 
   unsigned int currentValue = 0;
   unsigned int currentTargetValue = 0;
   
   
   
   PCA0MD &= ~0x40;     // WDTE = 0 (disable watchdog timer)
   ClockInit ();        // init. clock système
   PortInit ();         // init. config des ports
   Timer01Init ();      // init. timers 0 et 1
   InitLCD (PORT_LCD);  // Initialisation de l'affichage LCD
   UartInit ();         // init. uart
   
   SelectPosCaractLiCo (LIGNE0,COLONNE0);
   AfficherChaineAZT ("Connecter PC Prog");
   SelectPosCaractLiCo (LIGNE1,COLONNE0);
   AfficherChaineAZT ("PCMasterAlimentation.");
   // initialisation des interruptions
   ES0 = 1; // autorise les interruptions UART0
   ET0 = 1; // autorise les interruptions du TIMER0
   EA  = 1; // autorise toutes les interruptions

   // boucle principale  
   while (1)
   {
      
      // acquisition des valeurs de tension et de courant
      DataAcquisition(&voltageValue, &currentValue, &currentTargetValue);
      
      if(gFlagReception)
      {
         // reset flag
         gFlagReception = 0;
         // effacer écran
         EffacerEcran();
         // positionne le cursseur
         SelectPosCaractLiCo (LIGNE0,COLONNE0);
         // affichage du message reçu
         AfficherChaineAZT (gUART_Rx);
         // decodage du message reçu et réponse
         MessageDecoder(voltageValue, currentValue, currentTargetValue);
      }
      
      Delay_1ms (100);
      
      
      
   } // End while (1)
} // main =====================================================================



/*---------------------------------------------------------------------------*-
   DataAcquisition ()
  -----------------------------------------------------------------------------
   Descriptif: 
   Entrée    : 
   Sortie    : 
-*---------------------------------------------------------------------------*/
void DataAcquisition(unsigned int *pVoltageValue, unsigned int *pCurrentValue,
                     unsigned int *pCurrentTargetValue){
   *pVoltageValue = 12345;                      // valeur de la tenison mesurée
                        
                        
   *pCurrentValue = (*pCurrentValue + 1)%10000; // valeur du courant mesuré
                        
                        
   *pCurrentTargetValue = 321;                  // valeur du courant de 
                                                // consigne en cours d'utilisation
}

/*---------------------------------------------------------------------------*-
   MessageDecoder ()
  -----------------------------------------------------------------------------
   Descriptif: Decode la variable globale gUART_Rx et retourne la valeur désirée
               au PC au travers de l'UART0.
   Entrée    : 
   Sortie    : 
-*---------------------------------------------------------------------------*/
void MessageDecoder(unsigned int voltageValue, unsigned int currentValue,
                    unsigned int currentTargetValue){

   int valChkSum = 0;
   unsigned char valChkSumStr[4];
   unsigned char valStrTmp[10];
   int dbgCurrentValue = 0;
   
   if((gUART_Rx[0] == '$') && (gUART_Rx[1] == '*')){
      // positionne le cursseur
      SelectPosCaractLiCo (LIGNE1,COLONNE0);
      
      if((gUART_Rx[2] == 'O') && (gUART_Rx[3] == 'K')){
         // affichage du message reçu
         AfficherChaineAZT ("Receive OK\0");  

         strcpy(gUART_Tx,"$*ok\0");
         valChkSum = CalculCheckSum(gUART_Tx, 2,2);
         EntierToChaineHexa(valChkSum%256,valChkSumStr,0,'B');
         strcat(gUART_Tx,valChkSumStr);
         strcat(gUART_Tx,"\r\n\0");
         
         gTxIndex = 0;
         gNbrByte = 9;
         TI0 =1;
      }
 
      if((gUART_Rx[2] == 'G') && (gUART_Rx[3] == 'I')){
         // affichage du message reçu
         AfficherChaineAZT ("Receive GI\0");  

         strcpy(gUART_Tx,"$*gi");
         dbgCurrentValue = 853;
         EntierToChaineDecimal (currentValue, "9999",valStrTmp);
         strcat(gUART_Tx,valStrTmp);
         valChkSum = CalculCheckSum(gUART_Tx, 6,2);
         EntierToChaineHexa(valChkSum%256,valChkSumStr,0,'B');
         strcat(gUART_Tx,valChkSumStr);
         strcat(gUART_Tx,"\r\n\0");
        
//         strcpy(gUART_Tx,"$*gi020092\r\n\0");
         gTxIndex = 0;
         gNbrByte = 13;
         TI0 =1;
      }
 
      if((gUART_Rx[2] == 'G') && (gUART_Rx[3] == 'U')){
         // affichage du message reçu
         AfficherChaineAZT ("Receive GU\0");  
         
         strcpy(gUART_Tx,"$*gu12345DB\r\n\0");
         gTxIndex = 0;
         gNbrByte = 14;
         TI0 =1;
      }

      if((gUART_Rx[2] == 'G') && (gUART_Rx[3] == 'C')){
         // affichage du message reçu
         AfficherChaineAZT ("Receive GC\0");  
         
         strcpy(gUART_Tx,"$*gc032190\r\n\0");
         gTxIndex = 0;
         gNbrByte = 13;
         TI0 =1;
      }

      if((gUART_Rx[2] == 'R') && (gUART_Rx[3] == 'M')){
         // affichage du message reçu
         AfficherChaineAZT ("Receive RM\0");  
         
         strcpy(gUART_Tx,"$*rmDF\r\n\0");
         gTxIndex = 0;
         gNbrByte = 9;
         TI0 =1;
      }
      
      if((gUART_Rx[2] == 'L') && (gUART_Rx[3] == 'O')){
         // affichage du message reçu
         AfficherChaineAZT ("Receive LO\0");  
         
         strcpy(gUART_Tx,"$*loDB\r\n\0");
         gTxIndex = 0;
         gNbrByte = 9;
         TI0 =1;
      }
      
      // ajouter getion du message SC
      
      // affichage du message reçu + valeur de consigne
      
      // fin getion du message SC
   }
}

/*---------------------------------------------------------------------------*-
   CalculCheckSum ()
  -----------------------------------------------------------------------------
   Descriptif: Calcul le checkSum du nombre de caractères défini par le
               paramètre "nbOfCharToUse" en commencant à la position
               "nbOfCharToSkip" dans la chaîne de caractères pointée par
               "valStr"
   Entrée    : pointeur valStr : pointeur sur tableau de caractères à traiter
               nbOfCharToUse   : nombre de caractères à utiliser
               nbOfCharToSkip  : nombre de caractères à ne pas utiliser en 
                                 début de chaine
   Sortie    : somme de la valeur des caractères ASCII
-*---------------------------------------------------------------------------*/
unsigned int CalculCheckSum(unsigned char * valStr, unsigned char nbOfCharToUse,
                    unsigned char nbOfCharToSkip){
   unsigned int checkSum = 0;
   unsigned char i = 0;
   checkSum=0;
   for(i=0;i<nbOfCharToUse;i++){
      checkSum += valStr[nbOfCharToSkip+i];
   }
   return(checkSum);
}


/*****************************************************************************/

/*---------------------------------------------------------------------------*-
   ClockInit ()
  -----------------------------------------------------------------------------
   Descriptif: Initialisation du mode de fonctionnement du clock 
   Entrée    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
void ClockInit () 
{
   CLKSEL = 0x00;       // clock system = clock interne 
   OSCICN = 0x83;       // clock interne autorisé, f = 12 MHZ
} // ClockInit ----------------------------------------------------------------

/*---------------------------------------------------------------------------*-
   PortInit ()
  -----------------------------------------------------------------------------
   Descriptif: Initialisation du mode de fonctionnement des ports 
   Entrée    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
void PortInit () 
{
// PORT I/O INITIALIZATION
// Step 1. Select the input mode (analog or digital) for all Port pins, using the Port Input Mode register (PnMDIN).
// Step 2. Select the output mode (open-drain or push-pull) for all Port pins, using the Port Output Mode register (PnMDOUT).
// Step 3. Select any pins to be skipped by the I/O Crossbar using the Port Skip registers (PnSKIP).
// Step 4. Assign Port pins to desired peripherals (XBR0, XBR1).
// Step 5. Enable the Crossbar (XBARE = ‘1’).
// ------------------------------------------------------------------------------------------------
// PnMDIN  = 0x00;  // port Pn en entrée analogique
// PnMDIN  = 0xFF;  // port Pn n'est pas en entrée analogique               (valeur par défaut)
// ------------------------------------------------------------------------------------------------
// PnMDOUT = 0x00;  // port Pn en entrée numérique (output in open-drain)   (valeur par défaut)
// PnMDOUT = 0xFF;  // port Pn en sortie numérique (output in push-pull)
// ------------------------------------------------------------------------------------------------
// PnSKIP  = 0x00;  // port Pn n'est pas réservé (no skipped)               (valeur par défaut)         
// PnSKIP  = 0xFF;  // port Pn est réservé (skipped) 
// ------------------------------------------------------------------------------------------------ 

    // CROSSBAR
   XBR0 |= 0x01;     // UART TX0, RX0 routed to port P0.4 and P0.5
   XBR1 |= 0x40;     // autorise le fonctionnement du crossbar
   P0MDOUT   = 0x10;

} // PortInit ----------------------------------------------------------------

/*---------------------------------------------------------------------------*-
   Timer01Init ()
  -----------------------------------------------------------------------------
   Descriptif: Initialisation des timers 0 et 1
   Entrée    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
void Timer01Init () 
{
   // registre commun au timer 0 et timer 1
   CKCON &= ~0x0F;   // clear bit 0 to 3
                     //       +----- clock select timer1   : uses the prescale bits SCA1-SCA0
                     //       |+---- clock select timer0   : uses the prescale bits SCA1-SCA0
                     //       ||++-- Prescale bit timer0/1 : system clock / 4
                     //       ||||
   CKCON |= 0x01;    // 0bxxxx0001  
   
   // initialisation du timer 0 (16-bit timer mode)
   TR0 = 0;          // stop timer0
   TMOD &= ~0x0F;    // clear bit 0 to 3
                     //       +----- Gate control timer1 : pas utilisé   
                     //       |+---- Fonction     timer1 : timer
                     //       ||++-- Mode         timer1 : mode 1 (16-bit timer)
                     //       ||||
   TMOD |= 0x01;     // 0bxxxx0001
   TH0 = TIMER0_VAL /256;  // charge la valeur pour le timer 0
   TL0 = TIMER0_VAL %256;
   TR0 = 0;          // pas de start du timer0
  
   // initialisation du timer 1 (pour le module UART)
   TR1 = 0;          // stop timer1
   TMOD &= ~0xF0;    // clear bit 4 to 7
                     //   +----- Gate control timer1 : pas utilisé   
                     //   |+---- Fonction     timer1 : timer
                     //   ||++-- Mode         timer1 : mode 2 (8-bit auto-reload)
                     //   ||||
   TMOD |= 0x20;     // 0b0010xxxx
   TL1 = TH1 = BAUD_CST; // charge la valeur pour  xxxx baud
   TR1 = 1;          // start timer1
} // Timer01Init -------------------------------------------------------------

/*---------------------------------------------------------------------------*-
   UARTInit ()
  -----------------------------------------------------------------------------
   Descriptif: Initialisation du module UART
   Entrée    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
void UartInit () 
{
   // NOTE : 
   // The UART0 baud rate is generated by Timer 1 in 8-bit auto-reload mode.
   // The Tx and Rx clocks are generated from the TL1 overflows divided by 2.
   //             UartBaudRate = (T1clk / (256-T1H)) / 2
   // Table 17.1 (datasheet page 194)
   // For SYSCLK = 12 MHz
   // | Baud rate | Timer clock source | Prescale select | Timer1 reload |
   // | 115200    | SYSCLK             | XX              | 0xCC          |
   // |  57600    | SYSCLK             | XX              | 0x98          |
   // |  28800    | SYSCLK             | XX              | 0x30          |
   // |  14400    | SYSCLK /4          | 01              | 0x98          |
   // |   9600    | SYSCLK /4          | 01              | 0x64          |
   // |   2400    | SYSCLK /12         | 00              | 0x30          |
   // |   1200    | SYSCLK /48         | 10              | 0x98          |

   // Registre SCON0 : Serial Port 0 Control
   // BIT ADDRESSABLE
   // bit 7 : S0MODE : Serial Port 0 Operation mode (0: 8bit UART ; 1: 9bit UART)
   // bit 6 : unused 
   // bit 5 : MCE0   : Multiprocessor Communication Enable 
   // bit 4 : REN0   : Receive Enable (0: UART0 reception disabled ; 1 : UART0 reception enabled) 
   // bit 3 : TB80   : Ninth Transmission bit
   // bit 2 : RB80   : Ninth Receive bit
   // bit 1 : TI0    : Transmit Interrupt flag (this bit must be cleared manually by software)
   // bit 0 : RI0    : Receive Interrupt flag  (this bit must be cleared manually by software)
   S0MODE = 0;    // 8-bit UART mode
   MCE0   = 0;    // Mutiprocessor disabled
   REN0   = 1;    // UART receive enabled
   TI0    = 0;    // clear flag
   RI0    = 0;    // clear flag
   // Registre SBUF0 : Serial (UART0) Port Data Buffer   

} // UartInit ----------------------------------------------------------------

/*---------------------------------------------------------------------------*-
   InterruptionUART0 ()
  -----------------------------------------------------------------------------
   Descriptif: routine d'interruption du module UART
   Entrée    : 
   Sortie    : 

ATTENTION: utilisation des variables globales:
         - gUART_Rx
         - gRxIndex
         - gUART_Tx
         - gTxIndex
         - gNbrByte
-*---------------------------------------------------------------------------*/
void InterruptionUART0 () interrupt INTERRUPT_UART0
{
   // RI0 : RECEIVE  INTERRUPT FLAG (registre SCON0)
   // TI0 : TRANSMIT INTERRUPT FLAG (registre SCON0)
   
   if(RI0)        // set to 1 by hardware when a byte of date has been received
   {
      RI0 = 0;    // This bit must be cleared manually by software
      
      gUART_Rx[gRxIndex] = SBUF0;     // sauvegarde le byte reçu
      gRxIndex++;                    // incrémente l'index de trame
      
      // recharge le timer 0 pour la détection de fin de trame

      TR0 = 0;                      // stop le timer0 
      TH0 = TIMER0_VAL /256;        // charge la valeur pour le timer 0
      TL0 = TIMER0_VAL %256;
      TR0 = 1;                      // start le timer0   
   }
   
   // TI0 : TRANSMIT INTERRUPT FLAG (registre SCON0)
   if(TI0)        // set to 1 by hardware when a byte of data has been received
   {
      TI0 = 0;    // This bit must be cleared manually by software
      if(gTxIndex <= gNbrByte)
      {
         SBUF0 = gUART_Tx[gTxIndex];
         gTxIndex++;
      }
      else
      {
         gTxIndex = 0;
      }
   }
}


/*---------------------------------------------------------------------------*-
   InterruptionTimer0 ()
  -----------------------------------------------------------------------------
   Descriptif: routine d'interruption du module timer0
   Entrée    : 
   Sortie    : 

ATTENTION: utilisation des variables globales:
         - gUART_Rx
         - gRxIndex
         - gFlagReception
-*---------------------------------------------------------------------------*/
void InterruptionTimer0 () interrupt INTERRUPT_TIMER0
{
    TR0 = 0;                     // stop le timer 0
    gUART_Rx[gRxIndex] = 0x00;   // force \0 à la fin du tableau de réception
    gRxIndex = 0;                // réinitialise l'index de reception
    gFlagReception = 1;          // indique à la boucle principale qu'une trame a été reçu
}

