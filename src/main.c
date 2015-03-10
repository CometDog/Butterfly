/* 
 Image source:
 http://digitalwideresource.deviantart.com/art/butterfly-png-277770024
*/

#include "pebble.h"

// Main window
static Window *s_main_window;

// Animation
static GBitmap *s_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static GBitmapSequence *s_sequence = NULL;

// Layer for time
static TextLayer *s_time_layer;

// Font
static GFont s_time_font;

// Sequence of animation
static void load_sequence();

// Handles updating time
static void update_time() {
  // Get time
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create buffer for time
  static char buffer[] = "00:00";

  // Write time as 24h or 12h format onto the buffer
  if(clock_is_24h_style() == true) {
    // 24h format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // 12h format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display on Time Layer
  text_layer_set_text(s_time_layer, buffer);
}

// Handles updating time when called. Also runs the animation when called.
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();  
  if( (units_changed & MINUTE_UNIT) != 0 ) {
        load_sequence();
    }
}

// Handles running the animation
static void timer_handler(void *context) {
  uint32_t next_delay;

  // Move to next APNG frame
  if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
    bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
    layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));

    // Delay for next frame
    app_timer_register(next_delay, timer_handler, NULL);
  } else {
    // Calls to tick_handler to check if minute changed to start again
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  }
}

// Handles the process of creating the animation
static void load_sequence() {
  // Free up some space
  if(s_sequence) {
    gbitmap_sequence_destroy(s_sequence);
    s_sequence = NULL;
  }
  if(s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }

  // Create the sequence
  s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_BUTTERFLY);

  // Apply sequence to GBitmap
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);

  // Start the animation
  app_timer_register(1, timer_handler, NULL);
}

// Loads the layers onto the main window
static void main_window_load(Window *window) {
  
  // Creates window_layer as root and sets its bounds
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create the font
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OPEN_SANS_28));

  // Applies animation layer to window_layer
  s_bitmap_layer = bitmap_layer_create(bounds);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
  
  // Create time layer. Add time font to layer. Apply it to window_layer
  s_time_layer = text_layer_create(GRect(25, 32, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Display the time immediately
  update_time();
  
  // Starts animation after everything is applied
  load_sequence();
  
}

// Unloads all layers
static void main_window_unload(Window *window) {
  
  // Destroy the animation
  bitmap_layer_destroy(s_bitmap_layer);
  
  // Unload the font
  fonts_unload_custom_font(s_time_font);
  
  // Destroy the time layer
  text_layer_destroy(s_time_layer);
}

// Initializes the main window
static void init() {
  s_main_window = window_create();
  
  // Applies background color to main window
  window_set_background_color(s_main_window, GColorBlack);
  window_set_fullscreen(s_main_window, true);
  
  // Set handlers to the main window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  
  // Show the window with animations enabled (true)
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

// Deinitializes the main window
static void deinit() {
  // Destroy the main window
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}