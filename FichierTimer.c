/*===========================================================================*=
   Projet base F38C / DATE / STIRNEMANN Jonas
  =============================================================================
   Descriptif:
=*===========================================================================*/

#include <reg51f380.h>     // registres 51f38C

// ==== CONSTANTES ============================================================

// TIMER 0 
#define LOADVALUE    6     // valeur pour 250 us avec un clock ? 1 MHz
                           // valeur = 256 - (p?riode d?sir?e * clock)
                           //        = 256 - (250 us * 1 MHz)
                           //        = 256 - 250
                           //        = 6 


// ==== FONCTIONS PROTOTYPES===================================================

void ClockInit ();         // init. clock syst�me
void PortInit ();          // init. config des ports

// ==== MAIN ==================================================================
void main () 
{

   PCA0MD &= ~0x40;     // WDTE = 0 (disable watchdog timer)
   ClockInit ();        // init. clock syst�me
   PortInit ();         // init. config des ports
   Timer0Init ();

   // Etat des autorisations des interruptions
   ET0 = 1;    // autorisation de l'interruption du timer 0
   EA = 1;     // autorisation globale des interruptions

   //Enable Les interrupts
   TR0 = 1;   // Start le timer 0

   while (1)
   {

   } // End while (1)

} // main =====================================================================


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
      P0.4            : sortie (  ) - 
      P0.5            : entr?e (  ) - 
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

} // PortInit -----------------------------------------------------------------



/*---------------------------------------------------------------------------*-
   Timer0Init ()
  -----------------------------------------------------------------------------
   Descriptif: Initialisation du mode de fonctionnement du timer 0
                  - Timer 8 bits auto-reload avec un start soft
                  - clock = 1 MHz KHz (sysclk/48)
                  - P?riode d?sir?e = 250 us
   Entr?e    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
void Timer0Init ()
{
   //--- Bloque le fonctionnement du timer 0 ----------------------------------
   TR0 = 0;   // Stop le timer 0
   ET0 = 0;   // Inhibe l'interruption du timer 0 
 
   //--- Configuration du timer 0 ---------------------------------------------
   TMOD &= 0xF0;     // Reset l'initialisation du timer 0

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
   TMOD |= 0x02;     // xxxx0010
 
   //--- Configuration du clock du timer 0 ------------------------------------
   CKCON &= ~0x04;   // RAZ config. clock timer 0 
   CKCON &= ~0x03;   // RAZ config. pre-scalaire 

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
   CKCON |= 0x02;    // xxxxx010  
   
   
   TH0=LOADVALUE;       // Charge la valeur initiale 
   TL0=LOADVALUE;   
         
   //--- "Nettoyage" avant utilisation ----------------------------------------
   TF0 = 0;    // Efface une interruption r?siduelle

} // Timer0Init ---------------------------------------------------------------
