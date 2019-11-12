#ifndef F_CPU
#define F_CPU 16000000ul
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint16_t valor = 0;
uint8_t pino = 0;

// Ao receber um sinal no pino INT1 (pino 3) o valor do PWM (pino 10)
//  eh repassado para a saida (pino 13)
ISR(INT1_vect)
{
  PORTB = (PINB << 3);
}

// Ler o valor dos potenciometros
ISR(ADC_vect)
{
  // Ler o resultado da conversao AD
  valor = (ADCL >> 6);
  valor |= (ADCH << 2);

  switch(pino)
  {
    // Pino 0: Periodo
    case 0:
      OCR1A = (uint16_t)((double)(valor)/1023 * 0xffff);
      ADMUX |= (1 << MUX0);
      break;

    // Pino 1: Duty Cycle
    case 1:
      OCR1B = (uint16_t)((double)(valor)/1023 * OCR1A);
      ADMUX &= ~(1 << MUX0);
      break;
  }
  // Define o prox. pino a ser lido
  if(++pino > 1)
  {
    pino = 0;
  }

  // Inicia outra conversao
  ADCSRA |= (1 << ADSC);
}

int main()
{
  // PORTB5 (pino 13) e PORTB2 (pino 10) sao saidas
  // Todos os outros sao entradas
  DDRB = (1 << PORTB5) | (1 << PORTB2);

  // Interrupcao externa em INT1 (pino 3)
  EICRA = (1 << ISC11) | (1 << ISC10);
  EIMSK = (1 << INT1);

  // ADC (para os potenciometros)
  DIDR0 = (1 << ADC1D) | (1 << ADC0D);
  ADMUX = (1 << REFS0) | (1 << ADLAR);
  ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);

  // PWM (para o controle da carga)
  TCCR1A = (1 << COM1B1) | (1 << COM1B0) | (1 << WGM11) | (1 << WGM10);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS12) | (1 << CS10);

  // Frequencia do PWM
  OCR1A = 15624;
  OCR1B = 11718;

  // Habilitar interrupçoes
  sei();

  // Iniciar Conversao AD
  ADCSRA |= (1 << ADSC);
  
  while(1);
}
