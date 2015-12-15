#include "main.h"
#include "PORT.H"
#include "stdio.h"
#include "UART.h"
#include "SPI.h"
#include "SDcard.h"
#include "Long_Serial_In.h"
#include "File_System_v2.h"
#include "STA013_Config.h"
#include "Directory_Functions_struct.h"
#include "rtos.h"




//void rtos_init(void)
//{
//  uint16 reload;
//  uint8 reloadHigh;
//  uint8 reloadLow;
//  
//  // Arrays from config.asm and other MP3 variables
//  extern code uint8* CONFIG;
//  extern code uint8* CONFIG2;
//  uint8 code* configPtr;
//  uint8 idata temp[2] = {0x01};
//  uint8 i;
//  uint8 error = 0;
//  
//	// Tick rate (1 ms)
//  reload = (uint16)(65536UL - (uint32)TICK_MS * (OSC_FREQ/1000) / (uint32)OSC_PER_INST);
//  reloadHigh = (uint8)(reload / 256);
//  reloadLow = (uint8)(reload % 256);
//  
//  // X2 Mode
//  CKCON0 = 0x01;
//  CKCON1 = 0x00;
//  
//  // XRAM Size
//  AUXR = 0x0D;
//  
//	// Init timer2
//  T2CON = 0x04;
//  TH2 = reloadHigh;
//  RCAP2H = reloadHigh;
//  TL2 = reloadLow;
//  RCAP2L = reloadLow;
//  
//  IPH0 &= 0xDF; // Priority 1
//  IPL0 |= 0x20;
//  ET2 = 1;
//  TR2 = 1;
//  
//  uart_init();
//  spi_sdcard_init();
//  fs_init();
//  
//  // Reset MP3
//  mp3_reset = 0;
//  for(i=0; i<80; i++);
//  mp3_reset = 1;
//  
//  // Test TWI
//  do
//  {
//    error = twi_write(0x43, 1, temp); 
//  } while(error != 0);
//  do
//  {
//    error = twi_read(0x43, 1, temp);
//  } while(error != 0);
//  if(temp[0] != 0xAC)
//  	redLED = 0;
//  
//  // Send STAO13 config file
//  configPtr = CONFIG;
//  while(*configPtr != 0xFF)
//  {
//    temp[0] = *configPtr;
//    configPtr++;
//    temp[1] = *configPtr;
//    configPtr++;
//    error = twi_write(0x43, 2, temp);
//  }
//  
//  // Pause
//  for(i=0; i<80; ++i);
//  
//  // Send second STAO13 config
//  configPtr = CONFIG2;
//  while(*configPtr != 0xFF)
//  {
//    temp[0] = *configPtr;
//    configPtr++;
//    temp[1] = *configPtr;
//    configPtr++;
//    error = twi_write(0x43, 2, temp);
//  }
//  
//  startCluster = fs_findMP3(2);
//  
//  rtos_state = RTOS_CHECK_MEM;
//}

//void rtos_spin()
//{
//  bit spin = 1;
//  
//  EA = 1;
//  
//  while(spin)
//  {
//    // Sleep until next interrupt
//    PCON |= 0x01;
//  }
//}

//void rtos_tick_ISR(void) interrupt 5 using 3
//{
//  static bit song = 0;
//  static bit buf1 = 0;
//  static bit buf2 = 0;
//  static bit cluster = 0;
//  static bit currentBuf = 0;
//  static uint32 clusterNum = 0UL;
//  static uint8 relativeSector = 0;
//  static uint16 currentByte = 0;
//  
//  extern uint8 xdata* fs_buffer;
//  
//	TF2 = 0;
//  
//  if(clusterNum == 0UL)
//  {
//    clusterNum = startCluster;
//    cluster = 1;
//  }
//  
//  switch(rtos_state)
//  {
//  case RTOS_SEND_DATA:
//    fs_setBuffer(currentBuf);
//    while(mp3_data_req && (buf1 || buf2))
//    {
//      spi_mp3_data(1, fs_buffer + currentByte);
//      currentByte++;
//      if(currentByte == 512)
//      {
//        // Buffer empty, swap
//        if(currentBuf)
//          buf2 = 0;
//        else
//          buf1 = 0;
//        currentBuf = ~currentBuf;
//        fs_setBuffer(currentBuf);
//        currentByte = 0;
//      }
//    }
//    if(mp3_data_req == 0)
//    {
//      redLED=~redLED;
//      song = 1;
//    }
//    rtos_state = RTOS_CHECK_MEM;
//    break;
//  
//  case RTOS_CHECK_MEM:
//    if(buf1 == 0 || buf2 == 0)
//    {
//      if(relativeSector == 64)
//      {
//        rtos_state = RTOS_NEXT_CLUS;
//        cluster = 0;
//      }
//      else
//        rtos_state = RTOS_READ_SEC;
//    }
//    else 
//      rtos_state = RTOS_SEND_DATA;
//    break;
//  
//  case RTOS_READ_SEC:
//    if(buf1 == 0 || buf2 == 0)
//    {
//      if(buf1 == 0)
//      {
//        fs_setBuffer(0);
//        buf1 = 1;
//      }
//      else
//      {
//        fs_setBuffer(1);
//        buf2 = 1;
//      }
//      
//      fs_loadSector(clusterNum, relativeSector);
//      relativeSector++;
//    }
//    
//    rtos_state = RTOS_SEND_DATA;
//    break;
//  
//  case RTOS_NEXT_CLUS:
//    clusterNum = fs_FATentry(clusterNum);
//    relativeSector = 0;
//    if(fs_FATeof(clusterNum))
//    {
//      song = 0;
//      EA = 0; //TODO new song, find start cluster
//    }
//    else
//      cluster = 1;
//    
//    if(song)
//      rtos_state = RTOS_SEND_DATA;
//    else
//      rtos_state = RTOS_READ_SEC;
//    break;
//  
//  default:
//    rtos_state = RTOS_SEND_DATA;
//    break;
//  }
//}