// ULP-RISC-V code to set up a GPIO interrupt and then halt the ULP for extreme low power
// This code runs on the ULP-RISC-V coprocessor and is build separately

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ulp_riscv.h"
#include "ulp_riscv_utils.h"
#include "ulp_riscv_gpio.h"

// GPIO interrupt handler just wakes up the main processor
void gpio_int_handler(void *arg)
{
    ulp_riscv_wakeup_main_processor();
}

int main (void)
{
    ulp_riscv_gpio_init(GPIO_NUM_0);
    ulp_riscv_gpio_input_enable(GPIO_NUM_0);

	// configure rising edge interrupt for boot button (GPIO_0)
    ulp_riscv_gpio_isr_register(GPIO_NUM_0, ULP_RISCV_GPIO_INTR_POSEDGE, gpio_int_handler, NULL);

	// to quit is to halt
    return 0;

}