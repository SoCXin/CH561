/********************************** (C) COPYRIGHT *******************************
* File Name          : WDOG.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/3/8
* Description        : CH561 WDOG
*                     
*                      (1)������0��������Ϣ,115200bps;
*                      (2)����������ʾ���Ź�����
*******************************************************************************/


#define           DEBUG            1
/******************************************************************************/
/* ͷ�ļ����� */
#include <stdio.h>										  
#include <string.h>
#include "CH561SFR.H"
#include "SYSFREQ.H"

/******************************************************************************/
/* �������� */


/* ����һ��LED���ڼ����ʾ����Ľ���,�͵�ƽLED�� */
#define LED                     1<<3

#define LED_OUT_INIT(  )     { R32_PB_OUT |= LED; R32_PB_DIR |= LED; }          /* LED �ߵ�ƽΪ������� */
#define LED_OUT_ACT(  )      { R32_PB_CLR |= LED; }                             /* LED �͵�ƽ����LED��ʾ */
#define LED_OUT_INACT(  )    { R32_PB_OUT |= LED; }                             /* LED �ߵ�ƽ�ر�LED��ʾ */

UINT32  TIM_COUNT;

/*******************************************************************************
* Function Name  : IRQ_Handler
* Description    : IRQ�жϺ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
__irq void IRQ_Handler( void )
{
    if(R32_INT_FLAG & RB_IF_TMR0){                                              /* ��ʱ���ж� */
        R8_TMR0_INT_FLAG |= 0xff;                                               /* �����ʱ���жϱ�־ */
        TIM_COUNT ++;
        if( TIM_COUNT >= 1000 ){                  // time=1000*10 ms=10s
            R8_SAFE_ACCESS_SIG = 0x57 ;                                         /* unlock step 1 */
            R8_SAFE_ACCESS_SIG = 0xA8 ;                                         /* unlock step 2 */                     
            R8_GLOB_RST_CFG |= (0x40 | RB_GLOB_FORCE_RST);                      /* bit7:6 must write 0:1 */
            R8_SAFE_ACCESS_SIG = 0;                                             /* ��������ֹ��д */
        }
    }
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
* Function Name  : TIM_Time0Init
* Description    : ��ʱ��0��ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM_Time0Init( void )
{
    R8_TMR0_CTRL_MOD  = RB_TMR_ALL_CLEAR;
    R32_TMR0_COUNT    = 0x00000000; 
    R32_TMR0_CNT_END  = 0x186a0 * 10;                                           /* ����Ϊ10MS��ʱ */
    R8_TMR0_INTER_EN |= RB_TMR_IE_CYC_END;
    R8_TMR0_CTRL_MOD  = RB_TMR_COUNT_EN;
    R8_INT_EN_IRQ_0  |= RB_IE_IRQ_TMR0;                                         /* ����TIM0�ж� */
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
    Delay_ms( 100 );
    LED_OUT_INACT( );
    R32_PB_DIR &=  ~(1<<7);                                                     /* PB7�������� */                                
    R32_PB_PU  |=  (1<<7);                   
    mInitSTDIO( );                                                              /* Ϊ���ü����ͨ�����ڼ����ʾ���� */ 
    PRINT( "Start:\r\n" );
    TIM_Time0Init( );
    R8_INT_EN_IRQ_GLOB |= RB_IE_IRQ_GLOB;                                       /* ����IRQȫ���ж� */
    R8_SAFE_ACCESS_SIG = 0x57 ;                                                 /* unlock step 1 */
    R8_SAFE_ACCESS_SIG = 0xA8 ;                                                 /* unlock step 2 */                     
    R8_GLOB_RST_CFG |= (0x40 | RB_GLOB_WDOG_EN);                                /* bit7:6 must write 0:1 �������Ź���ʱ��=2^23=8388608����ʱ������*/
    R8_SAFE_ACCESS_SIG = 0;                                                     /* ��������ֹ��д */
    TIM_COUNT = 0;                                                            
    while(1){
        R8_WDOG_CLEAR = 0;
        if( (R32_PB_PIN & (1<<7) )== 0 ){                                       /* ��ʾ���Ź����� */
            while(1);
        }
//        TIM_COUNT = 0;                                                        /* ������ñ���ÿ60s��λһ�� */
    }
}

/*********************************** endfile **********************************/