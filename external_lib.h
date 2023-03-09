#include <stdio.h>

typedef void (*log_callback_fn)(const char *message);

struct zhandle_t_ {
        log_callback_fn callback;
};

typedef struct zhandle_t_ zhandle_t;

/* API of this function is inconvinient to us.
 * Instead we want to add additional void *data parameter.
 * This parameter should be reflected inside callback in zhandle_t_.
 */
void zoo_set_log_callback(zhandle_t *zh, log_callback_fn callback);

void zoo_do_log(zhandle_t *zh, const char* message);