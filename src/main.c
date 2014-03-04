#include <pebble.h>

Window *my_window;
TextLayer *time_layer;
TextLayer *base_layer;
Window *analog_window;
Layer *analog_layer;

int base = 10;

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed);
static void analog_window_config_provider(void *context);

//With help from: http://www.phanderson.com/C/baseprnt.html
char * convert_to_base(int num, int base) {
	
	char base_digits[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	
	int converted_number[64];
	int index = 0;
	
	while (num != 0) {
		converted_number[index] = num % base;
		num = num / base;
		++index;
	}
	
	//Print 0 for 0
	if (index == 0) {
		converted_number[0] = 0;
		index++;
	}
	//Print a leading 0 for one-digit numbers
	if (index == 1) {
		converted_number[1] = 0;
		index++;
	}
	
	/* now print the result in reverse order */
	--index; /* back up to the last entry in the array */
	char *result_string = malloc(8);
	int forwardIndex = 0;
	for ( ; index>=0; index--) {
		snprintf(&result_string[forwardIndex],2,"%c", base_digits[converted_number[index]]);
		forwardIndex++;
	}
	result_string[forwardIndex+1] = '\0';
	return result_string;
}

static void up_button_handler(ClickRecognizerRef recognizer, void *context) {
	base++;
	if (base==17) {
		base = 2;
	}
	time_t now = time(NULL);
	struct tm *current_time = localtime(&now);
	handle_second_tick(current_time, SECOND_UNIT);
}

static void down_button_handler(ClickRecognizerRef recognizer, void *context) {
	base--;
	if (base==1) {
		base = 16;
	}
	time_t now = time(NULL);
	struct tm *current_time = localtime(&now);
	handle_second_tick(current_time, SECOND_UNIT);
}

void analogLayer_update_callback(Layer *myLayer, GContext *ctx) {
		
	graphics_context_set_stroke_color(ctx, GColorWhite);
	
	GPoint center = GPoint(72,84);
	float radius = 60;
		
	graphics_draw_circle(ctx, center, radius);
	
	time_t now = time(NULL);
	struct tm *current_time = localtime(&now);

	
	//hour hand
	GPoint hourHand;
	const int16_t hourHandLength = radius*0.6;
	
	int32_t hour_angle = (TRIG_MAX_ANGLE * (((current_time->tm_hour % 12) * 6) + (current_time->tm_min / 10))) / (12 * 6);
	hourHand.x = (int16_t)(sin_lookup(hour_angle) * (int32_t)hourHandLength / TRIG_MAX_RATIO) + center.x;
	hourHand.y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)hourHandLength / TRIG_MAX_RATIO) + center.y;
		
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_draw_line(ctx, hourHand, center);
	
	
	//minute hand
	GPoint minuteHand;
	const int16_t minuteHandLength = radius*0.75;
	
	int32_t minute_angle = TRIG_MAX_ANGLE * current_time->tm_min / 60;
	minuteHand.x = (int16_t)(sin_lookup(minute_angle) * (int32_t)minuteHandLength / TRIG_MAX_RATIO) + center.x;
	minuteHand.y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)minuteHandLength / TRIG_MAX_RATIO) + center.y;
	
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_draw_line(ctx, minuteHand, center);
	
	
	//second hand
	GPoint secondHand;
	const int16_t secondHandLength = radius*0.9;
	
	int32_t second_angle = TRIG_MAX_ANGLE * (current_time->tm_sec % base) / base;
	secondHand.x = (int16_t)(sin_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.x;
	secondHand.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.y;

	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_draw_line(ctx, secondHand, center);
}

//Is this method actually necessary? What cleanup is done automatically?
static void analog_window_unload(Window *window) {
		
	layer_destroy(analog_layer);
	window_destroy(analog_window);
}

static void center_button_handler(ClickRecognizerRef recognizer, void *context) {
	
		analog_window = window_create();
		window_set_fullscreen(analog_window, true);
		window_stack_push(analog_window, true);
		window_set_background_color(analog_window, GColorBlack);
		window_set_click_config_provider(analog_window, (ClickConfigProvider) analog_window_config_provider);
	window_set_window_handlers(analog_window, (WindowHandlers) {
		.unload = analog_window_unload
	});
		
		analog_layer = layer_create(GRect(0, 20, 144, 168-20));
		layer_set_update_proc(analog_layer, analogLayer_update_callback);
		layer_add_child(window_get_root_layer(analog_window), analog_layer);
	
		layer_add_child(window_get_root_layer(analog_window), text_layer_get_layer(base_layer));
}

static void my_window_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)up_button_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)down_button_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)center_button_handler);
}

static void analog_window_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)up_button_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)down_button_handler);
}

void handle_init(void) {
	  my_window = window_create();
	  window_set_fullscreen(my_window, true);
	  window_stack_push(my_window, true);
	  window_set_background_color(my_window, GColorBlack);
		
	  window_set_click_config_provider(my_window, (ClickConfigProvider) my_window_config_provider);

	  time_layer = text_layer_create(GRect(0, 20, 144, 168-20));
	  text_layer_set_text_color(time_layer, GColorWhite);
	  text_layer_set_background_color(time_layer, GColorClear);
	  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	
	  base_layer = text_layer_create(GRect(0,0,144,20));
	  text_layer_set_text_color(base_layer, GColorWhite);
	  text_layer_set_background_color(base_layer, GColorClear);
	  text_layer_set_font(base_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	  text_layer_set_text_alignment(base_layer, GTextAlignmentLeft);
	
	  time_t now = time(NULL);
	  struct tm *current_time = localtime(&now);
	  handle_second_tick(current_time, SECOND_UNIT);
	  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
	
	  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(time_layer));
	  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(base_layer));
	  
}

void handle_deinit(void) {
	  text_layer_destroy(time_layer);
	  text_layer_destroy(base_layer);
	  window_destroy(my_window);
}

int main(void) {
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
	
	if (window_stack_contains_window(analog_window)) {
		layer_mark_dirty(analog_layer);
	}
	
	static char time_text[] = "000000:000000:000000";
	static char base_text[] = "Base 10";
	
	char *converted_hours = convert_to_base(tick_time->tm_hour,base);
	char *converted_minutes = convert_to_base(tick_time->tm_min,base);
	char *converted_seconds = convert_to_base(tick_time->tm_sec,base);
		
	snprintf(time_text,21,"%s\n%s\n%s", converted_hours, converted_minutes, converted_seconds);
	
	//Free the memory allocated in convert_to_base for the return string
	free(converted_hours);
	free(converted_minutes);
	free(converted_seconds);
	
	snprintf(base_text,8,"Base %d",base);
		
	//strftime(time_text, sizeof(time_text), "%T", tick_time); //This does the normal time
	text_layer_set_text(time_layer, time_text);
	text_layer_set_text(base_layer, base_text);
	
	//This is a crude hack. Why does popping the top window kill the base_layer?
	if (!layer_get_window(text_layer_get_layer(base_layer))) {
		layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(base_layer));
	}
	
}