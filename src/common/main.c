#include <interface.h>

#include <utils/string.h>

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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

static mon_data_t g;

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
    if(iscrlf(c) && ctx->arg_cnt < 1) {
        goto bad_exit;
    }

    // invalid character
    if(!ishex(lower) && !iswhitespace(c)) {
        goto bad_exit;
    }

    if(iscrlf(c) && ctx->arg_cnt == 1) {
        memrw(ctx->mem_read_addr, &data, 1, 0, false);
        reset_parse(ctx);
        ctx->arg_buf[0] = ' ';
        int2hex(ctx->arg_buf + 1, data);
        term_puts(NULL, ctx->arg_buf, 3);
        next_state = MON_STATE_NONE;
        goto good_exit;
    }

    term_puts(NULL, &c, 1);

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
                ctx->arg_chr_cnt++;
                next_state = MON_STATE_MEMORY;
            }
            goto good_exit;
        break;

        // taking data
        case 1:
            if(ctx->arg_chr_cnt > 1) {
                data = hex2int(g.arg_buf, 2);
                memrw(ctx->mem_read_addr, &data, 1, 0, true);
                next_state = MON_STATE_NONE;
                reset_parse(ctx);
            }
            else {
                ctx->arg_buf[ctx->arg_chr_cnt] = lower;
                ctx->arg_chr_cnt++;
                next_state = MON_STATE_MEMORY;
            }
            goto good_exit;
        break;
    }

bad_exit:
        puts("\r\nM?\r\n");
good_exit:
        return next_state;
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
        case '\n':
        case '\r':
            term_puts(NULL, "\r\n* ", 4);
            return MON_STATE_NONE;
        break;

        default:
            term_puts(NULL, &c, 1);
            term_puts(NULL, "\r\n", 2);
            term_puts(NULL, &c, 1);
            term_puts(NULL, "?\r\n* ", 5);
            return MON_STATE_NONE;
        break;
    }
done:
    term_puts(NULL, &c, 1);
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

int main(void) {
    port_init();
    term_init(NULL);
    bool ok = false;

    for(;;) {
        char raw = term_getc(NULL, &ok);
        if(ok) {
            run_state(raw);
        }
        port_tasks();
    }
    return 0;
}
