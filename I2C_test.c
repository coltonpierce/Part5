#include "stdio.h"
#include "main.h"
#include "PORT.H"
#include "UART.h"
#include "print_bytes.h"
#include "SPI.h"
#include "SDcard.h"
#include "Long_Serial_In.h"
#include "Directory_Functions_struct.h"
#include "File_System.h"
#include "STA013_Config.h"
#include "Play_Song.h"


uint8_t xdata buf1[512];
uint8_t xdata buf2[512];



// Private Function Prototypes
//void Play_Song(uint32_t Start_Cluster);


main()
{
   uint32_t idata Current_directory, Entry_clus;
   uint16_t idata i, num_entries, entry_num;
   uint8_t idata error_flag;
   FS_values_t * Drive_p;

   AMBERLED=OFF;
   YELLOWLED=OFF;
   GREENLED=OFF;
   REDLED = ON;
   STA013_RESET=0;
   i=0;
   while(i<=60000) i++;
   REDLED = OFF;
   AUXR=0x0c;   // make all of XRAM available
   if(OSC_PER_INST==6)
   {
      CKCON0=0x01;  // set X2 clock mode
   }
//   else if(OSC_PER_INST==12)
//   {
//      CKCON0=0x00;  // set standard clock mode
//   } 
   uart_init();
   printf("I2C Test Program\n\r\n\n");
   error_flag=SPI_Master_Init(400000UL);
   if(error_flag!=no_errors)
   {
      REDLED=ON;
      while(1);
   }
   printf("SD Card Initialization ... \n\r");
   error_flag=SD_card_init();
   if(error_flag!=no_errors)
   {
      REDLED=ON;
      while(1);
   }
   error_flag=SPI_Master_Init(20000000UL);
   if(error_flag!=no_errors)
   {
      REDLED=ON;
      while(1);
   }
   for(i=0;i<512;i++)
   {
      buf1[i]=0xff;  // erase valout for debug
      buf2[i]=0xff;
   }
   error_flag=Mount_Drive(buf1);
   if(error_flag!=no_errors)
   {
      REDLED=ON;
      while(1);
   }
   Drive_p=Export_Drive_values();
   Current_directory=Drive_p->FirstRootDirSec;
   
   

    STA013_init();   
   // Main Loop

   while(1)
   {
       printf("Directory Sector = %lu\n\r",Current_directory);
       num_entries=Print_Directory(Current_directory, buf1);
	   printf("Enter Selection = ");
  	   entry_num=(uint16_t)long_serial_input();
	   if(entry_num<=num_entries)
	   {
	      Entry_clus=Read_Dir_Entry(Current_directory, entry_num, buf1);
		  if(Entry_clus&directory_bit)
		  {
		     Entry_clus&=0x0FFFFFFF;
             Current_directory=First_Sector(Entry_clus);
		  }
 	      else
		  {
		    // Open_File(Entry_clus, buf1);
				Play_Song(Entry_clus);
		  }
		  
	   }
	   else
	   {
	      printf("Invalid Selection\n\r");
	   }  
   }
} 




