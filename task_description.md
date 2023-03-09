## Постановка

Иногда нам достаются библиотеки, которые обладают недостаточно гибким для нас API. Один из примеров реальной жизни - библиотека для работы с ZooKeeper в месте, где осуществляется логгирование.

```c
typedef void (*log_callback_fn)(const char *message);

struct zhandle_t_ {
        log_callback_fn callback;
};

typedef struct zhandle_t_ zhandle_t;

void zoo_set_log_callback(zhandle_t *zh, log_callback_fn callback) {
        zh->callback = callback;
}

void zoo_do_log(zhandle_t *zh, const char* message) {
        if (zh->callback != NULL) {
                zh->callback(message);
        }
}
```

Как видно, `log_callback_fn` в аргументах передает только строку, но нам хочется, чтобы также передавалась какая-то дополнительная метадата. Например, поинтер на объект логгера.
То есть, мы хотим такого:

```c
// Out shiny logging API with carried pointer.
typedef void (*my_log_callback_fn)(const char *message, void *data);

void my_log_callback(const char *message, void *data) {
        printf("ZK message: %s; Carried payload: %s\n", message, (char*)data);
}
```

Требуется написать функцию `zoo_set_my_log_callback`, которая бы делала то же самое, что и `zoo_set_log_callback`, но добавляла бы туда возможность проставлять дополнительные данные, которые бы передавались при вызове `zoo_do_log`.

Использование:

```c
int main() {
        char data[] = "MY ADDITIONAL MESSAGE";

        zhandle_t zh = {0};
        zoo_set_my_log_callback(&zh, &my_log_callback, data);
        zoo_do_log(&zh, "NORMAL MESSAGE");

        return 0;
}
```

Что мы ожидаем увидеть на экране:
```
ZK message: NORMAL MESSAGE; Carried payload: MY ADDITIONAL MESSAGE\n
```