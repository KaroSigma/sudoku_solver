#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define SIZE 9
#define BOX 3

// Generowanie liczb losowych
void init_random() {
    srand(time(NULL));
}

// Funkcja wyswietlenia macierzy Sudoku
void print_matrix(int matrix[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        if (i % BOX == 0 && i != 0) {
            printf("------ ------- ------\n");
        }
        for (int j = 0; j < SIZE; j++) {
            if (j % BOX == 0 && j != 0) {
                printf("| ");
            }
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

// Funkcja obliczająca koszt czyli liczbę konfliktow w wierszach oraz w kolumnach
int calculate_cost(int matrix[SIZE][SIZE]) {
    int cost = 0;
    
    // Konflikty w wierszach row
    for (int i = 0; i < SIZE; i++) {
        int count_row[SIZE + 1] = {0};
        for (int j = 0; j < SIZE; j++) {
            count_row[matrix[i][j]]++;
        }
        
        for (int k = 1; k <= SIZE; k++) {
            if (count_row[k] > 1) cost += count_row[k] - 1;
        }
    }
    
    // Konflikty w kolumnach col
    for (int j = 0; j < SIZE; j++) {
        int count_coul[SIZE + 1] = {0};
        for (int i = 0; i < SIZE; i++) {
            count_coul[matrix[i][j]]++;
        }
        for (int k = 1; k <= SIZE; k++) {
            if (count_coul[k] > 1) cost += count_coul[k] - 1;
        }
    }
    return cost;
}

// Przygotowuje plansze Sudoku zeby w kazdym malym kwadracie 3x3 byly wszystkie liczby od 1 do 9 (Jeszcze nie rozwiazuje)
void generate_box(int boxx[SIZE][SIZE], bool num[SIZE][SIZE]) {
    for (int block_row = 0; block_row < BOX; block_row++) {
        for (int block_col = 0; block_col < BOX; block_col++) {
            
            //Sprawdza jakie liczby sa juz w kwadraciku3x3
            bool now[SIZE + 1] = {false};
            for (int i = block_row * BOX; i < (block_row + 1) * BOX; i++) {
                for (int j = block_col * BOX; j < (block_col + 1) * BOX; j++) {
                    if (num[i][j] && boxx[i][j] != 0) {
                        now[boxx[i][j]] = true; // "Zbiera liczby ktora juz sa
                    }
                }
            }
            
            // Tworzy liste brakujących cyfr od 1 do 9
            int missingnum[SIZE];
            int missingnum_count = 0;
            for (int num = 1; num <= SIZE; num++) {
                if (!now[num]) {
                    missingnum[missingnum_count++] = num;
                }
            }
            
            // Wypełnij komorki w bloku brakujacymi liczbami
            int x = 0;
            for (int i = block_row * BOX; i < (block_row + 1) * BOX; i++) {
                for (int j = block_col * BOX; j < (block_col + 1) * BOX; j++) {
                    if (!num[i][j]) {
                        
                        // Losuje brakujace liczby zeby liczby nie staly w tej samej kolejnosci w kazdym 3x3 kwadracie
                        if (x < missingnum_count) {
                            int swap_x = x + rand() % (missingnum_count - x);
                            int temp = missingnum[swap_x];
                            missingnum[swap_x] = missingnum[x];
                            missingnum[x] = temp;
                            boxx[i][j] = missingnum[x++];
                        }
                    }
                }
            }
        }
    }
}

// Funkcja generujaca sąsiada przez zamiane dwoch komorek w bloku 
//Ten kod probuje rozwiązać Sudoku, zamieniajac miejscami dwie liczby w kwadratach 3x3 
//i sprawdzajac czy po zamianie jest mniej bledow (powtorzen w wierszach kolumnach). Dziala jak "zgadywanka" albo "cieplo-zimno"
int generate_neighbor(int matrix[SIZE][SIZE], bool num[SIZE][SIZE], int *row1, int *col1, int *row2, int *col2) {
    // Wybierz losowy blok(box)
    int block_x = rand() % (SIZE);
    int block_row = block_x / BOX;
    int block_col = block_x % BOX;

    //Zbierz wszystkie pola w tym bloku ktore mozna ruszac
    int cells[SIZE * SIZE][2];
    int count = 0;
    for (int i = block_row * BOX; i < (block_row + 1) * BOX; i++) {
        for (int j = block_col * BOX; j < (block_col + 1) * BOX; j++) {
            if (!num[i][j]) { // Jesli pole nie jest zablokowane
                cells[count][0] = i;
                cells[count][1] = j;
                count++;
            }
        }
    }
    //Jezeli jest mniej niz 2 pola do zamiany to koneic (nie da się)
    if (count < 2) return 0;

    // Wybierz dwie rozne pola
    int x1 = rand() % count; // 1 pole
    int x2; 
    do {
        x2 = rand() % count; // 2 pole
    } while (x2 == x1);

    *row1 = cells[x1][0];  // Wspolrzedne 1 pola
    *col1 = cells[x1][1];
    *row2 = cells[x2][0];// Wspolrzedne 2 pola
    *col2 = cells[x2][1];

    return 1;
}

// Główna funkcja SA dla Sudoku
void solve_sudoku(int matrix[SIZE][SIZE], bool num[SIZE][SIZE], double T_start, double T_end, double alpha, int max_iterations) {
    int current_cost = calculate_cost(matrix);
    int best_cost = current_cost;
    int best_matrix[SIZE][SIZE];
    memcpy(best_matrix, matrix, SIZE * SIZE * sizeof(int));

    double T = T_start;
    int iteration = 0;
    
    while (T > T_end && iteration < max_iterations && best_cost > 0) {
        //Zapamietuje aktualny stan i koszt
        int old_row1, old_col1, old_row2, old_col2;
        int old_value1, old_value2;
        
        //Generuje sasiada
        if (generate_neighbor(matrix, num, &old_row1, &old_col1, &old_row2, &old_col2)) {
            // Zapamietaj stare wartosci
            old_value1 = matrix[old_row1][old_col1];
            old_value2 = matrix[old_row2][old_col2];
            
            //Wykonaj zamiane
            matrix[old_row1][old_col1] = old_value2;
            matrix[old_row2][old_col2] = old_value1;
            
            //Oblicz nowy koszt
            int new_cost = calculate_cost(matrix);
            int all = new_cost - current_cost;
            
            //Zatwierdzenie nowej wersji
            if (all < 0 || (exp(-all / T) > (double)rand() / RAND_MAX)) {
                current_cost = new_cost;
                if (current_cost < best_cost) {
                    best_cost = current_cost;
                    memcpy(best_matrix, matrix, SIZE * SIZE * sizeof(int));
                }
            } else {
                // Cofnij zamiane
                matrix[old_row1][old_col1] = old_value1;
                matrix[old_row2][old_col2] = old_value2;
            }
        }
        // Chłodzenie
        T *= alpha;
        iteration++;
    }
    
    // Najlepsze rozwiazanie
    memcpy(matrix, best_matrix, SIZE * SIZE * sizeof(int));
    printf("\nFinal cost: %d\n", best_cost);
}

int main() {
    init_random();
    
    // Przykladowa macierz Sudoku
    int matrix[SIZE][SIZE] = {
        {5, 3, 0, 0, 7, 0, 0, 0, 0},
        {6, 0, 0, 1, 9, 5, 0, 0, 0},
        {0, 9, 8, 0, 0, 0, 0, 6, 0},
        {8, 0, 0, 0, 6, 0, 0, 0, 3},
        {4, 0, 0, 8, 0, 3, 0, 0, 1},
        {7, 0, 0, 0, 2, 0, 0, 0, 6},
        {0, 6, 0, 0, 0, 0, 2, 8, 0},
        {0, 0, 0, 4, 1, 9, 0, 0, 5},
        {0, 0, 0, 0, 8, 0, 0, 7, 9}
    };

    // Wybierz komórki
    bool num[SIZE][SIZE] = {false};
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (matrix[i][j] != 0) {
                num[i][j] = true;
            }
        }
    }

    printf("Initial matrix:\n");
    print_matrix(matrix);
    generate_box(matrix, num);
    
    printf("\nAfter initialization:\n");
    print_matrix(matrix);
    
    printf("\nCost: %d\n\n", calculate_cost(matrix));

    // Parametry SA
    double T_start = 1000.0; // Wyższa temp. początkowa dla problemów z większą skalą wartości
    double T_end = 0.01;
    double alpha = 0.99; // Współczynnik chłodzenia
    int max_iterations = 200000; // Więcej iteracji może być potrzebne

    solve_sudoku(matrix, num, T_start, T_end, alpha, max_iterations);

    printf("\nSolved Sudoku:\n");
    print_matrix(matrix);

    return 0;
}