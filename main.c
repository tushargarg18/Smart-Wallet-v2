#include "stm32f10x.h"
#include "delay.h"
#include "uart1_tx.h"

unsigned int count_ticks = 0;							//for timer2 interrupt
uint8_t val=0;						//global variable for adc convertion
const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";		//Base64 charachters
uint8_t *data; 
uint64_t samples = 0;
int count = 0,now = 0, flag = 0;

//47630  32768
#define stopTime	(TIM2->CR1 &= ~(1<<0))  //disable counter2
#define generateSOS												//sos						
#define startADC (ADC1->CR2 |= ADC_CR2_ADON)		//start adc convertion adc on/off bit
#define stopADC (ADC1->CR2 &= ~ADC_CR2_ADON)		//stop adc convertion adc on/off bit

void dma_init() 
{
	RCC->AHBENR |= (1<<0);    //dma1 clock
	ADC1->CR2 |= (1<<8);     //DMA ENABLED FOR ADC
	//USART1->CR3 |= (1<<7);     //DMA ENABLED FOR USART1 tx
	//USART1->CR3 |= (1<<6);     //DMA ENABLED FOR USART1 rx
	DMA1_Channel5->CCR |= (1<<13) | (1<<12);   //very high priority
	//DMA1_Channel1->CCR = (1<<8) | (1<<10);
	
	//uint16_t len = strlen(data);
	DMA1_Channel5->CNDTR = 1;		//14
	
	//DMA1_Channel4->CCR |= (1<<4);     //read from memory
	DMA1_Channel5->CCR &= (1<<4);     //read from peripheral
	DMA1_Channel5->CCR |= (1<<7);    //memory increment
	//DMA1_Channel5->CCR |= (1<<5);    //circular mode
	//DMA1_Channel5->CCR |= (1<<1);      //interrupt enable
	
	DMA1_Channel5->CPAR = (uint32_t) &ADC1->DR;
	DMA1_Channel5->CMAR = (uint32_t) data;
}


void startTime(void)
{
	TIM2->CNT = 0;						//initialise timer
	TIM2->CR1 |= (1<<0);			//start timer
	flag = 1;
}


void timer2_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //timer 2 clock enable
	TIM2->PSC = 0;											//clock 72MHz
	TIM2->ARR = 0xffff;									//maximum value
	TIM2->CR1 |= (1<<2);								//urs update request source only overflow counter can gen interrupt
	TIM2->DIER |= (1<<0);								//uie update interrupt enable
	TIM2->EGR |= (1<<0);								//ug update generation
	NVIC_EnableIRQ(TIM2_IRQn); 					//enable irq line for tim2
}


void TIM2_IRQHandler(void)
{
	TIM2->SR &=~ (1<<0);								//bit is cleared to exit the function
	count_ticks++;
	if(count_ticks == 3310)							//3295 for 3 sec delay  //7690 to make a delay of 7 sec	
	{
		stopTime;													//stop timer
		stopADC;													//stopsampling
		//delay_ms(2);													
		GPIOC->BSRR |=(1<<13); 						//set pin 13 high on bord led
		count_ticks = 0;
		flag = 0;
	}
}



void adc_config()
{
	RCC->CFGR |= (1<<15);
	RCC->CFGR &= ~(1<<14);							//10 prescaler divide by 72/6 
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN|RCC_APB2ENR_AFIOEN|RCC_APB2ENR_IOPAEN;
	
	GPIOA->CRL |= GPIO_CRL_CNF5_1;
	GPIOA->CRL &= ~(GPIO_CRL_CNF5_0);
	
	ADC1->CR1 |= ADC_CR1_EOCIE;
	
	NVIC_EnableIRQ(ADC1_2_IRQn);
	
	ADC1->SMPR2 |= ADC_SMPR2_SMP5_2 | ADC_SMPR2_SMP5_0 | ADC_SMPR2_SMP5_1;
	
	ADC1->SQR3 |= ADC_SQR3_SQ1_0 | ADC_SQR3_SQ1_2;
	
	ADC1->CR2 |= ADC_CR2_ADON |ADC_CR2_CONT;
	
	delay_ms(1);
	ADC1->CR2 |= ADC_CR2_CAL;
	delay_ms(2);
}

void gpio_init()
{
	//initialise the clock for port c
	RCC->APB2ENR |= (1<<4)|(1<<2);	
	
	//configuration pin3 of c port
	GPIOC->CRH |= (  (1<<20) | (1<<21)  ); 		//output mode maximum speed 50mhz
	GPIOC->CRH &= ~((1<<22) | (1<<23)); 			//general purpose output
	
	//gpio c15 as input floating type
	GPIOC->CRH &= ~((1<<30) | (1<<31));
	GPIOC->CRH |= (1<<28);
	GPIOC->CRH &= ~(1<<29);
	
	//********interrupt enable***********
	//RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;						//enable clock  for alternate register
	//EXTI->IMR |= (1<<0);													//interrupt mask register
	//EXTI->RTSR |=(1<<0);													//rising edge triggered
	//AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PA;		//port c15 selected 
	//EXTI->PR = 0;																	//clear all pending request
	//NVIC_EnableIRQ(6); 													//enable irq line for gpio c15
}

/*void  EXTI0_IRQ_IRQHandler(void)
{
	EXTI->PR |= (1<<15); 					//reset pending request
	delay_ms(1500);
	GPIOC->BSRR = (1<<(13+16)); //set pin13 low
	if((GPIOC->IDR & (1<<15)))
	{
		generateSOS;
		return;
	}
	startSampling;
	startTime();
	GPIOC->BSRR |=(1<<13); //set pin 13 high
}*/



void ADC1_2_IRQHandler(void)
{
	if(ADC1->SR & ADC_SR_EOC)//end of coversion
	{
		val = ADC1->DR;
		now++;
	}
}


int main()
 {
	gpio_init();
	timer2_init();
	tim3_init();							//timer for delay
	uart1_init();							
	adc_config();
	GPIOC->BSRR |=(1<<13); 						//set pin 13 high on bord led
	stopTime;
	stopADC;
	 while(1)
	{
		while(!(GPIOC->IDR & (1<<15)));
		delay_ms(1500);
		if((GPIOC->IDR & (1<<15)))
		{
		generateSOS;
		}
		else
		{
			startTime();
			while(flag)
			{
				GPIOC->BSRR &= ~(1<<13); 						//reset pin 13 on bord led
				startADC;
				delay_ms(1);
			}
			while(now)
			{
				now--;
				tx_char(*data++);
			}		
		}
	}
}




