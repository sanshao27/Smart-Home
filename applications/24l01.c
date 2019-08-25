#include "24l01.h"
#include "spi.h"

const char TX_ADDRESS[TX_ADR_WIDTH]={0xff,0xff,0xff,0xff,0xff}; //���͵�ַ
const char RX_ADDRESS[RX_ADR_WIDTH]={0xff,0xff,0xff,0xff,0xff}; //���͵�ַ

//���NRF24L01�޸�SPI1����
void NRF24L01_SPI_Init(void)
{
    __HAL_SPI_DISABLE(&SPI2_Handler);               //�ȹر�SPI2
    SPI2_Handler.Init.CLKPolarity=SPI_POLARITY_LOW; //����ͬ��ʱ�ӵĿ���״̬Ϊ�͵�ƽ
    SPI2_Handler.Init.CLKPhase=SPI_PHASE_1EDGE;     //����ͬ��ʱ�ӵĵ�1�������أ��������½������ݱ�����
    HAL_SPI_Init(&SPI2_Handler);
    __HAL_SPI_ENABLE(&SPI2_Handler);                //ʹ��SPI2
}

//��ʼ��24L01��IO��
int NRF24L01_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOB_CLK_ENABLE();			//����GPIOBʱ��
    __HAL_RCC_GPIOG_CLK_ENABLE();			//����GPIOGʱ��
    
	//GPIOB12��ʼ������:�������
    GPIO_Initure.Pin=GPIO_PIN_12; 			//PB12
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
    GPIO_Initure.Pull=GPIO_PULLUP;          //����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//����
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //��ʼ��

	//GPIOG7,8�������
    GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_8;	//PG7��8  //CS/CE
    HAL_GPIO_Init(GPIOG,&GPIO_Initure);     //��ʼ��
	
	//GPIOG6��������
//	GPIO_Initure.Pin=GPIO_PIN_6;			//PG6     //NRF_IRQ
//	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //����
//	HAL_GPIO_Init(GPIOG,&GPIO_Initure);     //��ʼ��
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);//PB12���1,��ֹSPI FLASH����NRF��ͨ��  //W25Q128��Ƭѡ�ź�
    
	SPI2_Init();    		                //��ʼ��SPI1
    NRF24L01_SPI_Init();                    //���NRF���ص��޸�SPI������	
	NRF24L01_CE=0; 			                //ʹ��24L01
	NRF24L01_CSN=1;			                //SPIƬѡȡ��	
	if(NRF24L01_Check()==0)
		  printf("NRF2401 is OK!\n");
	  else
		  printf("NRF2401 is failed to response!\n");	
  return 0;
}
//INIT_BOARD_EXPORT(NRF24L01_Init);
//���24L01�Ƿ����
//����ֵ:0���ɹ�;1��ʧ��	
uint8_t NRF24L01_Check(void)
{
	char buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	uint8_t i;
	SPI2_SetSpeed(SPI_BAUDRATEPRESCALER_8); //spi�ٶ�Ϊ10.5Mhz����24L01�����SPIʱ��Ϊ10Mhz,�����һ��û��ϵ��    	 
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5);//д��5���ֽڵĵ�ַ.	
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //����д��ĵ�ַ  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//���24L01����	
	return 0;		 //��⵽24L01
}	 	 
//SPIд�Ĵ���
//reg:ָ���Ĵ�����ַ
//value:д���ֵ
uint8_t NRF24L01_Write_Reg(uint8_t reg,uint8_t value)
{
	uint8_t status;	
   	NRF24L01_CSN=0;                 //ʹ��SPI����
  	status =SPI2_ReadWriteByte(reg);//���ͼĴ����� 
  	SPI2_ReadWriteByte(value);      //д��Ĵ�����ֵ
  	NRF24L01_CSN=1;                 //��ֹSPI����	   
  	return(status);       		    //����״ֵ̬
}
//��ȡSPI�Ĵ���ֵ
//reg:Ҫ���ļĴ���
uint8_t NRF24L01_Read_Reg(uint8_t reg)
{
	  uint8_t reg_val;	    
   	NRF24L01_CSN=0;             //ʹ��SPI����		
  	SPI2_ReadWriteByte(reg);    //���ͼĴ�����
  	reg_val=SPI2_ReadWriteByte(0XFF);//��ȡ�Ĵ�������
  	NRF24L01_CSN=1;             //��ֹSPI����		    
  	return(reg_val);            //����״ֵ̬
}	
//��ָ��λ�ö���ָ�����ȵ�����
//reg:�Ĵ���(λ��)
//*pBuf:����ָ��
//len:���ݳ���
//����ֵ,�˴ζ�����״̬�Ĵ���ֵ 
uint8_t NRF24L01_Read_Buf(uint8_t reg,char *pBuf,uint8_t len)
{
	uint8_t status,uint8_t_ctr;	       
  	NRF24L01_CSN=0;            //ʹ��SPI����
  	status=SPI2_ReadWriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬   	   
	for(uint8_t_ctr=0;uint8_t_ctr<len;uint8_t_ctr++)pBuf[uint8_t_ctr]=SPI2_ReadWriteByte(0XFF);//��������
  	NRF24L01_CSN=1;            //�ر�SPI����
  	return status;             //���ض�����״ֵ̬
}
//��ָ��λ��дָ�����ȵ�����
//reg:�Ĵ���(λ��)
//*pBuf:����ָ��
//len:���ݳ���
//����ֵ,�˴ζ�����״̬�Ĵ���ֵ
uint8_t NRF24L01_Write_Buf(uint8_t reg, char *pBuf, uint8_t len)
{
	uint8_t status,uint8_t_ctr;	    
	NRF24L01_CSN=0;             //ʹ��SPI����
  	status = SPI2_ReadWriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬
  	for(uint8_t_ctr=0; uint8_t_ctr<len; uint8_t_ctr++)//д������
        SPI2_ReadWriteByte(*pBuf++);  
  	NRF24L01_CSN=1;             //�ر�SPI����
  	return status;              //���ض�����״ֵ̬
}				   
//����NRF24L01����һ������
//txbuf:�����������׵�ַ
//����ֵ:�������״��
uint8_t NRF24L01_TxPacket(char *txbuf)
{
	uint8_t sta;
 	SPI2_SetSpeed(SPI_BAUDRATEPRESCALER_8); //spi�ٶ�Ϊ6.75Mhz��24L01�����SPIʱ��Ϊ10Mhz��   
	NRF24L01_CE=0;
  	NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//д���ݵ�TX BUF  32���ֽ�
 	NRF24L01_CE=1;                          //��������	   
	while(NRF24L01_IRQ!=0);                 //�ȴ��������
	sta=NRF24L01_Read_Reg(STATUS);          //��ȡ״̬�Ĵ�����ֵ	   
	printf("sta :0x%x\n",sta);
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	if(sta&MAX_TX)                          //�ﵽ����ط�����
	{
		NRF24L01_Write_Reg(FLUSH_TX,0xff);  //���TX FIFO�Ĵ��� 
		return MAX_TX; 
	}
	if(sta&TX_OK)                           //�������
	{
		return TX_OK;
	}
	return 0xff;//����ԭ����ʧ��
}
//����NRF24L01����һ������
//rxbuf:�������׵�ַ
//����ֵ:0��������ɣ��������������
uint8_t NRF24L01_RxPacket(char *rxbuf)
{
	uint8_t sta;		    							   
	SPI2_SetSpeed(SPI_BAUDRATEPRESCALER_8); //spi�ٶ�Ϊ6.75Mhz��24L01�����SPIʱ��Ϊ10Mhz��   
	sta=NRF24L01_Read_Reg(STATUS);          //��ȡ״̬�Ĵ�����ֵ    	 
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	if(sta&RX_OK)//���յ�����
	{
		NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//��ȡ����
		NRF24L01_Write_Reg(FLUSH_RX,0xff);  //���RX FIFO�Ĵ��� 
		return 0; 
	}	   
	return 1;//û�յ��κ�����
}					    
//�ú�����ʼ��NRF24L01��RXģʽ
//����RX��ַ,дRX���ݿ��,ѡ��RFƵ��,�����ʺ�LNA HCURR
//��CE��ߺ�,������RXģʽ,�����Խ���������		   
void NRF24L01_RX_Mode(void)
{
	NRF24L01_CE=0;	  
  	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(char*)RX_ADDRESS,RX_ADR_WIDTH);//дRX�ڵ��ַ
  	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);       //ʹ��ͨ��0���Զ�Ӧ��    
  	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01);   //ʹ��ͨ��0�Ľ��յ�ַ  	 
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,0);	        //����RFͨ��Ƶ��		  
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//ѡ��ͨ��0����Ч���ݿ�� 	    
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);    //����TX�������,0db����,2Mbps,���������濪��   
  	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0f);     //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ 
  	NRF24L01_CE=1; //CEΪ��,�������ģʽ 
}						 
//�ú�����ʼ��NRF24L01��TXģʽ
//����TX��ַ,дTX���ݿ��,����RX�Զ�Ӧ��ĵ�ַ,���TX��������,ѡ��RFƵ��,�����ʺ�LNA HCURR
//PWR_UP,CRCʹ��
//��CE��ߺ�,������TXģʽ,�����Խ���������		   
//CEΪ�ߴ���10us,����������.	 
void NRF24L01_TX_Mode(void)
{														 
	NRF24L01_CE=0;	    
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(char*)TX_ADDRESS,TX_ADR_WIDTH);//дTX�ڵ��ַ 
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(char*)RX_ADDRESS,RX_ADR_WIDTH); //����TX�ڵ��ַ,��ҪΪ��ʹ��ACK	  
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);     //ʹ��ͨ��0���Զ�Ӧ��    
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01); //ʹ��ͨ��0�Ľ��յ�ַ  
	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_RETR,0x1a);//�����Զ��ط����ʱ��:500us + 86us;����Զ��ط�����:10��
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,0);       //����RFͨ��Ϊ0
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);  //����TX�������,0db����,2Mbps,���������濪��   
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,0x0e);    //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ,���������ж�
	NRF24L01_CE=1;//CEΪ��,10us����������
}



//NRF24L01_TX_Mode();	
//uint8_t key_value = 0;
////char buff[] = {" 2                             "};
//char rx_buff[32];

//while(1)
//{
//    key_value = Key_Scan(0);
//	if(key_value!=0)
//	{
//		rt_kprintf("key_value:%d\n",key_value);
//		//����Ϊ����ģʽ
//		NRF24L01_CE=0;
//		NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0e);
//		NRF24L01_CE=1;
//		switch(key_value)
//		{
//				case UP:
//				{
//					if(NRF24L01_TxPacket(" 1                              ")==TX_OK)
//					printf("sent ok!\n");
//					else
//					printf("sent failed!\n");
//					break;
//					}
//				case LEFT:
//				{
//					if(NRF24L01_TxPacket(" 9                              ")==TX_OK)
//					printf("sent ok!\n");
//					else
//					printf("sent failed!\n");
//					break;
//				}
//				case SELECT:
//				{
//					if(NRF24L01_TxPacket(" 7                              ")==TX_OK)
//					printf("sent ok!\n");
//					else
//					printf("sent failed!\n");
//					break;
//				}
//				case RIGHT:
//				{
//					if(NRF24L01_TxPacket(" 8                              ")==TX_OK)
//					printf("sent ok!\n");
//					else
//					printf("sent failed!\n");
//					break;
//				}
//				default:break;
//		}
//	   //��������Ϊ����ģʽ
//	   NRF24L01_CE=0;
//	   NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0f); 	
//	   NRF24L01_CE=1;    
//	  }
//    //�ȴ���ѯNRFģ���Ƿ��յ�����
//	if(NRF24L01_IRQ==0)
//	{
//		if(NRF24L01_RxPacket(rx_buff)==0)
//		printf("data :%s\n",rx_buff);
//		LCD_Clear(WHITE);
//		LCD_ShowString(30,80,200,12,12,rx_buff);
//		RED_LED   = !RED_LED;
//	}  
//}
