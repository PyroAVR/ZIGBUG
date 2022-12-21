#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <stdint.h> 
#include <string.h>
#include <util/fifo.h>
#include <drivers/uart.h>
#include <drivers/spi_master.h>
#include <util/os_log.h>
#include <util/delay.h>
#include <float.h>

#define byteswap32(X) (\
    ((X & 0xff000000L) >> 24)\
    |((X & 0x00ff0000L) >> 8)\
    |((X & 0x0000ff00L) << 8)\
    |((X & 0x000000ffL) << 24)\
)

#define byteswap16(X) (\
    ((X & 0xff00) >> 8)\
    |((X & 0x00ff) << 8)\
)

#define BUF_SZ 64
static char uart_rx_buf[BUF_SZ];
static char uart_tx_buf[BUF_SZ];

typedef struct {
    
} link_t;

char get_byte(link_t *link) {
    char r = 0;
    while(!uart_gets(&r, 1));
    return r;
}

int put_byte(link_t *link, char c) {
    (void)link;
    uart_puts(&c, 1);
}

void puts(char *str) {
    while(*str) {
        put_byte(NULL, *str++);
    }
}
enum mon_state {
    MON_STATE_NONE,
    MON_STATE_MEMORY,
    MON_STATE_DUMP,
    MON_STATE_JUMP,
    MON_STATE_IHEX,
};

typedef struct {
    // memory command
    uintptr_t mem_read_addr;
    // dump command
    uintptr_t mem_stop_addr;
    enum mon_state cmd;
    // buffer is sizeof uintptr in hex chars
    char arg_buf[sizeof(uintptr_t) << 1];
    uint8_t arg_chr_cnt;
    uint8_t arg_cnt;
} mon_data_t;



mon_data_t g;

static char tolower(char c) {
    if(c > 'Z' || c < 'A') {
        return c;
    }
    else {
        return c + - 'A' + 'a';
    }
}

static char ishex(char c) {
    return ((c >= 'a' && c <= 'f') || (c >= '0' && c <= '9'));
}

static int hex2nibble(char c) {
    if(c < 'A') {
        return c - 'A';
    }
    else {
        return c - '0';
    }
}

static void int2hex(char *buf, char byte) {
    char lower = byte & 0x0f;
    char upper = (byte & 0xf0) >> 4;
    if(upper > 9) {
        buf[1] = (upper - 9) + 'A';
    }
    else {
        buf[1] = upper + '0';
    }
    if(lower > 9) {
        buf[0] = (lower - 9) + 'A';
    }
    else {
        buf[0] = lower + '0';
    }
}

static uintptr_t hex2int(char *buf, size_t len) {
    char r = 0;
    if(len > (sizeof(uintptr_t) << 1)) {
        return 0;
    }
    for(int i = len-1; i >= 0; i--) {
        if(!buf[i]) break;
        r |= hex2nibble(buf[i]) << ((len - i - 1)*4);
    }
    return r;
}

static inline void reset_parse(mon_data_t *ctx) {
    ctx->arg_chr_cnt = 0;
    ctx->arg_cnt++;
    memset(ctx->arg_buf, 0, sizeof(uintptr_t) << 1);
}


static enum mon_state mem_mode(mon_data_t *ctx, char c) {
    char lower = tolower(c);
    char data = 0;
    enum mon_state next_state = MON_STATE_MEMORY;


    // not enough args
    if(c == '\r' && ctx->arg_cnt < 1) {
        goto bad_exit;
    }

    // invalid character
    if(!ishex(lower) && c != '\r' && c != ' ') {
        goto bad_exit;
    }

    if(c == '\r' && ctx->arg_cnt == 1) {
        memcpy(&data, ctx->mem_read_addr, 1);
        reset_parse(ctx);
        ctx->arg_buf[0] = ' ';
        int2hex(ctx->arg_buf + 1, data);
        puts(ctx->arg_buf);
        next_state = MON_STATE_NONE;
        goto good_exit;
    }

    put_byte(NULL, c);

    switch(ctx->arg_cnt) {
        // taking address
        case 0:
            // enough chars specified to make an address
            if(ctx->arg_chr_cnt >= (sizeof(uintptr_t) << 1)) {
                ctx->mem_read_addr = hex2int(g.arg_buf, sizeof(uintptr_t) << 1);
                next_state = MON_STATE_MEMORY;
                reset_parse(ctx);
            }
            else {
                ctx->arg_buf[ctx->arg_chr_cnt] = lower;
            }
        break;

        // taking data
        case 1:
            if(ctx->arg_chr_cnt > 1) {
                data = hex2int(g.arg_buf, 2);
                memset(ctx->mem_read_addr, data, 1);
                next_state = MON_STATE_NONE;
                goto good_exit;
            }
        break;
    }

good_exit:
        return next_state;

bad_exit:
        puts("\r\nM?\r\n");
        return MON_STATE_NONE;
}

static int init_mode(mon_data_t *ctx, char c) {
    char lower = tolower(c);
    enum mon_state state;
    reset_parse(ctx);
    g.arg_chr_cnt = 0;

    switch(lower) {
        case 'm':
            state = MON_STATE_MEMORY;
            goto done;
        break;
        case 'd':
            state = MON_STATE_DUMP;
            goto done;
        break;
        case 'j':
            state = MON_STATE_JUMP;
            goto done;
        break;
        case ':':
            state = MON_STATE_IHEX;
            goto done;
        break;

        case '\r':
        default:
            put_byte(NULL, c);
            puts("\r\n");
            put_byte(NULL, c);
            puts("?\r\n* ");
            return MON_STATE_NONE;
        break;
    }
done:
    put_byte(NULL, c);
    return state;
}

static void run_state(char c) {
    switch(g.cmd) {

        case MON_STATE_MEMORY:
            g.cmd = mem_mode(&g,c);
        break;
        case MON_STATE_NONE:
        default:
            g.cmd = init_mode(&g, c);
        break;
    }
}
// commands:
// all commands terminate w/ \n
// M memory
//      M <address> <value>
//      M <address>
//      repeated \n without additional commands: increment address
// D dump
//      D <start> <stop>
//      dump memory in ihex format
//
// J jump
//      J <address>
//      jump to address
//
// : ihex
//      interpret line until \n as Intel HEX format


int main(void) {
    char addr_buf[sizeof(uintptr_t)];
    int line_chars = 0;
    int arg_chars = 0;
    configure_uart(9600, uart_rx_buf, uart_tx_buf, BUF_SZ, BUF_SZ);

    g.mem_read_addr = 0;
    g.mem_stop_addr = 0;
    g.cmd = MON_STATE_NONE;

    puts("\r\n* ");

    // main loop
    for(;;) {
        char raw = get_byte(NULL);
        run_state(raw);
    }
    return 0;
}
