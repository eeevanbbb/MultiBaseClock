#include <pebble.h>

Window *my_window;
TextLayer *time_layer;

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


void handle_init(void) {
	  my_window = window_create();
	  window_stack_push(my_window, true);
	  window_set_background_color(my_window, GColorBlack);

	  time_layer = text_layer_create(GRect(29, 54, 104, 114));
	  text_layer_set_text_color(time_layer, GColorWhite);
	  text_layer_set_background_color(time_layer, GColorClear);
	  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	
	  time_t now = time(NULL);
	  struct tm *current_time = localtime(&now);
	  handle_second_tick(current_time, SECOND_UNIT);
	  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
	
	  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(time_layer));
}

void handle_deinit(void) {
	  text_layer_destroy(time_layer);
	  window_destroy(my_window);
}

int main(void) {
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
	
	static char time_text[] = "000000:000000:000000";
	
	int base = 16;
	
	snprintf(time_text,21,"%s:%s:%s", convert_to_base(tick_time->tm_hour,base),
			 convert_to_base(tick_time->tm_min,base),
			 convert_to_base(tick_time->tm_sec,base));	
	
	//strftime(time_text, sizeof(time_text), "%T", tick_time); //This does the normal time
	text_layer_set_text(time_layer, time_text);
}
