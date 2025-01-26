#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "matriz_led.pio.h"

#define NUM_PIXELS 25 //número de leds na matriz
#define LED_PIN 7 //pino de saída do led

// Definição de pixel GRB
struct pixel_t {
  uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[NUM_PIXELS];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin) {

  // Cria programa PIO.
  uint offset = pio_add_program(pio0, &pio_matrix_program);
  np_pio = pio0;

  // Toma posse de uma máquina PIO.
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0) {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
  }

  // Inicia programa na máquina PIO obtida.
  pio_matrix_program_init(np_pio, sm, offset, pin);

  // Limpa buffer de pixels.
  for (uint i = 0; i < NUM_PIXELS; ++i) {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}

/**
 * Atribui uma cor RGB a um LED.
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

/**
 * Limpa o buffer de pixels.
 */
void npClear() {
  for (uint i = 0; i < NUM_PIXELS; ++i)
    npSetLED(i, 0, 0, 0);
}

//Função para habilitar o modo Bootsel
void bootsel(){
  reset_usb_boot(0,0);
}

//TECLADO
//definição das colunas e das linhas e mapeamento do teclado
uint8_t coluna[4] = {21,20,19,18};
uint8_t linha[4] = {28,27,26,22};

char teclas[4][4] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'};

//função para inicializar o teclado
void inicializar_teclado() {
  for (int i = 0; i < 4; i++) {
    gpio_init(linha[i]);
    gpio_set_dir(linha[i], GPIO_OUT);
    gpio_put(linha[i], 1); // Inicialmente em HIGH

    gpio_init(coluna[i]);
    gpio_set_dir(coluna[i], GPIO_IN);
    gpio_pull_up(coluna[i]); // Habilita pull-up nas colunas
  }
}

//função para ler o teclado
char ler_teclado(uint8_t *colunas, uint8_t *linhas) {
  for (int i = 0; i < 4; i++) {
    gpio_put(linhas[i], 0);
    for (int j = 0; j < 4; j++) {
            if (!gpio_get(colunas[j])) { // Verifica se a coluna está LOW
                gpio_put(linhas[i], 1);  // Restaura a linha
                return teclas[i][j];    // Retorna a tecla correspondente
            }
        }
    gpio_put(linhas[i], 1);
  }
  return 0;
}

//MATRIZ DE LEDS
//rotina para definição da intensidade de cores do led
uint matrix_rgb(float r, float g, float b)
{
  unsigned char R, G, B;
  R = r * 255;
  G = g * 255;
  B = b * 255;
  return (G << 24) | (R << 16) | (B << 8);
}

void npWrite() {
  // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
  for (uint i = 0; i < NUM_PIXELS; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}

// Função para converter a posição do matriz para uma posição do vetor.
int getIndex(int x, int y) {
    // Se a linha for par (0, 2, 4), percorremos da esquerda para a direita.
    // Se a linha for ímpar (1, 3), percorremos da direita para a esquerda.
    if (y % 2 == 0) {
        return 24-(y * 5 + x); // Linha par (esquerda para direita).
    } else {
        return 24-(y * 5 + (4 - x)); // Linha ímpar (direita para esquerda).
    }
}

//Funcao para desenhar a matriz
void desenhaMatriz(int matriz[5][5][3], int tempo_ms, float intensidade){
    for (int linha = 0; linha < 5; linha++){
        for (int coluna = 0; coluna < 5; coluna++){
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, (matriz[coluna][linha][0]*intensidade), (matriz[coluna][linha][1]*intensidade), (matriz[coluna][linha][2]*intensidade));
        }
    }
    }
    npWrite();
    sleep_ms(tempo_ms);
    npClear();
}

void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b){

    for (int16_t i = 0; i < NUM_PIXELS; i++) {
            valor_led = matrix_rgb(desenho[i] * r, desenho[i] * g, desenho[i] * b);
            pio_sm_put_blocking(pio, sm, valor_led);
        };
}

double apagar_leds[25] = {0.0, 0.0, 0.0, 0.0, 0.0,           //Apagar LEDs da matriz
                          0.0, 0.0, 0.0, 0.0, 0.0, 
                          0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 0.0};

double acender_leds[25] = {1.0, 1.0, 1.0, 1.0, 1.0, // Acender LEDs da matriz
                           1.0, 1.0, 1.0, 1.0, 1.0,
                           1.0, 1.0, 1.0, 1.0, 1.0,
                           1.0, 1.0, 1.0, 1.0, 1.0,
                           1.0, 1.0, 1.0, 1.0, 1.0};

double desenho1_1[25] = {0.0, 0.0, 0.3, 0.0, 0.0,           //Desenho Tecla 1 Parte 1
                         0.0, 0.0, 0.3, 0.0, 0.0, 
                         0.0, 0.0, 0.3, 0.0, 0.0,
                         0.0, 0.3, 0.0, 0.3, 0.0,
                         0.3, 0.0, 0.0, 0.0, 0.3};

double desenho1_2[25] = {0.3, 0.3, 0.3, 0.3, 0.3,           //Desenho Tecla 1 Parte 2
                         0.3, 0.0, 0.0, 0.0, 0.3, 
                         0.3, 0.0, 0.0, 0.0, 0.3,
                         0.3, 0.0, 0.0, 0.0, 0.3,
                         0.3, 0.3, 0.3, 0.3, 0.3};

double desenho1_3[25] = {0.3, 0.3, 0.3, 0.3, 0.3,           //Desenho Tecla 1 Parte 3
                         0.3, 0.0, 0.0, 0.0, 0.3, 
                         0.3, 0.0, 0.0, 0.0, 0.3,
                         0.3, 0.0, 0.0, 0.0, 0.3,
                         0.3, 0.0, 0.0, 0.0, 0.3};

double desenho1_4[25] = {0.0, 0.3, 0.3, 0.3, 0.0,           //Desenho Tecla 1 Parte 4
                         0.3, 0.3, 0.3, 0.3, 0.3, 
                         0.3, 0.0, 0.3, 0.0, 0.3,
                         0.3, 0.0, 0.3, 0.0, 0.3,
                         0.3, 0.0, 0.0, 0.0, 0.3};

double desenho1_5[25] = {0.0, 0.0, 0.3, 0.0, 0.0,           //Desenho Tecla 1 Parte 5
                         0.0, 0.0, 0.3, 0.0, 0.0, 
                         0.0, 0.0, 0.3, 0.0, 0.0,
                         0.0, 0.0, 0.3, 0.0, 0.0,
                         0.0, 0.0, 0.3, 0.0, 0.0};

double desenho1_6[25] = {0.3, 0.0, 0.0, 0.0, 0.3,           //Desenho Tecla 1 Parte 6
                         0.3, 0.0, 0.0, 0.3, 0.3, 
                         0.3, 0.0, 0.3, 0.0, 0.3,
                         0.3, 0.3, 0.0, 0.0, 0.3,
                         0.3, 0.0, 0.0, 0.0, 0.3};


//função principal
int main()
{
    PIO pio = pio0; 
    bool frequenciaClock;
    uint16_t i;
    uint valor_led;
    float r = 0.0, b = 0.0 , g = 0.0;

    frequenciaClock = set_sys_clock_khz(128000, false); //frequência de clock
    stdio_init_all();
    inicializar_teclado();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    printf("iniciando a transmissão PIO");
    if (frequenciaClock) printf("clock set to %ld\n", clock_get_hz(clk_sys));

    //configurações da PIO
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, LED_PIN);

    while (true) {
    
    char tecla = ler_teclado(coluna, linha);

    if (tecla)
    {
      printf("Tecla pressionada: %c\n", tecla);

    switch (tecla)
    {
    case '1':                             // Verifica se a tecla 1 foi pressionada

        desenho_pio(desenho1_1, valor_led, pio, sm, r, 1.0, b);
        sleep_ms(500);
        desenho_pio(desenho1_2, valor_led, pio, sm, r, 1.0, b);
        sleep_ms(500);
        desenho_pio(desenho1_3, valor_led, pio, sm, r, 1.0, b);
        sleep_ms(500);
        desenho_pio(apagar_leds, valor_led, pio, sm, r, 1.0, b);
        sleep_ms(500);
        desenho_pio(desenho1_4, valor_led, pio, sm, r, 1.0, b);
        sleep_ms(500);
        desenho_pio(desenho1_5, valor_led, pio, sm, r, 1.0, b);
        sleep_ms(500);
        desenho_pio(desenho1_6, valor_led, pio, sm, r, 1.0, b);
        sleep_ms(500);
        desenho_pio(apagar_leds, valor_led, pio, sm, r, 1.0, b);

        break;
    
    case '2':                                 // Verifica se a tecla 2 foi pressionada



        break;

    case '3':                                 // Verifica se a tecla 3 foi pressionada

// Representação do Omnitrix descarregando pela matriz de led
//Omnitrix no estado verde de escolha
 int animacao0[5][5][3] = {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
  };
   desenhaMatriz(animacao0, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
    int animacao1[5][5][3] = {
    {{0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}},
    {{0, 0, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 0, 0}},
    {{0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}}
  };
   desenhaMatriz(animacao1, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
    int animacao2[5][5][3] = {
    {{0, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 255, 0}},
    {{0, 255, 0}, {0, 255, 0}, {0, 0, 0}, {0, 255, 0}, {0, 255, 0}},
    {{0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}},
    {{0, 255, 0}, {0, 255, 0}, {0, 0, 0}, {0, 255, 0}, {0, 255, 0}},
    {{0, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 255, 0}}
  };
   desenhaMatriz(animacao2, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
    int animacao3[5][5][3] = {
    {{0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}},
    {{0, 0, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 0, 0}},
    {{0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}}
  };
   desenhaMatriz(animacao3, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia

   //Omnitrix começando a descarregar e entrar no estado de alerta amarelo
    int animacao4[5][5][3] = {
    {{255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}},
    {{0, 0, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {255, 255, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {0, 0, 0}},
    {{255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}}
  };
   desenhaMatriz(animacao4, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
    int animacao5[5][5][3] = {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
  };
   desenhaMatriz(animacao5, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
    int animacao6[5][5][3] = {
    {{255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}},
    {{0, 0, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {255, 255, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {0, 0, 0}},
    {{255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}}
  };
   desenhaMatriz(animacao6, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
    int animacao7[5][5][3] = {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
  };
   desenhaMatriz(animacao7, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
   
   //Omnitrix descarregado no estado vemelho
   int animacao8[5][5][3] = {
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}}
  };
   desenhaMatriz(animacao8, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia

        break;

    case '4':                                 // Verifica se a tecla 4 foi pressionada



        break;

    case '5':                                 // Verifica se a tecla 5 foi pressionada



        break;
    
    case '6':                                 // Verifica se a tecla 6 foi pressionada



        break;

    case '7':                                 // Verifica se a tecla 7 foi pressionada
         // Animação dos LEDs frame a frame monstrinho entrando, parando no meio piscando os olhos e saindo da tela
 int frame0[5][5][3] = {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
  };
   desenhaMatriz(frame0, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
  int frame1[5][5][3] = {
    {{246, 255, 0}, {6, 0, 158}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{6, 0, 158}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{246, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
  };
    desenhaMatriz(frame1, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
  int frame2[5][5][3] = {
    {{0, 0, 0}, {246, 255, 0}, {0, 2, 255}, {0, 0, 0}, {0, 0, 0}},
    {{246, 255, 0}, {0, 2, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{246, 255, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{246, 255, 0}, {246, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{246, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
  };
    desenhaMatriz(frame2, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
  int frame3[5][5][3] = {
     {{0, 0, 0}, {0, 0, 0}, {246, 255, 0}, {0, 2, 255}, {0, 0, 0}},
    {{0, 0, 0}, {246, 255, 0}, {0, 2, 255}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {246, 255, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {246, 255, 0}, {246, 255, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {246, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
  };
    desenhaMatriz(frame3, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
  int frame4[5][5][3] = {
     {{0, 2, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 2, 255}},
    {{246, 255, 0}, {0, 2, 255}, {246, 255, 0}, {0, 2, 255}, {246, 255, 0}},
    {{0, 0, 0}, {255, 0, 0}, {246, 255, 0}, {255, 0, 0}, {0, 0, 0}},
    {{246, 255, 0}, {246, 255, 0}, {246, 255, 0}, {246, 255, 0}, {246, 255, 0}},
    {{0, 0, 0}, {246, 255, 0}, {0, 0, 0}, {246, 255, 0}, {0, 0, 0}}
  };
   desenhaMatriz(frame4, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
  int frame5[5][5][3] = {
    {{0, 2, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 2, 255}},
    {{246, 255, 0}, {0, 2, 255}, {246, 255, 0}, {0, 2, 255}, {246, 255, 0}},
    {{0, 0, 0}, {246, 255, 0}, {246, 255, 0}, {246, 255, 0}, {0, 0, 0}},
    {{246, 255, 0}, {246, 255, 0}, {246, 255, 0}, {246, 255, 0}, {246, 255, 0}},
    {{0, 0, 0}, {246, 255, 0}, {0, 0, 0}, {246, 255, 0}, {0, 0, 0}}
  };
   desenhaMatriz(frame5, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
  int frame6[5][5][3] = {
    {{0, 2, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 2, 255}},
    {{246, 255, 0}, {0, 2, 255}, {246, 255, 0}, {0, 2, 255}, {246, 255, 0}},
    {{0, 0, 0}, {255, 0, 0}, {246, 255, 0}, {255, 0, 0}, {0, 0, 0}},
    {{246, 255, 0}, {246, 255, 0}, {246, 255, 0}, {246, 255, 0}, {246, 255, 0}},
    {{0, 0, 0}, {246, 255, 0}, {0, 0, 0}, {246, 255, 0}, {0, 0, 0}}
  };
   desenhaMatriz(frame6, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
  int frame7[5][5][3] = {
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {246, 255, 0}, {0, 2, 255}},
    {{0, 0, 0}, {0, 0, 0}, {246, 255, 0}, {0, 2, 255}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {246, 255, 0}, {255, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {246, 255, 0}, {246, 255, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {246, 255, 0}, {0, 0, 0}, {0, 0, 0}}
  };
   desenhaMatriz(frame7, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia
  int frame8[5][5][3] = {
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {246, 255, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {246, 255, 0}, {0, 2, 255}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {246, 255, 0}, {255, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {246, 255, 0}, {246, 255, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {246, 255, 0}, {0, 0, 0}}
  };
   desenhaMatriz(frame8, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia


  int frame9[5][5][3] = {
 {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {246, 255, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {246, 255, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {246, 255, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {246, 255, 0}}
  };
     desenhaMatriz(frame9, 500, 0.5); //chamando a funcao para desenhar a matriz e passando os parametros de referencia


        break;
    
    case 8:                                 // Verifica se a tecla 8 foi pressionada
    //Letreiro "C E P E D I + (CARINHA_FELIZ)"
    //Gerar a letra C na matriz leds, na cor azul
    int matrizC[5][5][3]= {
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}}
    };
    //Chama a funcao para desenhar a matriz, passando a matriz e o tempo em milisegundos
    desenhaMatriz(matrizC, 2000, 0.8);

    //Gerar a letra E na matriz leds, na cor azul
    int matrizE[5][5][3]= {
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}}
    };
    //Chama a funcao para desenhar a matriz, passando a matriz e o tempo em milisegundos
    desenhaMatriz(matrizE, 2000, 0.8);

    //Gerar a letra P na matriz leds, na cor azul
    int matrizP[5][5][3]= {
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 255}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
    };
    //Chama a funcao para desenhar a matriz, passando a matriz e o tempo em milisegundos
    desenhaMatriz(matrizP, 2000, 0.8);

    //Gerar a letra E na matriz leds, na cor azul
    int matrizE2[5][5][3]= {
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}}
    };
    //Chama a funcao para desenhar a matriz, passando a matriz e o tempo em milisegundos
    desenhaMatriz(matrizE2, 2000, 0.8);  

    //Gerar a letra D na matriz leds, na cor azul
    int matrizD[5][5][3]= {
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 255}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 255}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 255}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}}
    };
    //Chama a funcao para desenhar a matriz, passando a matriz e o tempo em milisegundos
    desenhaMatriz(matrizD, 2000, 0.8);

    //Gerar a letra I na matriz leds, na cor azul
    int matrizI[5][5][3]= {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}}
    };
    //Chama a funcao para desenhar a matriz, passando a matriz e o tempo em milisegundos
    desenhaMatriz(matrizI, 2000, 0.8);

    //Gerar um emoji de rosto sorrindo na matriz leds, na cor azul
    int matrizCarinha[5][5][3]= {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 255}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 255}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 0}}
    };
    //Chama a funcao para desenhar a matriz, passando a matriz e o tempo em milisegundos
    desenhaMatriz(matrizCarinha, 2000, 0.8);

    break;

    case '9':                                 // Verifica se a tecla 9 foi pressionada



        break;

    case 'A':                               // Verifica se a tecla A foi pressionada
        desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
        break;

    case 'B':                             // Verifica se a tecla B foi pressionada
        desenho_pio(acender_leds, valor_led, pio, sm, r, g, 1.0);
        break;

    case 'C':                             // Verifica se  a tecla C foi pressionada
        desenho_pio(acender_leds, valor_led, pio, sm, 0.8, g, b);
        break;

    case 'D':                             // Verifica se a tecla D foi pressionada
        desenho_pio(acender_leds, valor_led, pio, sm, r, 0.5, b);
        break;

    case '#':                             // Verifica se a tecla # foi pressionada
        desenho_pio(acender_leds, valor_led, pio, sm, 0.2, 0.2, 0.2);
        break;

    case '*':                             // Verifica se a tecla * foi pressionada
        bootsel();
        break;

      default:
        printf("Tecla não configurada\n");
      }
      sleep_ms(100); // Delay para evitar leitura repetida
    }
    sleep_ms(100);
    }
}
