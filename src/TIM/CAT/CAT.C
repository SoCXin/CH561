/********************************** (C) COPYRIGHT *******************************
* File Name          : CAT.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/3/8
* Description        : CH561 CAT DEMO
*                     
*                      (1)������0��������Ϣ,115200bps;
*                      (2)��CH561��Ƭ��C���ԵĲ���ʾ������,
*                           ��������ʾ��ʱ��0�����ܣ���ʱ��1��2��3���ƣ���֮ͬ��������������֮ͬ�����ڳ���ע����ָ��;
*                           ��������������PB1���ŵ�һ���ߵ�ƽ���ߵ͵�ƽ��
*                           �ߵ͵�ƽ���ʱ����8������16��ʱ�����ڣ�����ѡ�񣩣��ʱ����Pclk*(2^27),PclkΪϵͳʱ��.
*******************************************************************************/



/******************************************************************************/
/* ͷ�ļ����� */
#include <stdio.h>
#include <string.h>
#include "CH561SFR.H"
#include "SYSFREQ.H"

/******************************************************************************/
/* �������� */
UINT32 Capture;   /* ���沶���ֵ */
UINT8 FLAG=0;     /* FLAGΪ1ʱ��ʾ�б��ش��� */

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
    UINT8 i;
    if(R8_INT_FLAG_0&RB_IF_TMR0){
        i= R8_TMR0_INT_FLAG;
        if(i&RB_TMR_IF_DATA_ACT){                                               /* ��ƽ�仯�ж� */
            R8_TMR0_INT_FLAG = RB_TMR_IF_DATA_ACT | RB_TMR_IF_FIFO_HF |
                               RB_TMR_IF_FIFO_OV;                               /* �����Ӧ�жϱ�־ */
            R8_INT_FLAG_0 |= RB_IF_TMR0;                                        /* �����Ӧ�жϱ�־ */
            Capture=R32_TMR0_FIFO;                                              /* ���񵽵�ʱ��ֵ�������� */
            FLAG = 1;   
        }                
        if(i&RB_TMR_IF_CYC_END){                                                /* ����ʱ�ж� */
            R8_TMR0_INT_FLAG=0xff;                                              /* �����Ӧ�жϱ�־ */
            R8_INT_FLAG_0 |= RB_IF_TMR0;                                        /* �����Ӧ�жϱ�־ */
            R32_TMR0_FIFO;
            PRINT("overtime\n") ;
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
    /* ������е��жϱ�־ */    
    R8_TMR0_INT_FLAG=RB_TMR_IF_DATA_ACT|RB_TMR_IE_CYC_END|RB_TMR_IF_FIFO_HF|RB_TMR_IF_DMA_END|RB_TMR_IF_FIFO_OV|RB_TMR_IF_DMA_ERR;                                                                   
    R8_TMR0_INTER_EN=RB_TMR_IE_DATA_ACT|RB_TMR_IE_CYC_END;                      /* ����ģʽ���е�ƽ�仯�ж�ʹ�ܣ�����ģʽ�²���ʱ�ж�ʹ�� */
    R8_INT_EN_IRQ_0=RB_IE_IRQ_TMR0;                                             /* ������ʱ��0����жϲ��� */
    R8_INT_EN_IRQ_GLOB= RB_IE_IRQ_GLOB;                                         /* ֻ����IRQȫ���ж� */
}

/*******************************************************************************
* Function Name  : Timer0_Init
* Description    : ��ʱ��0��ʼ��
* Input          : max_time--  Ϊ��������ʱʱ�����ã���ʱʱ��time = Pclk*max_time, PclkΪϵͳʱ��
*                  min_time--  ��С��������:0:16��ʱ������,1:8��ʱ������
*                  catch_edge--����ģʽ�±��ش�����ʽ:    00:���ô���,01:�����κα��ش�����ʱ��,10:�����½��ص��½���֮��ʱ��,
*                              11:���������ص�������֮��ʱ��
* Output         : None
* Return         : None
*******************************************************************************/

void Timer0_Init( UINT32 max_time ,UINT8 min_time,UINT8 capture_edge  ) 
{
    R32_PB_PU|=0x00000002;                                                      /* ��������    */
    R32_PB_DIR&=0xfffffffd;                                                     /* PB1Ϊ���� */
    R8_TMR0_CTRL_MOD=(RB_TMR_CATCH_EDGE&(capture_edge<<6))|RB_TMR_MODE_IN|(RB_TMR_CAT_WIDTH&(min_time<<3));
                                                                                /* ���ش�����ʽ,����ģʽ,8ʱ�����ڲ��� */
    R32_TMR0_CNT_END=max_time;                                                  /* ��������ʱʱ�� */
    R8_TMR0_CTRL_MOD|=(RB_TMR_COUNT_EN|RB_TMR_OUT_EN);                          /* ������ʱ�� */
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
* Function Name  : UART1_SendByte
* Description    : ͨ�����ڷ���һ�ֽ�����
* Input          : dat--Ҫ���͵�һ�ֽ�����
* Output         : None
* Return         : None
*******************************************************************************/

void UART1_SendByte( UINT8 dat )   
{        
    R8_UART1_THR  = dat;
    while( ( R8_UART1_LSR & RB_LSR_TX_ALL_EMP ) == 0 );                         /* �ȴ����ݷ��� */         
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
    PRINT( "Start Capture demo\xd\xa" );
    IRQ_Init( );                                                                /* �жϳ�ʼ�� */
    Timer0_Init( 0x0fffffff,1,1);                                               /* ���ʱʱ�䣬��С��������Ϊ8��ʱ�ӣ�������ش��� */
    while ( 1 ){
        if(FLAG){
            FLAG = 0;
            if(Capture&(1<<27))                                                 /* �Ĵ���R32_TMR_FIFOֵ���λΪ1��ʾ�����ƽΪ�ߵ�ƽ,��֮Ϊ�͵�ƽ */
                PRINT("Hig=%ld\n",(Capture&0x07ffffff));
            else
                PRINT("Low=%ld\n",Capture);
        }   
    }
}

/*********************************** endfile **********************************/