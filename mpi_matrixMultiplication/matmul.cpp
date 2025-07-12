#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

// Helper to read a matrix from a file
vector<vector<double>> readMatrix(const string& filename, int &rows, int &cols) {
    ifstream file(filename);
    string line;
    vector<vector<double>> matrix;
    rows = 0;
    cols = 0;

    while (getline(file, line)) {
        stringstream ss(line);
        double val;
        vector<double> row;
        while (ss >> val) row.push_back(val);
        if (cols == 0) cols = row.size();
        matrix.push_back(row);
        rows++;
    }
    return matrix;
}

// Helper to write matrix to file
void writeMatrix(const string& filename, const vector<vector<double>>& matrix) {
    ofstream file(filename);
    for (const auto& row : matrix) {
        for (double val : row)
            file << val << " ";
        file << "\n";
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // rank of current process
    MPI_Comm_size(MPI_COMM_WORLD, &size); // total processes

    int A_rows = 0, A_cols = 0, B_rows = 0, B_cols = 0;
    vector<vector<double>> A, B;

    if (rank == 0) {
        // Read matrices only in root
        A = readMatrix("matrixA.txt", A_rows, A_cols);
        B = readMatrix("matrixB.txt", B_rows, B_cols);

        if (A_cols != B_rows) {
            cerr << "Matrix dimensions mismatch for multiplication\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Broadcast dimensions
    MPI_Bcast(&A_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&A_cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&B_cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Flatten matrix B and broadcast it
    vector<double> flatB;
    if (rank == 0) {
        for (const auto& row : B)
            flatB.insert(flatB.end(), row.begin(), row.end());
    }
    flatB.resize(A_cols * B_cols);
    MPI_Bcast(flatB.data(), A_cols * B_cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Scatter rows of A
    int rows_per_proc = A_rows / size;
    int remainder = A_rows % size;
    int local_rows = rows_per_proc + (rank < remainder ? 1 : 0);

    vector<double> localA(local_rows * A_cols);
    vector<int> sendcounts(size), displs(size);
    if (rank == 0) {
        int offset = 0;
        for (int i = 0; i < size; ++i) {
            int rows = rows_per_proc + (i < remainder ? 1 : 0);
            sendcounts[i] = rows * A_cols;
            displs[i] = offset;
            offset += rows * A_cols;
        }

        vector<double> flatA;
        for (const auto& row : A)
            flatA.insert(flatA.end(), row.begin(), row.end());

        MPI_Scatterv(flatA.data(), sendcounts.data(), displs.data(), MPI_DOUBLE,
                     localA.data(), local_rows * A_cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    } else {
        MPI_Scatterv(nullptr, nullptr, nullptr, MPI_DOUBLE,
                     localA.data(), local_rows * A_cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    // Multiply local rows
    vector<double> localC(local_rows * B_cols, 0);
    for (int i = 0; i < local_rows; ++i) {
        for (int j = 0; j < B_cols; ++j) {
            for (int k = 0; k < A_cols; ++k) {
                localC[i * B_cols + j] += localA[i * A_cols + k] * flatB[k * B_cols + j];
            }
        }
    }

    // Gather results
    vector<int> recvcounts(size), rdispls(size);
    if (rank == 0) {
        int offset = 0;
        for (int i = 0; i < size; ++i) {
            int rows = rows_per_proc + (i < remainder ? 1 : 0);
            recvcounts[i] = rows * B_cols;
            rdispls[i] = offset;
            offset += rows * B_cols;
        }
    }

    vector<double> flatC;
    if (rank == 0) flatC.resize(A_rows * B_cols);

    MPI_Gatherv(localC.data(), local_rows * B_cols, MPI_DOUBLE,
                flatC.data(), recvcounts.data(), rdispls.data(), MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        vector<vector<double>> result(A_rows, vector<double>(B_cols));
        for (int i = 0; i < A_rows; ++i)
            for (int j = 0; j < B_cols; ++j)
                result[i][j] = flatC[i * B_cols + j];
        writeMatrix("result.txt", result);
        cout << "Matrix multiplication complete. Result written to result.txt\n";
    }

    MPI_Finalize();
    return 0;
}
