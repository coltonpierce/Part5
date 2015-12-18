//***********************************      Play_Song.c      ***********************************//


#include "stdio.h"
#include "Main.h"
#include "PORT.H"
#include "SPI.h"
#include "File_System_v2.h"
#include "Read_Sector.h"
#include "Play_Song.h"
#include "SD_Card.h"
extern uint8_t xdata buf1[512];
extern uint8_t xdata buf2[512]; 
///*
uint8_t idata state_g, play_status, temp8;
uint16_t idata index1_g, index2_g;
uint32_t idata sector_offset, sector, cluster_g;

void Play_Song(uint32_t Start_Cluster)
{
	printf("Starting Cluster = %lu\n\r",Start_Cluster);

  sector=First_Sector(Start_Cluster);
  printf("Starting Sector = %lu\n\r",sector);
  sector_offset=0;

  index1_g=0;
  Read_Sector(sector+sector_offset, 512, buf1);

  sector_offset++;
  index2_g=0;
   
  Read_Sector(sector+sector_offset, 512, buf2);
  
	sector_offset++;
	
	cluster_g = Start_Cluster;
	state_g = DATA_SEND_1;
	play_status = 1;
	TH2 = TIMER2H;
	RCAP2H = TIMER2H;
	TL2 = TIMER2L;
	RCAP2L = TIMER2L;
	TF0 = 0;
	T2CON = 0x00;
	TMOD |= 0x01;
	ET2 = 1;
	EA = 1;
	
	TR2 = 1;
	while(play_status != 0 && SW3 == 1)
	{
		PCON |= 0x01;
	}
	
	TR2 = 0;
	
	
}
	
	
void timer2 (void) interrupt 5
{	
	
	TR0 = 0;
	TH0 = TIMER0H;
	TL0 = TIMER0L;
	TF0 = 0;
	TR0 = 1;
	
	
		if(SW1 == 0 && state_g == PAUSE)  // Play
	{
		state_g = temp8;
	}
	if(SW2 == 0 && state_g != PAUSE) // Pause
	{
		temp8 = state_g;
		state_g = PAUSE;
	}
	
	
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
			while((DATA_REQ == ACTIVE) && (TF0 == 0))  								
			{
				BIT_EN = 1;
				SPI_Transfer_ISR(buf1[index1_g], & temp8); 							
				index1_g++;																								
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
				BIT_EN = 0;
			}
			if((DATA_REQ == INACTIVE) && (state_g == DATA_SEND_1))
			{
				
				if(index2_g > 511) // Buffer 2 is empty
				{
					state_g = LOAD_BUFFER_2; // DR inactive and BUFF 2 empty
				}
				else
				{
					//state_g = DATA_SEND_1;
					state_g = DATA_IDLE_1; // DR interupt
				}
			}
			
			break;
		}
		
		//Load Buffer 1
		case LOAD_BUFFER_1:
		{
			LED1 = 1;
			LED2 = 1;
			LED3 = 0;
			LED4 = 1;
			//printf("LOAD_BUFFER_1\n\r");
			Read_Sector_ISR(sector+sector_offset, 512, buf1);
			//printf("%8.8lX", sector);
			sector_offset++;
			//state_g = DATA_SEND_2;
			state_g = DATA_IDLE_2;
			index1_g = 0;
			
			break;
		}
		
		// Find Cluster 1
		case FIND_CLUSTER_1:
		{
			LED1 = 1;
			LED2 = 1;
			LED3 = 1;
			LED4 = 0;
			//printf("FIND_CLUSTER_1\n\r");
			cluster_g = Find_Next_Clus_ISR(cluster_g, buf1);
			if(cluster_g == 0x0FFFFFFF) // Last cluster
			{
				//printf("DONE\n\r");
				play_status = 3;
				//state_g = DATA_SEND_2;
				state_g = DATA_IDLE_2;
			}
			else
			{
				sector = First_Sector_ISR(cluster_g);
				sector_offset = 0;
				//state_g = DATA_SEND_2;
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
			LED2 = 0;
			LED3 = 1;
			LED4 = 1;
			//printf("DATA_SEND_2\n\r");
			
			while((DATA_REQ == ACTIVE) && (TF0 == 0))  								// Can DATA_REQ go inactive while in the loop
			{
				BIT_EN = 1;
				SPI_Transfer_ISR(buf2[index2_g], & temp8); 							// What is temp8
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
				BIT_EN = 0;
			}
			if((DATA_REQ == INACTIVE) && (state_g == DATA_SEND_2))
			{
				//printf("INACTIVE 2\n\r");
				if(index1_g > 511) // Buffer 1 is empty
				{
					state_g = LOAD_BUFFER_1; // DR inactive and BUFF 1 empty
				}
				else
				{
					//state_g = DATA_SEND_2;
					state_g = DATA_IDLE_2; // DR interupt
				}
			}
			
			
			break;
		}
		
		//Load Buffer 2
		case LOAD_BUFFER_2:
		{
			LED1 = 1;
	LED2 = 1;
	LED3 = 0;
	LED4 = 1;
			//printf("LOAD_BUFFER_2\n\r");
			Read_Sector_ISR(sector+sector_offset, 512, buf2);
			sector_offset++;
			//state_g = DATA_SEND_1;
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
			cluster_g = Find_Next_Clus_ISR(cluster_g, buf2);
			if(cluster_g == 0x0FFFFFFF)
			{
				play_status = 3;
				//state_g = DATA_SEND_1;
				state_g = DATA_IDLE_1;
			}
			else
			{
				sector = First_Sector_ISR(cluster_g);
				sector_offset = 0;
				//state_g = DATA_SEND_1;
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
		case PAUSE:
		{
			//printf("%2.2bX", temp8);
			break;
		}
	}
	//trig = 1;
	TF2 = 0;
}

	
	
	
	
	
	

	
//   uint8_t buffer1, buffer2, temp8;
//   
//   AMBERLED=OFF;
//   do
//  {      
//     do
//     {
//        if(DATA_REQ==0)
//        {
//           GREENLED=ON;
//           BIT_EN=1;
//           SPI_Transfer(buf1[index1], &temp8);
//	       GREENLED=OFF;
//	       index1++;
//           if(index1>511)
//           {
//              if(index2>511)
//              {
//                  BIT_EN=0;              
//                  AMBERLED=ON;
//				  index2=0;
//				  
//				  Read_Sector(sector+sector_offset, 512, buf2);
//				  sector_offset++;
//                  AMBERLED=OFF;
//              }
//              buffer1=0;
//              buffer2=1;

//          }
//       }
//       else
//       {
//          if(index2>511)
//          {
//              BIT_EN=0;
//              AMBERLED=ON;
//			  index2=0;
//			  
//			  Read_Sector(sector+sector_offset, 512, buf2);
//			  sector_offset++;
//              AMBERLED=OFF;
//          }
//          else
//          {
//              if(index1>511)
//              {
//                  buffer1=0;
//                  buffer2=1;
//              }
//          }
//      }
//   }while(buffer1==1);
//   do
//   {
//      if(DATA_REQ==0)
//      {
//          REDLED=ON;
//          BIT_EN=1;
//          SPI_Transfer(buf2[index2], &temp8);
//          REDLED=OFF;
//          index2++;
//          if(index2>511)
//          {
//              if(index1>511)
//              {
//                  BIT_EN=0; 
//                  YELLOWLED=ON;
//				  index1=0;
//				  
//				  Read_Sector(sector+sector_offset, 512, buf1);
//				  sector_offset++;
//                  YELLOWLED=OFF;
//              }
//              buffer2=0;
//              buffer1=1;
//         
//           }
//        }
//        else
//        {
//           if(index1>511)
//           {
//              BIT_EN=0; 
//              YELLOWLED=ON;
//			  index1=0;
//			  
//			  Read_Sector(sector+sector_offset, 512, buf1);
//			  sector_offset++;
//              YELLOWLED=OFF;
//           }
//           else
//           {
//               if(index2>511)
//               {
//                  buffer2=0;
//                  buffer1=1;
//               }
//           }
//        }
//      }while(buffer2==1);
//  }while(sector_offset<512);
////P3_2=OFF;
//} 






//*/
/*

void Play_Song2(uint32_t Start_Cluster)
{
   uint16_t index1;
   uint8_t buff_emp, * p_out;
   uint32_t sector, sector_offset;
printf("Starting Cluster = %lu\n\r",Start_Cluster);
sector=First_Sector(Start_Cluster);
printf("Starting Sector = %lu\n\r",sector);
//P3_2=ON;
sector_offset=0;
YELLOWLED=ON;
nCS0=0;
SEND_COMMAND(17,sector+sector_offset);
read_block(512,buf1);
index1=0;
sector_offset++;
nCS0=1;
YELLOWLED=OFF;
//AMBERLED=ON;
//nCS0=0;
//SEND_COMMAND(17,sector+sector_offset);
//read_block(buf2,512);
//sector_offset++;
//nCS0=1;
//AMBERLED=OFF;


   p_out=buf1;
   buff_emp=0;
   do
   {
      if(DATA_REQ==0)
      {
         GREENLED=ON;
         BIT_EN=1;
         while((SPSTA&0x80)!=0x80);
         SPDAT=*(buf1+index1);
         index1++;
         if(index1==512)
         {
            buff_emp|=1;           
         }
		 if(index1==1024)
		 {
		    index1=0;
			buff_emp|=2;
	     }
         if(index1==768)
         {
           BIT_EN=0;
           GREENLED=OFF;
           if((buff_emp & 0x01)==0x01)
           {
              YELLOWLED=ON;
              nCS0=0;
              SEND_COMMAND(17,sector+sector_offset);
              read_block(512,buf1);
              nCS0=1;             
              YELLOWLED=OFF;
              buff_emp &= 0xFE;
              sector_offset++;            
           }
         }
         if(index1==256)
         {
           BIT_EN=0;
           GREENLED=OFF;
           if((buff_emp & 0x02)==0x02)
           {
              AMBERLED=ON;
              nCS0=0;
              SEND_COMMAND(17,sector+sector_offset);
              read_block(512,buf2);
              nCS0=1;             
              AMBERLED=OFF;
              buff_emp &= 0xFD;
              sector_offset++;            
           }
         }                        
       }
       else
       {
          GREENLED=OFF;
          BIT_EN=0;
          if((buff_emp & 0x01)==0x01)
          {
             YELLOWLED=ON;
             nCS0=0;
             SEND_COMMAND(17,sector+sector_offset);
             read_block(512,buf1);
             nCS0=1;             
             YELLOWLED=OFF;
             buff_emp &= 0xFE;
             sector_offset++;
 //            print_hex(1);
 //            print_hex(i);
            
          }
          else if((buff_emp & 0x02)==0x02)
          {
             AMBERLED=ON;
             nCS0=0;
             SEND_COMMAND(17,sector+sector_offset);
             read_block(512,buf2);
             nCS0=1;             
             AMBERLED=OFF;
             buff_emp &= 0xFD;
             sector_offset++;
          }
       }
   }while(sector_offset<128);   
   GREENLED=1;
//   P3_2=OFF;
  }  

 */  