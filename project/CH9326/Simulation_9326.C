/********************************** (C) COPYRIGHT *******************************
* File Name          :Compound_Dev.C
* Author             : WCH
* Version            : V1.3
* Date               : 2016/1/15
* Description        : CH558ģ��CH9326�豸
*******************************************************************************/
#include "DEBUG.C"                                                       //������Ϣ��ӡ
#include "DEBUG.H"
#include <string.h>

#define THIS_ENDP0_SIZE         DEFAULT_ENDP0_SIZE
#define DEF_BUF_LENGTH          0x40
#define DEF_THIS_IC_VER         0x13

UINT8X  Ep0Buffer[8<(THIS_ENDP0_SIZE+2)?8:(THIS_ENDP0_SIZE+2)] _at_ 0x0000;    //�˵�0 OUT&IN��������������ż��ַ
UINT8X  Ep2Buffer[128<(2*MAX_PACKET_SIZE+4)?128:(2*MAX_PACKET_SIZE+4)] _at_ 0x0010;//�˵�2 IN&OUT������,������ż��ַ
UINT8   SetupReq,SetupLen,Ready,Count,FLAG,UsbConfig;
PUINT8  pDescr;                                                                //USB���ñ�־
USB_SETUP_REQ   SetupReqBuf;                                                   //�ݴ�Setup��
#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)
#define DEBUG 0
sbit Ep2InKey = P1^0;                                                          //K3����
#pragma  NOAREGS
/*�豸������*/
UINT8C DevDesc[18] = {0x12,0x01,0x00,0x01,0x00,0x00,0x00,0x08,
                      0x96,0x1A,0x10,0xE0,0x00,DEF_THIS_IC_VER,0x01,0x02,
                      0x03,0x01
                     };
UINT8C CfgDesc[41] ={
    0x09,0x02,0x29,0x00,0x01,0x01,0x04,0x80,0x32,             //����������
    0x09,0x04,0x00,0x00,0x02,0x03,0x00,0x00,0x00,             //�ӿ�������,HID
    0x09,0x21,0x00,0x01,0x00,0x01,0x22,0x25,0x00,             //HID��������
    0x07,0x05,0x82,0x03,DEF_BUF_LENGTH,0x00,0x01,             //�˵�������
    0x07,0x05,0x02,0x03,DEF_BUF_LENGTH,0x00,0x01,             //�˵�������
};
/*�ַ���������*/
// ����������
UINT8C  LangDesc[] = { 0x04, 0x03, 0x09, 0x04 };
UINT8C ProDesc[28] ={                                         //��Ʒ��Ϣ�ַ���������
    0x1C,0x03,'H',0x00,'I',0x00,'D',0x00,' ',0x00,
    'T',0x00,'o',0x00,' ',0x00,'S',0x00,'e',0x00,
    'r',0x00,'i',0x00,'a',0x00,'l',0x00
};
UINT8C ManuDesc[18] ={                                        //������Ϣ�ַ���������
    0x12,0x03,'W',0x00,'C',0x00,'H',0x00,'.',0x00,
    'C',0x00,'N',0x00,' ',0x00,DEF_THIS_IC_VER,0x00
};
UINT8C SNDesc[18] ={                                          //�豸���к���Ϣ�ַ���������
    0x12,0x03,'1',0x00,'2',0x00,'3',0x00,'4',0x00,
    '5',0x00,'6',0x00,'7',0x00,'8',0x00
};
/*HID�౨��������*/
UINT8C HIDRepDesc[37] =
{
    0x06, 0xA0,0xff,
    0x09, 0x01,
    0xa1, 0x01,                                                   //���Ͽ�ʼ
    0x09, 0x01,                                                   //Usage Page  �÷�
    0x15, 0x00,                                                   //Logical  Minimun
    0x26, 0xFF,0x00,                                              //Logical  Maximun
    0x75, 0x08,                                                   //Report Size
    0x95, DEF_BUF_LENGTH,                                         //Report Counet
    0x81, 0x02,                                                   //Input
    0x09, 0x02,                                                   //Usage Page  �÷�
    0x75, 0x08,                                                   //Report Size
    0x95, DEF_BUF_LENGTH,                                         //Report Counet
    0x91, 0x02,                                                   //Output
    0x09, 0x03,
    0x75, 0x08,
    0x95, 0x05,
    0xB1, 0x02,
    0xC0
};
UINT8X UserEp2Buf[64] _at_ 0x0090;
/*******************************************************************************
* Function Name  : USBDeviceCfg()
* Description    : USB�豸ģʽ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceCfg()
{
    USB_CTRL = 0x00;                                                           //���USB���ƼĴ���
    USB_CTRL &= ~bUC_HOST_MODE;                                                //��λΪѡ���豸ģʽ
    USB_CTRL |=  bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;                    //USB�豸���ڲ�����ʹ��,���ж��ڼ��жϱ�־δ���ǰ�Զ�����NAK
    USB_DEV_AD = 0x00;                                                         //�豸��ַ��ʼ��
    UDEV_CTRL &= ~bUD_RECV_DIS;                                                //ʹ�ܽ�����
//     USB_CTRL |= bUC_LOW_SPEED;
//     UDEV_CTRL |= bUD_LOW_SPEED;                                                //ѡ�����1.5Mģʽ
    USB_CTRL &= ~bUC_LOW_SPEED;
    UDEV_CTRL &= ~bUD_LOW_SPEED;                                             //ѡ��ȫ��12Mģʽ��Ĭ�Ϸ�ʽ
    UDEV_CTRL |= bUD_DP_PD_DIS | bUD_DM_PD_DIS;                                //��ֹDM��DP��������
    UDEV_CTRL |= bUD_PORT_EN;                                                  //ʹ�������˿�
}
/*******************************************************************************
* Function Name  : USBDeviceIntCfg()
* Description    : USB�豸ģʽ�жϳ�ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceIntCfg()
{
    USB_INT_EN |= bUIE_SUSPEND;                                               //ʹ���豸�����ж�
    USB_INT_EN |= bUIE_TRANSFER;                                              //ʹ��USB��������ж�
    USB_INT_EN |= bUIE_BUS_RST;                                               //ʹ���豸ģʽUSB���߸�λ�ж�
    USB_INT_FG |= 0x1F;                                                       //���жϱ�־
    IE_USB = 1;                                                               //ʹ��USB�ж�
    EA = 1;                                                                   //������Ƭ���ж�
}
/*******************************************************************************
* Function Name  : USBDeviceEndPointCfg()
* Description    : USB�豸ģʽ�˵����ã�ģ�����HID�豸�����˶˵�0�Ŀ��ƴ��䣬�������˵�2�������´�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceEndPointCfg()
{
    UEP2_DMA = Ep2Buffer;                                                      //�˵�2���ݴ����ַ
    UEP2_3_MOD |= bUEP2_TX_EN;                                                 //�˵�2����ʹ��
    UEP2_3_MOD |= bUEP2_RX_EN;                                                 //�˵�2����ʹ��
    UEP2_3_MOD &= ~bUEP2_BUF_MOD;                                              //�˵�2�շ���64�ֽڻ�����
    UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;                 //�˵�2�Զ���תͬ����־λ��IN���񷵻�NAK��OUT����ACK
    UEP0_DMA = Ep0Buffer;                                                      //�˵�0���ݴ����ַ
    UEP4_1_MOD &= ~(bUEP4_RX_EN | bUEP4_TX_EN);                                //�˵�0��64�ֽ��շ�������
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;                                 //OUT���񷵻�ACK��IN���񷵻�NAK
}
/*******************************************************************************
* Function Name  : enp2BlukIn()
* Description    : USB�豸ģʽ�˵�2�������ϴ�
* Input          : UINT8 len
* Output         : None
* Return         : None
*******************************************************************************/
void enp2BlukIn(UINT8 len )
{
    Ep2Buffer[MAX_PACKET_SIZE] = len;
    memcpy( Ep2Buffer+MAX_PACKET_SIZE+1, UserEp2Buf, sizeof(UserEp2Buf));      //�����ϴ�����
    UEP2_T_LEN = sizeof(UserEp2Buf);                                           //�ϴ����ݳ���
    UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;                  //������ʱ�ϴ����ݲ�Ӧ��ACK
}
/*******************************************************************************
* Function Name  : DeviceInterrupt()
* Description    : CH559USB�жϴ�������
*******************************************************************************/
void    DeviceInterrupt( void ) interrupt INT_NO_USB using 1                      //USB�жϷ������,ʹ�üĴ�����1
{
    UINT8 len;
#if DEBUG
    printf("%02X ",(UINT16)USB_INT_FG);
#endif
    if(UIF_TRANSFER)                                                            //USB������ɱ�־
    {
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
        case UIS_TOKEN_IN | 2:                                                  //endpoint 2# �˵������ϴ�
            UEP2_T_LEN = 0;                                                    //Ԥʹ�÷��ͳ���һ��Ҫ���
//            UEP1_CTRL ^= bUEP_T_TOG;                                          //����������Զ���ת����Ҫ�ֶ���ת
            UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //Ĭ��Ӧ��NAK
            break;
        case UIS_TOKEN_OUT | 2:                                                 //endpoint 2# �˵������´�
//             if ( U_TOG_OK )                                                     // ��ͬ�������ݰ�������
//             {
//                 len = USB_RX_LEN;                                                 //�������ݳ��ȣ����ݴ�Ep2Buffer�׵�ַ��ʼ���
//                 for ( i = 0; i < len; i ++ )
//                 {
//                     Ep2Buffer[MAX_PACKET_SIZE+i] = Ep2Buffer[i] ^ 0xFF;           // OUT����ȡ����IN�ɼ������֤
//                 }
//                 UEP2_T_LEN = len;
//                 UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;         // �����ϴ�
//             }
            break;
        case UIS_TOKEN_SETUP | 0:                                                //SETUP����
            len = USB_RX_LEN;
            if(len == (sizeof(USB_SETUP_REQ)))
            {
                SetupLen = UsbSetupBuf->wLengthL;
                if(UsbSetupBuf->wLengthH || SetupLen > 0x7F )
                {
                    SetupLen = 0x7F;    // �����ܳ���
                }
                len = 0;                                                      // Ĭ��Ϊ�ɹ������ϴ�0����
                if ( ( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )/* ֻ֧�ֱ�׼���� */
                {
                    len = 0xFF;                                            // ����ʧ��
                    printf("ErrEp0ReqType=%02X\n",(UINT16)UsbSetupBuf->bRequestType);
                }
                else
                {
                    //��׼����
                    SetupReq = UsbSetupBuf->bRequest;
#if DEBUG
                    printf("REQ %02X ",(UINT16)SetupReq);
#endif
                    switch(SetupReq)                                                  //������
                    {
                    case USB_GET_DESCRIPTOR:
                        switch(UsbSetupBuf->wValueH)
                        {
                        case 1:                                                       //�豸������
                            pDescr = DevDesc;                                         //���豸�������͵�Ҫ���͵Ļ�����
                            len = sizeof(DevDesc);
                            break;
                        case 2:                                                        //����������													
                            pDescr = CfgDesc;                                          //���豸�������͵�Ҫ���͵Ļ�����
                            len = sizeof(CfgDesc);
                            break;
                        case 3:                                                        //�ַ���������
														switch( UsbSetupBuf->wValueL )
														{
														case 0:
																pDescr = (PUINT8)( &LangDesc[0] );
																len = sizeof( LangDesc );
																break;															
														case 1:
																pDescr = (PUINT8)( &ManuDesc[0] );
																len = sizeof( ManuDesc );
																break;
														case 2:
																pDescr = (PUINT8)( &ProDesc[0] );
																len = sizeof( ProDesc );
																break;
														case 3:
																pDescr = (PUINT8)( &SNDesc[0] );
																len = sizeof( SNDesc );
																break;
														default:
																len = 0xFF;  // ��֧�ֵ��ַ���������
																break;
														}
                            break;												
                        case 0x22:                                                     //����������
#if DEBUG
                            printf("RREQ %02X ",(UINT16)SetupReq);
#endif
                            pDescr = HIDRepDesc;                                       //����׼���ϴ�
                            len = sizeof(HIDRepDesc);
                            Ready = 1;                                                 //����и���ӿڣ��ñ�׼λӦ�������һ���ӿ�������ɺ���Ч
                            break;											
                        default:
                            len = 0xff;                                                //��֧�ֵ�������߳���
                            break;
                        }
                        if ( SetupLen > len )
                        {
                            SetupLen = len;    //�����ܳ���
                        }
                        len = SetupLen >= 8 ? 8 : SetupLen;                            //���δ��䳤��
                        memcpy(Ep0Buffer,pDescr,len);                                  //�����ϴ�����
                        SetupLen -= len;
                        pDescr += len;
                        break;
                    case USB_SET_ADDRESS:
                        SetupLen = UsbSetupBuf->wValueL;                              //�ݴ�USB�豸��ַ
                        break;
                    case USB_GET_CONFIGURATION:
                        Ep0Buffer[0] = UsbConfig;
                        if ( SetupLen >= 1 )
                        {
                            len = 1;
                        }
                        break;
                    case USB_SET_CONFIGURATION:
                        UsbConfig = UsbSetupBuf->wValueL;
                        break;
                    case 0x0A:
                        break;
                    case USB_CLEAR_FEATURE:                                            //Clear Feature
                        if ( ( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )// �˵�
                        {
                            switch( UsbSetupBuf->wIndexL )
                            {
                            case 0x82:
                                UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                                break;
                            case 0x81:
                                UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                                break;
                            case 0x02:
                                UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
                                break;
                            default:
                                len = 0xFF;                                         // ��֧�ֵĶ˵�
                                break;
                            }
                        }
                        else
                        {
                            len = 0xFF;                                                // ���Ƕ˵㲻֧��
                        }
                        break;
                    case USB_SET_FEATURE:                                          /* Set Feature */
                        if( ( UsbSetupBuf->bRequestType & 0x1F ) == 0x00 )                  /* �����豸 */
                        {
                            if( ( ( ( UINT16 )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
                            {
                                if( CfgDesc[ 7 ] & 0x20 )
                                {
                                    /* ���û���ʹ�ܱ�־ */
                                }
                                else
                                {
                                    len = 0xFF;                                        /* ����ʧ�� */
                                }
                            }
                            else
                            {
                                len = 0xFF;                                            /* ����ʧ�� */
                            }
                        }
                        else if( ( UsbSetupBuf->bRequestType & 0x1F ) == 0x02 )             /* ���ö˵� */
                        {
                            if( ( ( ( UINT16 )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x00 )
                            {
                                switch( ( ( UINT16 )UsbSetupBuf->wIndexH << 8 ) | UsbSetupBuf->wIndexL )
                                {
                                case 0x82:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* ���ö˵�2 IN STALL */
                                    break;
                                case 0x02:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* ���ö˵�2 OUT Stall */
                                    break;
                                case 0x81:
                                    UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* ���ö˵�1 IN STALL */
                                    break;
                                default:
                                    len = 0xFF;                                    /* ����ʧ�� */
                                    break;
                                }
                            }
                            else
                            {
                                len = 0xFF;                                      /* ����ʧ�� */
                            }
                        }
                        else
                        {
                            len = 0xFF;                                          /* ����ʧ�� */
                        }
                        break;
                    case USB_GET_STATUS:
                        Ep0Buffer[0] = 0x00;
                        Ep0Buffer[1] = 0x00;
                        if ( SetupLen >= 2 )
                        {
                            len = 2;
                        }
                        else
                        {
                            len = SetupLen;
                        }
                        break;
                    default:
                        len = 0xff;                                                    //����ʧ��
                        break;
                    }
                }
            }
            else
            {
                len = 0xff;                                                         //�����ȴ���
            }
            if(len == 0xff)
            {
                SetupReq = 0xFF;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;//STALL
            }
            else if(len <= 8)                                                       //�ϴ����ݻ���״̬�׶η���0���Ȱ�
            {
                UEP0_T_LEN = len;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//Ĭ�����ݰ���DATA1������Ӧ��ACK
#if DEBUG
                printf("S_U\n");
#endif
            }
            else
            {
                UEP0_T_LEN = 0;  //��Ȼ��δ��״̬�׶Σ�������ǰԤ���ϴ�0�������ݰ��Է�������ǰ����״̬�׶�
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//Ĭ�����ݰ���DATA1,����Ӧ��ACK
            }
            break;
        case UIS_TOKEN_IN | 0:                                                      //endpoint0 IN
            switch(SetupReq)
            {
            case USB_GET_DESCRIPTOR:
                len = SetupLen >= 8 ? 8 : SetupLen;                                 //���δ��䳤��
                memcpy( Ep0Buffer, pDescr, len );                                   //�����ϴ�����
                SetupLen -= len;
                pDescr += len;
                UEP0_T_LEN = len;
                UEP0_CTRL ^= bUEP_T_TOG;                                             //ͬ����־λ��ת
                break;
            case USB_SET_ADDRESS:
                USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            default:
                UEP0_T_LEN = 0;                                                      //״̬�׶�����жϻ�����ǿ���ϴ�0�������ݰ��������ƴ���
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            }
            break;
        case UIS_TOKEN_OUT | 0:  // endpoint0 OUT
            len = USB_RX_LEN;
            if(SetupReq == 0x09)
            {
                if(Ep0Buffer[0])
                {
                    printf("Light on Num Lock LED!\n");
                }
                else if(Ep0Buffer[0] == 0)
                {
                    printf("Light off Num Lock LED!\n");
                }
            }
            UEP0_T_LEN = 0;  //��Ȼ��δ��״̬�׶Σ�������ǰԤ���ϴ�0�������ݰ��Է�������ǰ����״̬�׶�
            UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_ACK;//Ĭ�����ݰ���DATA0,����Ӧ��ACK
            break;
        default:
            break;
        }
        UIF_TRANSFER = 0;                                                           //д0����ж�
    }
    if(UIF_BUS_RST)                                                                 //�豸ģʽUSB���߸�λ�ж�
    {
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        UEP1_CTRL = bUEP_AUTO_TOG | UEP_R_RES_ACK;
        UEP2_CTRL = bUEP_AUTO_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
        USB_DEV_AD = 0x00;
        UIF_SUSPEND = 0;
        UIF_TRANSFER = 0;
        UIF_BUS_RST = 0;                                                             //���жϱ�־
    }
    if (UIF_SUSPEND)                                                                 //USB���߹���/�������
    {
        UIF_SUSPEND = 0;
        if ( USB_MIS_ST & bUMS_SUSPEND )                                             //����
        {
#if DEBUG
            printf( "zz" );                                                             //˯��״̬
#endif
            while ( XBUS_AUX & bUART0_TX )
            {
                ;    //�ȴ��������
            }
            SAFE_MOD = 0x55;
            SAFE_MOD = 0xAA;
            WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO;                                     //USB����RXD0���ź�ʱ�ɱ�����
            PCON |= PD;                                                                 //˯��
            SAFE_MOD = 0x55;
            SAFE_MOD = 0xAA;
            WAKE_CTRL = 0x00;
        }
    }
    else {                                                                             //������ж�,�����ܷ��������
        USB_INT_FG = 0xFF;                                                             //���жϱ�־
//      printf("UnknownInt  N");
    }
}
main()
{
    UINT8 len = 5;
    CfgFsys( );                                                           //CH559ʱ��ѡ������
    mDelaymS(30);                                                         //�ϵ���ʱ,�ȴ��ڲ������ȶ�,�ؼ�	
    mInitSTDIO( );                                                        //����0,�������ڵ���
    printf("start ...\n");
    memset(UserEp2Buf,0,sizeof(UserEp2Buf));
    USBDeviceCfg();                                                       //ģ�����
    USBDeviceEndPointCfg();                                               //�˵�����
    USBDeviceIntCfg();                                                    //�жϳ�ʼ��
    UEP1_T_LEN = 0;                                                       //Ԥʹ�÷��ͳ���һ��Ҫ���
    UEP2_T_LEN = 0;                                                       //Ԥʹ�÷��ͳ���һ��Ҫ���
    FLAG = 0;
    Ready = 0;
    while(1)
    {
        if(Ready&& (Ep2InKey==0))
        {
            enp2BlukIn(len);                                              //Ҫ�ϴ������ݰ����ȣ�������64�ֽ�
            mDelaymS( 100 );
        }
        mDelaymS( 100 );                                                  //ģ�ⵥƬ����������
    }
}