#ifndef DYNGL_GPIO_H
#define DYNGL_GPIO_H

#include <stdbool.h>

typedef struct {
    bool pairing_mode;
} dyngl_gpio_state_t;

void dyngl_gpio_init();
void dyngl_gpio_handle_input(dyngl_gpio_state_t *state);

#endif // DYNGL_GPIO_H
