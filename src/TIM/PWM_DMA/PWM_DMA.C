/********************************** (C) COPYRIGHT *******************************
* File Name          : PWM_DMA.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/3/8
* Description        : CH561 PWM_DMA DEMO
*                       
*                      (1)������0��������Ϣ,115200bps;
*                      (2)��ARM9-CH561��Ƭ��C���ԵĶ�ʱ��ͨ��DMA��ʽ�������Ϊ20msռ�ձȲ��ϸı��PWM����ʾ������             
*                           ��������ʾ��ʱ��0���PWM���ܣ���ʱ��1��2��3���ƣ���֮ͬ��������������֮ͬ�����ڳ���ע����ָ��;  
*                           ������ͨ��PB0���������20ms��PWM���Σ����ε�ռ�ձ�ͨ���˳�����ڣ���һ������ߵ�ƽ100US���͵�ƽ
*                           ���ʱ��Ϊ(20ms-100us),�˲����ظ�16��;�ڶ�������ߵ�ƽ100us+100us(�Ժ�ÿ������ߵ�ƽʱ������100us)��
*                           �˲����ظ����16������е�����DMA���䣬�Դ�����ֱ����100�β����������;PMM�����PB0����.
*******************************************************************************/



/******************************************************************************/
/* ͷ�ļ����� */
#include <stdio.h>
#include <string.h>
#include "CH561SFR.H"
#include "SYSFREQ.H"

/******************************************************************************/
/* ͷ�ļ����� */
UINT32 FIFO_DMA[100]={0};    /* �������ݻ����� */

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
        if(i&RB_TMR_IF_DMA_END){
            R16_TMR0_DMA_NOW = (UINT32)FIFO_DMA;
            for( i=0;i!=100;i++ ){
                if(i%16 == 0)PRINT("\n");
                PRINT("%-8x",FIFO_DMA[i]);
            }
            PRINT("\n");
            R8_TMR0_INT_FLAG=RB_TMR_IF_DMA_END;                                 /* �����Ӧ�жϱ�־ */
            R8_INT_FLAG_0 |= RB_IF_TMR0;                                        /* �����Ӧ�жϱ�־ */
            R8_TMR0_CTRL_MOD&=0xf7;                                             /* �رն�ʱ�� */
            R8_TMR0_INTER_EN&=0xf7;                                             /* �ر�DMA����ʹ���ж� */
            PRINT("DMA OVER\n");                                                /* ���DMA�������򲻶������DMA OVER�� */
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
    R8_TMR0_INT_FLAG=0xff;                                                      /* ������е��жϱ�־ */                                           
    R8_TMR0_INTER_EN=RB_TMR_IE_DMA_END;                                         /* ֻ����DMA�����ж� */
    R8_INT_EN_IRQ_0=RB_IE_IRQ_TMR0;                                             /* ֻ������ʱ��0����жϲ��� */
    R8_INT_EN_IRQ_GLOB= RB_IE_IRQ_GLOB;                                         /* ֻ����IRQȫ���ж� */
}

/*******************************************************************************
* Function Name  : Timer0_Init
* Description    : ��ʱ��0��ʼ��
* Input          : end    -��ʱ��0��ʱ���ڣ���ʱ����Time = Pclk*end, PclkΪϵͳʱ��
*                  repeat -PWM���Դ�����00:�ظ�1�Σ�01:�ظ�4�Σ�10:�ظ�8�Σ�11:�ظ�16��
* Output         : None
* Return         : None
*******************************************************************************/

void Timer0_Init( UINT32 end ,UINT8 repeat ) 
{
    R32_PB_DIR |= PWM0;                                                         /* PB0Ϊ��ʱ��0������ţ�����Ϊ��� */
    R8_TMR0_CTRL_MOD=(RB_TMR_PWM_REPEAT&(repeat<<6));                           /* PWMģʽ��Ĭ���������Ϊ�ߵ�ƽ��������ʱ���������ʱ��/PWMģʽ��*/
    /* �����TIM1��TIM2,����Ҫ��RB_TMR_MODE_NRZIλ��0��                        */
    /* Ĭ�������R8_TMR0_CTRL_MOD��λRB_TMR_ALL_CLEARΪ1����ʼʱ����Ѵ�λ��0��*/
    /* ����R32_TMR0_FIFO��ʹ��ֵ�ɹ�Ҳ���ܹ���,�����R32_TMR0_CNT_END��ֵ�ɹ�  */
    /* ���������Ӱ�� */
    R32_TMR0_CNT_END=end;                                                       /* ����PWM���� */
    R8_TMR0_CTRL_DMA= (RB_TMR_DMA_ENABLE)|(RB_TMR_DMA_LOOP&0);                  /* ����DMA����ֹDMAѭ�� */
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
* Function Name  : main
* Description    : ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

int main( void ) 
{
    UINT32    i,j=1000;
    LED_OUT_INIT( );
    LED_OUT_ACT( );                                                             /* ������LED��һ����ʾ���� */
    Delay_ms( 100 );
    LED_OUT_INACT( );
    mInitSTDIO( );                                                              /* Ϊ���ü����ͨ�����ڼ����ʾ���� */
    LED_OUT_ACT( );
    LED_OUT_INACT( );
    PRINT( "Start PWM_DMA DEMO:\n" );
    /* ����FIFO_DMA��һ��ֵ���ȴ���R32_TMR0_FIFO�Ĵ�����R32_TMR0_FIFO�Ĵ��������ǲ���PWM������ռ�ձȣ���*/
    /* PWM��ɲ��Ҽ�����������R32_TMR0_CNT_ENDֵʱ������FIFO_DMA�ڶ���ֵ����R32_TMR0_FIFO�Ĵ�����������    */
    /* ��ֱ����������һ��Ԫ��ֵ����R32_TMR0_FIFOʱ��DMA����                                            */
    for(i=0;i<10;i++)FIFO_DMA[i]=(j*i+j);                            
    R16_TMR0_DMA_BEG=(UINT32)FIFO_DMA;                                          /* ������׵�ַ��DMA��ʼ��������ַ */
    R16_TMR0_DMA_END=(UINT32)(&FIFO_DMA[10]);                                   /* ����Ľ�����ַ��DMA������������ַ */
    IRQ_Init( );                                                                /* �жϳ�ʼ�� */
    Timer0_Init( 2000000,3 );                                                   /* PWM����Ϊ20ms,ÿ��PWM������Դ���Ϊ16�� */
    while(1);
}

/*********************************** endfile **********************************/