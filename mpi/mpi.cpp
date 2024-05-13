#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MAX_FILENAME_LENGTH 256

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0)
            printf("Usage: mpirun -n <number of processes> %s <file1> <file2> ... <fileN>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    // Determine number of files
    int num_files = argc - 1;

    // Open output file
    FILE *output_file;
    if (rank == 0)
        output_file = fopen("res.txt", "w");

    // Calculate sum of elements in each file
    double local_sum = 0.0;
    for (int i = rank; i < num_files; i += size) {
        char filename[MAX_FILENAME_LENGTH];
        snprintf(filename, MAX_FILENAME_LENGTH, "%s", argv[i + 1]); // Get filename

        FILE *file = fopen(filename, "r");
        if (file == NULL) {
            fprintf(stderr, "Error opening file: %s\n", filename);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        double value;
        while (fscanf(file, "%lf", &value) == 1)
            local_sum += value;

        fclose(file);
    }

    // Gather local sums to process 0
    double global_sum;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Write result to output file
    if (rank == 0) {
        fprintf(output_file, "Sum of elements in all files: %.2lf\n", global_sum);
        fclose(output_file);
    }

    MPI_Finalize();
    return 0;
}
