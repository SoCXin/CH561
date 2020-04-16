/********************************** (C) COPYRIGHT *******************************
* File Name          : COUNT.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/3/8
* Description        : CH561 COUNT DEMO
*                     
*                      (1)������0��������Ϣ,115200bps;
*                      (2)��CH561��Ƭ��C���Եļ���ʾ������,
*                           ��������ʾ��ʱ��3�������ܣ�
*                           ����PB6���ŵ��źż�������������ʱ����8������16��ʱ������.
*******************************************************************************/



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

/*******************************************************************************
* Function Name  : IRQ_Handler
* Description    : IRQ�жϺ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

__irq void IRQ_Handler( void )
{
    UINT8 i;

   PRINT( "irq#  " );
   if(R8_INT_FLAG_0&RB_IF_TMR3){
       i= R8_TMR3_INT_FLAG;
       if(i&RB_TMR_IF_CYC_END){                                                /* ����ʱ�ж� */
            R8_TMR3_INT_FLAG=0xff;                                             /* �����Ӧ�жϱ�־ */
            R8_INT_FLAG_0 |= RB_IF_TMR3;                                       /* �����Ӧ�жϱ�־ */
            R32_TMR3_FIFO;
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
* Function Name  : IRQ_Init
* Description    : IRQ�жϳ�ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void IRQ_Init( void )
{    
    R8_TMR0_INT_FLAG   = RB_TMR_IF_DATA_ACT|RB_TMR_IE_CYC_END|RB_TMR_IF_FIFO_HF|
                         RB_TMR_IF_DMA_END|RB_TMR_IF_FIFO_OV|RB_TMR_IF_DMA_ERR; /* ������е��жϱ�־ */    
    R8_TMR3_INTER_EN   = RB_TMR_IE_CYC_END;                                     /* ����������ֵ�ж�ʹ�� */
    R8_INT_EN_IRQ_0    = RB_IE_IRQ_TMR3;                                        /* ������ʱ��3����жϲ��� */
    R8_INT_EN_IRQ_GLOB = RB_IE_IRQ_GLOB;                                        /* ֻ����IRQȫ���ж� */
}

/*******************************************************************************
* Function Name  : Timer3_Init
* Description    : ��ʱ��3��ʼ��
* Input          : count_end-- ������ֵ
*                  min_time--  ��С��������:0:16��ʱ������,1:8��ʱ������
*                  catch_edge--����ģʽ����  
* Output         : None
* Return         : None
*******************************************************************************/

void Timer3_Init( UINT32 count_end ,UINT8 min_time,UINT8 capture_edge  ) 
{
    R32_PB_PU |= CAT3;                                                          /* ��������    */
    R32_PB_DIR &= ~CAT3;                                                    
    R8_TMR3_CTRL_MOD = RB_TMR_CATCH_EDGE&(capture_edge<<6) | 
                       (RB_TMR_CAT_WIDTH&(min_time<<3)) | RB_TMR3_MODE_COUNT;    
    R32_TMR3_CNT_END = count_end;                                               /* ��������ʱʱ�� */
    R8_TMR3_CTRL_MOD |= RB_TMR_COUNT_EN ;                                       /* ����������������ʱ�� */
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
    Delay_ms( 100 );
    LED_OUT_INACT( );
    mInitSTDIO( );                                                              /* Ϊ���ü����ͨ�����ڼ����ʾ���� */
    PRINT( "TIM3 count demo\xd\xa" );
    IRQ_Init( );                                                                /* �жϳ�ʼ�� */
    Timer3_Init( 10,1,1);                                               
    while ( 1 ){
        PRINT( "COUNT =%-8x",R32_TMR3_COUNT );
    }
}

/*********************************** endfile **********************************/