# Simulação de Campo de Temperatura no Microncontrolador ESP8266

![Status](https://img.shields.io/badge/Status-Em%20Andamento-yellow)
![Platform](https://img.shields.io/badge/Platform-ESP8266%20%7C%20KiCad-blue)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20%7C%20Python-orange)
![Tools](https://img.shields.io/badge/Tools-Paraview%20%7C%20LaTeX%20%7C%20GTL-brightgreen)

## 1. Introdução e Justificativa
Este projeto apresenta o desenvolvimento de um código computacional autoral para solução a **Equação Geral da Condução de Calor** aplicada ao microcontrolador ESP8266. O Método Numérico base utilizado foi o **Método das Diferenças Finitas** em um contexto 2D Transiente. Por fim sua implementação se justifativa primordialmente para proposição futura de um **Sistema Aletado** levando em consideração o campo de temperatura no microcontrolador.

## 2. Fundamentação Teórica e Metodologia
A métodologia, em suma, consistiu nos seguintes procedimentos 

1. **Definição da Equação Governante**
   * Formulação matemática do fenômeno físico.
   * Definição da equação de difusão de calor (ex: Fourier).

2. **Definição do Método Numérico e Condições de Contorno**
   * Escolha do método de discretização (Diferenças Finitas, Volumes Finitos, etc.).
   * Especificação das condições de contorno (Dirichlet, Neumann ou Robin).

3. **Desenvolvimento do Código Base (Parede Plana)**
   * Implementação do algoritmo inicial para geometria de parede plana.
   * Estruturação modular para facilitar futuras adaptações a outras geometrias.

4. **Validações e Testes de Implementação**
   * Testes das condições pré-estabelecidas no código base.
   * Validação final gráfica com utilização de software de pós processamento.

5. **Resultados e Discussões**
   * Análise dos dados gerados.

### 2.1 Equação Governante
O fenômeno de transporte térmico é modelado pela equação geral da condução de calor com geração de calor interna e propriedades heterogêneas:

$$\rho(x,y) c_p(x,y) \frac{\partial T}{\partial t} = \nabla \cdot (k(x,y) \nabla T) + \dot{q}$$

Onde $k(x,y)$ é definido pela matriz de binarização geométrica, assumindo $k_{cu} \approx 400 \, W/m \cdot K$ para trilhas e $k_{fr4} \approx 0.25 \, W/m \cdot K$ para o substrato. Após algumas readequações na equação original, desdensificando nomenclatura tem-se

### 2.2 Binarização Geométrica (Python/GTL)
A discretização do domínio físico utiliza a técnica de **Hit-Test Vetorial** via API `pcbnew`. O script Python interroga a geometria original do KiCad para gerar uma malha estruturada:
* **Domínio Condutor (0):** Identificado via colisão de vetores com polígonos da camada `F.Cu`.
* **Domínio Isolante (1):** Espaço vazio ou regiões de dielétrico.

### 2.3 Discretização Numérica (MDF Aplicado)
Utilizou-se como norteador o **Método das Diferenças Finitas (MDF)** para transformar as equações diferenciais parciais em um sistema de equações algébricas lineares. 

#### 2.3.1 Tratamento de Interfaces (Média Harmônica)
Dada a natureza bimodal da malha (Cobre vs FR4), a condutividade nas interfaces entre nós é calculada via **Média Harmônica**, garantindo a continuidade do fluxo de calor na fronteira de materiais:

$$k_{i+1/2, j} = \frac{2 k_{i,j} k_{i+1,j}}{k_{i,j} + k_{i+1,j}}$$

#### 2.3.2 Formulação Implícita e Estabilidade
Para garantir estabilidade numérica independente do passo de tempo ($\Delta t$), aplicou-se a formulação **implícita**. As derivadas espaciais são aproximadas por diferenças centrais de segunda ordem:

$$\frac{T_{i,j}^{n+1} - T_{i,j}^n}{\Delta t} = \alpha_{i,j} \left[ \frac{T_{i+1,j}^{n+1} - 2T_{i,j}^{n+1} + T_{i-1,j}^{n+1}}{\Delta x^2} + \frac{T_{i,j+1}^{n+1} - 2T_{i,j}^{n+1} + T_{i,j-1}^{n+1}}{\Delta y^2} \right] + \frac{\dot{q}}{\rho c_p}$$

#### 2.3.3 Solver Iterativo (Gauss-Seidel)
O sistema linear resultante é resolvido iterativamente. O algoritmo de **Gauss-Seidel** percorre a malha atualizando os valores de temperatura até que o resíduo máximo seja inferior à tolerância estipulada ($\epsilon < 10^{-6}$):

$$T_{i,j}^{k+1} = \frac{1}{1+2\beta_x+2\beta_y} \left( T_{i,j}^n + \beta_x(T_{i+1,j}^k + T_{i-1,j}^{k+1}) + \beta_y(T_{i,j+1}^k + T_{i,j-1}^{k+1}) + S_{i,j} \Delta t \right)$$

Onde $\beta = \alpha \Delta t / \Delta x^2$.



## 3. Implementação e Arquitetura

O projeto é dividido em dois módulos principais integrados:

### 3.1 Extrator de Malha (`extract_gtl.py`)
Realiza o mapeamento espacial do ESP8266. O usuário define a resolução ($res$), que impacta diretamente na precisão da localização:

| Resolução | Erro de Borda ($E_{max}$) | Aplicação |
| :--- | :--- | :--- |
| 0.2 mm | 0.100 mm | Validação rápida de fluxo térmico macro. |
| 0.1 mm | 0.050 mm | Localização precisa de Pads e Vias. |
| 0.05 mm | 0.025 mm | Análise de trilhas de alta densidade (Fine Pitch). |

### 3.2 Solver Térmico (`src/main.cpp`)
O motor em C++ processa a matriz binária e aplica o loop transiente:
1. **Leitura de Matriz:** Parsing do arquivo `.txt` gerado pelo extrator.
2. **Setup Termofísico:** Atribuição dinâmica de condutividade baseada nos bits da matriz ($0 \rightarrow k_{cu}$, $1 \rightarrow k_{fr4}$).
3. **Condições de Contorno:** Aplicação de fronteiras de Dirichlet (Temperaturas fixas) ou Neumann (Isolamento).
4. **Exportação Científica:** Geração de arquivos `.csv` para visualização no ParaView.



## 4. Como Executar

### Pré-requisitos
* **Python 3.x** + Bibliotecas do KiCad.
* **Compilador C++** (GCC/g++ compatível com C++17).
* **ParaView** para visualização dos campos de temperatura.

### Pipeline de Trabalho
```bash
# 1. Gerar a matriz binária no console do KiCad:
# Copie e execute o script de binarização para gerar 'matriz_visual.txt'

# 2. Compilar o solver de simulação:
g++ -O3 -o simulacao_termica src/main.cpp

# 3. Executar a simulação:
./simulacao_termica


