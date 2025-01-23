#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "matriz_led.pio.h"

#define NUM_PIXELS 25 //número de leds na matriz
#define LED_PIN 7 //pino de saída do led

//Função para habilitar o modo Bootsel
void bootsel(){
  reset_usb_boot(0,0);
}

//TECLADO
// definição das colunas e das linhas e mapeamento do teclado
uint8_t coluna[4] = {18,19,20,21};
uint8_t linha[4] = {22,26,27,28};

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

//rotina para acionar a matrix de leds - ws2812b
void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b){

    for (int16_t i = 0; i < NUM_PIXELS; i++) {

            valor_led = matrix_rgb(r, g, b);
            pio_sm_put_blocking(pio, sm, valor_led);
        }
    }

double apagar_leds[25] = {0.0, 0.0, 0.0, 0.0, 0.0,           //Apagar LEDs da matriz
                          0.0, 0.0, 0.0, 0.0, 0.0, 
                          0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 0.0};

double desenho1_1[25] = {0.3, 0.0, 0.0, 0.0, 0.3,           //Desenho Tecla 1 Parte 1
                         0.0, 0.3, 0.0, 0.3, 0.0, 
                         0.0, 0.0, 0.3, 0.0, 0.0,
                         0.0, 0.0, 0.3, 0.0, 0.0,
                         0.0, 0.0, 0.3, 0.0, 0.0};

double desenho1_2[25] = {0.3, 0.3, 0.3, 0.3, 0.3,           //Desenho Tecla 1 Parte 2
                         0.3, 0.0, 0.0, 0.0, 0.3, 
                         0.3, 0.0, 0.0, 0.0, 0.3,
                         0.3, 0.0, 0.0, 0.0, 0.3,
                         0.3, 0.3, 0.3, 0.3, 0.3};

double desenho1_3[25] = {0.3, 0.0, 0.0, 0.0, 0.3,           //Desenho Tecla 1 Parte 3
                         0.3, 0.0, 0.0, 0.0, 0.3, 
                         0.3, 0.0, 0.0, 0.0, 0.3,
                         0.3, 0.0, 0.0, 0.0, 0.3,
                         0.3, 0.3, 0.3, 0.3, 0.3};

double desenho1_4[25] = {0.3, 0.0, 0.0, 0.0, 0.3,           //Desenho Tecla 1 Parte 4
                         0.3, 0.0, 0.3, 0.0, 0.3, 
                         0.3, 0.0, 0.3, 0.0, 0.3,
                         0.3, 0.0, 0.3, 0.0, 0.3,
                         0.0, 0.3, 0.3, 0.3, 0.0};

double desenho1_5[25] = {0.0, 0.0, 0.3, 0.0, 0.0,           //Desenho Tecla 1 Parte 5
                         0.0, 0.0, 0.3, 0.0, 0.0, 
                         0.0, 0.0, 0.3, 0.0, 0.0,
                         0.0, 0.0, 0.3, 0.0, 0.0,
                         0.0, 0.0, 0.3, 0.0, 0.0};

double desenho1_6[25] = {0.3, 0.0, 0.0, 0.0, 0.3,           //Desenho Tecla 1 Parte 6
                         0.3, 0.3, 0.0, 0.0, 0.3, 
                         0.3, 0.0, 0.3, 0.0, 0.3,
                         0.3, 0.0, 0.0, 0.3, 0.3,
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
    case 1:                             // Verifica se a tecla 1 foi pressionada

        desenho_pio(desenho1_1, valor_led, pio, sm, r, g, b);
        sleep_ms(100);
        desenho_pio(desenho1_2, valor_led, pio, sm, r, g, b);
        sleep_ms(100);
        desenho_pio(desenho1_3, valor_led, pio, sm, r, g, b);
        sleep_ms(100);
        desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);
        sleep_ms(100);
        desenho_pio(desenho1_4, valor_led, pio, sm, r, g, b);
        sleep_ms(100);
        desenho_pio(desenho1_5, valor_led, pio, sm, r, g, b);
        sleep_ms(100);
        desenho_pio(desenho1_6, valor_led, pio, sm, r, g, b);
        sleep_ms(100);
        desenho_pio(apagar_leds, valor_led, pio, sm, r, g, b);

        break;
    
    case 2:                                 // Verifica se a tecla 2 foi pressionada



        break;

    case 3:                                 // Verifica se a tecla 3 foi pressionada



        break;

    case 4:                                 // Verifica se a tecla 4 foi pressionada



        break;

    case 5:                                 // Verifica se a tecla 5 foi pressionada



        break;
    
    case 6:                                 // Verifica se a tecla 6 foi pressionada



        break;

    case 7:                                 // Verifica se a tecla 7 foi pressionada



        break;
    
    case 8:                                 // Verifica se a tecla 8 foi pressionada



        break;

    case 9:                                 // Verifica se a tecla 9 foi pressionada



        break;

    case 'A':                               // Verifica se a tecla A foi pressionada
         
        apagar_leds;
        break;

    case 'B':                             // Verifica se a tecla B foi pressionada
        r = 0;
        g = 0;
        b = 1;
        break;

    case 'C':                             // Verifica se  a tecla C foi pressionada
        r = 0.8;
        g = 0;
        b = 0;
        break;

    case 'D':                             // Verifica se a tecla D foi pressionada
        r = 0;
        g = 0.5;
        b = 0;
        break;

    case '#':                             // Verifica se a tecla # foi pressionada
        r = 0.2;
        g = 0.2;
        b = 0.2;
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
