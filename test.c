#include <string.h>
#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
//#include "version.h"
//#include "tools.h"

// Uncomment to be able to make a screenshot
//#define USB_HID_AUTOFIRE_SCREENSHOT

    char All[36] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','1','2','3','4','5','6','7','8','9','0'};


typedef enum {
    EventTypeInput,
} EventType;

typedef struct {
    union {
        InputEvent input;
    };
    EventType type;
} UsbMouseEvent;

bool btn_left_autofire = false;
bool btn_right_autofire = false;
bool btn_select = false;
bool ison = false;
int btn_sel = 0; //     0 = off     1 = right     -1 = left
int testtext = 3
uint32_t autofire_delay = 10;

static void usb_hid_autofire_render_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    char autofire_delay_str[12];
    //std::string pi = "pi is " + std::to_string(3.1415926);
    itoa(autofire_delay, autofire_delay_str, 10);
    //sprintf(autofire_delay_str, "%lu", autofire_delay);


    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
 //   canvas_draw_str(canvas, 0, 10, "USB HID RightClick"); //16 charecters long
 //   canvas_draw_str(canvas, 0, 34, btn_left_autofire ? "<left>" : "<right>");
  //  canvas_draw_str(canvas, 50, 34, ison ? "<active>" : "<inactive>");

//    canvas_set_font(canvas, FontSecondary);
 //   canvas_draw_str(canvas, 100, 10, "v");
    canvas_draw_str(canvas, 106, 10, testtext);
 //   canvas_draw_str(canvas, 0, 22, "Press [ok] for auto clicking");
 //   canvas_draw_str(canvas, 0, 45, "delay [ms]:               [down] = off");
 //   canvas_draw_str(canvas, 0, 54, "up = switch left/right");
   // canvas_draw_str(canvas, 50, 46, autofire_delay_str);
    //canvas_draw_str(canvas, 0, 63, "Press [back] to exit");
}

static void usb_hid_autofire_input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;

    UsbMouseEvent event;
    event.type = EventTypeInput;
    event.input = *input_event;
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

int32_t usb_hid_autofire_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(UsbMouseEvent));
    furi_check(event_queue);
    ViewPort* view_port = view_port_alloc();

    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
#ifndef USB_HID_AUTOFIRE_SCREENSHOT
    furi_hal_usb_unlock();
    furi_check(furi_hal_usb_set_config(&usb_hid, NULL) == true);
#endif

    view_port_draw_callback_set(view_port, usb_hid_autofire_render_callback, NULL);
    view_port_input_callback_set(view_port, usb_hid_autofire_input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    UsbMouseEvent event;
    while(1) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 50);

        if(event_status == FuriStatusOk) {
            if(event.type == EventTypeInput) {
                if(event.input.key == InputKeyBack) {
                    break;
                }

                if(event.input.type != InputTypeRelease) {
                    continue;
                }

                switch(event.input.key) {
                    case InputKeyOk:
                       // btn_left_autofire = !btn_left_autofire;
                       // btn_right_autofire = false;
                        if(btn_sel == 0) {
                            btn_sel = -1; 
                            btn_left_autofire = true; 
                            ison = true;             
                        }
                        break;
                    case InputKeyUp: 
                        btn_sel = (btn_sel * -1); // 1 = right       2 = left
                        if(btn_sel == -1) {
                            btn_right_autofire = false;
                            btn_left_autofire = true;
                            ison = true;
                        }
                        if(btn_sel == 1) {
                            btn_left_autofire = false;
                            btn_right_autofire = true;
                            ison = true;
                        }
                        break; 
                        case InputKeyDown: 
                        btn_sel = 0; 
                        btn_right_autofire = false;
                        btn_left_autofire = false;
                        ison = false;
                        break;
                    case InputKeyLeft:
                        if(autofire_delay > 0) {
                            autofire_delay -= 5;
                        }
                        break;
                    case InputKeyRight:
                        autofire_delay += 5;
                        break;
                    default:
                        break;
                }
            }
        }

        if(btn_left_autofire) {
            furi_hal_hid_mouse_press(HID_MOUSE_BTN_LEFT);
            // TODO: Don't wait, but use the timer directly to just don't send the release event (see furi_hal_cortex_delay_us)
            furi_delay_us(autofire_delay * 500);
            furi_hal_hid_mouse_release(HID_MOUSE_BTN_LEFT);
            furi_delay_us(autofire_delay * 500);
        }
        if(btn_right_autofire) {
            furi_hal_hid_mouse_press(HID_MOUSE_BTN_RIGHT);
            // TODO: Don't wait, but use the timer directly to just don't send the release event (see furi_hal_cortex_delay_us)
            furi_delay_us(autofire_delay * 500);
            furi_hal_hid_mouse_release(HID_MOUSE_BTN_RIGHT);
            furi_delay_us(autofire_delay * 500);
        }

        view_port_update(view_port);
    }

    furi_hal_usb_set_config(usb_mode_prev, NULL);

    // remove & free all stuff created by app
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    return 0;
}
