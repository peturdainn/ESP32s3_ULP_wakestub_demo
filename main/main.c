#include <stdio.h>

#include "esp_check.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"

// deep sleep wakeup stuff
#include "esp_sleep.h"
#include "esp_attr.h"
#include "esp32s3/rom/rtc.h"
#include "esp32s3/rom/gpio.h"
#include "esp32s3/rom/ets_sys.h"
#include "soc/soc.h"
#include "soc/rtc.h"
#include "soc/rtc_periph.h"
#include "soc/sens_reg.h"
#include "soc/timer_group_reg.h"
#include "soc/timer_periph.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include "esp_rom_crc.h"
#include "hal\rtc_io_ll.h"
#include "bootloader_common.h"

#include "ulp_riscv.h"
#include "ulp_main.h"
extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

static void init_ulp_program(void)
{
    esp_err_t err = ulp_riscv_load_binary(ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start));
    ESP_ERROR_CHECK(err);

    err = ulp_riscv_run();
    ESP_ERROR_CHECK(err);
}


static const char RTC_IRAM_ATTR wakestubstring[] = "WAKESTUB %s";
static const char RTC_IRAM_ATTR wakestubstart[] = "start\n";
static const char RTC_IRAM_ATTR wakestubsleep[] = "sleep\n";
static const char RTC_IRAM_ATTR wakestubboot[] = "boot\n";
static const char RTC_IRAM_ATTR wakestubedge[] = "edge!\n";

// create some delay to get the bytes out of the UART
RTC_IRAM_ATTR void rtc_delay()
{
    uint32_t tmp = 100000;
    while(tmp--)
    {
        REG_WRITE(TIMG_WDTFEED_REG(0), 1);
    }
}

// wakestub running in RTC memory, called at wakeup before bootloader
static void RTC_IRAM_ATTR main_wakeup_stub(void)
{
    ets_printf(wakestubstring, wakestubstart);
    rtc_delay();

    bool sleep_not_boot = true; // modify based on whatever you monitor during deep sleep ;)

    // this is the default wake from deep sleep function
    // I once saw a recommendation to call the default function but I see no difference
    // so the below call can probably be left out
    esp_default_wake_deep_sleep();

    uint32_t wakeup_cause = REG_GET_FIELD(RTC_CNTL_SLP_WAKEUP_CAUSE_REG, RTC_CNTL_WAKEUP_CAUSE);

    if (wakeup_cause & RTC_COCPU_TRIG_EN)
    {
        ets_printf(wakestubstring, wakestubedge);
        rtc_delay();
    }
/*
    // enable this code if you need to trap execution problems from the ULP
    if (wakeup_cause & RTC_COCPU_TRAP_TRIG_EN)
    {
    }
*/

    // disable the pending ULP interrupt so we don't wakeup again from the event
    SET_PERI_REG_MASK(RTC_CNTL_INT_CLR_REG, RTC_CNTL_COCPU_INT_CLR);   
    
    // clear COCPU sw int trigger
    CLEAR_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_SW_INT_TRIGGER);

    if(sleep_not_boot)
    {
		ets_printf(wakestubstring, wakestubsleep);
		rtc_delay();

        // Go to sleep.
        CLEAR_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_SLEEP_EN);
        SET_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_SLEEP_EN);
        // A few CPU cycles may be necessary for the sleep to start...
        while(true);
    }
    else
    {
		ets_printf(wakestubstring, wakestubboot);
		rtc_delay();
		// to quit is to boot
    }
}


void app_main(void)
{
    printf("FULL BOOT\r\n");

    init_ulp_program(); // initialize low power coprocessor

    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    // Initialize selected GPIO as RTC IO, enable input, disable pullup and pulldown
    rtc_gpio_init(GPIO_NUM_0);
    rtc_gpio_set_direction(GPIO_NUM_0, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(GPIO_NUM_0);
    rtc_gpio_pullup_en(GPIO_NUM_0);
    rtc_gpio_hold_en(GPIO_NUM_0);

    esp_sleep_enable_ulp_wakeup();
    
    esp_set_deep_sleep_wake_stub(main_wakeup_stub);
    esp_deep_sleep_start();
	// goodnight
}


