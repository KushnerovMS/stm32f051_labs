#include <stdint.h>
#include "STM32F051.h"

//------
// Main
//------

#define SET_RCC_CFGR2_PREDIV_(VAL) SET_RCC_CFGR2_PREDIV(VAL - 1U)
#define SET_RCC_CFGR_PLLMUL_(VAL)  SET_RCC_CFGR_PLLMUL(VAL - 2U)

#define BLUE_LED_PIN    8U
#define GREEN_LED_PIN   9U

#define GPO_MODE 0b01U

#define CPU_FREQENCY 48000000U // CPU frequency: 48 MHz
#define ONE_MILLISECOND CPU_FREQENCY/1000U

void board_clocking_init()
{
    // (1) Clock HSE and wait for oscillations to setup.
    SET_RCC_CR_HSEON();
    while (READ_RCC_CR_HSERDY() != 1U);

    // (2) Configure PLL:
    // PREDIV output: HSE/2 = 4 MHz
    SET_RCC_CFGR2_PREDIV_(2U);

    // (3) Select PREDIV output as PLL input (4 MHz):
    SET_RCC_CFGR_PLLSRC(0b10U);

    // (4) Set PLLMUL to 12:
    // SYSCLK frequency = 48 MHz
    SET_RCC_CFGR_PLLMUL_(12U);

    // (5) Enable PLL:
    SET_RCC_CR_PLLON();
    while (READ_RCC_CR_PLLRDY() != 1U);

    // (6) Configure AHB frequency to 48 MHz:
    SET_RCC_CFGR_HPRE(0b000U);

    // (7) Select PLL as SYSCLK source:
    SET_RCC_CFGR_SW(0b10U);
    while (READ_RCC_CFGR_SWS() != 0b10U);

    // (8) Set APB frequency to 24 MHz
    SET_RCC_CFGR_PPRE(0b100U);
}

void board_gpio_init()
{
    // (1) Enable GPIOC clocking:
    SET_RCC_AHBENR_IOPCEN();


    // (2) Configure PC8 mode:
    SET_GPIOC_MODER_BITS(BLUE_LED_PIN, GPO_MODE);
    SET_GPIOC_MODER_BITS(GREEN_LED_PIN, GPO_MODE);

    // (3) Configure PC8 type:
    CLEAR_GPIOC_OTYPER_BIT(BLUE_LED_PIN);
    CLEAR_GPIOC_OTYPER_BIT(GREEN_LED_PIN);
}

void totally_accurate_quantum_femtosecond_precise_super_delay_3000_1000ms()
{
    for (uint32_t i = 0; i < 1000U * ONE_MILLISECOND; ++i)
    {
        // Insert NOP for power consumption:
        __asm__ volatile("nop");
    }
}

int main()
{
#ifndef INSIDE_QEMU
    board_clocking_init();
#endif

    board_gpio_init();

    while (1)
    {
        SET_GPIOC_ODR_BIT(BLUE_LED_PIN);
        totally_accurate_quantum_femtosecond_precise_super_delay_3000_1000ms();

        CLEAR_GPIOC_ODR_BIT(BLUE_LED_PIN);
        totally_accurate_quantum_femtosecond_precise_super_delay_3000_1000ms();

        SET_GPIOC_ODR_BIT(GREEN_LED_PIN);
        totally_accurate_quantum_femtosecond_precise_super_delay_3000_1000ms();

        CLEAR_GPIOC_ODR_BIT(GREEN_LED_PIN);
        totally_accurate_quantum_femtosecond_precise_super_delay_3000_1000ms();
    }
}
