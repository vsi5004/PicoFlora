#ifndef __STEPPER_SCREEN_H__
#define __STEPPER_SCREEN_H__

#include "lvgl.h"

/**
 * Stepper Motor Control Screen
 * 
 * This screen provides:
 * - A slider to set the number of steps (1-10000)
 * - A button to start/stop the stepper motor
 * - A progress bar showing current progress
 * - Labels showing current step count and target steps
 */

// Screen elements
extern lv_obj_t *steps_slider;
extern lv_obj_t *start_stop_btn;
extern lv_obj_t *progress_bar;
extern lv_obj_t *current_steps_label;
extern lv_obj_t *target_steps_label;
extern lv_obj_t *status_label;

#include "stepper_driver.h"

// Configuration
#define MIN_STEPS 1
#define MAX_STEPS (STEPPER_STEPS_PER_REV * 10)  // Up to 10 full revolutions
#define DEFAULT_STEPS STEPPER_STEPS_PER_REV     // 1 full revolution (1600 steps with 1/8 microstepping)

// Function prototypes
void stepper_screen_create(void);
void stepper_screen_update_progress(void);
int32_t stepper_screen_get_target_steps(void);
void stepper_screen_set_status(const char* status);
void stepper_screen_set_button_text(const char* text);
lv_obj_t* stepper_screen_get_screen(void);

#endif // __STEPPER_SCREEN_H__
