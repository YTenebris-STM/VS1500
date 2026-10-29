#include "nrf24.h"
#include "config.h"

#define NSS_HIGH() GPIOA->BSRR = GPIO_BSRR_BS4
#define NSS_LOW() GPIOA->BSRR = GPIO_BSRR_BR4

#define CE_HIGH() GPIOB->BSRR = GPIO_BSRR_BS0
#define CE_LOW() GPIOB->BSRR = GPIO_BSRR_BR0

static uint8_t NRF24_Exchange (uint8_t data)
{
    while (!(SPI1->SR & SPI_SR_TXE));
    SPI1->DR = data;

    while (!(SPI1->SR & SPI_SR_RXNE));
    return SPI1->DR;
}

static void NRF24_Write (uint8_t reg, uint8_t data)
{
    NSS_LOW();
    NRF24_Exchange(0x20 | (reg & 0x1F));
    NRF24_Exchange(data);
    while (SPI1->SR & SPI_SR_BSY);
    NSS_HIGH();
}

static uint8_t NRF24_Read (uint8_t reg)
{
    NSS_LOW();
    NRF24_Exchange(reg & 0x1F);
    uint8_t result = NRF24_Exchange(0xFF);
    while (SPI1->SR & SPI_SR_BSY);
    NSS_HIGH();
    return result;
}

void NRF24_Init (void)
{
    #if defined(RADIO_MODE_TX) && defined(RADIO_MODE_RX)
        #error "ERROR: Both RADIO_MODE_TX and RADIO_MODE_RX are active"
    #endif

    // Защита от дурака: проверяем, что пользователь вообще выбрал режим
    #if !defined(RADIO_MODE_TX) && !defined(RADIO_MODE_RX)
        #error "ERROR: Neither RADIO_MODE_TX nor RADIO_MODE_RX is active"
    #endif

    CE_LOW();
    
    NRF24_Write(0x01, 0x00);
    NRF24_Write(0x04, 0x00);
    NRF24_Write(0x03, 0x01);
    NRF24_Write(0x05, 0x4C);
    NRF24_Write(0x06, 0x06);
    
    NSS_LOW();

    #ifdef RADIO_MODE_TX

    NRF24_Exchange(0x20 | 0x10);
    NRF24_Exchange(RADIO_ADDR & 0xFF);
    NRF24_Exchange((RADIO_ADDR >> 8) & 0xFF);
    NRF24_Exchange((RADIO_ADDR >> 16) & 0xFF);
    while (SPI1->SR & SPI_SR_BSY);
    NSS_HIGH();
    NRF24_Write(0x00, 0x0E);
    for (volatile uint32_t i = 0; i < 150000; i++) {__NOP();};

    #endif

    #ifdef RADIO_MODE_RX
    
    NRF24_Exchange(0x20 | 0x0A);
    NRF24_Exchange(RADIO_ADDR & 0xFF);
    NRF24_Exchange((RADIO_ADDR >> 8) & 0xFF);
    NRF24_Exchange((RADIO_ADDR >> 16) & 0xFF);
    while (SPI1->SR & SPI_SR_BSY);
    NSS_HIGH();
    
    NRF24_Write(0x02, 0x01);
    NRF24_Write(0x11, DATA_SIZE);
    NRF24_Write(0x00, 0x0F);
    for (volatile uint32_t i = 0; i < 150000; i++) {__NOP();};

    NSS_LOW();
    NRF24_Exchange(0xE2); // Команда FLUSH_RX
    while (SPI1->SR & SPI_SR_BSY);
    NSS_HIGH();
    
    CE_HIGH();

    #endif
    
}

static uint8_t NRF24_TX_Packet (ControlData* control_data)
{
    CE_LOW();
    //NRF24_Write(0x07, 0x70);
    //NRF24_Write(0x01, 0x00);  // EN_AA = 0
    //NRF24_Write(0x07, 0x30);  // Сброс TX_DS и MAX_RT
    
    NSS_LOW();
    NRF24_Exchange(0xE1);
    while (SPI1->SR & SPI_SR_BSY);
    NSS_HIGH();
    
    NSS_LOW();
    NRF24_Exchange(0xA0);
    for (uint8_t i = 0; i < *control_data->length; i++)
    {
        NRF24_Exchange(control_data->data[i]);
    }
    while (SPI1->SR & SPI_SR_BSY);
    NSS_HIGH();
    
    CE_HIGH();
    for (volatile uint32_t i = 0; i < 2000; i++) {__NOP();};
    CE_LOW();

    uint8_t status = NRF24_Read(0x07);

    if (status & (1 << 5))
    {
        NRF24_Write(0x07, (1 << 5));
        return 1;  // Передача завершена
    }

    if (status & (1 << 4))
    {
        NRF24_Write(0x07, (1 << 4));
        return 0;  // Ошибка передачи
    }

    return 0;
}

static uint8_t NRF24_RX_Packet (ControlData* control_data)
{
    if (NRF24_Read(0x07) & (1 << 6))
    {
        CE_LOW();
        
        NSS_LOW();
        NRF24_Exchange(0x61);
        for (uint8_t i = 0; i < *control_data->length; i++)
        {
            control_data->data[i] = NRF24_Exchange(0xFF);
        }
        while (SPI1->SR & SPI_SR_BSY);
        NSS_HIGH();
        
        NRF24_Write(0x07, (1 << 6));
        CE_HIGH();
        return 1;
    }
    return 0;
}

uint8_t NRF24_ProcessPacket (ControlData* control_data)
{
    #ifdef RADIO_MODE_TX
    return NRF24_TX_Packet(control_data);
    #endif

    #ifdef RADIO_MODE_RX
    return NRF24_RX_Packet(control_data);
    #endif
}

