#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"
#include "matriz_led.pio.h"

#define NUM_PIXELS 25 // número de leds na matriz
#define LED_PIN 7     // pino de saída do led
#define gpio_buzzer 10 // pino de saida do buzzer
uint valor_led;

// Função para habilitar o modo Bootsel
void bootsel()
{
  reset_usb_boot(0, 0);
}

// TECLADO
// definição das colunas e das linhas e mapeamento do teclado
uint8_t coluna[4] = {21, 20, 19, 18};
uint8_t linha[4] = {28, 27, 26, 22};

char teclas[4][4] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'};

// função para inicializar o teclado
void inicializar_teclado()
{
  for (int i = 0; i < 4; i++)
  {
    gpio_init(linha[i]);
    gpio_set_dir(linha[i], GPIO_OUT);
    gpio_put(linha[i], 1); // Inicialmente em HIGH

    gpio_init(coluna[i]);
    gpio_set_dir(coluna[i], GPIO_IN);
    gpio_pull_up(coluna[i]); // Habilita pull-up nas colunas
  }
}

// função para ler o teclado
char ler_teclado(uint8_t *colunas, uint8_t *linhas)
{
  for (int i = 0; i < 4; i++)
  {
    gpio_put(linhas[i], 0);
    for (int j = 0; j < 4; j++)
    {
      if (!gpio_get(colunas[j]))
      {                         // Verifica se a coluna está LOW
        gpio_put(linhas[i], 1); // Restaura a linha
        return teclas[i][j];    // Retorna a tecla correspondente
      }
    }
    gpio_put(linhas[i], 1);
  }
  return 0;
}

// MATRIZ DE LEDS
// rotina para definição da intensidade de cores do led
uint matrix_rgb(float r, float g, float b)
{
  unsigned char R, G, B;
  R = r * 255;
  G = g * 255;
  B = b * 255;
  return (G << 24) | (R << 16) | (B << 8);
}

// Função para converter a posição do matriz para uma posição do vetor.
int getIndex(int x, int y)
{
  // Se a linha for par (0, 2, 4), percorremos da esquerda para a direita.
  // Se a linha for ímpar (1, 3), percorremos da direita para a esquerda.
  if (y % 2 == 0)
  {
    return 24 - (y * 5 + x); // Linha par (esquerda para direita).
  }
  else
  {
    return 24 - (y * 5 + (4 - x)); // Linha ímpar (direita para esquerda).
  }
}

// Funcao para desenhar a matriz

void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b)
{

  for (int16_t i = 0; i < NUM_PIXELS; i++)
  {
    valor_led = matrix_rgb(desenho[i] * r, desenho[i] * g, desenho[i] * b);
    pio_sm_put_blocking(pio, sm, valor_led);
  };
}

void acionar_buzzer(int interval)
{
  gpio_set_function(gpio_buzzer, GPIO_FUNC_PWM);      // Configura pino como saída PWM
    uint slice_num = pwm_gpio_to_slice_num(gpio_buzzer); // Obter o slice do PWM

    pwm_set_clkdiv(slice_num, 125.0);                  
    pwm_set_wrap(slice_num, 500);                      
    pwm_set_gpio_level(gpio_buzzer, 150);              
    pwm_set_enabled(slice_num, true);                  // Ativar o PWM

    sleep_ms(interval);                                    // Manter o som pelo intervalo

    pwm_set_enabled(slice_num, false);                 // Desativar o PWM  
}

double apagar_leds[25] = {0.0, 0.0, 0.0, 0.0, 0.0, // Apagar LEDs da matriz
                          0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 0.0};

double acender_leds[25] = {1.0, 1.0, 1.0, 1.0, 1.0, // Acender LEDs da matriz
                           1.0, 1.0, 1.0, 1.0, 1.0,
                           1.0, 1.0, 1.0, 1.0, 1.0,
                           1.0, 1.0, 1.0, 1.0, 1.0,
                           1.0, 1.0, 1.0, 1.0, 1.0};

// ------------ ANIMAÇÃO 1 --------------------
//Animação "You Win com uma estrela no final"
double desenho1_1[25] = {0.0, 0.0, 1.0, 0.0, 0.0, // Desenho Tecla 1 Parte 1 - Y
                         0.0, 0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 1.0, 0.0,
                         1.0, 0.0, 0.0, 0.0, 1.0};

double desenho1_2[25] = {1.0, 1.0, 1.0, 1.0, 1.0, // Desenho Tecla 1 Parte 2 - O
                         1.0, 0.0, 0.0, 0.0, 1.0,
                         1.0, 0.0, 0.0, 0.0, 1.0,
                         1.0, 0.0, 0.0, 0.0, 1.0,
                         1.0, 1.0, 1.0, 1.0, 1.0};

double desenho1_3[25] = {1.0, 1.0, 1.0, 1.0, 1.0, // Desenho Tecla 1 Parte 3 - U
                         1.0, 0.0, 0.0, 0.0, 1.0,
                         1.0, 0.0, 0.0, 0.0, 1.0,
                         1.0, 0.0, 0.0, 0.0, 1.0,
                         1.0, 0.0, 0.0, 0.0, 1.0};

double desenho1_4[25] = {0.0, 1.0, 1.0, 1.0, 0.0, // Desenho Tecla 1 Parte 4 - W
                         1.0, 1.0, 1.0, 1.0, 1.0,
                         1.0, 0.0, 1.0, 0.0, 1.0,
                         1.0, 0.0, 1.0, 0.0, 1.0,
                         1.0, 0.0, 0.0, 0.0, 1.0};

double desenho1_5[25] = {0.0, 0.0, 1.0, 0.0, 0.0, // Desenho Tecla 1 Parte 5 - I
                         0.0, 0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0, 0.0};

double desenho1_6[25] = {1.0, 0.0, 0.0, 0.0, 1.0, // Desenho Tecla 1 Parte 6 - N
                         1.0, 0.0, 0.0, 1.0, 1.0,
                         1.0, 0.0, 1.0, 0.0, 1.0,
                         1.0, 1.0, 0.0, 0.0, 1.0,
                         1.0, 0.0, 0.0, 0.0, 1.0};

double desenho1_7[25] = {0.0, 0.0, 0.0, 0.0, 0.0, // Desenho Tecla 1 Parte 7 - Início da Estrela
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0};

double desenho1_8[25] = {0.0, 0.0, 0.0, 0.0, 0.0, // Desenho Tecla 1 Parte 8 - Estrela formando
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 1.0, 1.0, 0.0,
                         0.0, 0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0};

double desenho1_9[25] = {0.0, 0.0, 0.0, 0.0, 0.0, // Desenho Tecla 1 Parte 9 - Estrela formada
                         1.0, 0.0, 0.0, 0.0, 1.0,
                         0.0, 1.0, 1.0, 1.0, 0.0,
                         1.0, 1.0, 1.0, 1.0, 1.0,
                         0.0, 0.0, 1.0, 0.0, 0.0};

// ------------ FIM ANIMACAO --------------------

// ------------ ANIMAÇÃO 2 --------------------
// Animação "New Year" com coração no final

double animacao2_1[25] = {1.0, 0.0, 0.0, 0.0, 1.0, // Desenho letra N
                          1.0, 0.0, 0.0, 1.0, 1.0,
                          1.0, 0.0, 1.0, 0.0, 1.0,
                          1.0, 1.0, 0.0, 0.0, 1.0,
                          1.0, 0.0, 0.0, 0.0, 1.0};

double animacao2_2[25] = {1.0, 1.0, 1.0, 1.0, 1.0, // Desenho letra E
                          1.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 1.0, 1.0, 1.0,
                          1.0, 0.0, 0.0, 0.0, 0.0,
                          1.0, 1.0, 1.0, 1.0, 1.0};

double animacao2_3[25] = {1.0, 0.0, 0.0, 0.0, 1.0, //Desenho letra W
                          1.0, 1.0, 0.0, 1.0, 1.0,
                          1.0, 0.0, 1.0, 0.0, 1.0,
                          1.0, 0.0, 0.0, 0.0, 1.0,
                          1.0, 0.0, 0.0, 0.0, 1.0};

double animacao2_4[25] = {0.0, 0.0, 1.0, 0.0, 0.0, //Desenho letra Y
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 1.0, 1.0, 1.0, 0.0,
                          1.0, 0.0, 0.0, 0.0, 1.0,
                          1.0, 0.0, 0.0, 0.0, 1.0};

double animacao2_5[25] = {1.0, 0.0, 0.0, 0.0, 1.0, //Desenho letra A
                          1.0, 0.0, 0.0, 0.0, 1.0,
                          1.0, 1.0, 1.0, 1.0, 1.0,
                          1.0, 0.0, 0.0, 0.0, 1.0,
                          1.0, 1.0, 1.0, 1.0, 1.0};

double animacao2_6[25] = {0.0, 1.0, 0.0, 0.0, 1.0, //Desenho letra R
                          1.0, 0.0, 1.0, 0.0, 0.0,
                          1.0, 1.0, 1.0, 1.0, 1.0,
                          1.0, 0.0, 0.0, 0.0, 1.0,
                          1.0, 1.0, 1.0, 1.0, 1.0};

double animacao2_7[25] = {0.0, 0.0, 1.0, 0.0, 0.0, //Desenho Coração 
                          0.0, 1.0, 0.0, 1.0, 0.0,
                          1.0, 0.0, 0.0, 0.0, 1.0,
                          1.0, 0.0, 1.0, 0.0, 1.0,
                          0.0, 1.0, 0.0, 1.0, 0.0};
// ------------ FIM ANIMACAO --------------------

// ------------ ANIMAÇÃO 3 --------------------

// Representação do Omnitrix descarregando pela matriz de led
// Omnitrix no estado verde de escolha
double animacao3_1[25] = {1.0, 1.0, 1.0, 1.0, 1.0, // Desenho Tecla 3 Parte 1
                          0.0, 1.0, 1.0, 1.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 1.0, 1.0, 1.0, 0.0,
                          1.0, 1.0, 1.0, 1.0, 1.0};

double animacao3_2[25] = {1.0, 0.0, 0.0, 0.0, 1.0, // Desenho Tecla 3 Parte 2
                          1.0, 1.0, 0.0, 1.0, 1.0,
                          1.0, 1.0, 1.0, 1.0, 1.0,
                          1.0, 1.0, 0.0, 1.0, 1.0,
                          1.0, 0.0, 0.0, 0.0, 1.0};

double animacao3_3[25] = {1.0, 1.0, 1.0, 1.0, 1.0, // Desenho Tecla 3 Parte 3
                          0.0, 1.0, 1.0, 1.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 1.0, 1.0, 1.0, 0.0,
                          1.0, 1.0, 1.0, 1.0, 1.0};

// Omnitrix começando a descarregar e entrar no estado de alerta amarelo
double animacao3_4[25] = {1.0, 1.0, 1.0, 1.0, 1.0, // Desenho Tecla 3 Parte 4
                          0.0, 1.0, 1.0, 1.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 1.0, 1.0, 1.0, 0.0,
                          1.0, 1.0, 1.0, 1.0, 1.0};

double animacao3_5[25] = {1.0, 1.0, 1.0, 1.0, 1.0, // Desenho Tecla 3 Parte 5
                          0.0, 1.0, 1.0, 1.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 1.0, 1.0, 1.0, 0.0,
                          1.0, 1.0, 1.0, 1.0, 1.0};

// Omnitrix descarregado no estado vemelho
double animacao3_6[25] = {1.0, 1.0, 1.0, 1.0, 1.0, // Desenho Tecla 3 Parte 6
                          0.0, 1.0, 1.0, 1.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 1.0, 1.0, 1.0, 0.0,
                          1.0, 1.0, 1.0, 1.0, 1.0};
// ------------ FIM ANIMACAO --------------------                          

// ------------ ANIMAÇÃO 4 --------------------
//helice girando e mudando de cor
void interpola(float *r, float *g, float *b, float t)
{
  float r1 = 1.0f, g1 = 0.0f, b1 = 0.0f; // Cor inicial (vermelho)
  float r2 = 0.0f, g2 = 0.0f, b2 = 1.0f; // Cor final (cinza avermelhado)

    // Interpolação linear para obter cores entre o vermelho e o violeta - tentando imitar o espectro da luz visível
    *r = r1 + (r2 - r1) * t;
    *g = g1 + (g2 - g1) * t;
    *b = b1 + (b2 - b1) * t;
}

double desenho4_5[25] =  {0.0, 0.0, 1.0, 0.0, 0.0, // posição inicial do hélice brilho forte
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 1.0, 0.0,
                          1.0, 0.0, 0.0, 0.0, 1.0};

double desenho4_2[25] =  {0.0, 0.0, 1.0, 0.0, 0.0, //brilho fraco
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 1.0, 1.0,
                          0.0, 0.0, 0.0, 1.0, 0.0,
                          1.0, 0.0, 0.0, 0.0, 0.0};

double desenho4_3[25] =  {0.0, 0.0, 0.0, 0.0, 1.0, //brilho forte
                          0.0, 1.0, 0.0, 0.0, 0.0,
                          1.0, 1.0, 1.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 1.0};

double desenho4_4[25] =  {1.0, 0.0, 0.0, 0.0, 0.0, //brilho fraco
                          0.0, 0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 1.0, 1.0, 1.0,
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0};

double desenho4_1[25] =  {1.0, 0.0, 0.0, 0.0, 1.0, //brilho forte
                          0.0, 1.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0};

double desenho4_6[25] =  {0.0, 0.0, 0.0, 0.0, 1.0, //brilho fraco
                          0.0, 1.0, 0.0, 0.0, 0.0,
                          1.0, 1.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0};

double desenho4_7[25] =  {1.0, 0.0, 0.0, 0.0, 0.0, //brilho forte
                          0.0, 0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 1.0, 1.0, 1.0,
                          0.0, 0.0, 0.0, 1.0, 0.0,
                          1.0, 0.0, 0.0, 0.0, 0.0};

double desenho4_8[25] =  {0.0, 0.0, 1.0, 0.0, 0.0, //posição final brilho fraco
                          0.0, 0.0, 1.0, 0.0, 0.0,
                          1.0, 1.0, 1.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 1.0};
// ------------ FIM ANIMACAO --------------------

// ------------ ANIMAÇÃO 5 --------------------

// Animação do jogo da cobrinha

double frame1_5[25] = {
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 1.0, 1.0};

double frame2_5[25] = {
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0};

double frame3_5[25] = {
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    1.0, 1.0, 1.0, 0.0, 0.0};

double frame4_5[25] = {
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 1.0,
    1.0, 1.0, 0.0, 0.0, 0.0};

double frame5_5[25] = {
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    1.0, 0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 1.0,
    1.0, 0.0, 0.0, 0.0, 0.0};

double frame6_5[25] = {
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    1.0, 1.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 1.0,
    0.0, 0.0, 0.0, 0.0, 0.0};

double frame7_5[25] = {
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    1.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0};

double frame8_5[25] = {
    0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0};

double frame9_5[25] = {
    0.0, 0.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0};

double frame10_5[25] = {
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0};

double frame11_5[25] = {
    1.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0};

// GAME OVER
double frame12_5[25] = {
    1.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0};
// ------------ FIM ANIMACAO --------------------

// ------------ ANIMAÇÃO 6 --------------------
//Explosão de pixel
    double animacao6_1[25]={
      0.0, 0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0
    };
    double animacao6_2[25]={
      0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0
    };
    double animacao6_3[25]={
      0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0
    };
    double animacao6_4[25]={
      0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 1.0, 0.0, 
      0.0, 0.0, 1.0, 0.0, 0.0, 
      0.0, 1.0, 0.0, 1.0, 0.0, 
      0.0, 0.0, 0.0, 0.0, 0.0
    };
    double animacao6_5[25]={
      1.0, 0.0, 0.0, 0.0, 1.0, 
      0.0, 1.0, 1.0, 1.0, 0.0, 
      0.0, 1.0, 1.0, 1.0, 0.0, 
      0.0, 1.0, 1.0, 1.0, 0.0, 
      1.0, 0.0, 0.0, 0.0, 1.0 
    };
    double animacao6_6[25]={
      1.0, 0.0, 1.0, 0.0, 1.0, 
      0.0, 1.0, 1.0, 1.0, 0.0, 
      1.0, 1.0, 1.0, 1.0, 1.0, 
      0.0, 1.0, 1.0, 1.0, 0.0, 
      1.0, 0.0, 1.0, 0.0, 1.0 
    };
    double animacao6_7[25]={
      1.0, 0.0, 1.0, 0.0, 1.0, 
      0.0, 0.0, 0.0, 0.0, 0.0,
      1.0, 0.0, 0.0, 0.0, 1.0, 
      0.0, 0.0, 0.0, 0.0, 0.0,
      1.0, 0.0, 1.0, 0.0, 1.0 
    };
// ------------ FIM ANIMACAO --------------------

// ------------ ANIMAÇÃO 7 --------------------
    //caveira começa a se formar na tela
        double frame1_d7[25] = {
            0.0, 1.0, 1.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0};
        
        double frame2_d7[25] = {
            0.0, 1.0, 1.0, 1.0, 0.0,
            1.0, 1.0, 1.0, 1.0, 1.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0};
        
        double frame3_d7[25] = {
            0.0, 1.0, 1.0, 1.0, 0.0,
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 0.0, 1.0, 0.0, 1.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0};
        
        double frame4_d7[25] = {
            0.0, 1.0, 1.0, 1.0, 0.0,
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 0.0, 1.0, 0.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0,
            0.0, 0.0, 0.0, 0.0, 0.0};
  //final da formação
        double frame5_d7[25] = {
            0.0, 1.0, 1.0, 1.0, 0.0,
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 0.0, 1.0, 0.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0};

            ///desligar leds
            ///frame5
            ///frame6
            ///frame5
            ///frame6
            ///frame5
            
        double frame6_d7[25] = {
            0.0, 0.0, 0.0, 0.0, 0.0,
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 0.0, 1.0, 0.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0};

        double frame7_d7[25] = {
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
            1.0, 0.0, 1.0, 0.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0};

        double frame8_d7[25] = {
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
             1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0};

        double frame9_d7[25] = {
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
            1.0, 1.0, 1.0, 1.0, 1.0};
          //desligar leds
// ------------ FIM ANIMACAO --------------------

// ------------ ANIMAÇÃO 8 --------------------

//Letreiro "C E P E D I + (CARINHA_FELIZ)"
    //Gerar a letra C na matriz leds
    double matrizC[25]={
      0.0, 1.0, 1.0, 1.0, 0.0,
      0.0, 1.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 1.0, 0.0,
      0.0, 1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 1.0, 1.0, 0.0
    };

      //Gerar a letra E na matriz leds
    double matrizE[25] = {
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0
    };


    //Gerar a letra P na matriz leds
    double matrizP[25] = {
        0.0, 0.0, 0.0, 1.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0
    };

    //Gerar a letra D na matriz leds
    double matrizD[25] = {
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 1.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 1.0, 1.0, 0.0
    };

    //Gerar a letra I na matriz leds
    double matrizI[25] = {
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
    };

    //Gerar um emoji de rosto sorrindo na matriz leds
    double matrizCarinha[25] = {
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };
// ------------ FIM ANIMACAO --------------------

// ------------ ANIMAÇÃO 9 --------------------

double desenho9_1[25] = {0.0, 0.0, 1.0, 0.0, 0.0, // Desenho Tecla 9 Parte 1
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0};

double desenho9_2[25] = {0.0, 0.0, 1.0, 0.0, 0.0, // Desenho Tecla 9 Parte 2
                         0.0, 1.0, 1.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0};

double desenho9_3[25] = {0.0, 0.0, 1.0, 0.0, 0.0, // Desenho Tecla 9 Parte 3
                         0.0, 1.0, 1.0, 1.0, 0.0,
                         1.0, 1.0, 1.0, 1.0, 1.0,
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0};

double desenho9_4[25] = {0.0, 0.0, 1.0, 0.0, 0.0, // Desenho Tecla 9 Parte 4
                         0.0, 1.0, 1.0, 1.0, 0.0,
                         1.0, 1.0, 1.0, 1.0, 1.0,
                         1.0, 1.0, 1.0, 1.0, 1.0,
                         0.0, 0.0, 0.0, 0.0, 0.0};

double desenho9_5[25] = {0.0, 0.0, 1.0, 0.0, 0.0, // Desenho Tecla 9 Parte 5
                         0.0, 1.0, 1.0, 1.0, 0.0,
                         1.0, 1.0, 1.0, 1.0, 1.0,
                         1.0, 1.0, 1.0, 1.0, 1.0,
                         0.0, 1.0, 0.0, 1.0, 0.0};

double desenho9_6[25] = {0.0, 0.0, 0.0, 0.0, 0.0, // Desenho Tecla 9 Parte 6
                         0.0, 1.0, 1.0, 1.0, 0.0,
                         1.0, 1.0, 1.0, 1.0, 1.0,
                         1.0, 1.0, 1.0, 1.0, 1.0,
                         0.0, 1.0, 0.0, 1.0, 0.0};

double desenho9_7[25] = {0.0, 0.0, 0.0, 0.0, 0.0, // Desenho Tecla 9 Parte 7
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         1.0, 1.0, 1.0, 1.0, 1.0,
                         1.0, 1.0, 1.0, 1.0, 1.0,
                         0.0, 1.0, 0.0, 1.0, 0.0};

double desenho9_8[25] = {0.0, 0.0, 0.0, 0.0, 0.0, // Desenho Tecla 9 Parte 8
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         1.0, 1.0, 1.0, 1.0, 1.0,
                         0.0, 1.0, 0.0, 1.0, 0.0};

double desenho9_9[25] = {0.0, 0.0, 0.0, 0.0, 0.0, // Desenho Tecla 9 Parte 9
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 1.0, 0.0};
// ------------ FIM ANIMACAO --------------------                         

// ------------ ANIMAÇÃO 0 -------------------- 
// animação "subgrupo 5"
        double frame1_d0[25] = {
            1.0, 1.0, 1.0, 1.0, 1.0,
            0.0, 0.0, 0.0, 0.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 0.0,
            1.0, 1.0, 1.0, 1.0, 1.0};
        
        double frame2_d0[25] = {
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0};
        
        double frame3_d0[25] = {
            0.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0,
            0.0, 1.0, 1.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0,
            0.0, 1.0, 1.0, 1.0, 1.0};
        
        double frame4_d0[25] = {
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0,
            1.0, 1.0, 1.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 0.0,
            1.0, 1.0, 1.0, 1.0, 1.0};

        double frame5_d0[25] = {
            1.0, 0.0, 0.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0,
            0.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0,
            0.0, 1.0, 1.0, 1.0, 1.0};
          
          //repete frame2_d0

        double frame7_d0[25] = {
            0.0, 0.0, 0.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 0.0,
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0};

        double frame8_d0[25] = {
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0};
           
            ///desligar leds

        double frame9_d0[25] = {
            1.0, 1.0, 1.0, 1.0, 1.0,
            0.0, 0.0, 0.0, 0.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 0.0, 0.0, 0.0, 0.0,
            1.0, 1.0, 1.0, 1.0, 1.0};

          //desligar leds
// ------------ FIM ANIMACAO --------------------

int main()
{
  PIO pio = pio0;
  bool frequenciaClock;
  uint16_t i;
  uint valor_led;
  float r = 0.0, b = 0.0, g = 0.0;

  frequenciaClock = set_sys_clock_khz(128000, false); // frequência de clock
  stdio_init_all();
  inicializar_teclado();
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  printf("iniciando a transmissão PIO");
  if (frequenciaClock)
    printf("clock set to %ld\n", clock_get_hz(clk_sys));

  // configurações da PIO
  uint offset = pio_add_program(pio, &pio_matrix_program);
  uint sm = pio_claim_unused_sm(pio, true);
  pio_matrix_program_init(pio, sm, offset, LED_PIN);

  while (true)
  {

    char tecla = ler_teclado(coluna, linha);

    if (tecla)
    {
      printf("Tecla pressionada: %c\n", tecla);

      switch (tecla)
      {
      case '1': // Verifica se a tecla 1 foi pressionada

        desenho_pio(desenho1_1, valor_led, pio, sm, 0.0, 1.0, 1.0);
        sleep_ms(500);
        desenho_pio(desenho1_2, valor_led, pio, sm, 0.0, 1.0, 1.0);
        sleep_ms(500);
        desenho_pio(desenho1_3, valor_led, pio, sm, 0.0, 1.0, 1.0);
        sleep_ms(500);
        desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
        sleep_ms(500);
        desenho_pio(desenho1_4, valor_led, pio, sm, 0.0, 1.0, 1.0);
        sleep_ms(500);
        desenho_pio(desenho1_5, valor_led, pio, sm, 0.0, 1.0, 1.0);
        sleep_ms(500);
        desenho_pio(desenho1_6, valor_led, pio, sm, 0.0, 1.0, 1.0);
        sleep_ms(500);
        desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
        sleep_ms(500);
        desenho_pio(desenho1_7, valor_led, pio, sm, 1.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(desenho1_8, valor_led, pio, sm, 1.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(desenho1_9, valor_led, pio, sm, 1.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
        sleep_ms(200);
        desenho_pio(desenho1_9, valor_led, pio, sm, 1.0, 1.0, 0.0);
        sleep_ms(200);
        desenho_pio(apagar_leds, valor_led, pio, sm, 1.0, 1.0, 0.0);
        sleep_ms(200);
        desenho_pio(desenho1_9, valor_led, pio, sm, 1.0, 1.0, 0.0);
        sleep_ms(200);
        desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);        
        break;

      case '2': // Verifica se a tecla 2 foi pressionada

        desenho_pio(animacao2_1, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(1000);
        desenho_pio(animacao2_2, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(1000);
        desenho_pio(animacao2_3, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(1000);
        desenho_pio(apagar_leds, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(animacao2_4, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(1000);
        desenho_pio(animacao2_2, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(1000);
        desenho_pio(animacao2_5, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(1000);
        desenho_pio(animacao2_6, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(1000);
         desenho_pio(apagar_leds, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(animacao2_7, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(1000);
         desenho_pio(apagar_leds, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        break;

      case '3': // Verifica se a tecla 3 foi pressionada

        desenho_pio(apagar_leds, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(animacao3_1, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(animacao3_2, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(animacao3_3, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(animacao3_4, valor_led, pio, sm, 1.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(apagar_leds, valor_led, pio, sm, 0.0, 0.0, 0.0);
        sleep_ms(500);
        desenho_pio(animacao3_5, valor_led, pio, sm, 1.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(apagar_leds, valor_led, pio, sm, 0.0, 0.0, 0.0);
        sleep_ms(500);
        desenho_pio(animacao3_6, valor_led, pio, sm, 1.0, 0.0, 0.0);
        sleep_ms(500);
        desenho_pio(apagar_leds, valor_led, pio, sm, 0.0, 0.0, 0.0);
        break;

      case '4': // Verifica se a tecla 4 foi pressionada
        // animação do hélice - Antonio Louro
        int espera = 100;
        for(int i = 0; i < 8; i++)
        {
          interpola(&r,&g,&b,i/7.0);
          desenho_pio(desenho4_1, valor_led, pio, sm, r, g, b);
          sleep_ms(espera);
          desenho_pio(desenho4_2, valor_led, pio, sm, r, g, b);
          sleep_ms(espera);
          desenho_pio(desenho4_3, valor_led, pio, sm, r, g, b);
          sleep_ms(espera);
          desenho_pio(desenho4_4, valor_led, pio, sm, r, g, b);
          sleep_ms(espera);
          desenho_pio(desenho4_5, valor_led, pio, sm, r, g, b);
          sleep_ms(espera);
          desenho_pio(desenho4_6, valor_led, pio, sm, r, g, b);
          sleep_ms(espera);
          desenho_pio(desenho4_7, valor_led, pio, sm, r, g, b);
          sleep_ms(espera);
          desenho_pio(desenho4_8, valor_led, pio, sm, r, g, b);
          sleep_ms(espera);
          desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
        }

        break;

      case '5': // Verifica se a tecla 5 foi pressionada

        desenho_pio(frame1_5, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(frame2_5, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(frame3_5, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(frame4_5, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(frame5_5, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(frame6_5, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(frame7_5, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(frame8_5, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(frame9_5, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(frame10_5, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(frame11_5, valor_led, pio, sm, 0.0, 1.0, 0.0);
        sleep_ms(500);
        desenho_pio(frame12_5, valor_led, pio, sm, 1.0, 0.0, 0.0);
        sleep_ms(500);
        desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
        break;

      case '6': // Verifica se a tecla 6 foi pressionada
        desenho_pio(apagar_leds, valor_led, pio, sm, 0.0, 0.0, 0.0);
        desenho_pio(animacao6_1, valor_led, pio, sm, 1.0, 0.0, 0.0);
        sleep_ms(200);        
        desenho_pio(animacao6_2, valor_led, pio, sm, 1.0, 0.0, 0.0);
        sleep_ms(200);        
        desenho_pio(animacao6_3, valor_led, pio, sm, 1.0, 0.0, 0.0);
        sleep_ms(200);        
        for(int i = 0; i < 3; i ++)
        {
          desenho_pio(animacao6_4, valor_led, pio, sm, 1.0, 1.0, 0.0);
          sleep_ms(200);
          desenho_pio(animacao6_5, valor_led, pio, sm, 1.0, 1.0, 0.0);
          sleep_ms(200);
          desenho_pio(animacao6_6, valor_led, pio, sm, 1.0, 1.0, 0.0);
          sleep_ms(200);
          desenho_pio(animacao6_7, valor_led, pio, sm, 1.0, 1.0, 0.0);
          sleep_ms(200);
          desenho_pio(apagar_leds, valor_led, pio, sm, 0.0, 0.0, 0.0);
        }
        desenho_pio(apagar_leds, valor_led, pio, sm, 0.0, 0.0, 0.0);
        break;

      case '7': // Verifica se a tecla 7 foi pressionada
                // Animação dos LEDs frame a frame caveira roxa, entrando na tela mexendo o quixo e saindo
            desenho_pio(frame1_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(500);
            desenho_pio(frame2_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(500);
            desenho_pio(frame3_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(500);
            desenho_pio(frame4_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(500);
            desenho_pio(frame5_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(500);
            desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
            sleep_ms(500);
            desenho_pio(frame5_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(1000);
            desenho_pio(frame6_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(300);
            desenho_pio(frame5_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(300);
            desenho_pio(frame6_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(300);
            desenho_pio(frame5_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(300);
            desenho_pio(frame6_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(500);
            desenho_pio(frame7_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(500);
            desenho_pio(frame8_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(500);
            desenho_pio(frame9_d7, valor_led, pio, sm, 0.5, 0.0, 1.0);
            sleep_ms(500);
            desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);   
        break;

      case '8': // Verifica se a tecla 8 foi pressionada
        // Letreiro "C E P E D I + (CARINHA_FELIZ)"
        // Chama a funcao para desenhar cada matriz, passando cada matriz, o tempo em milisegundos, e a intensidade da cor da matriz
        desenho_pio(matrizC, valor_led, pio, sm, 0.0, 0.0, 1.0);
        sleep_ms(1000);
        desenho_pio(matrizE, valor_led, pio, sm, 0.0, 0.0, 1.0);
        sleep_ms(1000);
        desenho_pio(matrizP, valor_led, pio, sm, 0.0, 0.0, 1.0);
        sleep_ms(1000);
        desenho_pio(matrizE, valor_led, pio, sm, 0.0, 0.0, 1.0);
        sleep_ms(1000);
        desenho_pio(matrizD, valor_led, pio, sm, 0.0, 0.0, 1.0);
        sleep_ms(1000);
        desenho_pio(matrizI, valor_led, pio, sm, 0.0, 0.0, 1.0);
        sleep_ms(1000);
        desenho_pio(matrizCarinha, valor_led, pio, sm, 0.0, 0.0, 1.0);
        sleep_ms(2000);
        desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
        break;

      case '9': // Verifica se a tecla 9 foi pressionada
        for (int i = 0; i < 3; i++)
        {
          desenho_pio(desenho9_1, valor_led, pio, sm, 1.0, 0.0, 0.0);
          sleep_ms(200);
          desenho_pio(desenho9_2, valor_led, pio, sm, 1.0, 0.0, 0.0);
          sleep_ms(200);
          desenho_pio(desenho9_3, valor_led, pio, sm, 1.0, 0.0, 0.0);
          sleep_ms(200);
          desenho_pio(desenho9_4, valor_led, pio, sm, 1.0, 0.0, 0.0);
          sleep_ms(200);
          for (int i = 0; i < 3; i++)
          {
            desenho_pio(desenho9_5, valor_led, pio, sm, 1.0, 0.0, 0.0);
            acionar_buzzer(200);
            sleep_ms(200);
            desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
            sleep_ms(200);
          }
          
          desenho_pio(desenho9_5, valor_led, pio, sm, 1.0, 0.0, 0.0);
          acionar_buzzer(200);
          sleep_ms(200);
          desenho_pio(desenho9_6, valor_led, pio, sm, 1.0, 0.0, 0.0);
          sleep_ms(200);
          desenho_pio(desenho9_7, valor_led, pio, sm, 1.0, 0.0, 0.0);
          sleep_ms(200);
          desenho_pio(desenho9_8, valor_led, pio, sm, 1.0, 0.0, 0.0);
          sleep_ms(200);
          desenho_pio(desenho9_9, valor_led, pio, sm, 1.0, 0.0, 0.0);
          sleep_ms(200);
          desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
        } 
          break;
 
       case '0':                              // Verifica se a tecla 0 foi pressionada
       
         desenho_pio(frame1_d0, valor_led, pio, sm, 0.5, 0.0, 1.0);
            acionar_buzzer(400);
            sleep_ms(400);
            desenho_pio(frame2_d0, valor_led, pio, sm, 0.5, 0.0, 1.0);
            acionar_buzzer(400);
            sleep_ms(400);
            desenho_pio(frame3_d0, valor_led, pio, sm, 0.5, 0.0, 1.0);
            acionar_buzzer(400);
            sleep_ms(400);
            desenho_pio(frame4_d0, valor_led, pio, sm, 0.5, 0.0, 1.0);
            acionar_buzzer(400);
            sleep_ms(400);
            desenho_pio(frame5_d0, valor_led, pio, sm, 0.5, 0.0, 1.0);
            acionar_buzzer(400);
            sleep_ms(400);
            desenho_pio(frame2_d0, valor_led, pio, sm, 0.5, 0.0, 1.0);
            acionar_buzzer(400);
            sleep_ms(400);
            desenho_pio(frame7_d0, valor_led, pio, sm, 0.5, 0.0, 1.0);
            acionar_buzzer(400);
            sleep_ms(400);
            desenho_pio(frame8_d0, valor_led, pio, sm, 0.5, 0.0, 1.0);
            acionar_buzzer(400);
            sleep_ms(400);
            desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
            sleep_ms(200);
            desenho_pio(frame9_d0, valor_led, pio, sm, 1.0, 0.5, 1.0);
            acionar_buzzer(400);
            sleep_ms(400);
            desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b); 
        break;

      case 'A': // Verifica se a tecla A foi pressionada
        desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
        break;

      case 'B': // Verifica se a tecla B foi pressionada
        desenho_pio(acender_leds, valor_led, pio, sm, 0.0, 0.0, 1.0);
        break;

      case 'C': // Verifica se  a tecla C foi pressionada
        desenho_pio(acender_leds, valor_led, pio, sm, 0.8, 0.0, 0.0);
        break;

      case 'D': // Verifica se a tecla D foi pressionada
        desenho_pio(acender_leds, valor_led, pio, sm, 0.0, 0.5, 0.0);
        break;

      case '#': // Verifica se a tecla # foi pressionada
        desenho_pio(acender_leds, valor_led, pio, sm, 0.2, 0.2, 0.2);
        break;

      case '*': // Verifica se a tecla * foi pressionada
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