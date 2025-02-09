# Projeto BitDogLab - Controle de LEDs, Botões e Display

## Descrição do Projeto

Neste projeto, foram utilizados os seguintes componentes conectados à placa BitDogLab:

- **Matriz 5x5 de LEDs WS2812** (endereçável) – Conectada à **GPIO 7**
- **LED RGB** – Pinos conectados às **GPIOs 11, 12 e 13**
- **Botão A** – Conectado à **GPIO 5**
- **Botão B** – Conectado à **GPIO 6**
- **Display SSD1306** – Conectado via **I2C (GPIO 14 e GPIO 15)**

## Funcionalidades do Projeto

1. **Modificação da Biblioteca font.h**

   - Foram adicionados caracteres minúsculos à biblioteca `font.h`.

2. **Entrada de caracteres via PC**

   - O Serial Monitor do VS Code permite a digitação de caracteres.
   - Cada caractere digitado no Serial Monitor é exibido no display SSD1306.
   - **Observação:** Apenas um caractere é enviado de cada vez, sem suporte para strings completas.
   - Quando um número entre 0 e 9 é digitado, um símbolo correspondente ao número é exibido também na matriz 5x5 WS2812.

3. **Interação com o Botão A**

   - Pressionar o botão A alterna o estado do **LED RGB Verde** (ligado/desligado).
   - A operação é registrada de duas formas:
     - Uma mensagem informativa sobre o estado do LED é exibida no **display SSD1306**.
     - Um texto descritivo sobre a operação é enviado ao **Serial Monitor**.

4. **Interação com o Botão B**

   - Pressionar o botão B alterna o estado do **LED RGB Azul** (ligado/desligado).
   - A operação é registrada de duas formas:
     - Uma mensagem informativa sobre o estado do LED é exibida no **display SSD1306**.
     - Um texto descritivo sobre a operação é enviado ao **Serial Monitor**.

## Requisitos do Projeto

1. **Uso de interrupções:** Todas as funcionalidades relacionadas aos botões foram implementadas utilizando **rotinas de interrupção (IRQ)**.
2. **Debouncing:** Foi implementado o tratamento do **bouncing** dos botões via software.
3. **Controle de LEDs:** O projeto inclui o uso de **LEDs comuns e LEDs WS2812**, demonstrando domínio de diferentes tipos de controle.
4. **Utilização do Display 128x64:**
   - Uso de fontes **maiúsculas e minúsculas**.
   - Demonstração do domínio do uso de bibliotecas.
   - Compreensão do funcionamento do **protocolo I2C**.
5. **Envio de informação pela UART:** Foi implementada a comunicação serial via UART, garantindo o correto envio de informações.

## Status do Projeto

Todas as funcionalidades do projeto foram **implementadas com sucesso** e **todos os requisitos foram atendidos**. O sistema está operando de forma estável e confiável, garantindo um funcionamento preciso e eficiente.

## Como Clonar

1. **Clone o repositório:**
   ```bash
   git clone https://github.com/icarosg/DisplayBitDogLab.git
   cd DisplayBitDogLab
   ```

