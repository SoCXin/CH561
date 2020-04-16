/********************************** (C) COPYRIGHT *******************************
* File Name          : EHCI_HCD.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/3/8
* Description        : Description: CH561MAC Non-OS firmware header file 
*******************************************************************************/



/******************************************************************************/
#ifndef __NETMAC_H__
#define __NETMAC_H__                            

/******************************************************************************/
#define ETHER_HEAD_LEN                     14
#define MAC_MAX_LEN                        1502
#define CH561_DEBUG                        1                                    /* ���ջ����������� */
#define RECE_BUF_LEN                       0x5ec                                /* ���ջ������Ĵ�С */


/*
 * MAC�㷢�ͽ��ջ�������ز�������
 */
/* ����������С��2 */
#define RX_QUEUE_NUM                       3                                    /* ���ջ����������� */
#define TX_QUEUE_NUM                       2
#define RX_BUF_SIZE                        0x5ec                                /* ���ջ�������С */
#define TX_BUF_SIZE                        0x5ec                                /* ���ͻ�������С */

#define BUF_IDLE                           0                                    /* ���������� */
#define BUF_MAC_LOCK                       1                                    /* ������MAC���� */
#define BUF_IP_LOCK                        2                                    /* ������IP���� */

#define IPADDR_LEN                         4
#define MACADDR_LEN                        6
#define NET_SOCKET_NUM                     20
/* 
 * PHY״̬���� 
 */
#define PHY_DISCONN                         (1<<0)                              /* PHY�Ͽ� */
#define PHY_10M_FLL                         (1<<1)                              /* 10Mȫ˫�� */
#define PHY_10M_HALF                        (1<<2)                              /* 10M��˫�� */
#define PHY_100M_FLL                        (1<<3)                              /* 100Mȫ˫�� */
#define PHY_100M_HALF                       (1<<4)                              /* 100M��˫�� */
#define PHY_AUTO                            (1<<5)                              /* PHY�Զ�ģʽ��CMD10H_SET_PHY */
        
#define FTPHY_REG_CONTROL                  0x00
#define FTPHY_REG_STATUS                   0x01
#define FTPHY_RB_PHYAD                     0x10
/*
 * ȫ�ֱ�������
 */
extern UINT8   MACRxBuf[RX_QUEUE_NUM][RX_BUF_SIZE];                             /* MAC���ջ�������4�ֽڶ��� */
extern UINT8   MACTxBuf[TX_QUEUE_NUM][TX_BUF_SIZE];                             /* MAC���ͻ�������4�ֽڶ��� */
extern UINT8   RecvBuf[RX_BUF_SIZE];
extern PUINT8  MACSendbuf;
extern UINT8V  PHYStatus;
extern UINT8V  SendSuc;                                                         /* �������ݰ� �ɹ���־ */
extern UINT8   CH561MACAddr[MACADDR_LEN];                                       /* MAC��ַ */
extern UINT8   CH561IPAddr[IPADDR_LEN];
extern UINT8   IPDestAddr[IPADDR_LEN];


/* 
 * RX���տ�����Ϣ�� 
 */
struct _RXBUFST
{
    UINT8  RecvIndex;                                                           /* MAC����ջ����������� */
    UINT8  ReadIndex;                                                           /* ��ȡ������������ֵ */
    UINT8  RemainCout;                                                          /* ʣ��δ��ȡ�����ݰ� */
    UINT8  RxDMAEnable;                                                         /* ����DMAʹ�ܱ�־���ڻ��������ʱ�����Թر�RX DMA*/
    UINT8  RxBufStau[RX_QUEUE_NUM];                                             /* ���ջ�������״̬ */
    UINT16 RxBufLen[RX_QUEUE_NUM];                                              /* ���ջ��������ݵ���Ч���� */
    UINT32 RxBufAddr[RX_QUEUE_NUM];                                             /* ���ջ���������ʼ��ַ */
};
extern struct  _RXBUFST    RxCtrl;                                     

/* 
 * TX���տ�����Ϣ�� 
 */                                       

struct _TXBUFST
{                                                                               
    UINT8  SendIndex;                                                           /* �������������� */
    UINT8  TxBufStat[TX_QUEUE_NUM];                                             /* ���ͻ�����״̬ */
};
extern struct _TXBUFST    TxCtrl;                                      

/*
 * ������������16�ֽڶ���
 */
typedef struct _CH561MAC_TXDES {
    UINT32 txdes0; /* ������֡״̬��������Ȩ�� */
    UINT32 txdes1; /* ����λ�뷢�ͻ�������С */
    UINT32 txdes2; /* ���ͻ������Ļ���ַ */
    UINT32 txdes3; /* û��ʹ�ã��������ڱ�����һ����������ַ */
}mac_txdes;
extern UINT8  TxDesBuf[TX_QUEUE_NUM*16];

/*
 * ������������16�ֽڶ���
 */
typedef struct _CH561MAC_RXDES 
{
    UINT32 rxdes0; /* ������֡״̬��������Ȩ�� */
    UINT32 rxdes1; /* ����λ�뷢�ͻ�������С */
    UINT32 rxdes2; /* ���ջ������Ļ���ַ */
    UINT32 rxdes3; /* û��ʹ�ã��������ڱ�����һ����������ַ */
}mac_rxdes;
extern UINT8  RxDesBuf[(RX_QUEUE_NUM )*16]; 

/*
 * CH561MAC���ƽṹ
 */
typedef struct _CH561MAC_Control
{
    mac_txdes *txdes_top;                                                       /* �������������� */
    mac_txdes *txdes_cur;                                                       /* ��ǰ���������� */
    mac_txdes *txdes_end;                                                       /* ���һ������������ */
    mac_rxdes *rxdes_top;                                                       /* �������������� */
    mac_rxdes *rxdes_cur;                                                       /* ��ǰ���������� */
    mac_rxdes *rxdes_end;                                                       /* ���һ�������� */
}MACCtrl;
extern MACCtrl   mac_cotrl;                                       


typedef union _ETHERHEAD
{
    UINT8  buffer[ETHER_HEAD_LEN];
    struct
    {
        UINT8   DestMAC[MACADDR_LEN];
        UINT8   SourMAC[MACADDR_LEN];
        UINT16  PROTOCOL;
        #define ETHER_TYPE_IP            0x0800    
        #define ETHER_TYPE_ARP          0x0806
    }Head;
}ETHERHEAD;
extern ETHERHEAD    ETH;                                                        /* mac���ͷ */
#define ARP_ENTRY_TIME_OUT            10
#define ARP_PACKAGE_LEN                28
#define ARP_ENTRY_MAX_NUM            8

/* arppacket->type */
#define ARP_CODE_QUEST                0x0001    
#define ARP_CODE_REPLY              0x0002
#define RARP_CODE_QUEST                0x0003
#define RARP_CODE_REPLY                0x0004

typedef union _ARPPACKAGE
{
    UINT8  buffer[ARP_PACKAGE_LEN];
    struct  
    {
        UINT16 HRDTYPE;        // Ӳ������
        #define ARP_HARDWARE_TYPE_ETHER     0x0001
        UINT16 PROTYPE;        
        #define ARP_PROTOCOL_TYPE_IP        0x0800
        UINT8  MACLEN;       
        UINT8  IPLEN;
        UINT16 OPCODE;      
        UINT8  SENDMAC[MACADDR_LEN];
        UINT8  SENDIP[IPADDR_LEN];
        UINT8  TARGMAC[MACADDR_LEN];
        UINT8  TARGIP[IPADDR_LEN];
    }Package;
}ARPPACKAGE;

/* 
 * ��������
 */
UINT16 GLOB_nTohs( UINT16 in );

void    NET_INIT( void );

void    NET_TimeIsr( UINT16 time );

UINT16  PHY_ReadData(UINT16 regad);

void    MAC_InitTXDes(void);

void    MAC_Isr(void);

UINT8   MAC_QueryInput(void);

UINT16  MAC_Input(UINT8 *buf,UINT16 *slen);

void    MAC_RecvFinishHanld( void );

UINT8   MAC_Output(UINT8 *buf,UINT16 *len,UINT16 code);

UINT8 ARP_Output( UINT8 *desip,UINT8 code );

void ARP_Input( UINT8 *arpdata,UINT16 len );

#endif