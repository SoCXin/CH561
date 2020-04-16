/********************************** (C) COPYRIGHT *******************************
* File Name          : GPIOA_IO.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/3/8
* Description        : CH561 GPIOA_IO DEMO
*                        
*                      (1)������0��������Ϣ,115200bps;
*                      (2)����ͨI/O�ڲ���,I/O�������������á��������á������������.
*******************************************************************************/



/******************************************************************************/
/* ͷ�ļ����� */
#include <stdio.h>
#include <string.h>
#include "CH561SFR.H"
#include "SYSFREQ.H"

/* ����һ��LED���ڼ����ʾ����Ľ���,�͵�ƽLED�� */
#define LED                     1<<3

#define LED_OUT_INIT(  )     { R32_PB_OUT |= LED; R32_PB_DIR |= LED; }         /* LED �ߵ�ƽΪ������� */
#define LED_OUT_ACT(  )      { R32_PB_CLR |= LED; }                            /* LED �͵�ƽ����LED��ʾ */
#define LED_OUT_INACT(  )    { R32_PB_OUT |= LED; }                            /* LED �ߵ�ƽ�ر�LED��ʾ */

/*******************************************************************************
* Function Name  : IRQ_Handler
* Description    : IRQ�жϺ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

__irq void IRQ_Handler( void )   
{
    while(1);
} 

/*******************************************************************************
* Function Name  : FIQ_Handler
* Description    : FIQ�жϺ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

__irq void FIQ_Handler( void )
{
    while(1);
}

/*******************************************************************************
* Function Name  : mInitSTDIO
* Description    : Ϊprintf��getkey���������ʼ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void mInitSTDIO( void )
{
    UINT32    x, x2;

    x = 10 * FREQ_SYS * 2 / 16 / 115200;                                        /* 115200bps */
    x2 = x % 10;
    x /= 10;
    if ( x2 >= 5 ) x ++;                                                        /* �������� */
    R8_UART0_LCR = 0x80;                                                        /* DLABλ��1 */
    R8_UART0_DIV = 1;                                                           /* Ԥ��Ƶ */
    R8_UART0_DLM = x>>8;
    R8_UART0_DLL = x&0xff;

    R8_UART0_LCR = RB_LCR_WORD_SZ ;                                             /* �����ֽڳ���Ϊ8 */
    R8_UART0_FCR = RB_FCR_FIFO_TRIG|RB_FCR_TX_FIFO_CLR|RB_FCR_RX_FIFO_CLR |    
                   RB_FCR_FIFO_EN ;                                             /* ����FIFO������Ϊ14���巢�ͺͽ���FIFO��FIFOʹ�� */
    R8_UART0_IER = RB_IER_TXD_EN;                                               /* TXD enable */
    R32_PB_SMT |= RXD0|TXD0;                                                    /* RXD0 schmitt input, TXD0 slow rate */
    R32_PB_PD &= ~ RXD0;                                                        /* disable pulldown for RXD0, keep pullup */
    R32_PB_DIR |= TXD0;                                                         /* TXD0 output enable */
}

/*******************************************************************************
* Function Name  : fputc
* Description    : ͨ��������������Ϣ
* Input          : c-- writes the character specified by c 
*                  *f--the output stream pointed to by *f
* Output         : None
* Return         : None
*******************************************************************************/

int fputc( int c, FILE *f )
{
    R8_UART0_THR = c;                                                           /* �������� */
    while( ( R8_UART0_LSR & RB_LSR_TX_FIFO_EMP ) == 0 );                        /* �ȴ����ݷ��� */
    return( c );
}

/*******************************************************************************
* Function Name  : main
* Description    : ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

int main( void )
{
    LED_OUT_INIT( );
    LED_OUT_ACT( );                                                             /* ������LED��һ����ʾ���� */
    Delay_ms(100);                                                              
    LED_OUT_INACT( );
    mInitSTDIO( );                                                              /* ����0��ʼ�� */
    PRINT("GPIOA DEMO....\n");                                                  /* ��س������� */
    R32_PA_PU  |= 0xffffffff;                                                   /* GPIO A ��������,��1��ʾ����,��0��ʾ���� */  
    R32_PA_DIR |= 0xffffffff;                                                   /* GPIO A ��������Ϊ��� , direction: 0=in, 1=out */
    while(1){
        R32_PA_OUT = 0x00000000;                                                /* GPIO A ����͵�ƽ */
        Delay_ms(200);
        R32_PA_OUT = 0xffffffff;                                                /* GPIO A ����ߵ�ƽ */
        Delay_ms(300);
    }    
}

/*********************************** endfile **********************************/