void uart1_init(void);
void tx_string(const char *);
void tx_number(uint32_t x);
void tx_char(const char x);

void tx_number(uint32_t x)
{
  char value[10]; //a temp array to hold results of conversion
  int i = 0; //loop index
  
  do
  {
    value[i++] = (char)(x % 10) + '0'; //convert integer to character
    x /= 10;
  } while(x);
  
  while(i) //send data
  {
    tx_char(value[--i]); 
  }
}

void tx_char(const char x)
{
		USART1->DR = x;
		while(!(USART1->SR & (1<<7)));//transmit complete
}

void tx_string(const char *x)
{
	while(*x != '\0')
	{
		USART1->DR = *x++;
		while(!(USART1->SR & (1<<7)));//transmit complete
	}
}

void uart1_init()
{
	RCC->APB2ENR |=(1<<0)|(1<<2)|(1<<14);//0 bit for AFIO , 2 //for IOPA ,14 for USARTEN1 clock
	GPIOA->CRH |= (3<<4)|(1<<7);//alternate function mode //output 50hzspeed
	GPIOA->CRH &= ~(1<<6);//conf as AF output push pull
	USART1->BRR = 0x1d4C ;//baud rate 9600
	USART1->CR1 |= (1<<3)|(1<<13);// uart enable + transmit 
}

