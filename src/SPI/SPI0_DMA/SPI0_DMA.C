/********************************** (C) COPYRIGHT *******************************
* File Name          : SPI0_DMA.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/3/8
* Description        : CH561 SPI0_DMA DEMO
*                   
*                      (1)������0��������Ϣ,115200bps;
*                      (2)����������ҪΪDMA���䣬����DMA�����ӳ���.
*******************************************************************************/



/******************************************************************************/
/* ͷ�ļ����� */
#include <stdio.h>
#include <string.h>
#include "CH561SFR.H"
#include "SYSFREQ.H"

/******************************************************************************/
/* �������� */
UINT8  upper[1024];

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
    if (R32_INT_FLAG & RB_IF_SPI0 ){                                           /* ��ѯ�Ƿ�ΪSPI0�ж� */
        if( R8_SPI0_INT_FLAG & RB_SPI_IF_DMA_END ){
            PRINT("R32_INT_FLAG    = %8lX \n", R32_INT_FLAG);   
            PRINT("R8_SPI0_INT_FLAG= %8lX \n", R8_SPI0_INT_FLAG);
            PRINT("DMA_BEG= %8lX \n", R16_SPI0_DMA_BEG);
            PRINT("DMA_END= %8lX \n", R16_SPI0_DMA_END);
            PRINT("DMA_NOW= %8lX \n", R16_SPI0_DMA_NOW);
            PRINT("DMA END \n"); 
            R16_SPI0_DMA_BEG = (UINT32)upper;                                   /* DMA���ã����ݿ�ʼ��ַ */
        }
        R8_SPI0_INT_FLAG = 0xff;                                                /* ���жϱ�־ */
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
* Function Name  : IRQInit
* Description    : �ж�ʹ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void IRQInit( void )
{
    R8_SPI0_INT_FLAG = 0xff;                                                    /* �жϱ�־��д1���� */ 
    R8_SPI0_INTER_EN |= RB_SPI_IE_DMA_END;                                      /* DMA�����ж� */
    R8_INT_EN_IRQ_0 |= RB_IE_IRQ_SPI0;                                          /* ��SPI0�ж� */
    R8_INT_EN_IRQ_GLOB |= RB_IE_IRQ_GLOB;                                       /* ��ȫ���ж� */
}

/*******************************************************************************
* Function Name  : SPI0_MasterInit
* Description    : SPI����ģʽ��ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void SPI0_MasterInit ( void )
{
    R8_SPI0_CTRL_MOD &= ~ RB_SPI_MODE_SLAVE;
    R8_SPI0_CTRL_MOD = RB_SPI_MOSI_OE|RB_SPI_SCK_OE;                            /* MOSI��SCK0���ʹ�� */
    R8_SPI0_CLOCK_DIV = 0x0a;                                                   /* 10��Ƶ��100/10=10M */
    R32_PB_DIR |= (MOSI | SCK0 | SCS );                                         /* MOSI(PB14),SCK0(PA13),SCS(PB12)Ϊ��� */
    R32_PB_PU  |=  SCS ;                         
    R32_PB_OUT |=  SCS ; 
}

/*******************************************************************************
* Function Name  : SPI0_DMASend
* Description    : DMA����   
* Input          : *BEG -���ݿ�ʼ��ַ
*                  *END -���ݽ�����ַ
*                  LEN  -���ݳ���
* Output         : None
* Return         : None
*******************************************************************************/

void SPI0_DMASend (UINT8 *BEG,UINT8 *END,UINT16 LEN)
{
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;                                       /*  ���÷���Ϊ��� */
    R16_SPI0_DMA_BEG = (UINT32)BEG;                                             /* DMA���ã����ݿ�ʼ��ַ */
    R16_SPI0_DMA_END = (UINT32)END;                                             /* ���ݽ�����ַ */
    R16_SPI0_TOTAL_CNT = LEN;                                                   /* ���ݳ��� */
    PRINT("R16_SPI0_DMA_BEG= %8lX \n", R16_SPI0_DMA_BEG);                       /* �����ַ��Ϣ */
    PRINT("R16_SPI0_DMA_END= %8lX \n", R16_SPI0_DMA_END);                       /* ������ַ */
    PRINT("R16_SPI0_DMA_NOW= %8lX \n", R16_SPI0_DMA_NOW);                       /* ��ǰ��ַ */
    R8_SPI0_CTRL_DMA |= RB_SPI_DMA_ENABLE;                                      /* DMAʹ�� */
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
    UINT16 i;

    LED_OUT_INIT( );
    LED_OUT_ACT( );                                                             /* ������LED��һ����ʾ���� */
    Delay_ms( 100 );
    LED_OUT_INACT( );
    mInitSTDIO( );                                                              /* ����0��ʼ�� */
    PRINT("start:\n");
    for(i=0;i<1024;i++) upper[i] = i;
    SPI0_MasterInit( );                                                         /* SPI����ģʽ��ʼ�� */
    IRQInit( );
    R32_PB_CLR |= SCS;                                                          /* Ƭѡʹ�� */
    SPI0_DMASend(upper,&upper[1024],1024);                                      /* DMA���� */
    while(1);
}

/*********************************** endfile **********************************/