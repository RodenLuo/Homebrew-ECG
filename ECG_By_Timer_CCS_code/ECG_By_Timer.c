/*
 * ECG_By_Timer.c
 *
 *  Created on: 2014-5-3
 *      Author: Luo Deng E-mail:luod@mail.sustc.edu.cn
 */
#include  <msp430G2553.h>
#include "Tnokia_5110.h"

void SetClock(void);
void SetTimerA(void);
void SetUART(void);
void LCD_init(void);
char * toArray(int number);

#define RED_LED BIT0

volatile long counter;
volatile long inpeak;
volatile long beatCounter;
const int beatThreshold = 700;

volatile long heartRaw;//Raw����ԭʼ����
volatile long heartC;//C����ת���������
volatile int heartRate;

void main(void)
{
  counter = 1;
  inpeak = 0;
  beatCounter = 0;

  WDTCTL = WDTPW + WDTHOLD;         // Stop watchdog timer
  SetClock();						//����ʱ�ӣ�MCLK��SMCLK
  P1DIR|=RED_LED;					//����RED_LEDΪ�����
  SetTimerA();						//����timerA�Ļ�������
  SetUART();						//����UART�Ļ�������
  LCD_init();						//����LCD�Ļ�������

  _BIS_SR(LPM0_bits+GIE);	//���ǻ�����ԣ��������˼�������� SR = LPM0_bits+GIE��SR��״̬�Ĵ�����status register��
  	  	  	  	  	  	  	  //LPM0��MSP430һ�ֵ͹���ģʽ���ر���һЩ���������UG��GIE: general interrupt enabled
/*
  ��������������߼�����:
  ���ú�����ʱ���źţ�MCLK��SMCLK��
  ����LED�������
  ����TimerA�ģ��ж�ʹ�ܣ���ʱʱ����ʱ���źţ�����ģʽ��
  ��������MSP430�Ĺ���ģʽ��LPM0����ʱCPU��MCLK�رգ�SMCLK������һ�������жϣ�CPU�����ѡ�
  ��дTimerA0�жϳ���
*/
}

void SetClock(void){
	BCSCTL1 = CALBC1_1MHZ; 		       // Set range
	DCOCTL = CALDCO_1MHZ;  		       // Set DCO step + modulation
	//����BCSCTL2���κθ���ʱ��Ĭ��ֵ��0��������MCLK��SMCLKΪDCO�����Ҷ���1��Ƶ
	BCSCTL2 = DIVM_0;
}

void SetTimerA(void){
	TACCTL0|=CCIE;			//ʹ��TimerA���жϣ���ʱ�䣬�����������ӡ�
	TACCR0=2000+8;				//TimerA��CCR0��ֵ����һ����ʱ����ֵ��
	TACTL|=TASSEL_2+MC_1;		//�趨timerA��ʱ���ź�ΪSMCLK�����ҹ���ģʽ��up mode�����Ի�Ҫ�趨SMCLK��ʱ���ź�
}

void SetUART(void){
	//UART register
	P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
	P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK ѡ������Ϊ����ͨѶ��ʱ��
	UCA0BR0 = 104;                            // 1MHz 9600 UCA0 Baud Rate�Ͱ�λ
	UCA0BR1 = 0;                              // 1MHz 9600 UCA0 Baud Rate�߰�λ
	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
	//���涼��Ϊ�����ò����ʣ����һ���С�����ֵ����йأ�ÿ��������û��
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}

void LCD_init(void){
	Lcd_Init();
	Lcd_Clear();
	Lcd_Set_XY(0,0);
	Lcd_Write_String(0,0,"Rw_ECG");
	Lcd_Write_String(0,1,"Hello! Your   heart rate is: ");
	Lcd_Write_String(0,4,"---");
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
	ADC10CTL1 = INCH_7 + ADC10DIV_0;  // ѡP1.7��Ϊ����
	/*
	 * ����ADC10������һЩ��������
	 * INCH_10: Input channel select, Selects Channel 10
	 * ADC10DIV_0: ADC10 Clock Divider Select 0
	 * ������û����ʾ����ADC10��ʱ�ӣ���ʵ���ϣ���ΪĬ��0��ʱ����������ʱ���ź�ADC10OSC�������������˵ģ�Ҫ�鵽����Ƶ�ʣ�
	 */
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + REF2_5V;
	/*
	 * ����ADC10������һЩ��������
	 * select REF: VR+ = VREF+ and VR- = AVSS
	 * ADC10SHT_3: ADC10 Sample and Hold Time, 64 x ADC10CLKs�����ֵ��ζ��ʲô��
	 * REFON: ADC10 Reference on�� ���������ã������ǿ�����������Ҫ30us�����Ժ��������һ�����5��cycles
	 * ADC10ON: ADC10 ON������ADC10
	 */
	_delay_cycles(5);                  // Wait for ADC Ref to settle
	ADC10CTL0 |= ENC + ADC10SC;        // Sampling and conversion start����ʼ������ת���������ر���⣬�ǿ�ʼ�����˵���˼��

    _delay_cycles(100);//����ͣ����֮�����ڸ�ʲô���ǲ���ADC10�����delay��ʱ������

    ADC10CTL0 &= ~ENC;//ֹͣת��
    ADC10CTL0 &= ~(REFON + ADC10ON);//�ر�REF��ADC10
    heartRaw = ADC10MEM;//��ADC10MEM����ȡֵ
    //heartC = (unsigned char)(((heartRaw - 673)*423)>>10); //ת���������¶ȣ�����Ǹ���������λ���㣬û��ϸ�顣
    heartC = heartRaw;
    //UART send
    while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
    	  UCA0TXBUF=heartC;

    //����R��ʱ������
    if (heartC > beatThreshold){
    	P1OUT = RED_LED; 			    	   // red LED on
    }
    else if(heartC < beatThreshold){
        P1OUT = 0; 		                   // RED LED off
    }

    //����heart rate�������Ƿ�inpeak���ۼ���������
    if(heartC > beatThreshold && inpeak == 0){
    	beatCounter++;
    	inpeak = 1;
    	if(beatCounter > 5){
    		heartRate = 60*500*(beatCounter-1)/counter;
    		Lcd_Write_Int(0,4,heartRate);
    	}
    	else{
    		Lcd_Write_String(0,4,"---");
    	}
    }
    //�˳�inpeak
    else if(heartC < beatThreshold){
    	inpeak = 0;
    }

    //10��֮��ȫ�����㣬���¼���
    if (counter == 5000){
    	counter = 0;
    	inpeak = 0;
    	beatCounter = 0;
    }
    counter++;
}
