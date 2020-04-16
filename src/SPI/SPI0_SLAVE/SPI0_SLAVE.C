/********************************** (C) COPYRIGHT *******************************
* File Name          : SPI0_SLAVE.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/3/8
* Description        : CH561 SPI0_SLAVE DEMO
*                      
*                      (1)������0��������Ϣ,115200bps;
*                      (2)����������Ҫ��SPI���豸�������ݣ��������ͺͽ����ӳ���.
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
* Function Name  : SPI0_SlaveInit
* Description    : SPI�豸ģʽ��ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void SPI0_SlaveInit( void )
{
    R8_SPI0_CTRL_MOD |= ( RB_SPI_MISO_OE | RB_SPI_MODE_SLAVE );                 /* ģʽ���üĴ���,�豸ģʽ */
    R8_SPI0_CTRL_MOD |= ( RB_SPI_FIFO_DIR | RB_SPI_ALL_CLEAR );                 /* ���FIFO,FIFO����Ĭ������Ϊ����*/
    R8_SPI0_CTRL_MOD &= ~RB_SPI_ALL_CLEAR ;                                    
    R8_SPI1_CTRL_DMA =  0x00;                                                   /* ������DMA��ʽ */
//    R32_PB_DIR |= MISO;                                                       /* MISO output enable,����ж���ӻ��豸������һ�𣬴����Ų�����Ϊ��� */
}

/*******************************************************************************
* Function Name  : SPI0_SlaveRecv
* Description    : ����һ�ֽ�����  
* Input          : None
* Output         : None
* Return         : R8_SPI0_FIFO -���յ�����
*******************************************************************************/

UINT8 SPI0_SlaveRecv( void )
{
    while( R8_SPI0_FIFO_COUNT == 0 );                                           /* FIFO���ݳ��ȼĴ�����FIFO������Ϊ32���ֽ�,��Ϊ0˵�������� */
    return R8_SPI0_FIFO;                                                       
}

/*******************************************************************************
* Function Name  : SPI0_SlaveTrans
* Description    : ����һ�ֽ�����  
* Input          : data -���������� 
* Output         : None
* Return         : None
*******************************************************************************/

void SPI0_SlaveTrans( UINT8 data )
{
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;                                       /* FIFO��������Ϊ�����׼���������� */
    R16_SPI0_TOTAL_CNT = 0x01;                                                  /* �������ݳ��� */
    R8_SPI0_FIFO = data;                    
    while( R8_SPI0_FIFO_COUNT != 0 );                                           /* �ȴ�������� */
    R8_SPI0_CTRL_MOD |= RB_SPI_FIFO_DIR;                                        /* FIFO��������Ϊ���룬׼���������� */
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
    UINT8    a;

    LED_OUT_INIT( );
    LED_OUT_ACT( );                                                             /* ������LED��һ����ʾ���� */
    Delay_ms( 100 );
    LED_OUT_INACT( );
    mInitSTDIO( );                                                              /* Ϊ���ü����ͨ�����ڼ����ʾ���� */ 
    PRINT("Start SPI0 Slave:\n");
    SPI0_SlaveInit( );                                                          /* SPI�豸ģʽ��ʼ�� */
    while(1){
        a = SPI0_SlaveRecv();
        SPI0_SlaveTrans( a );
    }      
}

/*********************************** endfile **********************************/