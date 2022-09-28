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
    uintptr_t mem_base_addr;
    // dump command
    uintptr_t mem_stop_addr;
    enum mon_state cmd;
    uint8_t arg_cnt;
} mon_data_t;

static mon_data_t g;

struct get_arg_ctx {
    size_t char_cnt; // set this to max #chars in arg
    // a sane user types in big endian, this will end up little endian...
    // may need a per-platform endianness swapper macro.
    uintptr_t result;
};

static struct get_arg_ctx g_ctx;

// return negative chars remaining when invalid char, else pos chars remaining
static int get_arg_r(struct get_arg_ctx *ctx, char c) {
    int status = ctx->char_cnt;
    if(ishex(c) && (ctx->char_cnt != 0)) {
        // FIXME this will swap nibbles and bytes
        ctx->result |= (hex2nibble(c) << (ctx->char_cnt << 2));
        ctx->char_cnt--;
        status = ctx->char_cnt;
    }
    else {
        status = -status;
    }
    return status;
}

// pass len = 0 to continue existing parse (like strtok)
// also only accepts hex numbers
// returns 1 if arg is valid in output arg, 0 if not, -1 if error
int get_arg(char c, size_t len, uintptr_t *out) {
    int status =  0;
    if(len != 0) {
        g_ctx.char_cnt = len;
        g_ctx.result = 0;
    }

    status = get_arg_r(&g_ctx, c);

    if(status == 0)  {
        *out = g_ctx.result;
        status = 1;
    }
    else if (status < 0) {
        status = 0;
    }
    else {
        status = -1;
    }

    return status;
}

static enum mon_state mem_mode(mon_data_t *ctx, char c) {
    char lower = tolower(c);
    char data = 0;
    uintptr_t arg_result = 0;
    int status = 0;
    char action_mask = 0;
    enum mon_state next_state = MON_STATE_MEMORY;


    // not enough args
    // TODO: pretty sure this check is unnecessary
    /*
     *if(iscrlf(c) && ctx->arg_cnt < 1) {
     *    goto bad_exit;
     *}
     */

    // invalid character
    if(!ishex(lower) && !iswhitespace(c)) {
        goto bad_exit;
    }

/*
 *    if(iscrlf(c) && ctx->arg_cnt == 1) {
 *        memrw(ctx->mem_read_addr, &data, 1, 0, false);
 *        ctx->arg_buf[0] = ' ';
 *        int2hex(ctx->arg_buf + 1, data);
 *        term_puts(NULL, ctx->arg_buf, 3);
 *        next_state = MON_STATE_NONE;
 *        goto good_exit;
 *    }
 *
 *    term_puts(NULL, &c, 1);
 */

    switch(ctx->arg_cnt) {
        // taking address
        case 0:
            // FIXME need a way to know not to reset the size, eg. pass 0 in order to enable correct parsing.
            status = get_arg(lower, sizeof(uintptr_t) << 1, &arg_result);
            if(status == 0) {
                // enough chars specified to make an address
                ctx->mem_base_addr = arg_result;
                ctx->arg_cnt++;
            }
            else if(status < 0) {
                // invalid char, break to init mode
                next_state = MON_STATE_NONE;
            }
            else {
                // not enough chars, continue taking address
            }
        break;

        // taking data
        case 1:
            // TODO: can no value received be ishex(c)?
            action_mask = (iscrlf(c) ? 1:0) | (/* no value received */ 0 << 1);
            switch(action_mask) {
                case 1:
                    // crlf and no value received:
                    // memory read requested
                break;

                case 2:
                    // no crlf and value received:
                    // memory write requested, accepting value
                break;

                case 3:
                    // crlf and value received
                    // perform memory write
                    next_state = MON_STATE_NONE;
                break;

                default:
                break;
            }
    }

bad_exit:
        term_puts(NULL, "\r\nM?\r\n", 6);
good_exit:
        return next_state;
}

static int init_mode(mon_data_t *ctx, char c) {
    char lower = tolower(c);
    enum mon_state state;

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
    enum mon_state next_state = MON_STATE_NONE;
    switch(g.cmd) {
        case MON_STATE_MEMORY:
            next_state = mem_mode(&g,c);
        break;
        case MON_STATE_NONE:
        default:
            next_state = init_mode(&g, c);
        break;
    }
    // TODO something is wrong here w/ switching modes...
    // entering "M" immediately exits (issue w/ M function)
    // but we never get the "*" prompt back
    g.cmd = next_state;
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
