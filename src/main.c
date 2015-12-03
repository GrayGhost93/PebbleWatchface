#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_SUNRISE 2
#define KEY_SUNSET 3

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_day_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_sun_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap;

static bool bluetooth_connected() {
  return connection_service_peek_pebble_app_connection();
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[9] = "----";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "lädt ...");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }  
  text_layer_set_text(s_battery_layer, battery_text);                              
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void update_date(){
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Write the buffers
  static char s_buffer[14];
  static char s_day[11];
  setlocale (LC_TIME,"de_DE");

  strftime(s_day, sizeof(s_day), "%A,", tick_time);
  strftime(s_buffer, sizeof(s_buffer), "%d. %B", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_day_layer, s_day);
  text_layer_set_text(s_date_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_date();
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
}

}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  battery_state_service_subscribe(handle_battery);
  
  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_BT_DC);
  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(GRect(5, 99, 134,3));
  s_icon_layer = bitmap_layer_create(GRect(94, 5, 40, 40));

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(GRect(0, 96, bounds.size.w, 49));
  s_date_layer = text_layer_create(GRect(5, 68, bounds.size.w, 28));
  s_day_layer = text_layer_create(GRect(5, 48, bounds.size.w, 28));
  s_weather_layer = text_layer_create(GRect(10, 5, bounds.size.w, 25));
  s_battery_layer = text_layer_create(GRect(-5, 148, bounds.size.w, 34));
  s_sun_layer = text_layer_create(GRect(5, 148, bounds.size.w, 14));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentLeft);
  
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentLeft);
  text_layer_set_text(s_weather_layer, "Lädt ...");
  
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text(s_battery_layer, "----");
  
  text_layer_set_background_color(s_sun_layer, GColorClear);
  text_layer_set_text_color(s_sun_layer, GColorWhite);
  text_layer_set_font(s_sun_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_sun_layer, GTextAlignmentLeft);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer)); 
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_sun_layer));
  bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));
  
  handle_battery(battery_state_service_peek());
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_sun_layer);
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  gbitmap_destroy(s_icon_bitmap);
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_icon_layer);
  // Destroy weather elements
  text_layer_destroy(s_weather_layer);
  battery_state_service_unsubscribe();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char weather_layer_buffer[32];
  static char sun_layer_buffer[15];
  
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);
  Tuple *sunrise_tuple = dict_find(iterator, KEY_SUNRISE);
  Tuple *sunset_tuple = dict_find(iterator, KEY_SUNSET);
  
  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d °C", (int)temp_tuple->value->int32);
    if(!bluetooth_connected()) {
      s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_BT_DC);
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
    }
    else if(conditions_tuple->value->int32 == 0) {
      s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_SUNNY);
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
    }
    else if(conditions_tuple->value->int32 == 1) {
      s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_CLOUDY);
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
    }
    else if(conditions_tuple->value->int32 == 2) {
      s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_RAINY);
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
    }
      
    else if(conditions_tuple->value->int32 == 3) {
      s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_SNOWY);
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
    }
    snprintf(sun_layer_buffer, sizeof(sun_layer_buffer), "%s - %s", sunrise_tuple->value->cstring, sunset_tuple->value->cstring);
  }
  
  // Assemble full string and display
  if(bluetooth_connected()) {
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", temperature_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
    snprintf(sun_layer_buffer, sizeof(sun_layer_buffer), "%s", sun_layer_buffer);
    text_layer_set_text(s_sun_layer, sun_layer_buffer);
  } else {
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "Getrennt!");
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
  }
  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  app_message_register_inbox_received(inbox_received_callback);
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  update_date();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}