
// Include this to enable the M5 global instance.
#include <M5Unified.h>

#include "iot_knob.h"
#include "iot_button.h"

#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "lv_example_pub.h"

static const char *TAG = "main";

static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

uint32_t startTime, frame = 0;  // For frames-per-second estimate
char info_temp[100] = {0};

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    auto pos     = M5.Touch.getTouchPointRaw();
    bool touched = (pos.x == -1) ? false : true;
    if (!touched) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        data->state = LV_INDEV_STATE_PR;
        /*Set the coordinates*/
        data->point.x = pos.x;
        data->point.y = pos.y;
    }
}

/*Read the touchpad*/
void my_encoder_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    static bool but_flag = true;
    lv_indev_state_t button_status;
    int32_t encoder_diff = 0;
    M5.update();
    if (M5.BtnA.wasPressed()) {
        button_status = LV_INDEV_STATE_PRESSED;  // 按下
    } else {
        button_status = LV_INDEV_STATE_RELEASED;  // 松开
    }

    // printf("btn %d\r\n", button_status);
    // int KEY_VAL = enc.value();
    // if (KEY_VAL < 0)  // 编码器左转
    // {
    //     encoder_diff--;
    //     but_flag = false;
    // } else if ((KEY_VAL > 0))  // 编码器右转
    // {
    //     encoder_diff++;
    //     but_flag = false;
    // }
    data->enc_diff = encoder_diff;
    data->state    = button_status;
}

//=====================================================================
/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                   lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    M5.Display.startWrite();
    M5.Display.setAddrWindow(area->x1, area->y1, w, h);
    M5.Display.pushColors(&color_p->full, w * h);
    M5.Display.endWrite();
    lv_disp_flush_ready(disp_drv);
}

typedef enum {
    BSP_BTN_PRESS = (GPIO_NUM_42),
} bsp_button_t;

#define BSP_ENCODER_A (GPIO_NUM_40)
#define BSP_ENCODER_B (GPIO_NUM_41)

// static lv_group_t *group;

static void event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        printf("Clicked");

    } else if (code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
        printf("Toggled");

    } else if (code == LV_EVENT_PRESSED) {
        LV_LOG_USER("LV_EVENT_PRESSED");
        printf("LV_EVENT_PRESSED");
    }
}

lv_obj_t *label;
lv_obj_t *btn1;
lv_obj_t *btn2;

extern "C" {

void app_main() {
    ESP_LOGI(TAG, "Compile time: %s %s", __DATE__, __TIME__);

    auto cfg = M5.config();
    // begin M5Unified.
    M5.begin(cfg);

    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL,
                          screenWidth * screenHeight / 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res  = screenWidth;
    disp_drv.ver_res  = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_t *disp   = lv_disp_drv_register(&disp_drv);

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    button_config_t bsp_encoder_btn_config;

    bsp_encoder_btn_config.type                            = BUTTON_TYPE_GPIO;
    bsp_encoder_btn_config.gpio_button_config.active_level = false;
    bsp_encoder_btn_config.gpio_button_config.gpio_num     = BSP_BTN_PRESS;

    knob_config_t bsp_encoder_a_b_config;

    bsp_encoder_a_b_config.default_direction = 0;
    bsp_encoder_a_b_config.gpio_encoder_a    = BSP_ENCODER_A;
    bsp_encoder_a_b_config.gpio_encoder_b    = BSP_ENCODER_B;

    const lvgl_port_encoder_cfg_t encoder = {
        .disp          = disp,
        .encoder_a_b   = &bsp_encoder_a_b_config,
        .encoder_enter = &bsp_encoder_btn_config,
    };

    lvgl_port_add_encoder(&encoder);
    lv_create_home(&boot_Layer);

    lv_create_clock(&clock_screen_layer, TIME_ENTER_CLOCK_2MIN);
    lvgl_port_unlock();
}
}
