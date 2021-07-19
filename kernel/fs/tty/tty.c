#include "tty.h"
#include <fs/devfs/devfs.h>
#include <util/lock.h>
#include <acpi/apic.h>
#include <cpu/idt.h>
#include <proc/proc.h>
#include <proc/smp.h>

static inline int events_await(event_t **event, int *out_events, int n) {
    int wake = 0;

    for(int i = 0; i < n; i++) {
        if(locked_read(event_t, event[i])) {
            wake = 1;
            locked_dec(event[i]);
            out_events[i] = 1;
        }
    }
    if(wake) {
        return 0;
    }

    thread_t *currentThread = get_active_thread(cpuLocals[current_cpu].currentActiveThread);
    currentThread->eventNum = n;
    currentThread->outEventPtr = out_events;
    locked_write(event_t **, &currentThread->eventPtr, event);
    
    yield();
    return 0;
}

static inline int event_await(event_t *event) {
    event_t *evts[1] = {event};
    event_t out_evts[1] = {0};
    return events_await(evts, out_evts, 1);
}

static inline void event_trigger(event_t *event) {
    locked_inc(event);
}

#define KBD_BUF_SIZE 2048
#define BIG_BUF_SIZE 65536

typedef struct {
    int ttyID;
    struct termios termios;
    spinlock_t write_lock;
    spinlock_t read_lock;
    int tcioff;
    int tcooff;
    spinlock_t kbd_lock;
    size_t kbd_buf_i;
    char kbd_buf[KBD_BUF_SIZE];
    size_t big_buf_i;
    char big_buf[BIG_BUF_SIZE];
    event_t kbd_event;
} tty_t;

static tty_t ttys[6];

static const char *tty_names[6] = {
    "tty0", "tty1", "tty2", "tty3", "tty4", "tty5"
};

//TODO
static size_t tty_write(vfs_node_t *file, char *buffer, size_t size) {
    (void)file;
    printf("%s", buffer);
    return size;
}

static size_t tty_isatty(vfs_node_t *file) {
    (void)file;
    return 1;
}

static size_t tty_tcflow(vfs_node_t *file, int action) {
    tty_t *tty = (tty_t *)(file->device);
    spinlock_lock(&tty->read_lock);
    spinlock_lock(&tty->write_lock);

    size_t ret = 0;

    switch(action) {
        case TCOOFF:
            tty->tcooff = 1;
            break;
        case TCOON:
            tty->tcooff = 0;
            break;
        case TCIOFF:
            tty->tcioff = 1;
            break;
        case TCION:
            tty->tcioff = 0;
            break;
        default:
            ret = -1;
            break;
    }

    spinlock_unlock(&tty->write_lock);
    spinlock_unlock(&tty->read_lock);
    return ret;
}

static size_t tty_tcsetattr(vfs_node_t *file, int optional_actions, struct termios *termios_p) {
    tty_t *tty = (tty_t *)(file->device);
    spinlock_lock(&tty->read_lock);
    spinlock_lock(&tty->write_lock);
    tty->termios = *termios_p;
    spinlock_unlock(&tty->write_lock);
    spinlock_unlock(&tty->read_lock);
    return 0;
}

static size_t tty_tcgetattr(vfs_node_t *file, struct termios *termios_p) {
    tty_t *tty = (tty_t *)(file->device);
    spinlock_lock(&tty->read_lock);
    spinlock_lock(&tty->write_lock);
    *termios_p = tty->termios;
    spinlock_unlock(&tty->write_lock);
    spinlock_unlock(&tty->read_lock);
    return 0;
}

#define CAPSLOCK 0x3a
#define LEFT_ALT 0x38
#define LEFT_ALT_REL 0xb8
#define RIGHT_SHIFT 0x36
#define LEFT_SHIFT 0x2a
#define RIGHT_SHIFT_REL 0xb6
#define LEFT_SHIFT_REL 0xaa
#define CTRL 0x1d
#define CTRL_REL 0x9d

static const uint8_t ascii_capslock[] = {
    '\0', '\e', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', '\0', '\0', '\0', ' '
};

static const uint8_t ascii_shift[] = {
    '\0', '\e', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', '\0', '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', '\0', '\0', '\0', ' '
};

static const uint8_t ascii_shift_capslock[] = {
    '\0', '\e', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', '\0', '|', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', '<', '>', '?', '\0', '\0', '\0', ' '
};

static const uint8_t ascii_nomod[] = {
    '\0', '\e', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '\0', '\0', '\0', ' '
};

static int capslock_active = 0;
static int shift_active = 0;
static int ctrl_active = 0;
static int alt_active = 0;
static int extra_scancodes = 0;

static void add_to_buf_char(tty_t *tty, char c) {
    spinlock_lock(&tty->read_lock);

    if(tty->termios.c_lflag & ICANON) {
        switch(c) {
            case '\n':
                if (tty->kbd_buf_i == KBD_BUF_SIZE)
                    goto out;
                tty->kbd_buf[tty->kbd_buf_i++] = c;
                if (tty->termios.c_lflag & ECHO)
                    printf("%c", c);
                for (size_t i = 0; i < tty->kbd_buf_i; i++) {
                    if (tty->big_buf_i == BIG_BUF_SIZE)
                        goto out;
                    tty->big_buf[tty->big_buf_i++] = tty->kbd_buf[i];
                }
                tty->kbd_buf_i = 0;
                goto out;
            case '\b':
                if (!tty->kbd_buf_i)
                    goto out;
                tty->kbd_buf[--tty->kbd_buf_i] = 0;
                if (tty->termios.c_lflag & ECHO) {
                    printf("\b \b");
                }
                goto out;
        }
    }

    if (tty->termios.c_lflag & ICANON) {
        if (tty->kbd_buf_i == KBD_BUF_SIZE)
            goto out;
        tty->kbd_buf[tty->kbd_buf_i++] = c;
    } else {
        if (tty->big_buf_i == BIG_BUF_SIZE)
            goto out;
        tty->big_buf[tty->big_buf_i++] = c;
    }

    if((c >= 0x20 && c <= 0x7e) && tty->termios.c_lflag & ECHO)
        printf("%c", c);

out:
    spinlock_unlock(&tty->read_lock);
}

static void add_to_buf(int ttyNum, const char *s, size_t cnt) {
    tty_t *tty = &ttys[ttyNum];
    for(size_t i = 0; i < cnt; i++) {
        add_to_buf_char(tty, s[i]);
    }
    event_trigger(&tty->kbd_event);
}

static size_t tty_read(vfs_node_t *file, char *buffer, size_t size) {
    (void)file;
    tty_t *tty = (tty_t *)file->device;
    
    if(tty->tcioff) {
        return -1;
    }

    char *buf = buffer;
    int wait = 1;

    while(!spinlock_try_lock(&tty->read_lock)) {
        if(event_await(&tty->kbd_event)) {
            return -1;
        }
    }

    for (size_t i = 0; i < size; ) {
        if (tty->big_buf_i) {
            buf[i++] = tty->big_buf[0];
            tty->big_buf_i--;
            for (size_t j = 0; j < tty->big_buf_i; j++) {
                tty->big_buf[j] = tty->big_buf[j+1];
            }
            wait = 0;
        } else {
            if(wait) {
                spinlock_unlock(&tty->read_lock);
                do {
                    if(event_await(&tty->kbd_event)) {
                        return -1;
                    }
                } while(!spinlock_try_lock(&tty->read_lock));
            } else {
                spinlock_unlock(&tty->read_lock);
                return (size_t)i;
            }
        }
    }

    spinlock_unlock(&tty->read_lock);
    return (size_t)size;
}

static void kbd_handler(irq_regs_t *regs) {
    uint8_t keyCode = inb(0x60);

    char c = '\0';

    if(keyCode == 0xE0) {
        extra_scancodes = 1;
        return;
    }

    if(extra_scancodes) {
        extra_scancodes = 0;

        switch (keyCode) {
            case CTRL:
                ctrl_active = 1;
                return;
            case CTRL_REL:
                ctrl_active = 0;
                return;
            default:
                break;
        }
    }

    switch(keyCode) {
        case LEFT_ALT:
            alt_active = 1;
            return;
        case LEFT_ALT_REL:
            alt_active = 0;
            return;
        case LEFT_SHIFT:
        case RIGHT_SHIFT:
            shift_active = 1;
            return;
        case LEFT_SHIFT_REL:
        case RIGHT_SHIFT_REL:
            shift_active = 0;
            return;
        case CTRL:
            ctrl_active = 1;
            return;
        case CTRL_REL:
            ctrl_active = 0;
            return;
        case CAPSLOCK:
            capslock_active = !capslock_active;
            return;
        default:
            break;
    }

    if(ctrl_active) {
        switch(keyCode) {
            default:
                break;
        }
    }

    if (keyCode < 0x57) {
        if (ctrl_active)
            c = ascii_capslock[keyCode] - ('?' + 1);
        else if (!capslock_active && !shift_active)
            c = ascii_nomod[keyCode];
        else if (!capslock_active && shift_active)
            c = ascii_shift[keyCode];
        else if (capslock_active && shift_active)
            c = ascii_shift_capslock[keyCode];
        else
            c = ascii_capslock[keyCode];
    } else {
        return;
    }

    add_to_buf(0, &c, 1);
}

void initTTY() {
    for(size_t i = 0; i < 6; i++) {
        devfs_node_t *ttyNode = (devfs_node_t *)kcalloc(sizeof(devfs_node_t *), 1);
        strcpy(ttyNode->name, tty_names[i]);
        ttyNode->functions.write = tty_write;
        ttyNode->functions.read = tty_read;
        ttyNode->functions.isatty = tty_isatty;
        ttyNode->functions.tcflow = tty_tcflow;
        ttyNode->functions.tcsetattr = tty_tcsetattr;
        ttyNode->functions.tcgetattr = tty_tcgetattr;
        ttyNode->flags |= FS_FILE;
        ttyNode->device = (void *)&ttys[i];
        ttys[i].ttyID = i;
        ttys[i].write_lock = INIT_LOCK();
        ttys[i].read_lock = INIT_LOCK();
        ttys[i].kbd_lock = INIT_LOCK();
        ttys[i].termios.c_lflag = (ISIG | ICANON | ECHO);
        ttys[i].termios.c_cc[VINTR] = 0x03;
        devfs_mount((char *)tty_names[i], ttyNode);
    }

    irq_functions[0x1] = (idt_fn_t)kbd_handler;
    lapic_legacy_irq(0, 1, 1);
}