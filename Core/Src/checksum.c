#include <stdio.h>
#include <stdlib.h>
/* STM32 HAL's top level header */
#include "stm32f4xx_hal.h"

__root const uint32_t __checksum       @ ".checksum";
__root const uint32_t __checksum_begin @ ".marker_crc_start" = 0x5A5A5A5A;
__root const uint32_t __checksum_end   @ ".marker_crc_end"   = 0xA5A5A5A5;
//extern uint32_t __checksum;
//extern uint32_t __checksum_begin;
//extern uint32_t __checksum_end;

static void chksumErrorHandler(void) {
  __disable_irq();
  while (1) { }
}

/* The fastCRC32() software implementation matches the 
 * STM32F407 HW-CRC, using the standard CRC-32 algorithm
 * with initial value fixed at 0xFFFF_FFFF 
 */
#define CRC32_POLY 0x04C11DB7
/* The `no_size_constraints` directive will make the function run as fast as possible */
#pragma optimize=no_size_constraints
uint32_t fastCRC32(uint32_t crc, uint32_t const* data, uint32_t words) {
  const uint32_t crc32NibbleLUT[16] = {
    0x00000000,CRC32_POLY,0x09823B6E,0x0D4326D9,
    0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
    0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,
    0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD, };
  while(words--) {
    crc = crc ^ *data++;
    /* 8 rounds * 4-bit(nibble) = 32 bit(word) */
    crc = (crc << 4) ^ crc32NibbleLUT[crc >> 28];
    crc = (crc << 4) ^ crc32NibbleLUT[crc >> 28];
    crc = (crc << 4) ^ crc32NibbleLUT[crc >> 28];
    crc = (crc << 4) ^ crc32NibbleLUT[crc >> 28];
    crc = (crc << 4) ^ crc32NibbleLUT[crc >> 28];
    crc = (crc << 4) ^ crc32NibbleLUT[crc >> 28];
    crc = (crc << 4) ^ crc32NibbleLUT[crc >> 28];
    crc = (crc << 4) ^ crc32NibbleLUT[crc >> 28];
  }
  return(crc);
}

void selfChecksum(void) {
  CRC_HandleTypeDef crcHandle;
  
  crcHandle.Instance = CRC;
  
  if (HAL_OK != HAL_CRC_Init(&crcHandle)) {
    /* CRC Peripheral initialization error */
    chksumErrorHandler();
  }
  
  uint32_t appCRC32 = fastCRC32(0xFFFFFFFF, (uint32_t*)&__checksum_begin, (&__checksum_end - &__checksum_begin) + 1);
  
  printf("Checksum starts   @: 0x%08x\n", &__checksum_begin );
  printf("Checksum ends     @: 0x%08x\n", &__checksum_end );
  printf("Checksum appCRC32  : 0x%08x\n", appCRC32);
  
  if (appCRC32 != __checksum) {
    printf("App checksum FAIL  : 0x%08x\n",__checksum);
  } else {
    printf("App checksum PASS  : 0x%08x\n",__checksum);
  }
  
  printf("                   -- The end.\n");
}
