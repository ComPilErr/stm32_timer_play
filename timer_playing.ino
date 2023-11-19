// flash storage address [24 1K] Pages from 0x0801A000 up to (0x0801FC00 + 1K)
// Page 104
#define MEM_ADDRESS 0x0801A000
//#define MEM_ADDRESS 0x08019FF0

#define MY_LED_PIN PC13
HardwareTimer timer(TIM2);
uint32_t Rx_Data[32];
uint32_t Tx_Data[32]= {0xBEAF, 0xC0FFEE, 0xACDC, 0xBBB, 0xBCBCBC};

volatile bool dark = true;

#define FLASH_OK              0x00u /**< The action was successful. */
#define FLASH_ERROR_SIZE      0x01u /**< The binary is too big. */
#define FLASH_ERROR_WRITE     0x02u /**< Writing failed. */
#define FLASH_ERROR_READBACK  0x04u /**< Writing was successful, but the content of the memory is wrong. */
#define FLASH_ERROR           0xFFu /**< Generic error. */


void setup() {

pinMode(MY_LED_PIN, OUTPUT);
digitalWrite(MY_LED_PIN, dark);
__disable_irq();

RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //(1<<0);   // Enable clock for TIM2
TIM2->PSC = 7200-1;     // Set PSC+1 = 7200-1
TIM2->ARR = 10000;     // Reset at maximum CNT value 10000
TIM2->CR1 |= TIM_CR1_ARPE | TIM_CR1_URS | TIM_CR1_CEN;
TIM2->DIER |= TIM_DIER_UIE;  // (1<<0);   // Enable timer interrupt generation
TIM2->SR &= ~(1<<0);
TIM2->CNT = 0;

NVIC_SetPriority(TIM2_IRQn, 0);
NVIC_EnableIRQ(TIM2_IRQn);
timer.attachInterrupt(OnTimerInterrupt);
__enable_irq();
memset(Rx_Data, 0, 32);
//flash_erase(MEM_ADDRESS,1);
//flash_write(MEM_ADDRESS, Tx_Data, 0);
//Flash_Read_Data(MEM_ADDRESS, Rx_Data, 1);
}

void OnTimerInterrupt(void){
    dark = !dark;
    digitalWrite(MY_LED_PIN, dark);
    TIM2->SR &= ~(1<<0);
}

void loop() {;delay(6000);
TIM2->CR1 &= ~TIM_CR1_CEN;
delay(6000);TIM2->CR1 |= TIM_CR1_CEN;
}

uint32_t flash_erase(uint32_t address, uint32_t length)
{
  HAL_FLASH_Unlock();

  uint32_t status = FLASH_ERROR;
  FLASH_EraseInitTypeDef erase_init;
  uint32_t error = 0u;

	uint32_t StartPage = GetPage(address);
	uint32_t EndPageAdress = address + length*4;
	uint32_t EndPage = GetPage(EndPageAdress);

  erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  erase_init.PageAddress = address;
  erase_init.Banks = FLASH_BANK_1;
  /* Calculate the number of pages from "address" and the end of flash. */

  erase_init.NbPages = ( (EndPage - StartPage) ) + 1;
  
  //erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  //erase_init.PageAddress = address;
  //erase_init.Banks = FLASH_BANK_1;
  /* Calculate the number of pages from "address" and the end of flash. */
  //erase_init.NbPages = ( (FLASH_BANK1_END - address) / FLASH_PAGE_SIZE );

  /* Do the actual erasing. */
  if (HAL_OK == HAL_FLASHEx_Erase(&erase_init, &error))
  {
    status = FLASH_OK;
  }
 
  HAL_FLASH_Lock();
 
  return status;
}

uint32_t flash_write(uint32_t address, uint32_t *data, uint32_t length)
{
  flash_erase(address, length);
  uint32_t status = FLASH_OK;
 
  HAL_FLASH_Unlock();
 
  /* Loop through the array. */
  for (uint32_t i = 0u; ((i < length) && (FLASH_OK == status)); i++)
  {
    /* If we reached the end of the memory, then report an error and don't do anything else.*/

      /* The actual flashing. If there is an error, then report it. */
      if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data[i]))
      {
        status |= FLASH_ERROR_WRITE;
      }
      /* Read back the content of the memory. If it is wrong, then report an error. */
      if (((data[i])) != (*(volatile uint32_t*)address))
      {
        status |= FLASH_ERROR_READBACK;
      }
 
      /* Shift the address by a word. */
      address += 4u;
  }
 
  HAL_FLASH_Lock();
 
  return status;
}

static uint32_t GetPage(uint32_t Address)
{
for (int indx=0; indx<128; indx++)
{
  if (Address <= (0x08000000 + FLASH_PAGE_SIZE*indx))
  {
    return indx; 
  }
}
return -1;
}

void Flash_Read_Data (uint32_t StartPageAddress, uint32_t *RxBuf, uint16_t numberofwords)
{
	while (1)
	{

		*RxBuf = *(__IO uint32_t *)StartPageAddress;
		StartPageAddress += 4;
		RxBuf++;
		if (!(numberofwords--)) break;
	}
}
