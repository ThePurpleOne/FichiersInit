/*===========================================================================*=
   UART F38C / 14/10/2020 / STIRNEMANN Jonas
  =============================================================================
   Descriptif: 
=*===========================================================================*/

#include <reg51f380.h>     // registres 51f38C
#include <stdio.h>

// TIMER 0 

                           
                           
// ==== CONSTANTES ============================================================

// ==== FONCTIONS PROTOTYPES===================================================

void ClockInit ();         // init. clock syst�me
void PortInit ();          // init. config des ports
void UART_Send_Byte(char text);
void UART_Send(char *text);
void UART0_init();
void Timer1Init ();


// ==== MAIN ==================================================================
void main () 
{

   unsigned char i;
   long x;

   char bufferText[30];
   
   
   PCA0MD &= ~0x40;     // WDTE = 0 (disable watchdog timer)
   ClockInit ();        // init. clock syst�me
   PortInit ();         // init. config des ports
   UART0_init();
   Timer1Init ();
   
   TR1 = 1;   // Start le timer 1
   while (1)
   {

      sprintf(bufferText, "Compteur de secondes : %03bu\r", i);

      UART_Send(&bufferText);

      i++;

      for(x = 0; x < 10000; x++);
      
      
   } // End while (1)

} // main =====================================================================


void UART_Send_Byte(char text)
{
   TI0 = 0;
   SBUF0 = text;
   while(!TI0);
   TI0 = 0;
}

void UART_Send(char *text)
{
   int i = 0;
   while( *(text + i) != 0x00 )
   {
      UART_Send_Byte(*(text + i));
      i++;
   }
}


void UART0_init()
{
//   S0MODE = 0;      // 8-bit UART mode
//   MCE0   = 0;    // Mutiprocessor disabled
//   REN0   = 1;    // UART receive enabled
//   TI0    = 0;        // clear flag
//   RI0    = 0;        // clear flag
}



/*---------------------------------------------------------------------------*-
   Timer1Init ()
  -----------------------------------------------------------------------------
   Descriptif: Initialisation du mode de fonctionnement du timer 0
                  - Timer 8 bits auto-reload avec un start soft
                  - clock = 1 MHz KHz (sysclk/48)
                  - P?riode d?sir?e = 250 us
   Entr?e    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
void Timer1Init ()
{
   //--- Bloque le fonctionnement du timer 0 ----------------------------------
   TR1 = 0;   // Stop le timer 0
   ET1 = 0;   // Inhibe l'interruption du timer 0 
 
   //--- Configuration du timer 1 ---------------------------------------------


                     // ++++------ non utilis?s (uniquement pour le timer 1)
                     // ||||+----- Choix du mode de d?clenchement :
                     // |||||            0 : Start soft
                     // |||||            1 : Start externe 
                     // |||||+---- Choix Compteur/Timer :
                     // ||||||           0 : fonction timer 
                     // ||||||           1 : fonction compteur  
                     // ||||||++-- choix du mode de fonctionnement
                     // ||||||||       (00 : mode 0 = 13 bits           )
                     // ||||||||       (01 : mode 1 = 16 bits           )
                     // ||||||||       (10 : mode 2 = 8 bits auto-reload)
                     // ||||||||       (11 : mode 3 = pas utilis?       )
                     // |||||||| 
   TMOD = 0x20;     //  xxxx0010
 
   //--- Configuration du clock du timer 1 ------------------------------------

                     // ++-------- non utilis?s : uniquement pour le timer 3
                     // ||++------ non utilis?s : uniquement pour le timer 2
                     // ||||+----- non utilis?s : uniquement pour le timer 1    
                     // |||||+---- clock timer
                     // ||||||           0 : pr? diviseur
                     // ||||||           1 : clock syst?me
                     // ||||||++-- choix de la pr? division (pas utilis?)
                     // ||||||||       (00 : clock syst?me / 12)
                     // ||||||||       (01 : clock syst?me /  4)
                     // ||||||||       (10 : clock syst?me / 48)
                     // ||||||||       (11 : clock extern  /  8)
                     // |||||||| 
   CKCON |= 0x00;    // xxxxx000  
   
   
   TH1=0x30;       // Charge la valeur initiale 
   TL1=0x00;   
         
   //--- "Nettoyage" avant utilisation ----------------------------------------
   TF1 = 0;    // Efface une interruption r?siduelle

} // Timer0Init ---------------------------------------------------------------



/*---------------------------------------------------------------------------*-
   ClockInit ()
  -----------------------------------------------------------------------------
   Descriptif: Initialisation du mode de fonctionnement du clock systeme 
         choix : SYSCLK : oscillateur HF interne 48 MHz

   Entree    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
void ClockInit()
{  
   OSCLCN = 0x00;       
   CLKSEL = 0x23;     
   OSCICN = 0x83; 
   FLSCL  = 0x90; 

} // ClockInit ----------------------------------------------------------------

/*---------------------------------------------------------------------------*-
   PortInit ()
  -----------------------------------------------------------------------------
   Descriptif: Initialisation du mode de fonctionnement des ports 
      P0.0            : sortie (  ) -  
      P0.1            : sortie (  ) - 
      P0.2            : sortie (  ) - 
      P0.3            : sortie (  ) - 
      P0.4            : sortie (  ) - TX UART 0
      P0.5            : entr?e (  ) - RX UART 1
      P0.6            : entr?e (  ) - 
      P0.7            : entr?e (  ) - 
                               
      P1.0            : Lib    (  ) -
      P1.1            : Lib    (  ) -  
      P1.2            : Lib    (  ) -  
      P1.3            : Lib    (  ) -  
      P1.4            : Lib    (  ) -
      P1.5            : Lib    (  ) -
      P1.6            : Lib    (  ) -
      P1.7            : Lib    (  ) -
      
      P2.0            : sortie (  ) - 
      P2.1            : sortie (  ) - 
      P2.2            : analog (  ) - 
      P2.3            : analog (  ) - 
      P2.4            : analog (  ) - 
      P2.5            : analog (  ) - 
      P2.6            : analog (  ) - 
      P2.7            : analog (  ) - 

   Entree    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
void PortInit () 
{
    P0MDOUT   = 0x10;
    XBR0      = 0x01;
    XBR1      = 0x40;

} // PortInit -----------------------------------------------------------------
