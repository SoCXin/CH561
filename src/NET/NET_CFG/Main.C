/********************************** (C) COPYRIGHT ******************************
* File Name          : CH561.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/3/8
* Description        :������������ʾ��λ������ͨ��UDP�㲥������CH561�Ĺ���.
                       561������һ��UDP server������λ������ͨ��

˵���������������ӿ��ļ�\SRC\ISPXT56X.O ��\NET\PUB CH561NET.lib
*******************************************************************************/



/******************************************************************************/
/* ͷ�ļ�����*/
#include <stdio.h>
#include <string.h>
#include "CH561SFR.H"
#include "SYSFREQ.H"
#include "ModuleConfig.h"
#include "CH561NET.H"
#include "ISPXT56X.H"
#define  CH561NET_DBG                          1

/* ����Ļ�������ȫ�ֱ�������Ҫ���壬���е��� */
__align(16)UINT8    CH563MACRxDesBuf[(RX_QUEUE_ENTRIES )*16];                   /* MAC������������������16�ֽڶ��� */
__align(4) UINT8    CH563MACRxBuf[RX_QUEUE_ENTRIES*RX_BUF_SIZE];                /* MAC���ջ�������4�ֽڶ��� */
__align(4) SOCK_INF SocketInf[CH563NET_MAX_SOCKET_NUM];                         /* Socket��Ϣ����4�ֽڶ��� */
const UINT16 MemNum[8] = {CH563NET_NUM_IPRAW,
                       CH563NET_NUM_UDP,
                       CH563NET_NUM_TCP,
                       CH563NET_NUM_TCP_LISTEN,
                       CH563NET_NUM_TCP_SEG,
                       CH563NET_NUM_IP_REASSDATA,
                       CH563NET_NUM_PBUF,
                       CH563NET_NUM_POOL_BUF
                       };
const UINT16 MemSize[8] = {CH563NET_MEM_ALIGN_SIZE(CH563NET_SIZE_IPRAW_PCB),
                        CH563NET_MEM_ALIGN_SIZE(CH563NET_SIZE_UDP_PCB),
                        CH563NET_MEM_ALIGN_SIZE(CH563NET_SIZE_TCP_PCB),
                        CH563NET_MEM_ALIGN_SIZE(CH563NET_SIZE_TCP_PCB_LISTEN),
                        CH563NET_MEM_ALIGN_SIZE(CH563NET_SIZE_TCP_SEG),
                        CH563NET_MEM_ALIGN_SIZE(CH563NET_SIZE_IP_REASSDATA),
                        CH563NET_MEM_ALIGN_SIZE(CH563NET_SIZE_PBUF) + CH563NET_MEM_ALIGN_SIZE(0),
                        CH563NET_MEM_ALIGN_SIZE(CH563NET_SIZE_PBUF) + CH563NET_MEM_ALIGN_SIZE(CH563NET_SIZE_POOL_BUF)
                       };
__align(4)UINT8 Memp_Memory[CH563NET_MEMP_SIZE];
__align(4)UINT8 Mem_Heap_Memory[CH563NET_RAM_HEAP_SIZE];
__align(4)UINT8 Mem_ArpTable[CH563NET_RAM_ARP_TABLE_SIZE];
/******************************************************************************/
/* ����ʾ�������غ� */
#define RECE_BUF_LEN                          1600                              /* ���ջ������Ĵ�С */                   
                                                                                                                         
/* CH563��ض��� */
UINT8 Default_MAC[6]={0X02,0X03,0X04,0X05,0X06,0X07};                     
UINT8 MACAddr[6];                                                               /* MAC��ַ */                       
UINT8 IPAddr[4];                                                                /* IP��ַ */                        
UINT8 GWIPAddr[4];                                                              /* ���� */                          
UINT8 IPMask[4];                                                                /* �������� */                      
UINT8 SocketId;                                                                 /* ����socket���������Բ��ö��� */       
UINT8 SocketRecvBuf[CH563NET_MAX_SOCKET_NUM ][RECE_BUF_LEN];                    /* socket���ջ����� */                   
UINT8 MyBuf[RECE_BUF_LEN];                                                      /* ����һ����ʱ������ */                 
UINT8 Configbuf[MODULE_CFG_LEN];
pmodule_cfg CFG = (pmodule_cfg)Configbuf;
UINT8 socket_flag;                                                              /*������ʾ��λ�����õ���ʲôģʽ*/


/*******************************************************************************
* Function Name  : IRQ_Handler
* Description    : IRQ�жϷ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
__irq void IRQ_Handler( void )
{
    if(R32_INT_FLAG & 0x8000)                                                   /* ��̫���ж� */
    {                                                                           /* ��̫���ж��жϷ����� */
        CH563NET_ETHIsr();
    }
    if(R32_INT_FLAG & RB_IF_TMR0)                                               /* ��ʱ���ж� */
    {
        CH563NET_TimeIsr(CH563NETTIMEPERIOD);                                   /* ��ʱ���жϷ����� */
        R8_TMR0_INT_FLAG |= 0xff;                                               /* �����ʱ���жϱ�־ */
   }
}

__irq void FIQ_Handler( void )
{
    while(1);
}

/*******************************************************************************
* Function Name  : SysTimeInit
* Description    : ϵͳ��ʱ����ʼ����@100MHZ TIME0 10ms������CH563NETTIMEPERIOD
*                ������ʼ����ʱ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTimeInit(void)
{
    R8_TMR0_CTRL_MOD = RB_TMR_ALL_CLEAR;
    R32_TMR0_COUNT = 0x00000000; 
    R32_TMR0_CNT_END = 0x186a0 * CH563NETTIMEPERIOD;                            /* ����Ϊ10MS��ʱ */
    R8_TMR0_INTER_EN |= RB_TMR_IE_CYC_END;
    R8_TMR0_CTRL_MOD = RB_TMR_COUNT_EN;
}

/*******************************************************************************
* Function Name  : InitSysHal
* Description    : Ӳ����ʼ������������TIM0��ETH�ж�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void InitSysHal(void)
{
    R8_INT_EN_IRQ_0 |= RB_IE_IRQ_TMR0;                                          /* ����TIM0�ж� */
    R8_INT_EN_IRQ_1 |= RB_IE_IRQ_ETH;                                           /* ����ETH�ж� */
    R8_INT_EN_IRQ_GLOB |= RB_IE_IRQ_GLOB;                                       /* ����IRQȫ���ж� */
}

/*******************************************************************************
* Function Name  : mStopIfError
* Description    : ����ʹ�ã���ʾ�������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void mStopIfError(UINT8 iError)
{
    if (iError == CH563NET_ERR_SUCCESS) return;                                 /* �����ɹ� */
#if CH561NET_DBG
    printf("Error: %02X\n", (UINT16)iError);                                    /* ��ʾ���� */
#endif    
}


void CH561_RESET()
{
#if CH561NET_DBG
    printf("reset \n");
#endif  
    R8_SAFE_ACCESS_SIG = 0x57 ;                                                 /* unlock step 1 */
    R8_SAFE_ACCESS_SIG = 0xA8 ;                                                 /* unlock step 2 */                     
    R8_GLOB_RST_CFG = 0x40 | RB_GLOB_FORCE_RST;                                 /*������λ*/  
    R8_SAFE_ACCESS_SIG = 0;                                                     /* ��������ֹ��д */
}


/*******************************************************************************
* Function Name  : CH561NET_LibInit
* Description    : ���ʼ������
* Input          : ip      ip��ַָ��
*                ��gwip    ����ip��ַָ��
*                 : mask    ����ָ��
*                 : macaddr MAC��ַָ�� 
* Output         : None
* Return         : ִ��״̬
*******************************************************************************/
UINT8 CH561NET_LibInit( UINT8 *ip, UINT8 *gwip, UINT8 *mask, UINT8 *macaddr)
{
    UINT8 i;
    struct _CH563_CFG cfg;
    if(CH563NET_GetVer() != CH563NET_LIB_VER)return 0xfc;                       /* ��ȡ��İ汾�ţ�����Ƿ��ͷ�ļ�һ�� */
    CH563NETConfig = LIB_CFG_VALUE;                                             /* ��������Ϣ���ݸ�������ñ��� */
    cfg.RxBufSize = RX_BUF_SIZE; 
    cfg.TCPMss   = CH563NET_TCP_MSS;
    cfg.HeapSize = CH563_MEM_HEAP_SIZE;
    cfg.ARPTableNum = CH563NET_NUM_ARP_TABLE;
    cfg.MiscConfig0 = CH563NET_MISC_CONFIG0;
    CH563NET_ConfigLIB(&cfg);
    i = CH563NET_Init(ip,gwip,mask,macaddr);
    return (i);              /* ���ʼ�� */
}


void CH561NET_UdpServerRecv(struct _SCOK_INF *socinf,UINT32 ipaddr,UINT16 port,UINT8 *buf,UINT32 len)
{
    UINT8 i;    
     
    i =ParseConfigbuf(buf) ;                                                    /*��������*/
    if(i){
     CH561_RESET();
    }                 
}



void CH561NET_UdpSRecv(struct _SCOK_INF *socinf,UINT32 ipaddr,UINT16 port,UINT8 *buf,UINT32 len)
{
  UINT8 ip_addr[4],i;
  
  printf("ip:");
  for(i=0;i<4;i++){
    ip_addr[i]=ipaddr&0xff;
    printf("%-4d",ip_addr[i]);
    ipaddr=ipaddr>>8;
  }
#if CH561NET_DBG
  printf("port=%-8d  len=%-8d  sockeid=%-4d\n",port,len,socinf->SockIndex);
#endif  
  CH563NET_SocketUdpSendTo(socinf->SockIndex,buf,&len,ip_addr,port);
}

void CH561NET_Creat_Communication_Socket(void)
{   
   UINT8 i;                                                             
   UINT8 *desip=CFG->dest_ip;                                                   /* Ŀ��IP��ַ */
   SOCK_INF TmpSocketInf;                                                       /* ������ʱsocket���� */
   memset((void *)&TmpSocketInf,0,sizeof(SOCK_INF));                            /* ���ڲ��Ὣ�˱������ƣ�������ý���ʱ������ȫ������ */
   memcpy((void *)TmpSocketInf.IPAddr,desip,4);                                 /* ����Ŀ��IP��ַ */
   TmpSocketInf.DesPort = ((UINT16)CFG->dest_port[0]+(UINT16)CFG->dest_port[1]*256);                /* ����Ŀ�Ķ˿� */
   TmpSocketInf.SourPort = ((UINT16)CFG->src_port[0]+(UINT16)CFG->src_port[1]*256);                 /* ����Դ�˿� */
   switch(CFG->type){
     case NET_MODULE_TYPE_TCP_S:     
          TmpSocketInf.ProtoType = PROTO_TYPE_TCP;                              /* ����socekt���� */         
          socket_flag= NET_MODULE_TYPE_TCP_S;
          break;
     case NET_MODULE_TYPE_TCP_C:
          TmpSocketInf.ProtoType = PROTO_TYPE_TCP;          
          socket_flag= NET_MODULE_TYPE_TCP_C;
          break;
     case NET_MODULE_TYPE_UDP_S:
          TmpSocketInf.ProtoType = PROTO_TYPE_UDP;
          TmpSocketInf.AppCallBack = CH561NET_UdpSRecv;
          socket_flag= NET_MODULE_TYPE_UDP_S;
          break;
     case NET_MODULE_TYPE_UDP_C:
          TmpSocketInf.ProtoType = PROTO_TYPE_UDP;
          socket_flag= NET_MODULE_TYPE_UDP_C;
          break; 
     default:
          break;
   }     
   TmpSocketInf.RecvStartPoint = (UINT32)SocketRecvBuf[1];                      /* ���ý��ջ������Ľ��ջ����� */
   TmpSocketInf.RecvBufLen = RECE_BUF_LEN ;                                     /* ���ý��ջ������Ľ��ճ��� */
   i = CH563NET_SocketCreat(&SocketId,&TmpSocketInf);                           /* ����socket�������ص�socket����������SocketId�� */
   mStopIfError(i);                                                             /* ������ */
   switch(CFG->type){
     case NET_MODULE_TYPE_TCP_S:
          CH563NET_SocketListen(SocketId);
          printf("listening\n");
          break;
     case NET_MODULE_TYPE_TCP_C:
          CH563NET_SocketConnect(SocketId);
          printf("connecting\n");
          break;
     default:
          break;
   }

}

/*******************************************************************************
* Function Name  : CH561NET_HandleSockInt
* Description    : Socket�жϴ�������
* Input          : sockeid  socket����
*                ��initstat �ж�״̬
* Output         : None
* Return         : None
*******************************************************************************/
void CH561NET_HandleSockInt(UINT8 sockeid,UINT8 initstat)
{
    UINT32 len;


    if(initstat & SINT_STAT_RECV) {                                             /* �����ж� */
      len = CH563NET_SocketRecvLen(sockeid,NULL);                               /* ��ѯ���� */
      CH563NET_SocketRecv(sockeid,MyBuf,&len);                                  /* �����ջ����������ݶ���MyBuf��*/           
    }
    if(initstat & SINT_STAT_CONNECT) {                                          /* TCP�����ж� */
      if(socket_flag==NET_MODULE_TYPE_TCP_S){
        CH563NET_ModifyRecvBuf(sockeid,(UINT32)SocketRecvBuf[sockeid],RECE_BUF_LEN);
      }                    
    }
    if(initstat & SINT_STAT_DISCONNECT){                                        /* TCP�Ͽ��ж� */    
      Delay_ms(200);
      CH561NET_Creat_Communication_Socket();      
    }
    if(initstat & SINT_STAT_TIM_OUT){                                           /* TCP��ʱ�ж� */

      Delay_ms(200);
      CH561NET_Creat_Communication_Socket();     
    }

}

/*******************************************************************************
* Function Name  : CH561NET_HandleGloableInt
* Description    : ȫ���жϴ�������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH561NET_HandleGlobalInt(void)
{
    UINT8 initstat;
    UINT8 i;
    UINT8 socketinit;
    
    initstat = CH563NET_GetGlobalInt();                                         /* ��ȫ���ж�״̬����� */
    if(initstat & GINT_STAT_UNREACH)                                            /* ���ɴ��ж� */
    {
#if CH561NET_DBG
        printf("UnreachCode ��%d\n",CH563Inf.UnreachCode);                      /* �鿴���ɴ���� */
        printf("UnreachProto ��%d\n",CH563Inf.UnreachProto);                    /* �鿴���ɴ�Э������ */
        printf("UnreachPort ��%d\n",CH563Inf.UnreachPort);                      /* ��ѯ���ɴ�˿� */
#endif       
    }
   if(initstat & GINT_STAT_IP_CONFLI)                                           /* IP��ͻ�ж� */
   {
   
   }
   if(initstat & GINT_STAT_PHY_CHANGE)                                          /* PHY�ı��ж� */
   {
       i = CH563NET_GetPHYStatus();                                             /* ��ȡPHY״̬ */
#if CH561NET_DBG
       printf("GINT_STAT_PHY_CHANGE %02x\n",i);
#endif   
   }
   if(initstat & GINT_STAT_SOCKET)                                              /* Socket�ж� */
   {
       for(i = 0; i < CH563NET_MAX_SOCKET_NUM; i ++)                     
       {
           socketinit = CH563NET_GetSocketInt(i);                               /* ��socket�жϲ����� */
           if(socketinit)CH561NET_HandleSockInt(i,socketinit);                  /* ������ж������� */
       }    
   }
}




/*******************************************************************************
* Function Name  : CH561NET_CreatUdpSocket
* Description    : ����UDP socket
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH561NET_CreatUdpSocket(void)
{
   UINT8 i;                                                             
   UINT8 desip[4] = {255,255,255,255};                                          /* Ŀ��IP��ַ */
   SOCK_INF TmpSocketInf;                                                       /* ������ʱsocket���� */

   memset((void *)&TmpSocketInf,0,sizeof(SOCK_INF));                            /* ���ڲ��Ὣ�˱������ƣ�������ý���ʱ������ȫ������ */
   memcpy((void *)TmpSocketInf.IPAddr,desip,4);                                 /* ����Ŀ��IP��ַ */
   TmpSocketInf.DesPort = 60000;                                                /* ����Ŀ�Ķ˿� */
   TmpSocketInf.SourPort = 50000;                                               /* ����Դ�˿� */
   TmpSocketInf.ProtoType = PROTO_TYPE_UDP;                                     /* ����socekt���� */
   TmpSocketInf.AppCallBack = CH561NET_UdpServerRecv;
   TmpSocketInf.RecvStartPoint = (UINT32)SocketRecvBuf[0];                       /* ���ý��ջ������Ľ��ջ����� */
   TmpSocketInf.RecvBufLen = RECE_BUF_LEN ;                                     /* ���ý��ջ������Ľ��ճ��� */
   i = CH563NET_SocketCreat(&SocketId,&TmpSocketInf);                           /* ����socket�������ص�socket����������SocketId�� */
   mStopIfError(i);                                                             /* ������ */
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
    R32_PB_SMT |= RXD0|TXD0;                                                    /* RXD1 schmitt input, TXD1 slow rate */
    R32_PB_PD &= ~ RXD0;                                                        /* disable pulldown for RXD0, keep pullup */
    R32_PB_DIR |= TXD0;                                                         /* TXD1 output enable */
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
    UINT8 i = 0;
  
    mInitSTDIO( );                                                              /* Ϊ���ü����ͨ�����ڼ����ʾ���� */   
    R32_PB_PU  |=1<<6;
    R32_PB_DIR &=~(1<<6);
    EEPROM_READ( 0x1000,CFG,MODULE_CFG_LEN );                                   /* ��ȡEEPROM�ﱣ���������Ϣ */   
    if(CFG->cfg_flag[0]!=checkcode1||CFG->cfg_flag[1]!=checkcode2){             /*У��EEPROM�����Ϣ�Ƿ�Ϸ�*/                                                                               /*��EEPROM�ﱣ�����Ϣ���Ϸ�����561��δ������λ������,��ô�Ͱ���Ĭ����������ʼ��561*/                                                                                                         
      i = EEPROM_ERASE(0x1000,4096);                                                   
      i = EEPROM_WRITE(0x1000,Default_cfg,MODULE_CFG_LEN );                                       
      CH561_RESET();
    }
    i=CH56X_GetMac( MACAddr ); 
    if(i){                                                                      /*�����ȡeeprom���macʧ�ܣ���ô561��ʹ��Ĭ��MAC*/
      memcpy(MACAddr,Default_MAC,6);
    }
#if CH561NET_DBG    
    printf("MACAddr:");    
    for(i=0;i<6;i++){
      printf("%02x  ",MACAddr[i]);
    }
    printf("\n");
#endif
    memcpy(IPAddr,CFG->src_ip,sizeof(CFG->src_ip));         
    memcpy(GWIPAddr,CFG->getway,sizeof(CFG->getway));   
    memcpy(IPMask,CFG->mask,sizeof(CFG->mask));       
    i = CH561NET_LibInit(IPAddr,GWIPAddr,IPMask,MACAddr);                       /* ���ʼ�� */
    mStopIfError(i);                                                            /* ������ */
#if CH561NET_DBG
     printf(" 561 ip:%d.%d.%d.%d\n 561 gwip:%d.%d.%d.%d\n 561 mask:%d.%d.%d.%d\n", IPAddr[0],IPAddr[1],IPAddr[2],IPAddr[3],\
            GWIPAddr[0],GWIPAddr[1],GWIPAddr[2],GWIPAddr[3],IPMask[0],IPMask[1],IPMask[2],IPMask[3]); 
     printf("CH561IPLibInit Success\n");
#endif    
    SysTimeInit();                                                              /* ϵͳ��ʱ����ʼ�� */
    InitSysHal();                                                               /* ��ʼ���ж� */
    CH561NET_CreatUdpSocket();                                                  /* ����UDP Socket */
    if(CFG->type!=NET_MODULE_TYPE_NONE){                                        /*Ĭ�������²������κ�ģʽ*/      
    CH561NET_Creat_Communication_Socket();                                      /*������λ������ģʽ�µ�socket*/
    }  
    if((R32_PB_PIN&(1<<6))==0) {                                                /*�������º󣬳�ʼ��561��ִ��Ĭ������*/ 
       Delay_ms(50);
       if((R32_PB_PIN&(1<<6))==0){      
				 while((R32_PB_PIN&(1<<6))==0);
       i = EEPROM_ERASE(0x1000,4096);
       i = EEPROM_WRITE( 0x1000,(pmodule_cfg)Default_cfg,MODULE_CFG_LEN );   
       CH561_RESET();
       }
    }
    while(1){
      CH563NET_MainTask();                                                      /* CH563NET��������������Ҫ����ѭ���в��ϵ��� */
      if(CH563NET_QueryGlobalInt())CH561NET_HandleGlobalInt();                  /* ��ѯ�жϣ�������жϣ������ȫ���жϴ������� */     
    }
}

/*********************************** endfile **********************************/