#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <omp.h>
#include <unistd.h>

// #define STENCIL_SIZE 2

typedef float stencil_t;

/** conduction coeff used in computation */
static const stencil_t alpha = 0.02;

/** threshold for convergence */
static const stencil_t epsilon = 0.0001;

/** max number of steps */
static const int stencil_max_steps = 100000;

static stencil_t *values = NULL;
static stencil_t *prev_values = NULL;

static int size_x; // = STENCIL_SIZE;
static int size_y; // = STENCIL_SIZE;

/** init stencil values to 0, borders to non-zero */
static void stencil_init(void) {
  values = malloc(size_x * size_y * sizeof(stencil_t));
  prev_values = malloc(size_x * size_y * sizeof(stencil_t));
  int x, y;
  for (x = 0; x < size_x; x++) {
    for (y = 0; y < size_y; y++) {
      values[x + size_x * y] = 0.0;
    }
  }
  for (x = 0; x < size_x; x++) {
    values[x + size_x * 0] = x;
    values[x + size_x * (size_y - 1)] = size_x - 1 - x;
  }
  for (y = 0; y < size_y; y++) {
    values[0 + size_x * y] = y;
    values[size_x - 1 + size_x * y] = size_y - 1 - y;
  }
  memcpy(prev_values, values, size_x * size_y * sizeof(stencil_t));
}

static void stencil_free(void) {
  free(values);
  free(prev_values);
}

/** display a (part of) the stencil values */
static void stencil_display(int x0, int x1, int y0, int y1) {
  int x, y;
  for (y = y0; y <= y1; y++) {
    for (x = x0; x <= x1; x++) {
      printf("%8.5g ", values[x + size_x * y]);
    }
    printf("\n");
  }
}

/** compute the next stencil step, return 1 if computation has converged */
static int stencil_step(void) {
  int convergence = 1;
  /* switch buffers */
  stencil_t *tmp = prev_values;
  prev_values = values;
  values = tmp;
  int x, y;
  for (y = 1; y < size_y - 1; y++) {
    for (x = 1; x < size_x - 1; x++) {
      values[x + size_x * y] =
          alpha * (prev_values[x - 1 + size_x * y] +
                   prev_values[x + 1 + size_x * y] +
                   prev_values[x + size_x * (y - 1)] +
                   prev_values[x + size_x * (y + 1)]) +
          (1.0 - 4.0 * alpha) * prev_values[x + size_x * y];
      if (convergence && (fabs(prev_values[x + size_x * y] -
                               values[x + size_x * y]) > epsilon)) {
        convergence = 0;
      }
    }
  }
  return convergence;
}

static int stencil_step_omp(void) {
  int convergence = 1;
  stencil_t *tmp = prev_values;
  prev_values = values;
  values = tmp;
  int x, y;
  for (y = 1; y < size_y - 1; y++) {
    for (x = 1; x < size_x - 1; x++) {
      values[x + size_x * y] =
          alpha * (prev_values[x - 1 + size_x * y] +
                   prev_values[x + 1 + size_x * y] +
                   prev_values[x + size_x * (y - 1)] +
                   prev_values[x + size_x * (y + 1)]) +
          (1.0 - 4.0 * alpha) * prev_values[x + size_x * y];
      if (convergence && fabs(prev_values[x + size_x * y] -
                              values[x + size_x * y]) > epsilon) {
        convergence = 0;
      }
    }
  }
  return convergence;
}

int main(int argc, char **argv) {

  int stencil_size = 10;
  int test_mode = 0;

  int opt;
  while ((opt = getopt(argc, argv, "t")) != -1) {
    switch (opt) {
    case 't':
      test_mode = 1;
      break;
    default:
      fprintf(stderr, "Usage: %s [stencil size] [-t]\n", argv[0]);
      return EXIT_FAILURE;
    }
  }
  if (optind < argc) {
    stencil_size = atoi(argv[optind]);
    if (stencil_size < 2) {
      fprintf(stderr, "Stencil size must be >= 2. Using default (10).\n");
      stencil_size = 10;
    }
  }

  size_x = stencil_size;
  size_y = stencil_size;

  stencil_init();
  printf("# init:\n");

  struct timespec t1, t2;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  int s;
  for (s = 0; s < stencil_max_steps; s++) {
    int convergence = stencil_step_omp();
    if (convergence) {
      break;
    }
  }
  clock_gettime(CLOCK_MONOTONIC, &t2);
  const double t_usec =
      (t2.tv_sec - t1.tv_sec) * 1000000.0 + (t2.tv_nsec - t1.tv_nsec) / 1000.0;
  printf("# size = %d\n", stencil_size);
  printf("# steps = %d\n", s);
  printf("# time = %g usecs.\n", t_usec);
  printf("# gflops = %g\n", (6.0 * size_x * size_y * s) / (t_usec * 1000));

  if (test_mode) {
    printf("Test mode\n");
    stencil_display(0, size_x - 1, 0, size_y - 1);
    stencil_t *test_values = malloc(size_x * size_y * sizeof(stencil_t));
    memcpy(test_values, values, size_x * size_y * sizeof(stencil_t));
    stencil_free();
    stencil_init();
    for (s = 0; s < stencil_max_steps; s++) {
      int convergence = stencil_step();
      if (convergence) {
        break;
      }
    }

    int mismatch = 0;
    int x, y;
    for (x = 0; x < size_x; x++) {
      for (y = 0; y < size_y; y++) {
        if (fabs(values[x + size_x * y] - test_values[x + size_x * y]) >
            epsilon) {
          mismatch = 1;
          printf("Mismatch at (%d, %d): seq = %g, test = %g\n", x, y,
                 values[x + size_x * y], test_values[x + size_x * y]);
        }
      }
    }
    if (mismatch) {
      printf("Results do not match! Expected:\n");
      stencil_display(0, size_x - 1, 0, size_y - 1);
    } else {
      printf("Results match perfectly.\n");
    }
    free(test_values);
  }
  stencil_free();
  return 0;
}
