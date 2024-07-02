from antlr4 import *
from GCodeLexer import GCodeLexer
from GCodeParser import GCodeParser
from GCodeListener import GCodeListener
import serial as sc

# Função para configurar a comunicação serial
def setup_serial(port, baudrate):
    ser = sc.Serial(port, baudrate)  # Use o alias `sc` para acessar `serial`
    return ser

# Função para enviar mensagem Modbus para o comando Pause
def send_modbus_message_pause(ser):
    message = bytearray([0x3A, 0x30, 0x31, 0x30, 0x36, 0x30, 0x31, 0x30, 0x31, 0x37, 0x37, 0x0D, 0x0A])
    ser.write(message)

# Função para enviar mensagem Modbus para o comando Start
def send_modbus_message_start(ser):
    message = bytearray([0x3A, 0x30, 0x31, 0x30, 0x36, 0x30, 0x30, 0x30, 0x31, 0x37, 0x38, 0x0D, 0x0A])
    ser.write(message)

# Função para processar comandos
def process_command(command, ser):
    if command == "START":
        send_modbus_message_start(ser)
        ser.write(b"START\n")
    elif command == "PAUSE":
        send_modbus_message_pause(ser)
        ser.write(b"PAUSE\n")
    else:
        print(f"Comando desconhecido: {command}")

class WalkListener(GCodeListener):
    def __init__(self, serial):
        self.serial = serial
        self.coordenadas = []
        self.set = False
        self.x = 0
        self.y = 0

    def enterStatement(self, ctx):
        if ctx.coordx() is not None:
            self.x = int(ctx.coordx().coord().getText())
            self.set = True
        if ctx.coordy() is not None:
            self.y = int(ctx.coordy().coord().getText())
            self.set = True
        if self.set:
            self.coordenadas.append((self.x, self.y))
        if ctx.mfunc() is not None:
            command = ctx.mfunc().getText()
            if command.startswith('M01'):
                process_command("PAUSE", self.serial)
            elif command == 'M02':
                process_command("START", self.serial)

def main():
    # Configuração da comunicação serial
    ser = setup_serial('/dev/ttyUSB1', 9600)  # Substitua pelo seu dispositivo serial correto

    # Processamento do arquivo GCode
    with open("ANTLR+MODBUS/GCode-example") as file:
        data = f'{file.read()}'
    lexer = GCodeLexer(InputStream(data))
    stream = CommonTokenStream(lexer)
    parser = GCodeParser(stream)
    tree = parser.gcode()
    listener = WalkListener(ser)
    walker = ParseTreeWalker()
    walker.walk(listener, tree)
    print(listener.coordenadas)

    # Loop para processar comandos adicionais
    while True:
        command = input("Digite um comando (START, PAUSE): ").strip().upper()
        process_command(command, ser)

    # Fechar conexões (não será alcançado devido ao loop infinito)
    ser.close()

if __name__ == '__main__':
    main()
