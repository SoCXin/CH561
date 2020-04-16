/********************************** (C) COPYRIGHT ******************************
* File Name          : ADC_INT.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/3/8
* Description        : CH561 ADC_INT Demo   
*                      (1)������0��������Ϣ,115200bps;
*                      (2)��ʵ��ADC��������.AD���̲���DMA����,Ĭ��ѡ��ͨ��2����ADCת��.
*******************************************************************************/



/******************************************************************************/
/* ͷ�ļ����� */
#include <stdio.h>
#include <string.h>
#include "CH561SFR.H"
#include "SYSFREQ.H"

/******************************************************************************/
/* �������� */
UINT16 ADC_buf[1024]; 

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
    UINT16 i;
    UINT32 SUM=0;

    if( R8_INT_FLAG_1 & RB_IF_ADC ){                                            /* ADC�ж� */
        if( R8_ADC_INT_FLAG & RB_ADC_IF_DMA_END ){                              /* DMA���� */
            R8_ADC_CTRL_DMA &= ~(RB_ADC_DMA_ENABLE|RB_ADC_DMA_BURST | RB_ADC_MAN_SAMPLE );
                                                                                /* �ر�ADC��DMA */
            R8_ADC_CTRL_DMA &= ~RB_ADC_CHAN_OE;                                 /* �����ֹADCͨ������ADCS������������λ */
            R16_ADC_DMA_BEG = (UINT32)ADC_buf;
			R8_ADC_INT_FLAG = 0xff;                                             /* ���ж� */ 
            for( i=0;i!=1024;i++ ){                                             /* ��ʾADC�ɼ�����,ADC�ɼ�����λ10λ */
                SUM +=ADC_buf[i];
                if(i%16 == 0) PRINT("\n");
                PRINT("%04x ",(UINT16)ADC_buf[i]);
            } 
            PRINT("\r\naverage=%06x\n",(UINT32)(SUM/1024));
            
#if 0
            /* �����Ҫѭ������,����������� */
            i = R8_ADC_FIFO_COUNT;                                              /* ��Ϊ���FIFO�ڵ����� */
            while(i){
                R16_ADC_FIFO;
                i--;
            }
            R16_ADC_DMA_BEG = (UINT32)&ADC_buf[0];
            R16_ADC_DMA_END = (UINT32)&ADC_buf[1024];
            R8_ADC_CTRL_DMA |= (RB_ADC_DMA_ENABLE|RB_ADC_DMA_BURST | RB_ADC_MAN_SAMPLE );
                                                                                /* ����DMA */
            R8_ADC_CTRL_DMA &= ~RB_ADC_CHAN_OE;                                 /* �����ֹADCͨ������ADCS������������λ */
            R8_ADC_INT_FLAG = 0xff;                                             /* ���ж� */    
#endif
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
* Function Name  : ADC_INIT
* Description    : ADC��ʼ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/* ������Ҫ����ADC�������á�ADC�ж������Լ�DMA����,����������ж����ж����ÿ���*/
/* ȥ��,���������DMA��������Զ�ȡR16_ADC_DATA�Ĵ���,ADC�ɼ�����ڴ˼Ĵ���    */

void ADC_INIT( void )
{
    UINT16 i;

    R8_ADC_CTRL_MOD = 0;
    R8_ADC_CTRL_MOD |= RB_ADC_POWER_ON;                                         /* ����ADCʹ�� */
    R8_ADC_CLOCK_DIV = 0x40;                                                    /* ADC����ʱ���ź� */
    R8_ADC_CTRL_MOD |= (RB_ADC_SAMPLE_WID | RB_ADC_CYCLE_CLK );                 /* ���������Լ������Զ�ѭ������(RB_ADC_CYCLE_CLK�������11�� */
                                                                                /* ���Ϊ0��Ϊ�ֶ������� */
    R8_ADC_CTRL_MOD |= 0x20;                                                    /* ѡ��ͨ��2 */    
    Delay_us(100);  

    /*����Ϊ�ж�����*/
    R8_ADC_INT_FLAG = 0xff;                                                     /* ���жϱ�־ */
    R8_ADC_INTER_EN |= RB_ADC_IE_DMA_END;                                       /* ����DMA�����жϣ������ʹ��DMA��ʽ����ADC����ж� */
    R8_INT_EN_IRQ_1 |= RB_IE_IRQ_ADC;                                           /* ����ADC�ж� */
    
    R8_ADC_INT_FLAG = 0xff;                                                     /* ���жϱ�־ */
    R8_INT_EN_IRQ_GLOB |= RB_IE_IRQ_GLOB;                                       /* ����ȫ���ж� */
    i = R8_ADC_FIFO_COUNT;                                                      /* ��Ϊ���FIFO�ڵ����� */
    while(i){
        R16_ADC_FIFO;
        i--;
    }
    /* ������ֶ�������������RB_ADC_MAN_SAMPLEλΪ1����ʱ150ns���ϣ�����0�Բ����ֶ��������� */
    /* ����ΪDMA���� */    
    R16_ADC_DMA_BEG = (UINT32)&ADC_buf[0];
    R16_ADC_DMA_END = (UINT32)&ADC_buf[1024];
    R8_ADC_CTRL_DMA |=(RB_ADC_DMA_ENABLE|RB_ADC_DMA_BURST | RB_ADC_MAN_SAMPLE );/* ����DMA */
    R8_ADC_CTRL_DMA &= ~RB_ADC_CHAN_OE;                                         /* �����ֹADCͨ������ADCS������������λ */
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
    LED_OUT_ACT( );
    LED_OUT_INACT( );
    PRINT( "ADC Demo....." );
    ADC_INIT(  );
    while(1);                                                                   /* �ȴ�ADC��DMA���� */
}

/*********************************** endfile **********************************/