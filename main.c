#include "external_lib.h"

#include <libgccjit.h>
#include <stdlib.h>
#include <stdio.h>

typedef void (*my_log_callback_fn)(const char *message, void *data);

void my_log_callback(const char *message, void *data) {
    printf("ZK message: %s; Carried payload: %s\n", message, (char*)data);
}

/* Allocate memory for new callback function using mmap (it will save data adress automatically)
 * This void functions takes one const char *message
 * Inside it calls callback function with message and puts data after it
 */
void zoo_set_my_log_callback(zhandle_t *zh, my_log_callback_fn callback, void *data) {
    gcc_jit_context *ctxt = gcc_jit_context_acquire(); // Initialilze context for jit compilation
    if (!ctxt) {
        fprintf (stderr, "NULL ctxt");
        exit (1);
    }

    // Declare types that will be used in our JIT compilation
    gcc_jit_type *void_type =
        gcc_jit_context_get_type (ctxt, GCC_JIT_TYPE_VOID);
    gcc_jit_type *const_char_ptr_type =
        gcc_jit_context_get_type (ctxt, GCC_JIT_TYPE_CONST_CHAR_PTR);
    gcc_jit_type *void_ptr_type =
        gcc_jit_context_get_type (ctxt, GCC_JIT_TYPE_VOID_PTR);

    gcc_jit_type *param_types[2];
    param_types[0] = const_char_ptr_type;
    param_types[1] = void_ptr_type;

    gcc_jit_type * fn_ptr_type =
        gcc_jit_context_new_function_ptr_type (ctxt, NULL,
                                               void_type,
                                               2, param_types, 0);

    // Predefine signature for jit_callback function
    gcc_jit_param *param_message =
        gcc_jit_context_new_param (ctxt, NULL, const_char_ptr_type, "message");

    gcc_jit_function *func =
        gcc_jit_context_new_function (ctxt, NULL,
                                      GCC_JIT_FUNCTION_EXPORTED,
                                      void_type,
                                      "jit_callback",
                                      1, &param_message,
                                      0);

    // Function consists of a single computation block
    gcc_jit_block *block = gcc_jit_function_new_block (func, NULL);

    // Predefine arguments to our actual function (now stored as fn_prt)
    gcc_jit_rvalue *args_to_callback[2];
    args_to_callback[0] = gcc_jit_param_as_rvalue(param_message); // This argument is passed from jit_callback
    args_to_callback[1] = gcc_jit_context_new_rvalue_from_ptr(ctxt, void_ptr_type, data); // This argument is compilation defined

    // Add call to our callback function with two arguments
    gcc_jit_block_add_eval(
        block, NULL,
        gcc_jit_context_new_call_through_ptr (
            ctxt,
            NULL,
            gcc_jit_context_new_rvalue_from_ptr(ctxt, fn_ptr_type, callback),
            2, args_to_callback));
    gcc_jit_block_end_with_void_return(block, NULL);

    // Compile and store results
    gcc_jit_result *result = gcc_jit_context_compile (ctxt);
    zh->callback = gcc_jit_result_get_code(result, "jit_callback");
    if (result == NULL) {
        fprintf(stderr, "GCC_JIT_RESULT is empty (no code compiled)\n");
        exit(1);
    }
    gcc_jit_context_release(ctxt);
}

int main() {
    char data[] = "MY ADDITIONAL MESSAGE";

    zhandle_t zh = {0};
    zoo_set_my_log_callback(&zh, &my_log_callback, data);
    zoo_do_log(&zh, "NORMAL MESSAGE");

    return 0;
}