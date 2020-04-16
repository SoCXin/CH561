/********************************** (C) COPYRIGHT *******************************
* File Name          : SPI0_HOST.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/3/8
* Description        : CH561 SPI0_HOST DEMO
*                     
*                      (1)������0��������Ϣ,115200bps;
*                      (2)��SPI0�������ӳ���,��ѯ��ʽ���ͺͽ�������.
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
UINT8 SPI_BUF[1024];

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
* Function Name  : SPI_MASTER_INIT
* Description    : SPI����ģʽ��ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void SPI_MASTER_INIT ( void )
{
    R8_SPI0_CTRL_MOD &= ~ RB_SPI_MODE_SLAVE;                                    /* ģʽ���üĴ���,����ģʽ */
    R8_SPI0_CTRL_MOD |= RB_SPI_MOSI_OE|RB_SPI_SCK_OE;                           /* MOSI,SCK output enable */
    R8_SPI0_CTRL_MOD |= ( RB_SPI_FIFO_DIR | RB_SPI_ALL_CLEAR );                 /* ���FIFO,FIFO����Ĭ������Ϊ����*/
    R8_SPI0_CTRL_MOD &= ~RB_SPI_ALL_CLEAR;                                      /* ���FIFO,FIFO����Ĭ������Ϊ����*/
    R8_SPI0_CLOCK_DIV = 0x0a;                                                   /* 10��Ƶ��100/10=10M */
    R32_PB_DIR |= (MOSI | SCK0 | SCS );                                         /* MOSI��SCK0, SCSΪ��� */
    R32_PB_PU  |=  SCS ;
    R8_SPI0_CTRL_DMA = 0x00;                                                    /* ������DMA��ʽ */
}

/*******************************************************************************
* Function Name  : SPI0_MasterTransByte
* Description    : ��������
* Input          : data -���������� 
* Output         : None
* Return         : None
*******************************************************************************/
void SPI0_MasterTransByte( UINT8 data )
{
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;                                       /* ���÷���Ϊ��� */
    R16_SPI0_TOTAL_CNT = 1;                                                     /* ������Ϊ1 */
    R32_SPI0_FIFO = data;                                                       /* ��FIFO��д�����ݣ���FIFO�������ҳ��Ȳ�Ϊ0ʱ����������� */
    while( R8_SPI0_FIFO_COUNT != 0 );                                           /* �ȴ����ݷ������ */
    R8_SPI0_CTRL_MOD |= RB_SPI_FIFO_DIR;                                        /* ����Ĭ�Ϸ���Ϊ���� */
}

// ʹ��SPI���ݻ������Ĵ�����������
void SPI0_MasterTrans1( UINT8 data )
{
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;                                        /* ����λ��0 */
    R8_SPI0_BUFFER = data;                                                       /* д���ݲ��������� */
    while( R8_SPI0_INT_FLAG&RB_SPI_FREE ==0 );                                   /* �ȴ����ݴ������ */
    R8_SPI0_CTRL_MOD |= RB_SPI_FIFO_DIR;                                         /* ����Ĭ�Ϸ���Ϊ���� */
}

// ʹ��FIFO���Ͷ��ֽ�
void SPI0_MasterTrans( UINT16 len, UINT8 *pbuf )
{
    UINT16 sendlen;
    
    sendlen = len;
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;                                       /* �������ݷ���Ϊ��� */
    R16_SPI0_TOTAL_CNT = len;                                                   /* ����Ҫ���͵����ݳ��� */
    while( sendlen ){                                                           /* ����Ҫ���͵����ݳ��� */
        if( R8_SPI0_FIFO_COUNT < SPI0_FIFO_SIZE ){                              /* FIFO��δ�������ݸ���С��FIFO��С����Լ�����FIFOд���� */
            R32_SPI0_FIFO = *pbuf;
            pbuf++;
            sendlen--;
        }
    }
    while( R8_SPI0_FIFO_COUNT != 0 );                                           /* �ȴ�FIFO�е�����ȫ��������� */
    R8_SPI0_CTRL_MOD |= RB_SPI_FIFO_DIR;                                        /* ����Ĭ�Ϸ���Ϊ���� */
}

/*******************************************************************************
* Function Name  : SPI0_MASTER_Recv
* Description    : ��������
* Input          : None
* Output         : None
* Return         : data -���յ�����
*******************************************************************************/
UINT8 SPI0_MasterRecvByte( void )
{
    R16_SPI0_TOTAL_CNT = 1;                                                     /* ������Ϊ1 */
    while( R8_SPI0_FIFO_COUNT == 0 );                                           /* �ȴ����ݻ��� */
    return R8_SPI0_FIFO;                                                        /* ��FIFO�е����� */
}
// ʹ��SPI���ݻ������Ĵ�����ȡ����
void SPI0_MasterRecv1( UINT8 len, UINT8 *pbuf )
{
    R8_SPI0_BUFFER = 0xff;                                                      /* ��������*/
    while(len>1){
//        Delay_us( 10 );
        *pbuf = R8_SPI0_BUFFER;                                                 /* ��ȡ���ݣ����������� */
        pbuf++;
        len--;
    }
//    Delay_us( 10 );
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;                                       /* ����λ����� */
    *pbuf = R8_SPI0_BUFFER;                                                     /* ��ȡ���һ�ֽ� */
    R8_SPI0_CTRL_MOD |= RB_SPI_FIFO_DIR;                                        /* ����Ĭ�Ϸ���Ϊ���� */
}

// ʹ��FIFO��ȡ���ֽ�����
void SPI0_MasterRecv( UINT16 len, UINT8 *pbuf )
{
    UINT16 readlen;
    
    readlen = len;
    R16_SPI0_TOTAL_CNT = len;                                                   /* ������Ҫ���յ����ݳ��ȣ�FIFO����Ϊ���볤�Ȳ�Ϊ0����������� */
    while( readlen ){                                                           /* ʣ�����ݳ��ȣ���FIFO������ݣ�ÿ��һ�ֽڳ��Ȼ��Զ���1 */
        if( R8_SPI0_FIFO_COUNT ){                                               /* ��ѯFIFO���Ƿ������� */
            *pbuf = R8_SPI0_FIFO;                                               /* ������ */
            pbuf++;
            readlen--;
        }
    }
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
    UINT8 i;
    UINT8 spi_buf[1024];

    LED_OUT_INIT( );
    LED_OUT_ACT( );                                                             /* ������LED��һ����ʾ���� */
    Delay_ms( 100 );
    LED_OUT_INACT( );
    mInitSTDIO( );                                                              /* ����0��ʼ�� */
    PRINT("START SPI0 HOST:\n");  
    SPI_MASTER_INIT ( );                                                        /* ����ģʽ��ʼ�� */
    R32_PB_CLR |= SCS;                                                          /* Ƭѡʹ�� */
    while(1){
        SPI0_MasterTransByte(0xaa);                                             /* �������� */
        Delay_ms( 10);
        SPI0_MasterRecv( 100, spi_buf);
        PRINT("\r\n");
        for(i=0;i<10;i++) PRINT("%2lX ", spi_buf[i]);                           /* ͨ������0��ӡ���� */
        PRINT("\r\n");
        Delay_ms( 5000 );
    }
}

/*********************************** endfile **********************************/