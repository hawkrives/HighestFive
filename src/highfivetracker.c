
#include "pebble.h"

#define REPEAT_INTERVAL_MS 50
#define ACCEL_STEP_MS 50
// This is a custom defined key for saving our count field
#define NUM_HIGHFIVES_PKEY 1

// You can define defaults for values in persistent storage
#define NUM_HIGHFIVES_DEFAULT 0

static Window *s_main_window;
long long highestFive=0;
long long lastFive=0;
static long long averageFive=2000;
double hx;
double hy;
double hz;
/*static void data_handler(AccelData *data, uint32_t num_samples) {
 AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
 accel_service_peek(&accel);
  hx=accel.x;
  hy=accel.y;
  if (hx>highestFive || hy>highestFive){
    if (hx<hy){
      highestFive=hx;
    }
    else{
      highestFive=hy;
    }
  }
}
*/
static long long max(long long x, long long y , long long z){
  if (x<y){
    if (z<y){
      return y;
    } else if (z>y)
        return z;
  }
  if (y<x){
    if (z<x){
      return x;
    } else if (x<z)
      return z;
  }
  return z;
}
static TextLayer *s_acc_layer;

static ActionBarLayer *s_action_bar;
static TextLayer *s_header_layer, *s_body_layer, *s_label_layer;
static GBitmap *s_icon_plus, *s_icon_minus;

static int s_num_fives = NUM_HIGHFIVES_DEFAULT;

static void update_text() {
  static char s_body_text[18];
  snprintf(s_body_text, sizeof(s_body_text), "%u High Fives", s_num_fives);
  text_layer_set_text(s_body_layer, s_body_text);
}

static void data_handler(AccelData *data, uint32_t num_samples) {
  long long mx,my,mz;
  mx=max(data[0].x,data[1].x,data[2].x);
  my=max(data[0].y,data[1].y,data[2].y);
  mz=max(data[0].z,data[1].z,data[2].z);
  int acceleration_2 = data[0].x * data[0].x + data[0].y * data[0].y + data[0].z * data[0].z;
  int acceleration = acceleration_2;
  int next = (acceleration + acceleration_2 / acceleration) / 2;
  while (acceleration != next) {
    acceleration = next;
    next = (acceleration + acceleration_2 / acceleration) / 2;
  }
  if (acceleration>highestFive)
    highestFive=acceleration;
  long long temp=max(mx,my,mz);
  if (temp>highestFive)
    highestFive=temp;
  static char acc_text[32];
  lastFive=acceleration;
  snprintf(acc_text, sizeof(acc_text), "%5d", (int)highestFive);
  if (lastFive>averageFive){
    snprintf(acc_text, sizeof(acc_text), "Nice Five!");
      s_num_fives++;
      update_text();
      vibes_short_pulse();
  }
  text_layer_set_text(s_acc_layer, acc_text);;
}

static void increment_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_num_fives++;
  update_text();
}

static void decrement_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_num_fives <= 0) {
    // Keep the counter at zero
    return;
  }
  s_num_fives--;
  update_text();
}

static void click_config_provider(void *context) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP, REPEAT_INTERVAL_MS, increment_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, REPEAT_INTERVAL_MS, decrement_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);

  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP, s_icon_plus);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_icon_minus);

  int width = layer_get_frame(window_layer).size.w - ACTION_BAR_WIDTH - 3;

  s_header_layer = text_layer_create(GRect(4, 0, width, 60));
  text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_background_color(s_header_layer, GColorClear);
  text_layer_set_text(s_header_layer, "High Five Counter");
  layer_add_child(window_layer, text_layer_get_layer(s_header_layer));

  s_header_layer = text_layer_create(GRect(4, 36, width, 60));
  text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_background_color(s_header_layer, GColorClear);
  text_layer_set_text(s_header_layer, "highest five");
  layer_add_child(window_layer, text_layer_get_layer(s_header_layer));
 
   s_acc_layer = text_layer_create(GRect(4, 70, width-5, 60));
  text_layer_set_font(s_acc_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_acc_layer, "No data yet.");
  //text_layer_set_overflow_mode(s_acc_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_acc_layer));
      
  
  s_body_layer = text_layer_create(GRect(4, 115, width, 60));
  text_layer_set_font(s_body_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_background_color(s_body_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_body_layer));
  update_text();
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_header_layer);
  text_layer_destroy(s_body_layer);
  text_layer_destroy(s_label_layer);
  text_layer_destroy(s_acc_layer);
  action_bar_layer_destroy(s_action_bar);
}


static void init() {
  s_icon_plus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLUS);
  s_icon_minus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_MINUS);

  // Get the count from persistent storage for use if it exists, otherwise use the default
  s_num_fives = persist_exists(NUM_HIGHFIVES_PKEY) ? persist_read_int(NUM_HIGHFIVES_PKEY) : NUM_HIGHFIVES_DEFAULT;

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  
  uint32_t num_samples = 3;
  accel_data_service_subscribe(num_samples, data_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
  
}

static void deinit() {
  // Save the count into persistent storage on app exit
  accel_data_service_unsubscribe();
  persist_write_int(NUM_HIGHFIVES_PKEY, s_num_fives);

  window_destroy(s_main_window);

  gbitmap_destroy(s_icon_plus);
  gbitmap_destroy(s_icon_minus);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
