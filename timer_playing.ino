
#define MY_LED_PIN PC13
HardwareTimer timer(TIM2);

volatile bool dark = true;

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

//
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
