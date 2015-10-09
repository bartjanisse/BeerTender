#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "pwm_j.h"

// 15 DMA channels are usable on the RPi (0..14)
#define DMA_CHANNELS    15

// Standard page sizes
#define PAGE_SIZE       4096
#define PAGE_SHIFT      12

// Define which Raspberry Pi board are you using. Take care to have defined only one at time.
#define RPI
//#define RPI2

#ifdef RPI
#define BCM2708_PERI_BASE       0x20000000
#define BCM2708_PHYS_BASE       0x7e000000
#define DRAM_PHYS_BASE          0x40000000
#endif

#ifdef RPI2
#define BCM2708_PERI_BASE       0x3F000000
#define BCM2708_PHYS_BASE       0x7e000000
#define DRAM_PHYS_BASE          0xc0000000
#endif	

// Memory Addresses
#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000)
#define DMA_BASE                (BCM2708_PERI_BASE + 0x007000)
#define PWM_BASE                (BCM2708_PERI_BASE + 0x20C000)
#define CLK_BASE                (BCM2708_PERI_BASE + 0x101000)

#define GPIO_PHYS               (BCM2708_PHYS_BASE + 0x200000)
#define PWM_PHYS                (BCM2708_PHYS_BASE + 0x20C000)


#define DMA_CHANNEL_INC 0x100
#define DMA_LEN         0x24
#define PWM_LEN         0x28
#define CLK_LEN         0xA8
#define GPIO_LEN        0x100

// Datasheet p. 51:
#define DMA_NO_WIDE_BURSTS  (1<<26)
#define DMA_WAIT_RESP   (1<<3)
#define DMA_D_DREQ      (1<<6)
#define DMA_PER_MAP(x)  ((x)<<16)
#define DMA_END         (1<<1)
#define DMA_RESET       (1<<31)
#define DMA_INT         (1<<2)

// Each DMA channel has 3 writeable registers:
#define DMA_CS          (0x00/4)
#define DMA_CONBLK_AD   (0x04/4)
#define DMA_DEBUG       (0x20/4)

// GPIO Memory Addresses
#define GPIO_FSEL0      (0x00/4) //(0x7E200000)
#define GPIO_SET0       (0x1c/4)
#define GPIO_CLR0       (0x28/4)
#define GPIO_LEV0       (0x34/4)
#define GPIO_PULLEN     (0x94/4)
#define GPIO_PULLCLK    (0x98/4)

// GPIO Modes (IN=0, OUT=1)
#define GPIO_MODE_IN    0
#define GPIO_MODE_OUT   1

// PWM Memory Addresses
#define PWM_CTL         (0x00/4)
#define PWM_DMAC        (0x08/4)
#define PWM_RNG1        (0x10/4)
#define PWM_FIFO        (0x18/4)

#define PWMCLK_CNTL     40
#define PWMCLK_DIV      41

#define PWMCTL_MODE1    (1<<1)
#define PWMCTL_PWEN1    (1<<0)
#define PWMCTL_CLRF     (1<<6)
#define PWMCTL_USEF1    (1<<5)

#define PWMDMAC_ENAB    (1<<31)
#define PWMDMAC_THRSHLD ((15<<8) | (15<<0))

// DMA Control Block Data Structure (p40): 8 words (256 bits)
typedef struct {
    uint32_t info;   // TI: transfer information
    uint32_t src;    // SOURCE_AD
    uint32_t dst;    // DEST_AD
    uint32_t length; // TXFR_LEN: transfer length
    uint32_t stride; // 2D stride mode
    uint32_t next;   // NEXTCONBK
    uint32_t pad[2]; // _reserved_
} dma_cb_t;

// Memory mapping
typedef struct {
    uint8_t *virtaddr;
    uint32_t physaddr;
} page_map_t;

// Main control structure per channel
struct channel {
    uint8_t *virtbase;
    uint32_t *sample;
    dma_cb_t *cb;
    page_map_t *page_map;
    volatile uint32_t *dma_reg;

    // Set by user
    uint32_t subcycle_time_us;

    // Set by system
    uint32_t num_samples;
    uint32_t num_cbs;
    uint32_t num_pages;

    // Used only for control purposes
    uint32_t width_max;
};

// One control structure per channel
static struct channel channels[DMA_CHANNELS];

// Pulse width increment granularity
static uint16_t pulse_width_incr_us = -1;
static uint8_t _is_setup = 0;
static int gpio_setup = 0; // bitfield for setup gpios (setup = out/low)

// Common registers
static volatile uint32_t *pwm_reg;
static volatile uint32_t *clk_reg;
static volatile uint32_t *gpio_reg;

// Defaults
static int delay_hw = DELAY_VIA_PWM;
static int log_level = LOG_LEVEL_DEFAULT;

// if set to 1, calls to fatal will not exit the program or shutdown DMA/PWM, but just sets
// the error_message and returns an error code. soft_fatal is enabled by default by the 
// python wrapper, in order to convert calls to fatal(..) to exceptions.
static int soft_fatal = 0;

// cache for a error message
static char error_message[256];

// Debug logging
void
set_loglevel(int level)
{
    log_level = level;
}

static void
log_debug(char* fmt, ...)
{
    if (log_level > LOG_LEVEL_DEBUG)
        return;

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

// Sets a GPIO to either GPIO_MODE_IN(=0) or GPIO_MODE_OUT(=1)
static void
gpio_set_mode(uint32_t pin, uint32_t mode)
{
    uint32_t fsel = gpio_reg[GPIO_FSEL0 + pin/10];

    fsel &= ~(7 << ((pin % 10) * 3));
    fsel |= mode << ((pin % 10) * 3);
    gpio_reg[GPIO_FSEL0 + pin/10] = fsel;
}

// Sets the gpio to input (level=1) or output (level=0)
static void
gpio_set(int pin, int level)
{
    if (level)
        gpio_reg[GPIO_SET0] = 1 << pin;
    else
        gpio_reg[GPIO_CLR0] = 1 << pin;
}

// Set GPIO to OUTPUT, Low
static void
init_gpio(int gpio)
{
    log_debug("init_gpio %d\n", gpio);
    gpio_set(gpio, 0);
    gpio_set_mode(gpio, GPIO_MODE_OUT);
    gpio_setup |= 1 << gpio;
}

// Very short delay as demanded per datasheet
static void
udelay(int us)
{
    struct timespec ts = { 0, us * 1000 };

    nanosleep(&ts, NULL);
}

// Shutdown -- its important to reset the DMA before quitting
void
shutdown(void)
{
    int i;

    for (i = 0; i < DMA_CHANNELS; i++) {
        if (channels[i].dma_reg && channels[i].virtbase) {
            log_debug("shutting down dma channel %d\n", i);
            clear_channel(i);
            udelay(channels[i].subcycle_time_us);
            channels[i].dma_reg[DMA_CS] = DMA_RESET;
            udelay(10);
        }
    }
}

// Terminate is triggered by signals
static void
terminate(void)
{
    shutdown();
    if (soft_fatal) {
        return;
    }
    exit(EXIT_SUCCESS);
}

// Shutdown with an error message. Returns EXIT_FAILURE for convenience.
// if soft_fatal is set to 1, a call to `fatal(..)` will not shut down
// PWM/DMA activity (used in the Python wrapper).
static int
fatal(char *fmt, ...)
{
    va_list ap;

    // Handle error
    va_start(ap, fmt);
    if (soft_fatal) {
        vsprintf (error_message, fmt, ap);
        return EXIT_FAILURE;
    }
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    // Shutdown all DMA and PWM activity
    shutdown();
    exit(EXIT_FAILURE);
}

// Catch all signals possible - it is vital we kill the DMA engine
// on process exit!
static void
setup_sighandlers(void)
{
    int i;
    for (i = 1; i < 32; i++) {
        // whitelist non-terminating signals
        if (i == SIGCHLD ||
            i == SIGCONT ||
            i == SIGTSTP ||
            i == SIGTTIN ||
            i == SIGTTOU ||
            i == SIGURG ||
            i == SIGWINCH ||
            i == SIGPIPE || // Python handles this
            i == SIGINT || // Python handles this
            i == SIGIO) {
            continue;
        }
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = (void *) terminate;
        sigaction(i, &sa, NULL);
    }
}

// Memory mapping
static uint32_t
mem_virt_to_phys(int channel, void *virt)
{
    uint32_t offset = (uint8_t *)virt - channels[channel].virtbase;
    return channels[channel].page_map[offset >> PAGE_SHIFT].physaddr + (offset % PAGE_SIZE);
}

// Peripherals memory mapping
static void *
map_peripheral(uint32_t base, uint32_t len)
{
    int fd = open("/dev/mem", O_RDWR);
    void * vaddr;

    if (fd < 0) {
        fatal("rpio-pwm: Failed to open /dev/mem: %m\n");
        return NULL;
    }
    vaddr = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, base);
    if (vaddr == MAP_FAILED) {
        fatal("rpio-pwm: Failed to map peripheral at 0x%08x: %m\n", base);
        return NULL;
    }
    close(fd);

    return vaddr;
}

// Returns a pointer to the control block of this channel in DMA memory
uint8_t*
get_cb(int channel)
{
    return channels[channel].virtbase + (sizeof(uint32_t) * channels[channel].num_samples);
}

// Reset this channel to original state (all samples=0, all cbs=clr0)
int
clear_channel(int channel)
{
    int i;
    uint32_t phys_gpclr0 = GPIO_PHYS + 0x28;
    dma_cb_t *cbp = (dma_cb_t *) get_cb(channel);
    uint32_t *dp = (uint32_t *) channels[channel].virtbase;

    log_debug("clear_channel: channel=%d\n", channel);
    if (!channels[channel].virtbase)
        return fatal("Error: channel %d has not been initialized with 'init_channel(..)'\n", channel);

    // First we have to stop all currently enabled pulses
    for (i = 0; i < channels[channel].num_samples; i++) {
        cbp->dst = phys_gpclr0;
        cbp += 2;
    }

    // Let DMA do one cycle to actually clear them
    udelay(channels[channel].subcycle_time_us);

    // Finally set all samples to 0 (instead of gpio_mask)
    for (i = 0; i < channels[channel].num_samples; i++) {
        *(dp + i) = 0;
    }

    return EXIT_SUCCESS;
}


// Clears all pulses for a specific gpio on this channel. Also sets the GPIO to Low.
int
clear_channel_gpio(int channel, int gpio)
{
    int i;
    uint32_t *dp = (uint32_t *) channels[channel].virtbase;

    log_debug("clear_channel_gpio: channel=%d, gpio=%d\n", channel, gpio);
    if (!channels[channel].virtbase)
        return fatal("Error: channel %d has not been initialized with 'init_channel(..)'\n", channel);
    if ((gpio_setup & 1<<gpio) == 0)
        return fatal("Error: cannot clear gpio %d; not yet been set up\n", gpio);

    // Remove this gpio from all samples:
    for (i = 0; i < channels[channel].num_samples; i++) {
        *(dp + i) &= ~(1 << gpio);  // set just this gpio's bit to 0
    }

    // Let DMA do one cycle before setting GPIO to low.
    //udelay(channels[channel].subcycle_time_us);

    gpio_set(gpio, 0);
    return EXIT_SUCCESS;
}


// Update the channel with another pulse within one full cycle. Its possible to
// add more gpios to the same timeslots (width_start). width_start and width are
// multiplied with pulse_width_incr_us to get the pulse width in microseconds [us].
//
// Be careful: if you try to set one GPIO to high and another one to low at the same
// point in time, only the last added action (eg. set-to-low) will be executed on all pins.
// To create these kinds of inverted signals on two GPIOs, either offset them by 1 step, or
// use multiple DMA channels.
int
add_channel_pulse(int channel, int gpio, int width_start, int width)
{
    int i;
    uint32_t phys_gpclr0 = GPIO_PHYS + 0x28;
    uint32_t phys_gpset0 = GPIO_PHYS + 0x1c;
    dma_cb_t *cbp = (dma_cb_t *) get_cb(channel) + (width_start * 2);
    uint32_t *dp = (uint32_t *) channels[channel].virtbase;

    log_debug("add_channel_pulse: channel=%d, gpio=%d, start=%d, width=%d\n", channel, gpio, width_start, width);
    if (!channels[channel].virtbase)
        return fatal("Error: channel %d has not been initialized with 'init_channel(..)'\n", channel);
    if (width_start + width > channels[channel].width_max + 1 || width_start < 0)
        return fatal("Error: cannot add pulse to channel %d: width_start+width exceed max_width of %d\n", channel, channels[channel].width_max);

    if ((gpio_setup & 1<<gpio) == 0)
        init_gpio(gpio);

    // enable or disable gpio at this point in the cycle
    *(dp + width_start) |= 1 << gpio;
    cbp->dst = phys_gpset0;

    // Do nothing for the specified width
    for (i = 1; i < width - 1; i++) {
        *(dp + width_start + i) &= ~(1 << gpio);  // set just this gpio's bit to 0
        cbp += 2;
    }

    // Clear GPIO at end
    *(dp + width_start + width) |= 1 << gpio;
    cbp->dst = phys_gpclr0;
    return EXIT_SUCCESS;
}



// Get a channel's pagemap
static int
make_pagemap(int channel)
{
    int i, fd, memfd, pid;
    char pagemap_fn[64];

    channels[channel].page_map = malloc(channels[channel].num_pages * sizeof(*channels[channel].page_map));

    if (channels[channel].page_map == 0)
        return fatal("rpio-pwm: Failed to malloc page_map: %m\n");
    memfd = open("/dev/mem", O_RDWR);
    if (memfd < 0)
        return fatal("rpio-pwm: Failed to open /dev/mem: %m\n");
    pid = getpid();
    sprintf(pagemap_fn, "/proc/%d/pagemap", pid);
    fd = open(pagemap_fn, O_RDONLY);
    if (fd < 0)
        return fatal("rpio-pwm: Failed to open %s: %m\n", pagemap_fn);
    if (lseek(fd, (uint32_t)channels[channel].virtbase >> 9, SEEK_SET) !=
                        (uint32_t)channels[channel].virtbase >> 9) {
        return fatal("rpio-pwm: Failed to seek on %s: %m\n", pagemap_fn);
    }
    for (i = 0; i < channels[channel].num_pages; i++) {
        uint64_t pfn;
        channels[channel].page_map[i].virtaddr = channels[channel].virtbase + i * PAGE_SIZE;
        // Following line forces page to be allocated
        channels[channel].page_map[i].virtaddr[0] = 0;
        if (read(fd, &pfn, sizeof(pfn)) != sizeof(pfn))
            return fatal("rpio-pwm: Failed to read %s: %m\n", pagemap_fn);
        if (((pfn >> 55) & 0x1bf) != 0x10c)
            return fatal("rpio-pwm: Page %d not present (pfn 0x%016llx)\n", i, pfn);
        channels[channel].page_map[i].physaddr = (uint32_t)pfn << PAGE_SHIFT | DRAM_PHYS_BASE;
    }
    close(fd);
    close(memfd);
    return EXIT_SUCCESS;
}

static int
init_virtbase(int channel)
{
    channels[channel].virtbase = mmap(NULL, channels[channel].num_pages * PAGE_SIZE, PROT_READ|PROT_WRITE,
            MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE|MAP_LOCKED, -1, 0);
    if (channels[channel].virtbase == MAP_FAILED)
        return fatal("rpio-pwm: Failed to mmap physical pages: %m\n");
    if ((unsigned long)channels[channel].virtbase & (PAGE_SIZE-1))
        return fatal("rpio-pwm: Virtual address is not page aligned\n");
    return EXIT_SUCCESS;
}

// Initialize control block for this channel
static int
init_ctrl_data(int channel)
{
    dma_cb_t *cbp = (dma_cb_t *) get_cb(channel);
    uint32_t *sample = (uint32_t *) channels[channel].virtbase;

    uint32_t phys_fifo_addr;
    uint32_t phys_gpclr0 = GPIO_PHYS + 0x28;
    int i;

    channels[channel].dma_reg = map_peripheral(DMA_BASE, DMA_LEN) + (DMA_CHANNEL_INC * channel);
    if (channels[channel].dma_reg == NULL)
        return EXIT_FAILURE;

	phys_fifo_addr = PWM_PHYS + 0x18;

    // Reset complete per-sample gpio mask to 0
    memset(sample, 0, sizeof(channels[channel].num_samples * sizeof(uint32_t)));

    // For each sample we add 2 control blocks:
    // - first: clear gpio and jump to second
    // - second: jump to next CB
    for (i = 0; i < channels[channel].num_samples; i++) {
        cbp->info = DMA_NO_WIDE_BURSTS | DMA_WAIT_RESP;
        cbp->src = mem_virt_to_phys(channel, sample + i);  // src contains mask of which gpios need change at this sample
        cbp->dst = phys_gpclr0;  // set each sample to clear set gpios by default
        cbp->length = 4;
        cbp->stride = 0;
        cbp->next = mem_virt_to_phys(channel, cbp + 1);
        cbp++;

        // Delay
		cbp->info = DMA_NO_WIDE_BURSTS | DMA_WAIT_RESP | DMA_D_DREQ | DMA_PER_MAP(5);

        cbp->src = mem_virt_to_phys(channel, sample); // Any data will do
        cbp->dst = phys_fifo_addr;
        cbp->length = 4;
        cbp->stride = 0;
        cbp->next = mem_virt_to_phys(channel, cbp + 1);
        cbp++;
    }

    // The last control block links back to the first (= endless loop)
    cbp--;
    cbp->next = mem_virt_to_phys(channel, get_cb(channel));

    // Initialize the DMA channel 0 (p46, 47)
    channels[channel].dma_reg[DMA_CS] = DMA_RESET; // DMA channel reset
    udelay(10);
    channels[channel].dma_reg[DMA_CS] = DMA_INT | DMA_END; // Interrupt status & DMA end flag
    channels[channel].dma_reg[DMA_CONBLK_AD] = mem_virt_to_phys(channel, get_cb(channel));  // initial CB
    channels[channel].dma_reg[DMA_DEBUG] = 7; // clear debug error flags
    channels[channel].dma_reg[DMA_CS] = 0x10880001;    // go, mid priority, wait for outstanding writes

    return EXIT_SUCCESS;
}

// Initialize PWM hardware once for all channels (10MHz)
static void
init_hardware(void)
{
	// Initialise PWM
	pwm_reg[PWM_CTL] = 0;
	udelay(10);
	clk_reg[PWMCLK_CNTL] = 0x5A000006;        // Source=PLLD (500MHz)
	udelay(100);
	clk_reg[PWMCLK_DIV] = 0x5A000000 | (50<<12);    // set pwm div to 50, giving 10MHz
	udelay(100);
	clk_reg[PWMCLK_CNTL] = 0x5A000016;        // Source=PLLD and enable
	udelay(100);
	pwm_reg[PWM_RNG1] = pulse_width_incr_us * 10;
	udelay(10);
	pwm_reg[PWM_DMAC] = PWMDMAC_ENAB | PWMDMAC_THRSHLD;
	udelay(10);
	pwm_reg[PWM_CTL] = PWMCTL_CLRF;
	udelay(10);
	pwm_reg[PWM_CTL] = PWMCTL_USEF1 | PWMCTL_PWEN1;
	udelay(10);
}

// Setup a channel with a specific subcycle time. After that pulse-widths can be
// added at any time.
int
init_channel(int channel, int subcycle_time_us)
{
    log_debug("Initializing channel %d...\n", channel);
    if (_is_setup == 0)
        return fatal("Error: you need to call `setup(..)` before initializing channels\n");
    if (channel > DMA_CHANNELS-1)
        return fatal("Error: maximum channel is %d (requested channel %d)\n", DMA_CHANNELS-1, channel);
    if (channels[channel].virtbase)
        return fatal("Error: channel %d already initialized.\n", channel);
    if (subcycle_time_us < SUBCYCLE_TIME_US_MIN)
        return fatal("Error: subcycle time %dus is too small (min=%dus)\n", subcycle_time_us, SUBCYCLE_TIME_US_MIN);

    // Setup Data
    channels[channel].subcycle_time_us = subcycle_time_us;
    channels[channel].num_samples = channels[channel].subcycle_time_us / pulse_width_incr_us;
    channels[channel].width_max = channels[channel].num_samples - 1;
    channels[channel].num_cbs = channels[channel].num_samples * 2;
    channels[channel].num_pages = ((channels[channel].num_cbs * 32 + channels[channel].num_samples * 4 + \
                                       PAGE_SIZE - 1) >> PAGE_SHIFT);

    // Initialize channel
    if (init_virtbase(channel) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if (make_pagemap(channel) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if (init_ctrl_data(channel) == EXIT_FAILURE)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

// Print some info about a channel
int
print_channel(int channel)
{
    if (channel > DMA_CHANNELS - 1)
        return fatal("Error: you tried to print channel %d, but max channel is %d\n", channel, DMA_CHANNELS-1);
    log_debug("Subcycle time: %dus\n", channels[channel].subcycle_time_us);
    log_debug("PW Increments: %dus\n", pulse_width_incr_us);
    log_debug("Num samples:   %d\n", channels[channel].num_samples);
    log_debug("Num CBS:       %d\n", channels[channel].num_cbs);
    log_debug("Num pages:     %d\n", channels[channel].num_pages);
    return EXIT_SUCCESS;
}

void
set_softfatal(int enabled)
{
    soft_fatal = enabled;
}

char *
get_error_message(void)
{
    return error_message;
}

// setup(..) needs to be called once and starts the PWM timer. delay hardware
// and pulse-width-increment-granularity is set for all DMA channels and cannot
// be changed during runtime due to hardware mechanics (specific PWM timing).
int
setup(int pw_incr_us, int hw)
{
    delay_hw = hw;
    pulse_width_incr_us = pw_incr_us;

    if (_is_setup == 1)
        return fatal("Error: setup(..) has already been called before\n");

    log_debug("Using hardware: PWM\n");
    log_debug("PW increments:  %dus\n", pulse_width_incr_us);

    // Catch all kind of kill signals
    setup_sighandlers();

    // Initialize common stuff
    pwm_reg = map_peripheral(PWM_BASE, PWM_LEN);
    clk_reg = map_peripheral(CLK_BASE, CLK_LEN);
    gpio_reg = map_peripheral(GPIO_BASE, GPIO_LEN);
    if (pwm_reg == NULL || clk_reg == NULL || gpio_reg == NULL)
        return EXIT_FAILURE;

    // Start PWM timing activity
    init_hardware();

    _is_setup = 1;
    return EXIT_SUCCESS;
}

int
is_setup(void)
{
    return _is_setup;
}

int
is_channel_initialized(int channel)
{
    return channels[channel].virtbase ? 1 : 0;
}

int
get_pulse_incr_us(void)
{
    return pulse_width_incr_us;
}

int
get_channel_subcycle_time_us(int channel)
{
    return channels[channel].subcycle_time_us;
}

int
main(int argc, char **argv)
{
	printf("test\n");
	setup(PULSE_WIDTH_INCREMENT_GRANULARITY_US_DEFAULT, DELAY_VIA_PWM);


    // Setup demo parameters
    int demo_timeout = 10 * 1000000;
    int gpio = 17; //17
    int channel = 5; //0
    int subcycle_time_us = SUBCYCLE_TIME_US_DEFAULT; //10ms;

    // Setup channel 
    init_channel(channel, subcycle_time_us);
    print_channel(channel);

    // Use the channel for various pulse widths
    // Increase the width to obtain a brigther led
    add_channel_pulse(channel, gpio, 0, 50);
    sleep(1);
    add_channel_pulse(channel, gpio, 100, 100);
    sleep(1);
    add_channel_pulse(channel, gpio, 200, 200);
    sleep(1);
    add_channel_pulse(channel, gpio, 0, 1000);
    usleep(demo_timeout);

    // Clear and start again
    clear_channel_gpio(channel, gpio);
    add_channel_pulse(channel, gpio, 0, 50);
    usleep(demo_timeout);

    // All done
    shutdown();
    exit(0);
}
