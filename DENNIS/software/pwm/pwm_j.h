int setup(int pw_incr_us, int hw);
void shutdown(void);
void set_loglevel(int level);

int init_channel(int channel, int subcycle_time_us);
int clear_channel(int channel);
int clear_channel_gpio(int channel, int gpio);
int print_channel(int channel);

int add_channel_pulse(int channel, int gpio, int width_start, int width);
char* get_error_message(void);
void set_softfatal(int enabled);

int is_setup(void);
int is_channel_initialized(int channel);
int get_pulse_incr_us(void);
int get_channel_subcycle_time_us(int channel);

#define DELAY_VIA_PWM   0

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_ERRORS 1
#define LOG_LEVEL_DEFAULT LOG_LEVEL_DEBUG

// Default subcycle time
#define SUBCYCLE_TIME_US_DEFAULT 20000

// Subcycle minimum. We kept seeing no signals and strange behavior of the RPi
#define SUBCYCLE_TIME_US_MIN 3000

// Default pulse-width-increment-granularity
#define PULSE_WIDTH_INCREMENT_GRANULARITY_US_DEFAULT 10
