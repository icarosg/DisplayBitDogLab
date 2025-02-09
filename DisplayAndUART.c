#include <stdio.h>
#include "pico/stdlib.h"
#include "ws2818b.pio.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include <string.h>

// macros
#define LED_COUNT 25
#define LED_PIN 7
#define DEBOUNCE_DELAY_MS 200

#define LED_R 13
#define LED_G 11
#define LED_B 12

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define LED_BRIGHTNESS 200 // define a intensidade dos leds

// protótipos
void init_hardware(void);
void init_leds(void);
void set_led(int index, uint8_t r, uint8_t g, uint8_t b);
void clear_leds(void);
void write_leds(void);
void exibirNumero(int countBotao);
static void gpio_irq_handler(uint gpio, uint32_t events);
void atualizarDisplay(void);

// variáveis globais
int countBotao = 0;
static volatile uint32_t last_time = 0; // armazena o tempo do último evento (em microssegundos)
ssd1306_t ssd;                          // inicializa a estrutura do display
bool cor = true;
char c;
char estadoLED[15];

// se for 1, deve acender
int numerosMatriz[10][25] = {
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0}, // 0

    {0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 1, 1, 0, 0,
     0, 0, 1, 0, 0}, // 1

    {0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0}, // 2

    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0}, // 3

    {0, 1, 0, 0, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0}, // 4

    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0}, // 5

    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0}, // 6

    {0, 0, 0, 1, 0,
     0, 0, 1, 0, 0,
     0, 1, 0, 0, 0,
     0, 0, 0, 0, 1,
     1, 1, 1, 0, 0}, // 7

    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0}, // 8

    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0} // 9
};

PIO np_pio;
uint sm;
struct pixel_t
{
  uint8_t G, R, B;
};
typedef struct pixel_t npLED_t;
npLED_t leds[LED_COUNT];

// variável para controlar o debounce
absolute_time_t last_press_time;

void init_hardware(void)
{
  // configura botao A na GPIO 5 com pull-up e interrupção na borda de descida
  gpio_init(5);
  gpio_set_dir(5, GPIO_IN);
  gpio_pull_up(5);
  gpio_set_irq_enabled_with_callback(5, GPIO_IRQ_EDGE_FALL, true, gpio_irq_handler);

  // configura botão B na GPIO 6 com pull-up e interrupção na borda de descida
  gpio_init(6);
  gpio_set_dir(6, GPIO_IN);
  gpio_pull_up(6);
  gpio_set_irq_enabled_with_callback(6, GPIO_IRQ_EDGE_FALL, true, gpio_irq_handler);

  gpio_init(LED_R);              // inicializa LED_R como saída
  gpio_set_dir(LED_R, GPIO_OUT); // configura LED_R como saída

  gpio_init(LED_G);              // inicializa LED_G como saída
  gpio_set_dir(LED_G, GPIO_OUT); // configura LED_G como saída

  gpio_init(LED_B);              // inicializa LED_B como saída
  gpio_set_dir(LED_B, GPIO_OUT); // configura LED_B como saída

  init_leds(); // inicializa os LEDs
  clear_leds();
  write_leds();

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // pull up the data line
  gpio_pull_up(I2C_SCL);                                        // pull up the clock line
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // inicializa o display
  ssd1306_config(&ssd);                                         // configura o display
  ssd1306_send_data(&ssd);                                      // envia os dados para o display

  // limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  last_press_time = get_absolute_time(); // inicializa o tempo do último botão pressionado

  printf("RP2040 inicializado. Digite um caracter ou número para exibir no Display ou na Matriz de LEDs.\n");
}

void init_leds(void)
{
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;
  sm = pio_claim_unused_sm(np_pio, true);
  ws2818b_program_init(np_pio, sm, offset, LED_PIN, 800000.f);
  for (int i = 0; i < LED_COUNT; i++)
  {
    leds[i].R = leds[i].G = leds[i].B = 0;
  }
}

void set_led(int index, uint8_t r, uint8_t g, uint8_t b)
{
  if (index < LED_COUNT)
  {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
  }
}

void clear_leds(void)
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    set_led(i, 0, 0, 0);
  }
}

void write_leds(void)
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
}

void exibirNumero(int countBotao)
{
  clear_leds();
  if (countBotao > 9 || countBotao < 0)
    countBotao = 0;

  for (int incremento = 0; incremento < 25; incremento++)
  {
    if (numerosMatriz[countBotao][incremento] == 1)
    {
      set_led(incremento, LED_BRIGHTNESS, LED_BRIGHTNESS / 2, LED_BRIGHTNESS / 3);
    }
  }
  write_leds();
}

/**
 * Função de interrupção para tratar os botões A (GPIO 5) e B (GPIO 6)
 * Implementa debouncing software usando a constante DEBOUNCE_DELAY_MS.
 */
static void gpio_irq_handler(uint gpio, uint32_t events)
{
  uint32_t current_time = to_us_since_boot(get_absolute_time());
  if (current_time - last_time < (DEBOUNCE_DELAY_MS * 1000))
    return;

  last_time = current_time;

  if (gpio == 5)
  {                                    // botão A:
    gpio_put(LED_B, 0);                // desliga o led azul
    gpio_put(LED_G, !gpio_get(LED_G)); // alterna o estado do LED verde

    strcpy(estadoLED, gpio_get(LED_G) ? "LED VERDE ON" : "LED VERDE OFF");
    printf("\nEstado do LED verde alternado\n%s \n", gpio_get(LED_G) ? "LED verde on" : "LED verde off");

    atualizarDisplay();
  }

  if (gpio == 6)
  {                                    // botão B:
    gpio_put(LED_G, 0);                // desliga o led verde
    gpio_put(LED_B, !gpio_get(LED_B)); // alterna o estado do LED azul

    strcpy(estadoLED, gpio_get(LED_B) ? "LED AZUL ON" : "LED AZUL OFF");
    printf("\nEstado do LED azul alternado\n%s \n", gpio_get(LED_B) ? "LED azul on" : "LED azul off");

    atualizarDisplay();
  }
}

void leituraUSB()
{
  if (stdio_usb_connected())
  { // certifica-se de que o USB está conectado

    if (scanf(" %c", &c) == 1)
    {
      printf("Recebido: '%c'\n", c);

      if (c >= '0' && c <= '9')
      {
        int n = c - '0'; // converte o caracter numérico para inteiro
        cor = !cor;

        ssd1306_fill(&ssd, !cor);                    // limpa o display
        ssd1306_draw_string(&ssd, "NUMERO", 15, 48); // desenha uma string
        ssd1306_draw_string(&ssd, &c, 73, 48);       // desenha uma string, o número digitado pelo usuário

        exibirNumero(n); // exibe o número na matriz de LEDs
      }
      else
      {
        cor = !cor;

        ssd1306_fill(&ssd, !cor);                       // limpa o display
        ssd1306_draw_string(&ssd, "CARACTER ", 15, 48); // desenha uma string
        ssd1306_draw_string(&ssd, &c, 89, 48);          // digita o caractere digitado pelo usuário
      }
    }

    // atualiza o conteúdo do display com animações
    ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);       // desenha um retângulo
    ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 10); // desenha uma string
    ssd1306_send_data(&ssd);                            // atualiza o display
  }

  sleep_ms(40);
  atualizarDisplay();
}

void atualizarDisplay()
{
  if (stdio_usb_connected())
  {                           // certifica-se de que o USB está conectado

    if (c >= '0' && c <= '9')
    {
      int n = c - '0'; // converte o caracter numérico para inteiro
      cor = !cor;
      ssd1306_fill(&ssd, !cor); // limpa o display

      ssd1306_draw_string(&ssd, "NUMERO", 15, 48); // desenha uma string
      ssd1306_draw_string(&ssd, &c, 65, 48);       // desenha uma string, o número digitado pelo usuário

      exibirNumero(n); // exibe o número na matriz de LEDs
    }
    else
    {
      cor = !cor;
      ssd1306_fill(&ssd, !cor); // limpa o display

      ssd1306_draw_string(&ssd, "CARACTER ", 15, 48); // desenha uma string
      ssd1306_draw_string(&ssd, &c, 89, 48);          // digita o caractere digitado pelo usuário
    }
  }

  if (estadoLED)
    ssd1306_draw_string(&ssd, estadoLED, 20, 30); // desenha uma string referente ao estado do led

  ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);       // desenha um retângulo
  ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 10); // desenha uma string
  ssd1306_send_data(&ssd);                            // atualiza o display

  sleep_ms(40);
}

int main()
{
  stdio_init_all();
  init_hardware();

  // loop principal (as ações dos botões são tratadas via IRQ)
  while (true)
  {
    leituraUSB();
    sleep_ms(10); // pequeno atraso
  }
}