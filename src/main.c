#include <pebble.h>

Window *my_window;
TextLayer *time_layer;
TextLayer *base_layer;

int base = 10;

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed);

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

static void window_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)up_button_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)down_button_handler);
}

void handle_init(void) {
	  my_window = window_create();
	  window_stack_push(my_window, true);
	  window_set_background_color(my_window, GColorBlack);
		
	  window_set_click_config_provider(my_window, (ClickConfigProvider) window_config_provider);

	  time_layer = text_layer_create(GRect(0, 0, 144, 168));
	  text_layer_set_text_color(time_layer, GColorWhite);
	  text_layer_set_background_color(time_layer, GColorClear);
	  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	
	  base_layer = text_layer_create(GRect(0,0,50,10));
	  text_layer_set_text_color(base_layer, GColorWhite);
	  text_layer_set_background_color(base_layer, GColorClear);
	  text_layer_set_font(base_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	  text_layer_set_text_alignment(base_layer, GTextAlignmentRight);
	
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
	
	static char time_text[] = "000000:000000:000000";
	static char base_text[] = "Base 10";
		
	snprintf(time_text,21,"%s\n%s\n%s", convert_to_base(tick_time->tm_hour,base),
			 convert_to_base(tick_time->tm_min,base),
			 convert_to_base(tick_time->tm_sec,base));
	
	snprintf(base_text,8,"Base %d",base);
	
	//strftime(time_text, sizeof(time_text), "%T", tick_time); //This does the normal time
	text_layer_set_text(time_layer, time_text);
	text_layer_set_text(base_layer, base_text);
	
}