#include "external_lib.h"

void zoo_set_log_callback(zhandle_t *zh, log_callback_fn callback) {
        zh->callback = callback;
}

void zoo_do_log(zhandle_t *zh, const char* message) {
        if (zh->callback != NULL) {
                zh->callback(message);
        }
}