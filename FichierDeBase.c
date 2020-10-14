/*===========================================================================*=
   Projet base F38C / DATE / STIRNEMANN Jonas
  =============================================================================
   Descriptif:
=*===========================================================================*/

#include <reg51f380.h>     // registres 51f38C

// ==== CONSTANTES ============================================================

// ==== FONCTIONS PROTOTYPES===================================================

void ClockInit ();         // init. clock syst�me
void PortInit ();          // init. config des ports

// ==== MAIN ==================================================================
void main () 
{

   PCA0MD &= ~0x40;     // WDTE = 0 (disable watchdog timer)
   ClockInit ();        // init. clock syst�me
   PortInit ();         // init. config des ports

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
	
   P1MDOUT = 0xFF;   

   // PnSKIP : R�servation des entr�es/sorties utilis�es ----------------------
   P1SKIP = 0xFF;       // r�serve P1
   P2SKIP = 0xFF;       // R�serve P2

   // XBRn : Autorisation des ressources internes -----------------------------
   XBR1 |= 0x40;        // autorise le fonctionnement du crossbar

} // PortInit -----------------------------------------------------------------
