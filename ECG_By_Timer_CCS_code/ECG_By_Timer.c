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

volatile long heartRaw;//Raw就是原始数据
volatile long heartC;//C就是转换后的数据
volatile int heartRate;

void main(void)
{
  counter = 1;
  inpeak = 0;
  beatCounter = 0;

  WDTCTL = WDTPW + WDTHOLD;         // Stop watchdog timer
  SetClock();						//设置时钟，MCLK和SMCLK
  P1DIR|=RED_LED;					//设置RED_LED为输出口
  SetTimerA();						//设置timerA的基本配置
  SetUART();						//设置UART的基本配置
  LCD_init();						//设置LCD的基本配置

  _BIS_SR(LPM0_bits+GIE);	//这是汇编语言，话语的意思可以理解成 SR = LPM0_bits+GIE。SR是状态寄存器，status register。
  	  	  	  	  	  	  	  //LPM0是MSP430一种低功耗模式，关闭了一些东西，详查UG。GIE: general interrupt enabled
/*
  所以整个程序的逻辑就是:
  设置好两个时钟信号，MCLK，SMCLK；
  开启LED的输出；
  设置TimerA的，中断使能，定时时长，时钟信号，工作模式；
  设置整个MSP430的工作模式：LPM0，此时CPU和MCLK关闭，SMCLK开启。一旦进入中断，CPU被唤醒。
  编写TimerA0中断程序
*/
}

void SetClock(void){
	BCSCTL1 = CALBC1_1MHZ; 		       // Set range
	DCOCTL = CALDCO_1MHZ;  		       // Set DCO step + modulation
	//不对BCSCTL2做任何更改时，默认值是0，会设置MCLK和SMCLK为DCO，并且都是1分频
	BCSCTL2 = DIVM_0;
}

void SetTimerA(void){
	TACCTL0|=CCIE;			//使能TimerA的中断，到时间，他就竖起旗子。
	TACCR0=2000+8;				//TimerA的CCR0的值。第一个定时器的值。
	TACTL|=TASSEL_2+MC_1;		//设定timerA的时钟信号为SMCLK，并且工作模式是up mode，所以还要设定SMCLK的时钟信号
}

void SetUART(void){
	//UART register
	P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
	P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK 选中它作为串口通讯的时钟
	UCA0BR0 = 104;                            // 1MHz 9600 UCA0 Baud Rate低八位
	UCA0BR1 = 0;                              // 1MHz 9600 UCA0 Baud Rate高八位
	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
	//上面都是为了设置波特率，最后一句跟小数部分调制有关，每句具体读法没懂
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
	ADC10CTL1 = INCH_7 + ADC10DIV_0;  // 选P1.7作为输入
	/*
	 * 设置ADC10工作的一些基本参数
	 * INCH_10: Input channel select, Selects Channel 10
	 * ADC10DIV_0: ADC10 Clock Divider Select 0
	 * 这里面没有显示设置ADC10的时钟，但实际上，因为默认0的时候是其内置时钟信号ADC10OSC，所以是设置了的，要查到它的频率：
	 */
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + REF2_5V;
	/*
	 * 设置ADC10采样的一些基本参数
	 * select REF: VR+ = VREF+ and VR- = AVSS
	 * ADC10SHT_3: ADC10 Sample and Hold Time, 64 x ADC10CLKs，这个值意味着什么？
	 * REFON: ADC10 Reference on， 上面是设置，这里是开启，开启需要30us，所以后面紧接着一句等了5个cycles
	 * ADC10ON: ADC10 ON，开启ADC10
	 */
	_delay_cycles(5);                  // Wait for ADC Ref to settle
	ADC10CTL0 |= ENC + ADC10SC;        // Sampling and conversion start，开始采样并转换？不是特别理解，是开始工作了的意思吗？

    _delay_cycles(100);//这里停下来之后是在干什么？是不是ADC10在这个delay的时候工作？

    ADC10CTL0 &= ~ENC;//停止转换
    ADC10CTL0 &= ~(REFON + ADC10ON);//关闭REF和ADC10
    heartRaw = ADC10MEM;//从ADC10MEM里面取值
    //heartC = (unsigned char)(((heartRaw - 673)*423)>>10); //转换成摄氏温度，最后那个运算是移位运算，没仔细查。
    heartC = heartRaw;
    //UART send
    while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
    	  UCA0TXBUF=heartC;

    //到达R波时，亮灯
    if (heartC > beatThreshold){
    	P1OUT = RED_LED; 			    	   // red LED on
    }
    else if(heartC < beatThreshold){
        P1OUT = 0; 		                   // RED LED off
    }

    //计算heart rate，利用是否inpeak来累加心跳次数
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
    //退出inpeak
    else if(heartC < beatThreshold){
    	inpeak = 0;
    }

    //10秒之后全部归零，重新计算
    if (counter == 5000){
    	counter = 0;
    	inpeak = 0;
    	beatCounter = 0;
    }
    counter++;
}
