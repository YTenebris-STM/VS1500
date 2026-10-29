#include "stm32f1xx.h"
#include "init.h"
#include "nrf24.h"
#include "config.h"

void delay(volatile uint32_t count) {
    while (count--) {
        __NOP();
    }
}

int main (void)
{
    INIT_System(); 

    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOC->CRH |= GPIO_CRH_MODE13_1; 

    GPIOC->BSRR = GPIO_BSRR_BR13;
    delay(2000000);
    GPIOC->BSRR = GPIO_BSRR_BS13;

    NRF24_Init();

    control_data->data[0] = 0xDE;

    while(1)
    {
        if(NRF24_ProcessPacket(control_data))
        {delay(2900000);
            GPIOC->BSRR = GPIO_BSRR_BR13;
            delay(100000);
            GPIOC->BSRR = GPIO_BSRR_BS13;
        }
    }
    
    /*
    if (test_val == 0x5A) {
        
        GPIOC->BSRR = GPIO_BSRR_BR13;
        delay(10000000);
    } else {
        for (uint8_t i = 0; i < 21; i++)
        {
            GPIOC->ODR ^= GPIO_ODR_ODR13;
            delay(300000);
        }
    }
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOC->CRH |= GPIO_CRH_MODE13_1; 

    while (1) {
        GPIOC->ODR ^= GPIO_ODR_ODR13;
        delay(1000000);
    }
    
    */
}
