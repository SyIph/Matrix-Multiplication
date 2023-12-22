#include <stdio.h>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include<omp.h>
using namespace std;



int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");

    int process_count = 3;

    if (argc > 1)
        process_count = stoi(argv[1]);

    int* matrixA = NULL, * matrixB = NULL, M = -1, aColumn = -1, bRow = -1, N = -1; //aColumn = bRow = K

    int * matrixRes;

    int* arr;   

    //0) ������ �� �����
    string line, word;
    ifstream in("C:\\OwnGame\\Matrix Multiplication\\MatrMultOpenMP\\MatrMultOpenMP\\matrix.txt");
    int count = 0;

    if (in.is_open()) {
        printf("������ �� �����\n\n");
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

    printf("������� ������\n\n");

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

    unsigned int timeStart = clock();

    arr = (int*)new int[4] { M, aColumn, bRow, N};

    //1) �������� �� ����� � ��������� �������
    int partOfARow = M / process_count;
    int size = partOfARow * aColumn;
    int remain = M % process_count;
    matrixRes = (int*)malloc(M * N * sizeof(int));



    #pragma omp parallel num_threads(process_count) 
    {
        int process_index = omp_get_thread_num(), i0 = partOfARow * process_index, i1 = partOfARow * (process_index + 1);
        for (int i = i0; i < i1; i++) {
            //printf("{%i} i = %i\n", process_index, i);
            for (int j = 0; j < N; j++) {
                int res = 0;
                for (int k = 0; k < aColumn; k++) {
                    res += matrixA[i * aColumn + k] * matrixB[k * N + j];
                }
                matrixRes[i * N + j] = res;
            }
        }
    }

    //5) �������� �������
    if (remain != 0) {
        for (int i = M - remain; i < M; i++) {
            //printf("{ost} i = %i\n", i);
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

    printf("\n���������! �����  ���������� �� %i �������: %lfs\n\n", process_count, (clock() - timeStart) / 1000.0);

    //6) �����

    /*printf("Result:\n");
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            printf("%i ", matrixRes[i * N + j]);
        }
        printf("\n");
    }*/
}