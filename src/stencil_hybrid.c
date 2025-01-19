#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mpi.h>
#include <omp.h>
#include <unistd.h>

typedef float stencil_t;

/** conduction coeff used in computation */
static const stencil_t alpha = 0.02;

/** threshold for convergence */
static const stencil_t epsilon = 0.0001;

/** max number of steps */
static const int stencil_max_steps = 100000;

// ONLY RANK 0
static int test_mode = 0;
static stencil_t *values = NULL;
static stencil_t *prev_values = NULL;
static int size_x; // global size borders
static int size_y; // global size borders

// ALL RANKS
static int rank;                            // MPI rank
static int size;                            // MPI size
static int local_size_x;                    // local size without halo
static int local_size_y;                    // local size without halo
static stencil_t *local_values = NULL;      // local values with halo
static stencil_t *local_prev_values = NULL; // local prev_values with halo

static int grid_dim[2];                               // grid dimensions
static int grid_coord[2];                             // grid coordinates
static int rank_up, rank_down, rank_left, rank_right; // neighbors
static MPI_Comm comm2d; // 2D communicator for Cartesian topology

static MPI_Datatype halo_column; // halo column datatype
static MPI_Datatype halo_row;    // halo row datatype

#define IND(x, y)                                                              \
  ((x) + (local_size_x + 2) * (y)) // 2D indexing macro with halo

static void setup_process() {
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
}

static void setup_2D_topology() {
  // Compute the grid dimensions
  grid_dim[0] = grid_dim[1] = 0;
  MPI_Dims_create(size, 2, grid_dim);

  // Create the 2D Cartesian communicator
  MPI_Cart_create(MPI_COMM_WORLD, 2, grid_dim, (int[]){0, 0}, 0, &comm2d);
  MPI_Cart_coords(comm2d, rank, 2, grid_coord);

  // Compute the neighbors
  MPI_Cart_shift(comm2d, 0, 1, &rank_left, &rank_right);
  MPI_Cart_shift(comm2d, 1, 1, &rank_up, &rank_down);

  // Compute the local size without halo or borders
  local_size_x = (size_x - 2) / grid_dim[0];
  local_size_y = (size_y - 2) / grid_dim[1];
}

static void allocate_local_stencil() {
  // Allocate the local arrays with halos and initialize to 0
  local_values =
      malloc((local_size_x + 2) * (local_size_y + 2) * sizeof(stencil_t));
  local_prev_values =
      malloc((local_size_x + 2) * (local_size_y + 2) * sizeof(stencil_t));
  memset(local_values, 0,
         (local_size_x + 2) * (local_size_y + 2) * sizeof(stencil_t));
  memset(local_prev_values, 0,
         (local_size_x + 2) * (local_size_y + 2) * sizeof(stencil_t));
}

static void clean_process() {
  free(local_values);
  free(local_prev_values);
  MPI_Type_free(&halo_column);
  MPI_Type_free(&halo_row);
  MPI_Comm_free(&comm2d);
  MPI_Finalize();
}

static void create_halo_type() {
  // Create the halo column datatype
  MPI_Type_vector(local_size_y, 1, local_size_x + 2, MPI_FLOAT, &halo_column);
  MPI_Type_commit(&halo_column);

  // Create the halo row datatype
  MPI_Type_contiguous(local_size_x, MPI_FLOAT, &halo_row);
  MPI_Type_commit(&halo_row);
}

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

static int setup_option(int argc, char **argv) {
  if (rank == 0) {
    int stencil_size;
    int opt;
    while ((opt = getopt(argc, argv, "t")) != -1) {
      switch (opt) {
      case 't':
        test_mode = 1;
        break;
      default:
        fprintf(stderr, "Usage: %s [stencil size] [-t]\n", argv[0]);
        return -1;
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

    MPI_Bcast(&size_x, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&size_y, 1, MPI_INT, 0, MPI_COMM_WORLD);

    printf("# init:\n");
    stencil_init();
    printf("# size = %d\n", stencil_size);
  } else {
    MPI_Bcast(&size_x, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&size_y, 1, MPI_INT, 0, MPI_COMM_WORLD);
  }
  return 0;
}

static void stencil_free(void) {
  free(values);
  free(prev_values);
}

static void stencil_display(int start_x, int end_x, int start_y, int end_y) {
  for (int y = start_y; y <= end_y; y++) {
    for (int x = start_x; x <= end_x; x++) {
      printf("%g ", values[x + size_x * y]);
    }
    printf("\n");
  }
}

static void distribute_stencils() {
  if (rank == 0) {
    for (int r = 0; r < size; r++) {
      int coords[2];
      MPI_Cart_coords(comm2d, r, 2, coords);

      int start_x = coords[0] * local_size_x;
      int start_y = coords[1] * local_size_y;

      stencil_t *temp =
          malloc((local_size_x + 2) * (local_size_y + 2) * sizeof(stencil_t));
      memset(temp, 0,
             (local_size_x + 2) * (local_size_y + 2) * sizeof(stencil_t));

      for (int y = 0; y < local_size_y + 2; y++) {
        for (int x = 0; x < local_size_x + 2; x++) {
          temp[IND(x, y)] = values[(start_x + x) + size_x * (start_y + y)];
        }
      }

      if (r != 0) {
        MPI_Send(temp, (local_size_x + 2) * (local_size_y + 2), MPI_FLOAT, r, 0,
                 MPI_COMM_WORLD);
      } else {
        memcpy(local_values, temp,
               (local_size_x + 2) * (local_size_y + 2) * sizeof(stencil_t));
        memcpy(local_prev_values, temp,
               (local_size_x + 2) * (local_size_y + 2) * sizeof(stencil_t));
      }

      free(temp);
    }
  } else {
    MPI_Recv(local_values, (local_size_x + 2) * (local_size_y + 2), MPI_FLOAT,
             0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    memcpy(local_prev_values, local_values,
           (local_size_x + 2) * (local_size_y + 2) * sizeof(stencil_t));
  }
}

static void global_stencil() {
  if (rank == 0) {
    for (int r = 0; r < size; r++) {
      int coords[2];
      MPI_Cart_coords(comm2d, r, 2, coords);

      int start_x = coords[0] * local_size_x + 1;
      int start_y = coords[1] * local_size_y + 1;

      if (r != 0) {
        stencil_t *recv_temp =
            malloc(local_size_x * local_size_y * sizeof(stencil_t));
        memset(recv_temp, 0, local_size_x * local_size_y * sizeof(stencil_t));
        MPI_Recv(recv_temp, local_size_x * local_size_y, MPI_FLOAT, r, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int y = 0; y < local_size_y; y++) {
          for (int x = 0; x < local_size_x; x++) {
            values[(start_x + x) + size_x * (start_y + y)] =
                recv_temp[x + local_size_x * y];
          }
        }
      } else {
        for (int y = 0; y < local_size_y; y++) {
          for (int x = 0; x < local_size_x; x++) {
            values[(start_x + x) + size_x * (start_y + y)] =
                local_values[IND(x + 1, y + 1)];
          }
        }
      }
    }
  } else {
    stencil_t *temp = malloc(local_size_x * local_size_y * sizeof(stencil_t));
    memset(temp, 0, local_size_x * local_size_y * sizeof(stencil_t));

    for (int y = 1; y <= local_size_y; y++) {
      for (int x = 1; x <= local_size_x; x++) {
        temp[(x - 1) + local_size_x * (y - 1)] = local_values[IND(x, y)];
      }
    }
    MPI_Send(temp, local_size_x * local_size_y, MPI_FLOAT, 0, 0,
             MPI_COMM_WORLD);
    free(temp);
  }
}

static int stencil_step(void) {
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
      if (convergence && (fabs(prev_values[x + size_x * y] -
                               values[x + size_x * y]) > epsilon)) {
        convergence = 0;
      }
    }
  }
  return convergence;
}

static void test() {
  printf("Test mode\n");
  stencil_display(0, size_x - 1, 0, size_y - 1);
  stencil_t *test_values = malloc(size_x * size_y * sizeof(stencil_t));
  memcpy(test_values, values, size_x * size_y * sizeof(stencil_t));
  stencil_free();
  stencil_init();
  int s;
  int convergence = 0;
  for (s = 0; s < stencil_max_steps; s++) {
    convergence = stencil_step();
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

  stencil_free();
}

static void halo() {

#pragma omp master
  {

    MPI_Sendrecv(&local_values[IND(1, 1)], 1, halo_row, rank_up, 0,
                 &local_values[IND(1, local_size_y + 1)], 1, halo_row,
                 rank_down, 0, comm2d, MPI_STATUS_IGNORE);

    MPI_Sendrecv(&local_values[IND(1, local_size_y)], 1, halo_row, rank_down, 0,
                 &local_values[IND(1, 0)], 1, halo_row, rank_up, 0, comm2d,
                 MPI_STATUS_IGNORE);

    MPI_Sendrecv(&local_values[IND(1, 1)], 1, halo_column, rank_left, 0,
                 &local_values[IND(local_size_x + 1, 1)], 1, halo_column,
                 rank_right, 0, comm2d, MPI_STATUS_IGNORE);

    MPI_Sendrecv(&local_values[IND(local_size_x, 1)], 1, halo_column,
                 rank_right, 0, &local_values[IND(0, 1)], 1, halo_column,
                 rank_left, 0, comm2d, MPI_STATUS_IGNORE);
  }
#pragma omp barrier
}

static int stencil_step_hybrid(void) {
  int convergence = 1;

  stencil_t *tmp = local_prev_values;
  local_prev_values = local_values;
  local_values = tmp;

#pragma omp parallel for collapse(2) shared(local_values, local_prev_values)   \
    reduction(& : convergence)
  for (int y = 1; y < local_size_y + 1; y++) {
    for (int x = 1; x < local_size_x + 1; x++) {
      local_values[IND(x, y)] =
          alpha * (local_prev_values[IND(x - 1, y)] +
                   local_prev_values[IND(x + 1, y)] +
                   local_prev_values[IND(x, y - 1)] +
                   local_prev_values[IND(x, y + 1)]) +
          (1.0 - 4.0 * alpha) * local_prev_values[IND(x, y)];
      if (fabs(local_prev_values[IND(x, y)] - local_values[IND(x, y)]) >
          epsilon) {
        convergence = 0;
      }
    }
  }
  halo();
  return convergence;
}

int main(int argc, char **argv) {

  MPI_Init(&argc, &argv);
  setup_process();

  if (setup_option(argc, argv) != 0) {
    return EXIT_FAILURE;
  }

  setup_2D_topology();
  allocate_local_stencil();
  create_halo_type();

  struct timespec t1, t2;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  distribute_stencils();
  int s;
  int global_convergence = 0;
  for (s = 0; s < stencil_max_steps; s++) {
    int local_convergence = stencil_step_hybrid();
    MPI_Allreduce(&local_convergence, &global_convergence, 1, MPI_INT, MPI_LAND,
                  MPI_COMM_WORLD);
    if (global_convergence) {
      break;
    }
  }
  global_stencil();
  clock_gettime(CLOCK_MONOTONIC, &t2);

  if (rank == 0) {
    const double t_usec = (t2.tv_sec - t1.tv_sec) * 1000000.0 +
                          (t2.tv_nsec - t1.tv_nsec) / 1000.0;
    printf("# steps = %d\n", s);
    printf("# time = %g usecs.\n", t_usec);
    printf("# gflops = %g\n", (6.0 * size_x * size_y * s) / (t_usec * 1000));
  }

  if (test_mode) {
    test();
  }

  clean_process();
}
