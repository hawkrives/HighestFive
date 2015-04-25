#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_text_layer;
static int max_acceleration_axes[3];
static int max_acceleration;

//static void tap_handler(AccelAxisType axis, int32_t direction) {
static void data_handler(AccelData *data, uint32_t num_samples) {
  static char buffer[] = "x:-0000\ny:-0000\nz:-0000\nM:-0000";
  int acceleration_2 = data[0].x * data[0].x + data[0].y * data[0].y + data[0].z * data[0].z;
  int acceleration = acceleration_2;
  int next = (acceleration + acceleration_2 / acceleration) / 2;
  while (acceleration != next) {
    acceleration = next;
    next = (acceleration + acceleration_2 / acceleration) / 2;
  }
  if (acceleration > max_acceleration) {
    max_acceleration = acceleration;
    max_acceleration_axes[0] = data[0].x;
    max_acceleration_axes[1] = data[0].y;
    max_acceleration_axes[2] = data[0].z;
  }
  snprintf(buffer, sizeof(buffer), "x:%5d\ny:%5d\nz:%5d\nM:%5d",
           data[0].x, data[0].y, data[0].z, max_acceleration);
  text_layer_set_text(s_text_layer, buffer);
}

static void main_window_load(Window *window) {
  s_text_layer = text_layer_create(GRect(0, 5, 144, 150));
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_text_color(s_text_layer, GColorBlack);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_text_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void init() {
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = main_window_unload
  });

  //accel_tap_service_subscribe(tap_handler);
  accel_data_service_subscribe(1, data_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);

  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  max_acceleration_axes[0] = -4000;
  max_acceleration_axes[1] = -4000;
  max_acceleration_axes[2] = -4000;
  max_acceleration = -4000;
  init();
  app_event_loop();
  deinit();
}
