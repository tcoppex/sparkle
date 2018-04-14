#include <cstdlib>
#include "app.h"

/* -------------------------------------------------------------------------- */

#define WINDOW_TITLE    "- spärkle -"

/* -------------------------------------------------------------------------- */

int main(int, char *[]) {
  App app;

  if (!app.init(WINDOW_TITLE)) {
    return EXIT_FAILURE;
  }

  app.run();
  app.deinit();

  return EXIT_SUCCESS;
}

/* -------------------------------------------------------------------------- */
