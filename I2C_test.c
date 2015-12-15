#include "stdio.h"
#include "main.h"
#include "PORT.H"
#include "UART.h"
#include "print_bytes.h"
#include "SPI.h"
#include "SDcard.h"
#include "Long_Serial_In.h"
#include "Directory_Functions_struct.h"
#include "File_System_v2.h"
#include "STA013_Config.h"
//#include "rtos.h"




uint8_t xdata buf1[512];
uint8_t xdata buf2[512];



// Private Function Prototypes

	 static uint32_t Current_directory, Entry_clus;
   static uint16_t i, num_entries, entry_num;
   
//void Play_Song(uint32_t Start_Cluster);


main()
{
   rtos_init();
	
   // Main Loop
EA = 1;
   while(1)
   {
		// Sleep until next interrupt
    PCON |= 0x01;
   }
} 


void rtos_init()
{
	 uint8_t error_flag;
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
   else if(OSC_PER_INST==12)
   {
      CKCON0=0x00;  // set standard clock mode
   } 
	 	// Tick rate (1 ms)
  reload = (uint16)(65536UL - (uint32)TICK_MS * (OSC_FREQ/1000) / (uint32)OSC_PER_INST);
  reloadHigh = (uint8)(reload / 256);
  reloadLow = (uint8)(reload % 256);
	 
	 // Init timer2
  T2CON = 0x04;
  TH2 = reloadHigh;
  RCAP2H = reloadHigh;
  TL2 = reloadLow;
  RCAP2L = reloadLow;
  
  IPH0 &= 0xDF; // Priority 1
  IPL0 |= 0x20;
  ET2 = 1;
  TR2 = 1;
	 
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
		     Open_File(Entry_clus, buf1);
			 //Play_Song(Entry_clus);
		  }
		  
	   }
	   else
	   {
	      printf("Invalid Selection\n\r");
	   }  
}




void timer2_ISR(void) interrupt 5
{	

	TF2 = 0;
	TR0 = 0;
	TH0 = TIMER0H;
	TL0 = TIMER0L;
	TF0 = 0;
	TR0 = 1;
	
	
	if(state_g == LOAD_BUFFER_1)
	{
		if(sector_offset >= 64)
		{
			state_g = FIND_CLUSTER_1;
		}
	}

	if(state_g == LOAD_BUFFER_2)
	{
		if(sector_offset >= 64)
		{
			state_g = FIND_CLUSTER_2;
		}
	}


	switch(state_g)
	{
		// Data Send 1
		case DATA_SEND_1:
		{
			LED1 = 0;
			LED2 = 1;
			LED3 = 1;
			LED4 = 1;
			spi_en = 1;
			trig = 0;
			while((DATA_REQ == ACTIVE) && (TF0 == 0))  								
			{
				SPI_transfer_ISR(buff1[index1_g], & temp8); 							// What is temp8
				index1_g++;																								// Loosing last bit?
				if(index1_g > 511) // Buffer 1 empty
				{
					if(index2_g >511)
					{
						if(play_status == 3)																	
						{
							play_status = 0;
						}
						else
						{
							state_g = LOAD_BUFFER_2; // Buff 2 and Buff 1 empty
						}
					}																												
					else
					{
						state_g = DATA_SEND_2; // BUFF 1 empty  							//Seems like this is when buffer 1 still has stuff
					}
					TF0 = 1;
				}				// What does this do? is this the interupt for the whole system to keep pace
			}
			if((DATA_REQ == INACTIVE) && (state_g == DATA_SEND_1))
			{
				printf("INACTIVE 1\n\r");
				if(index2_g > 511) // Buffer 2 is empty
				{
					state_g = LOAD_BUFFER_2; // DR inactive and BUFF 2 empty
				}
				else
				{
					state_g = DATA_IDLE_1; // DR interupt
				}
			}
			if(DATA_REQ == INACTIVE)
			{
				printf("INACTIVE 2\n\r");
			}
			spi_en = 0;
			break;
		}
		
		//Load Buffer 1
		case LOAD_BUFFER_1:
		{
			LED1 = 1;
			LED2 = 1;
			LED3 = 1;
			LED4 = 1;
			//printf("LOAD_BUFFER_1\n\r");
			sector = sector_base_g + sector_offset;
			read_sector_ISR(sector, buff1);
			//printf("%8.8lX", sector);
			sector_offset++;
			state_g = DATA_IDLE_2;
			index1_g = 0;
			
			break;
		}
		
		// Find Cluster 1
		case FIND_CLUSTER_1:
		{
			LED1 = 1;
			LED2 = 0;
			LED3 = 1;
			LED4 = 1;
			//printf("FIND_CLUSTER_1\n\r");
			cluster_g = find_next_cluster_ISR(cluster_g, buff1);
			if(cluster_g == 0x0FFFFFFF) // Last cluster
			{
				printf("DONE\n\r");
				play_status = 3;
				state_g = DATA_IDLE_2;
			}
			else
			{
				sector_base_g = first_sector_ISR(cluster_g);
				sector_offset = 0;
				state_g = DATA_IDLE_2;
			}
			break;
		}
		
		case DATA_IDLE_1:
		{
			LED1 = 1;
	LED2 = 1;
	LED3 = 1;
	LED4 = 1;
			//printf("DATA_IDLE_1\n\r");
			if(DATA_REQ == ACTIVE)
			{
				state_g = DATA_SEND_1;
			}
			break;
		}
				
		// Data Send 2
		case DATA_SEND_2:
		{
			LED1 = 1;
			LED2 = 1;
			LED3 = 0;
			LED4 = 1;
			//printf("DATA_SEND_2\n\r");
			spi_en = 1;
			while((DATA_REQ == ACTIVE) && (TF0 == 0))  								// Can DATA_REQ go inactive while in the loop
			{
				SPI_transfer_ISR(buff1[index2_g], & temp8); 							// What is temp8
				index2_g++;
				if(index2_g > 511) // Buffer 2 empty
				{
					if(index1_g > 511) // Buffer 1 empty
					{
						if(play_status == 3)																	// Works only if after FIND_CLUSTER. why?
						{
							play_status = 0;
						}
						else
						{
							state_g = LOAD_BUFFER_1; // Buff 1 and Buff 2 empty
						}
					}																												// No FIND_CLUSTER
					else
					{
						state_g = DATA_SEND_1; // BUFF 2 empty  							//Seems like this is when buffer 2 still has stuff
					}
					TF0 = 1;																								// What does this do? is this the interupt for the whole system to keep pace
				}
			}
			if((DATA_REQ == INACTIVE) && (state_g == DATA_SEND_2))
			{
				printf("INACTIVE 2\n\r");
				if(index1_g > 511) // Buffer 1 is empty
				{
					state_g = LOAD_BUFFER_1; // DR inactive and BUFF 1 empty
				}
				else
				{
					state_g = DATA_IDLE_2; // DR interupt
				}
			}
			if(DATA_REQ == INACTIVE)
			{
				printf("INACTIVE 2\n\r");
			}
			
			spi_en = 0;
			break;
		}
		
		//Load Buffer 2
		case LOAD_BUFFER_2:
		{
			LED1 = 1;
	LED2 = 1;
	LED3 = 1;
	LED4 = 1;
			//printf("LOAD_BUFFER_2\n\r");
			sector = sector_base_g + sector_offset;
			read_sector_ISR(sector, buff2);
			sector_offset++;
			state_g = DATA_IDLE_1;
			index2_g = 0;
			break;
		}
		
		// Find Cluster 2
		case FIND_CLUSTER_2:
		{
			LED1 = 1;
	LED2 = 1;
	LED3 = 1;
	LED4 = 0;
			//printf("FIND_CLUSTER_2\n\r");
			cluster_g = find_next_cluster_ISR(cluster_g, buff2);
			if(cluster_g == 0x0FFFFFFF)
			{
				play_status = 3;
				state_g = DATA_IDLE_1;
			}
			else
			{
				sector_base_g = first_sector_ISR(cluster_g);
				sector_offset = 0;
				state_g = DATA_IDLE_1;
			}
			break;
		}
		
		case DATA_IDLE_2:
		{
			LED1 = 1;
	LED2 = 1;
	LED3 = 1;
	LED4 = 1;
			//printf("DATA_IDLE_2\n\r");
			if(DATA_REQ == ACTIVE)
			{
				state_g = DATA_SEND_2;
			}
			break;
		}						
	}
	trig = 1;
}
