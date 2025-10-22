#include "new_thread0.h"
#include "lvgl.h"
#include "port/lv_port_disp.h"
#include "port/lv_port_indev.h"
#include "lv_demos.h"

/* New Thread entry function */
/* pvParameters contains TaskHandle_t */
void new_thread0_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED (pvParameters);
    fsp_err_t err;

    err = R_SCI_B_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    assert(FSP_SUCCESS == err);

    lv_init();

    lv_port_disp_init();

    lv_port_indev_init();

#if (1 == LV_USE_DEMO_BENCHMARK)
    lv_demo_benchmark();
#endif

#if (1 == LV_USE_DEMO_MUSIC)
    lv_demo_music();
#endif

#if (1 == LV_USE_DEMO_KEYPAD_AND_ENCODER)
    lv_demo_keypad_encoder();
#endif

#if (1 == LV_USE_DEMO_STRESS)
    lv_demo_stress();
#endif

#if (1 == LV_USE_DEMO_WIDGETS && 0 == LV_USE_DEMO_BENCHMARK)
    lv_demo_widgets();
#endif

    /* TODO: add your own code here */
    while (1)
    {
      lv_timer_handler();
      vTaskDelay (1);
    }
}
