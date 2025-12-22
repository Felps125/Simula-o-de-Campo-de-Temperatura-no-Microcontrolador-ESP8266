#include <iostream>
#include <iomanip>
#include <ctime>
#include <string>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <windows.h>
#include <vector>
#include <sstream>

using namespace std;

// ------------------ DATA ATUAL ------------------
string hoje() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y", now);
    return buffer;
}

// ------------------ MAIN COMPLETO ------------------
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    cout << fixed << setprecision(6);

    cout << "=================================================================================================\n";
    cout << "*                                                                                               *\n";
    cout << "*                              CURSO DO CÁLCULO NUMÉRICO A SIMULAÇÃO                            *\n";
    cout << "*                                                                                               *\n";
    cout << "=================================================================================================\n";
    cout << "* Programa: Determinação de Campo de Temperatura no Microcontrolador ESP8266                    *\n";
    cout << "* Equação Governante: Equação Geral da Condução de Calor(2D Transiente)                         *\n";
    cout << "* Métodos Numéricos Utilizados: - Método das Diferenças Finitas(MDF);                           *\n";
    cout << "*                               - Gauss Seidel;                                                 *\n";
    cout << "* Ferramenta de Pós Processamento: Paraview 2D                                                  *\n";
    cout << "* Repositório:                                                                                  *\n";
    cout << "* Última Atualização: 13/12/2025                                                                *\n";
    cout << "* Autor: Felipe Antunes Alves                                                                   *\n";
    cout << "*                                                                                               *\n";
    cout << "* https://github.com/Felps125/Simula-o-de-Campo-de-Temperatura-no-Microcontrolador-ESP8266.git  *\n";
    cout << "================================================================================\n\n";

    // Glossário de Variáveis
    int i, j, i_2, j_2, j_3;
    double xsup, xinf, ysup, yinf;
    double deltax, deltay, deltat;

    std::string caminhoArquivo = R"(C:\Users\mfand\Documents\matriz_005mm.txt)";
    std::ifstream arquivo(caminhoArquivo);

    if (!arquivo.is_open()) {
        std::cerr << "Erro: Nao foi possivel encontrar o arquivo em: " << caminhoArquivo << std::endl;
        return 1;
    }
    std::vector<std::vector<int>> matriz;
    std::string linha;
    while (std::getline(arquivo, linha)) {
        std::vector<int> linhaTemporaria;
        std::stringstream ss(linha);
        int valor;
        while (ss >> valor) {
            linhaTemporaria.push_back(valor);
        }
        if (!linhaTemporaria.empty()) {
            matriz.push_back(linhaTemporaria);
        }
    }
    arquivo.close();

    i = matriz.size();
    j = matriz[0].size();
    int size = i * j;

    double *T = new double[size];
    double *T_old = new double[size];
    double *x_out = new double[i];
    double *y_out = new double[j];

    xinf = 0; xsup = 2.5e-2;
    yinf = 0; ysup = 4.9e-2;
    std::cout << "Input de Dados" << std::endl;
    std::cout << "Número de Pontos em x: " << i << std::endl;
    std::cout << "Número de Pontos em y: " << j << std::endl;
    std::cout << "Largura(xsup): " << xsup << std::endl;
    std::cout << "Altura:(ysup): " << ysup << std::endl;

    deltax = (xsup - xinf) / (i - 1);
    deltay = (ysup - yinf) / (j - 1);

    for (i_2 = 0; i_2 < i; i_2++) {
        x_out[i_2] = xinf + i_2 * deltax;
    }
    for (j_2 = 0; j_2 < j; j_2++) {
        y_out[j_2] = yinf + j_2 * deltay;
    }

    deltat = 0.1;

    // Constantes para o Cobre
    double alpha_cobre = 1.11e-4;
    double CT1_cobre = (1.0 / deltat) + 2.0 * alpha_cobre * (1.0 / (deltax * deltax) + 1.0 / (deltay * deltay));
    double CT2_cobre = alpha_cobre / (deltax * deltax);
    double CT3_cobre = alpha_cobre / (deltay * deltay);

    // Constantes para o FR4(Resina)
    double alpha_fr4 = 1.2e-7;
    double CT1_fr4 = (1.0 / deltat) + 2.0 * alpha_fr4 * (1.0 / (deltax * deltax) + 1.0 / (deltay * deltay));
    double CT2_fr4 = alpha_fr4 / (deltax * deltax);
    double CT3_fr4 = alpha_fr4 / (deltay * deltay);

    fill_n(T, size, 25.0);

    //Condições de Contorno
    double T_inf = 12.0, T_sup = 100, T_esq = 50.0, T_dir = 40.0;

    for (j_2 = 0; j_2 < j; j_2++)
    {
        T[j_2] = T_inf;
    }
    for (j_2 = 0; j_2 < j; j_2++)
    {
        T[(i - 1) * j + j_2] = T_sup;
    }
    for (i_2 = 1; i_2 < i - 1; i_2++) {
        T[i_2 * j] = T_esq;
        T[i_2 * j + j - 1] = T_dir;
    }

    double tol = 1e-3;
    int iter_max = 300;
    double omega = 1.7;
    int N_passos = 100;

    std::cout << "\nINICIANDO SIMULAÇÃO...\n" << std::endl;

    for (int passo = 1; passo <= N_passos; passo++) {
        copy(T, T + size, T_old);
        double erro = 1.0;
        int iter = 0;

        while (erro > tol && iter < iter_max) {
            erro = 0.0;
            for (i_2 = 1; i_2 < i - 1; i_2++) {
                int base = i_2 * j;
                for (j_2 = 1; j_2 < j - 1; j_2++) {
                    j_3 = base + j_2;

                    double CT1, CT2, CT3;
                    if (matriz[i_2][j_2] == 0) {
                        CT1 = CT1_cobre; CT2 = CT2_cobre; CT3 = CT3_cobre;
                    }
                    else {
                        CT1 = CT1_fr4; CT2 = CT2_fr4; CT3 = CT3_fr4;
                    }

                    double Told_val = T[j_3];
                    double T_GS = ( (T_old[j_3] / deltat)
                        + CT2 * (T[j_3 + j] + T[j_3 - j])
                        + CT3 * (T[j_3 + 1] + T[j_3 - 1])
                        ) / CT1;

                    T[j_3] = (1.0 - omega) * Told_val + omega * T_GS;

                    double diff = fabs(T[j_3] - Told_val);
                    if (diff > erro) erro = diff;
                }
            }
            iter++;
        }

        if(passo % 5 == 0) {
            cout << "Passo: " << passo << "/" << N_passos << endl;

            // Exportação sequencial para animação
            string nomeArq = "laplace_passo_" + to_string(passo) + ".csv";
            ofstream arq(nomeArq);
            if (arq.is_open()) {
                arq << fixed << setprecision(6) << "X,Y,T\n";
                for (int r = 0; r < i; r++) {
                    for (int c = 0; c < j; c++) {
                        arq << x_out[r] << "," << y_out[c] << "," << T[r * j + c] << "\n";
                    }
                }
                arq.close();
            }
        }
    }

    // Arquivo final
    ofstream arquivo_saida("laplace_transiente.csv");
    if (arquivo_saida.is_open()) {
        arquivo_saida << fixed << setprecision(6) << "X,Y,T\n";
        for (i_2 = 0; i_2 < i; i_2++) {
            for (j_2 = 0; j_2 < j; j_2++) {
                arquivo_saida << x_out[i_2] << "," << y_out[j_2] << "," << T[i_2 * j + j_2] << "\n";
            }
        }
        arquivo_saida.close();
        cout << "\nSIMULAÇÃO CONCLUÍDA COM SUCESSO.\n";
    }

    delete[] T; delete[] T_old; delete[] x_out; delete[] y_out;
    return 0;
}