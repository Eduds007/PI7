# PI-7 — Perna Robótica / Prótese de Membro Inferior

Projeto da disciplina **Projeto Integrado — 7º Semestre (PI-7)** da Escola
Politécnica da USP (PMR3403 - Atuadores e Acionamentos / PMR3405 - Mecanismos
para Automação), com apoio de PMR3404 (sistema de controle) e PMR3409/aula de
Controle Digital. Disciplina com 10 edições anteriores (2010-2019); esta
edição (2024) propôs uma **maquete de prótese de membro inferior para
reabilitação motora**, capaz de reproduzir os movimentos de flexão/extensão
de quadril e joelho durante a ultrapassagem de um obstáculo na marcha.

Autoria do firmware-base: Jun Okamoto Jr., a partir de um template original
do Prof. Marcos Barretto. Desenvolvimento do PI-7 2024: Victor Nascimento
Pereira, Danilo Dacca, Vitor Viana e Eduardo Araujo.

---

## Vídeos

| Protótipo real | Simulação MuJoCo |
| --- | --- |
| [![Vídeo da perna PI-7 real em funcionamento](docs/midia/leg_demo_thumb.jpg)](docs/midia/leg_demo.mp4) | [![Vídeo da simulação MuJoCo da perna PI-7](docs/midia/mujoco_sim_thumb.jpg)](docs/midia/mujoco_sim.mp4) |
| [`leg_demo.mp4`](docs/midia/leg_demo.mp4) — base em "T invertido" de MDF, motor+redutor no quadril, ligação coxa/joelho e pé impresso em 3D executando o movimento real. | [`mujoco_sim.mp4`](docs/midia/mujoco_sim.mp4) — reconstrução em C + MuJoCo (ver [`sim/`](sim/README.md)) da mesma trajetória, cinemática inversa e controlador PID, rodando sobre um modelo físico simulado. |

*O GitHub não reproduz vídeo `.mp4` embutido no README — clique na miniatura
para abrir o arquivo. Mais imagens/vídeos do protótipo real serão
adicionados aqui em breve.*

---

## Requisitos do projeto

- **Objetivo**: projetar e construir um mecanismo atuado e controlado que
  demonstre o potencial de substituir o membro inferior humano nos
  movimentos de flexão/extensão da coxa e do joelho durante a caminhada,
  com foco específico na ultrapassagem de um obstáculo.
- **Escala**: aproximadamente 1:2,5 de uma perna humana (elos finais:
  coxa/L1 = 210mm, canela/L2 = 247mm — cerca de 1:2,1 dado que o
  comprimento humano de referência era 954mm para uma pessoa de 1,80m).
- **Mecanismo**: 2 graus de liberdade (quadril + joelho), montado sobre uma
  base fixa em formato de "T invertido" (o quadril não se desloca — só a
  coxa e a canela se movem, como num rig de bancada).
- **Obstáculo de referência**: altura 260mm, largura 60mm (em escala
  humana, conforme o enunciado da disciplina).
- Ver [`docs/Tema_PI7_2024_v2.pdf`](docs/Tema_PI7_2024_v2.pdf),
  [`docs/Apresentação do PI7.pdf`](docs/Apresentação%20do%20PI7.pdf) e
  [`docs/Conexoes PI-7 2024.pdf`](docs/Conexoes%20PI-7%202024.pdf) (não
  versionados neste git, mas presentes no diretório do projeto) para o
  enunciado completo, requisitos biomecânicos e o diagrama de conexões
  entre os módulos.

---

## Hardware

Arquitetura de controle distribuída em 3 níveis, conforme
[`docs/Conexoes PI-7 2024.pdf`](docs/Conexoes%20PI-7%202024.pdf):

```
PC  <--USB/Modbus-ASCII-->  Raspberry Pi Pico W  <--UART/protocolo próprio-->  PIC16F886 (quadril)
                                                  <--UART/protocolo próprio-->  PIC16F886 (joelho)
```

- **Raspberry Pi Pico W**: recebe a trajetória e os ganhos PID do PC via
  Modbus-ASCII simplificado (USB), calcula a cinemática inversa 2 elos
  (coxa+canela) e repassa os setpoints angulares e ganhos aos dois PICs.
- **2× PIC16F886**: um por junta (quadril = endereço `'a'`, joelho =
  endereço `'b'`), cada um rodando seu próprio laço de controle PID local
  e lendo seu próprio encoder — ver seção Controle.
- **2× ponte H L298** (uma por motor) + regulador **7805** (gera 5V a
  partir dos 12V da fonte) para acionamento dos motores DC.
- **Fonte chaveada +12V @ 10A** alimentando as pontes H.
- **Encoders** (1852 pulsos por rotação do eixo de saída do redutor),
  lidos por Interrupt-on-Change no Port B de cada PIC.
- Conector de 26 vias (flat cable) entre cada PIC e sua ponte H, com sinais
  de PWM1/PWM2/DIR1, encoder (ENC_A/ENC_B), alimentação e programação ICSP.
- Conversores serial↔USB usados para testes/bypass (substituindo o Pico W
  ou os PICs diretamente a partir de um PC, quando necessário).

*(fotos da montagem eletrônica/mecânica real serão adicionadas aqui em breve)*

---

## Software

O projeto tem três "camadas" de software, cada uma num diretório/projeto
separado:

### 1. Firmware do Raspberry Pi Pico W (`src/`)

FreeRTOS rodando no RP2040 (build via `CMakeLists.txt` + Pico SDK, veja
`pico_sdk_import.cmake`). Módulos principais (`src/pi7/`):

- `comm_pc/modbus.c` — protocolo Modbus-ASCII simplificado com o PC (função
  0x03 leitura de registrador, 0x06 escrita de registrador, 0x15 upload de
  trajetória, 0x08 upload de ganhos PID).
- `command_interpreter/` — mapa de registradores (start/pause/resume/stop,
  posição atual, linha atual do programa).
- `trj_program/` — armazena o programa de trajetória (até 28 pontos (x,y)
  do pé) — ver seção Trajetória.
- `trj_control/` — cinemática inversa 2 elos que converte cada ponto (x,y)
  em ângulos de quadril e joelho — ver seção Trajetória.
- `trj_state/` — estado corrente (linha do programa, X/Y).
- `comm_pic/` — protocolo de baixo nível com os PICs
  (`:<endereço><comando><valor>;`, comandos `p`=posição, `g`/`i`/`d`=ganhos
  P/I/D, `h`=home) sobre UART0 (quadril) e UART1 (joelho).

### 2. Firmware dos PICs (`pidx.X/pidx.X/`)

Projeto MPLAB X/XC8 para o **PIC16F886**, um por junta — este é o
controlador que de fato fecha a malha de posição, lendo o encoder próprio e
acionando o motor via PWM (detalhes na seção Controle). **Não está
versionado neste repositório git** (existe apenas no disco local), mas é
peça central do projeto. Ver `pidx.X/pidx.X/README.md` para a explicação
completa da implementação.

### 3. Software do PC (`ANTLR+MODBUS/`)

- `serial_communication.py` — parseia o G-code da trajetória (via parser
  gerado pelo ANTLR), monta as mensagens Modbus-ASCII (ganhos PID, upload
  de trajetória, comandos start/pause/continue/stop) e as envia ao Pico W
  por serial USB.
- Ganhos PID finais usados: quadril Kp=25 Ki=3 Kd=0, joelho Kp=5 Ki=1 Kd=0
  (`set_ganho()`).

---

## Controle

Malha de controle em dois níveis, rodando em cadências diferentes:

- **Pico W (200ms)**: `taskNCProcessing` gera um novo setpoint angular a
  cada 200ms a partir do próximo ponto (x,y) da trajetória, via cinemática
  inversa (`tcl_generateSetpoint`), e envia aos PICs.
- **PIC16F886, um por junta (100ms)**: `pid.c` fecha a malha local lendo o
  próprio encoder e recalculando o PID a cada 100ms (`PID_INTERVAL`) —
  entre um tick e outro, o setpoint recebido é mantido (sample-and-hold).

O controlador, apesar do nome, **só aplica a parcela proporcional**:
`activation = kp * error`. Os setters de Ki/Kd existem e recebem valores
via protocolo serial, mas nunca são somados em `pid_pid()`. Duas
particularidades do firmware real:

- **"Chute inicial"** de ±150 (de uma faixa de saturação ±1000) aplicado a
  qualquer erro não-nulo antes de saturar — necessário porque o motor
  usado só girava a partir de ~15% de duty cycle (zona morta mecânica do
  motor+redutor).
- **Escala do encoder**: pulsos convertidos para graus dividindo por 5
  (aproximação inteira de 1852 pulsos/rotação ÷ 360 ≈ 5,14, truncada),
  gerando um pequeno erro de regime permanente (~2,9%) no hardware real.

Ganhos usados (`ANTLR+MODBUS/serial_communication.py`, `set_ganho()`):
quadril Kp=25 Ki=3 Kd=0, joelho Kp=5 Ki=1 Kd=0 — mas como só a parcela P é
aplicada, apenas o Kp de fato importa.

---

## Trajetória

A trajetória do pé é descrita num dialeto simples de G-code, em
`ANTLR+MODBUS/GCode-example`:

```
N001 G01 X000 Y420
N002 G01 X006 Y398
...
N027 G01 X314 Y408
N028 M30
```

Cada linha é um ponto (x,y) em mm da trajetória do pé durante a fase de
balanço sobre o obstáculo. Os pontos foram desenhados/ajustados à mão de
forma iterativa (não há geração algorítmica de marcha no repositório):
começou como um teste de 2 pontos, passou por uma versão de 50 pontos e foi
por fim afinado para os **27 pontos atuais** (removendo intermediários,
preservando a forma da curva).

Pipeline de processamento:

1. `ANTLR+MODBUS/GCode.g4` — gramática ANTLR4 do dialeto G-code, compilada
   para um lexer/parser/listener Python (`GCodeLexer.py`, `GCodeParser.py`,
   `GCodeListener.py`).
2. `serial_communication.py` — usa o parser gerado para extrair os pontos
   (x,y) e envia como upload de trajetória (Modbus-ASCII) ao Pico W.
3. `trj_control.c` (`tcl_generateSetpoint`, no Pico W) — converte cada
   ponto (x,y) em ângulos de quadril/joelho por cinemática inversa 2 elos
   (lei dos cossenos), um novo setpoint a cada 200ms.

O programa é um único passo por cima do obstáculo (não um ciclo de marcha
completo): termina no ar, perto do último ponto, sem retornar
automaticamente ao chão.

---

## Simulação

Como a perna física e o hardware não estão mais disponíveis, o diretório
[`sim/`](sim/README.md) contém uma **simulação em C + MuJoCo** que
reconstrói o comportamento do sistema completo — cinemática inversa,
trajetória real e o controlador PID real (portado do firmware do PIC) —
rodando sobre um modelo físico simulado da perna (quadril fixo, coxa e
canela, obstáculo no chão). Vídeo em [Vídeos](#vídeos) acima.

Principais pontos (detalhes completos em [`sim/README.md`](sim/README.md)):

- Cinemática inversa portada literalmente de `trj_control.c`.
- Controlador PID portado literalmente de `pidx.X/pidx.X/pid.c` (incluindo
  o "chute" de ±150 e o fato de ser proporcional puro), com pequenos
  ajustes extras (zona morta de erro, amortecimento das juntas) para
  compensar o atrito estático do motor real que o MuJoCo não modela.
- Trajetória real de 27 pontos embutida.
- Dois binários: `leg_view` (viewer interativo) e `leg_record` (grava
  vídeo `.mp4`).

```bash
cd sim
./build/leg_view          # viewer interativo (depois de compilado, ver sim/README.md)
./build/leg_record out.mp4
```

---

## Licença

MIT — ver [`LICENSE`](LICENSE). Base FreeRTOS/Pico W original por Jun
Okamoto Jr.
