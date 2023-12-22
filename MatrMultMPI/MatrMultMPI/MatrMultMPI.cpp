#include <mpi.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
    setlocale(LC_ALL, "Russian");

    MPI_Init(NULL, NULL);

    int process_count;
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);

    int process_index;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_index);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_length;
    MPI_Get_processor_name(processor_name, &name_length);

    int* matrixA = NULL, * matrixB = NULL, M = -1, aColumn = -1, bRow = -1, N = -1; //aColumn = bRow = K

    int partOfARow, * partOfMatrixA = NULL, * partOfResult = NULL;

    int remain, * matrixRes;

    int* arr;

    double timeStart;

    if (process_index == 0) {
        printf("������ �� �����\n\n");
        //0) ������ �� �����
        string line, word;
        ifstream in("C:\\OwnGame\\Matrix Multiplication\\MatrMultMPI\\MatrMultMPI\\matrix.txt");
        int count = 0;

        if (in.is_open()) {
            int i = 0;
            int row = NULL, column = NULL;
            while (getline(in, line)) {
                if (line.length() == 0) continue;
                istringstream iss(line);
                bool isSize = false;
                int j = 0;
                while (iss >> word) {
                    if (word == "!") {
                        isSize = true;
                        i = -1;
                        row = NULL;
                        column = NULL;
                        continue;
                    } if (isSize) {
                        if (row == NULL)
                            row = stoi(word);
                        else {
                            column = stoi(word);
                            count++;
                            if (count == 1) {
                                matrixA = (int*)malloc(row * column * sizeof(int));
                                M = row;
                                aColumn = column;
                            }
                            else if (count == 2) {
                                matrixB = (int*)malloc(row * column * sizeof(int));
                                bRow = row;
                                N = column;
                            }
                        }
                    }
                    else if (count > 0) {
                        isSize = false;
                        if (count == 1) {
                            matrixA[i * column + j] = stoi(word);
                        }
                        else if (count == 2) {
                            matrixB[i * column + j] = stoi(word);
                        }
                    }
                    j++;
                }
                i++;
            }
        }
        in.close();

        /*printf("A[%ix%i]:\n", M, aColumn);
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < aColumn; j++) {
                printf("%i ", matrixA[i * aColumn + j]);
            }
            printf("\n");
        }

        printf("\nB[%ix%i]:\n", bRow, N);
        for (int i = 0; i < bRow; i++) {
            for (int j = 0; j < N; j++) {
                printf("%i ", matrixB[i * N + j]);
            }
            printf("\n");
        }*/
        arr = (int*)new int[4] { M, aColumn, bRow, N};
        MPI_Bcast(arr, 4, MPI_INT, 0, MPI_COMM_WORLD);

        timeStart = MPI_Wtime();

        printf("������� ������\n\n");

    } else {
        arr = (int*)malloc(4 * sizeof(int));
        MPI_Bcast(arr, 4, MPI_INT, 0, MPI_COMM_WORLD);
        M = arr[0];
        aColumn = arr[1];
        bRow = arr[2];
        N = arr[3];

        matrixA = (int*)malloc(M * aColumn * sizeof(int));
        matrixB = (int*)malloc(bRow * N * sizeof(int));
    }

    //1) �������� �� ����� � ��������� �������
    partOfARow = M / process_count;
    int size = partOfARow * aColumn;
    remain = M % process_count;
    partOfMatrixA = (int*)malloc(size * sizeof(int));
    partOfResult = (int*)malloc(partOfARow * N * sizeof(int));
    matrixRes = (int*)malloc(M * N * sizeof(int));


    //2) ��������� ��������� ������� ������ �� ���������
    MPI_Scatter(matrixA, size, MPI_INT, partOfMatrixA, size, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(matrixB, bRow * N, MPI_INT, 0, MPI_COMM_WORLD);

    //3) ������� �������� �� ������� ������
    for (int i = 0; i < partOfARow; i++) {
        for (int j = 0; j < N; j++) {
            int res = 0;
            for (int k = 0; k < aColumn; k++) {
                res += partOfMatrixA[i * aColumn + k] * matrixB[k * N + j];
            }
            partOfResult[i * N + j] = res;
        }
    }
    free(partOfMatrixA);

    //4) ���� �����������
    MPI_Gather(partOfResult, partOfARow * N, MPI_INT, matrixRes, partOfARow * N, MPI_INT, 0, MPI_COMM_WORLD);
    free(partOfResult);

    //5) �������� �������
    if (process_index == 0 && remain != 0) {
        for (int i = M - remain; i < M; i++) {
            for (int j = 0; j < N; j++) {
                int res = 0;
                for (int k = 0; k < aColumn; k++) {
                    res += matrixA[i * aColumn + k] * matrixB[k * N + j];
                }
                matrixRes[i * N + j] = res;
            }
        }
    }
  
    free(matrixA);
    free(matrixB);

    //6) �����
    if (process_index == 0) {
        printf("\n���������! �����  ���������� �� %i �������: %lfs\n\n", process_count, MPI_Wtime() - timeStart);
        /*printf("Result:\n");
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                printf("%i ", matrixRes[i * N + j]);
            }
            printf("\n");
        }*/
    }

    MPI_Finalize();
}
