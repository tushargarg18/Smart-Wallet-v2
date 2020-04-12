void TIM3_IRQHandler(void);
void delay_us(int us);
void delay_ms(int ms);
void tim3_init(void);

unsigned int myticks=0;

void tim3_init(void)
{
	RCC->APB1ENR |= (1<<1);//tim3 clock enable
	TIM3->PSC = 0;
	TIM3->ARR = 71;//1 mhz
	TIM3->CR1 |= (1<<2);//urs update request source only overflow counter can gen interrupt
	TIM3->DIER |= (1<<0);//uie update interrupt enable
	TIM3->EGR |= (1<<0);//ug update generation
  //TIM3->CR1 |= (1<<0);//counter enable
	NVIC_EnableIRQ(TIM3_IRQn); // enable irq line for tim3
}
void TIM3_IRQHandler(void)
{
	/*if(TIM3->SR & TIM_SR_UIF)
	{
		myticks++;
	}*/
	TIM3->SR &=~ (1<<0);//bit is cleared to exit the function
	myticks++;
}

void delay_us(int us)
{
	TIM3->CR1 |= (1<<0);//counter enable
	myticks=0;
	while(myticks<us);
	TIM3->CR1 &=~ (1<<0);//counter disable
	
}

void delay_ms(int ms)
{
	TIM3->CR1 |= (1<<0);//counter enable
	myticks=0;
	while(myticks<ms*1000);
	TIM3->CR1 &=~ (1<<0);//counter disable	
}




