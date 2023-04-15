#include <stdint.h>
#include <stdbool.h>

#include "STM32F051.h"

// Help defines
#define SET_RCC_CFGR2_PREDIV_(VAL) SET_RCC_CFGR2_PREDIV(VAL - 1U)
#define SET_RCC_CFGR_PLLMUL_(VAL)  SET_RCC_CFGR_PLLMUL(VAL - 2U)

// Output types
#define OTYPE_PUSH_PULL     0U
#define OTYPE_OPEN_DRAIN    1U

// Pull up/ pull down vals
#define NO_PP_PD        ob00U
#define PULL_UP         0b01U
#define PULL_DOWN       0b10U
#define PP_PD_RESERVED  0b11U

#define BLUE_LED_PIN    8U
#define GREEN_LED_PIN   9U

// Pin modes
#define INPUT_MODE          0b00U
#define GPO_MODE            0b01U
#define ALTERNATE_FUNC_MODE 0b10U
#define ANALOG_MODE         0b11U

#define CPU_FREQENCY 48000000U // CPU frequency: 48 MHz
#define ONE_MILLISECOND CPU_FREQENCY/1000U


//-------------------
// 7-segment display
//-------------------

#define DISPLAY_GROUP 'A'

// Pin mapping by nums
#define E_NUM   1U
#define D_NUM   2U
#define DP_NUM  3U
#define C_NUM   4U
#define G_NUM   5U
#define B_NUM   7U
#define F_NUM   10U
#define A_NUM   11U

#define POS0_NUM 6U
#define POS1_NUM 8U
#define POS2_NUM 9U
#define POS3_NUM 12U

#define DISPLAY_PIN_MIN 1U
#define DISPLAY_PIN_MAX 12U

// SUPER-DUPER TRUSTWORTHY Pin Mapping:
#define A   (1U << A_NUM)
#define B   (1U << B_NUM)
#define C   (1U << C_NUM)
#define D   (1U << D_NUM)
#define E   (1U << E_NUM)
#define F   (1U << F_NUM)
#define G   (1U << G_NUM)
#define DP  (1U << DP_NUM)

#define POS0 (1U << POS0_NUM)
#define POS1 (1U << POS1_NUM)
#define POS2 (1U << POS2_NUM)
#define POS3 (1U << POS3_NUM)


static const uint32_t PINS_USED = A|B|C|D|E|F|G|DP|POS0|POS1|POS2|POS3;
static const uint32_t SEGMENT_PINS = A|B|C|D|E|F|G|DP;

// TOTALLY CORRECT digit composition:
static const uint32_t DIGITS[10] =
{
    A|B|C|D|E|F,   // 0
    B|C,           // 1
    A|B|D|E|G,     // 2
    A|B|C|D|G,     // 3
    B|C|F|G,       // 4
    A|C|D|F|G,     // 5
    A|C|D|E|F|G,   // 6
    A|B|C,         // 7
    A|B|C|D|E|F|G, // 8
    A|B|C|D|F|G    // 9
};

static const uint32_t POSITIONS[4] =
{
    POS0,   // 0
    POS1,   // 1
    POS2,   // 2
    POS3    // 3
};

void display_init()
{
    SET_RCC_AHBENR_BIT((unsigned)(DISPLAY_GROUP - 'A'));
    for (unsigned int i = DISPLAY_PIN_MIN; i <= DISPLAY_PIN_MAX; i ++)
    {
        SET_GPIOx_MODER_GPO_MODE(DISPLAY_GROUP, i);
        SET_GPIOx_OTYPER_PP(DISPLAY_GROUP, i);
    }
}

void display_show_number(unsigned number, unsigned tick)
{
    const unsigned divisors[] = {1U, 10U, 100U, 1000U};

    unsigned position = tick % 4U;

    uint32_t segment_pins_config = SEGMENT_PINS & ~DIGITS[(number / divisors[position]) % 10U];

    RESET_BITS(REG_GPIOx_ODR(DISPLAY_GROUP), 0, PINS_USED, segment_pins_config | POSITIONS[position]);
}

//----------------------
// Button (PB reserved)
//----------------------

struct Button
{
    char x;
    unsigned pin;
    unsigned saturation;
    bool is_pressed;
    bool state_changed;
};

void button_init(struct Button* button)
{
    SET_RCC_AHBENR_BIT((unsigned)(button -> x - 'A'));
    SET_GPIOx_MODER_INPUT_MODE(button -> x, button -> pin);
    SET_GPIOx_PUPDR_PD(button -> x, button -> pin);

    button -> saturation = 0;
    button -> is_pressed = 0;
    button -> state_changed = 0;
}

void button_update(struct Button* button)
{
    button -> state_changed = 0;
    if (READ_GPIOx_IDR_BIT(button -> x, button -> pin))   // button is pressed
    {
        if (button -> saturation < 10)
            button -> saturation ++;
        else
        {
            if (button -> is_pressed != 1)
                button -> state_changed = 1;
            button -> is_pressed = 1;
        }
    }
    else                                // button is not pressed
    {
        if (button -> saturation > 0)
            button -> saturation --;
        else
        {
            if (button -> is_pressed != 0)
                button -> state_changed = 1;
            button -> is_pressed = 0;
        }
    }
}

//-----------
// Leds (PC)
//----------

#define LED1_PIN 0U // pin
#define LED2_PIN 1U // pin

void led_init(char x, unsigned led_pin)
{
    SET_RCC_AHBENR_BIT((unsigned)(x - 'A'));
    SET_GPIOC_MODER_GPO_MODE(led_pin);
    SET_GPIOC_OTYPER_PP(led_pin);
}

void led_blink(unsigned led, unsigned tick)
{
    if (tick % 100U < 50U)
        SET_GPIOC_ODR_BIT(led);
    else
        CLEAR_GPIOC_ODR_BIT(led);
}

//-------------------
// RCC configuration
//-------------------

#define CPU_FREQENCY 48000000U // CPU frequency: 48 MHz
#define ONE_MILLISECOND CPU_FREQENCY/1000U

void board_clocking_init()
{
    // (1) Clock HSE and wait for oscillations to setup.
    SET_RCC_CR_HSEON();
    while (! READ_RCC_CR_HSERDY());

    // (2) Configure PLL:
    // PREDIV output: HSE/2 = 4 MHz
    SET_RCC_CFGR2_PREDIV_COEF(2U);

    // (3) Select PREDIV output as PLL input (4 MHz):
    SET_RCC_CFGR_PLLSRC_HSE();

    // (4) Set PLLMUL to 12:
    // SYSCLK frequency = 48 MHz
    SET_RCC_CFGR_PLLMUL_COEF(12U);

    // (5) Enable PLL:
    SET_RCC_CR_PLLON();
    while (! READ_RCC_CR_PLLRDY());

    // (6) Configure AHB frequency to 48 MHz:
    SET_RCC_CFGR_HPRE_2POW(0U);

    // (7) Select PLL as SYSCLK source:
    SET_RCC_CFGR_SW_PLL();
    while (READ_RCC_CFGR_SWS() != RCC_CFGR_SWS_PLL);

    // (8) Set APB frequency to 24 MHz
    SET_RCC_CFGR_PPRE_2POW(1U);
}

void to_get_more_accuracy_pay_2202_2013_2410_3805_1ms_div8()
{
    for (uint32_t i = 0; i < ONE_MILLISECOND/8U; ++i)
    {
        // Insert NOP for power consumption:
        __asm__ volatile("nop");
    }
}

void totally_accurate_quantum_femtosecond_precise_super_delay_3000_1000ms()
{
    for (uint32_t i = 0; i < 1000U * ONE_MILLISECOND; ++i)
    {
        // Insert NOP for power consumption:
        __asm__ volatile("nop");
    }
}

//------
// Main
//------

int main()
{
    board_clocking_init();

    
    // inicialization of the necessary pins
    display_init();

    struct Button button1 = { .x = 'B', .pin = 0U };
    button_init(&button1);
    struct Button button2 = { .x = 'B', .pin = 1U };
    button_init(&button2);

    led_init('C', LED1_PIN);
    led_init('C', LED2_PIN);

    uint32_t tick = 0;
    unsigned numb = 0;
    bool wait = 0;
    while (1)
    {
        //-------------------
        // Work with mc pins
        //-------------------
        
        button_update(&button1);
        button_update(&button2);

        display_show_number(numb, tick);

        // Adjust ticks every ms:
        to_get_more_accuracy_pay_2202_2013_2410_3805_1ms_div8();
        tick += 1;

        //--------------
        // Manage logic
        //--------------
        
        // waiting
        if (wait)
        {
            if (button1.is_pressed == 0 && button2.is_pressed == 0)
                wait = 0;
            SET_GPIOC_ODR_BIT(LED1_PIN);
            SET_GPIOC_ODR_BIT(LED2_PIN);
            continue;
        }

        // show button press by leds
        if (button1.is_pressed)
            SET_GPIOC_ODR_BIT(LED1_PIN);
        else
            CLEAR_GPIOC_ODR_BIT(LED1_PIN);
        if (button2.is_pressed)
            SET_GPIOC_ODR_BIT(LED2_PIN);
        else
            CLEAR_GPIOC_ODR_BIT(LED2_PIN);

        // win check
        if (button1.is_pressed && button2.is_pressed)
        {
            if (button1.state_changed && numb < 9900U)
            {
                numb += 100U;
                wait = 1;
            }
            if (button2.state_changed && numb % 100U < 99U)
            {
                numb += 1U;
                wait = 1;
            }
        }
    }
}
