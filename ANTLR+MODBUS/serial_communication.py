from antlr4 import *
from GCodeLexer import GCodeLexer
from GCodeParser import GCodeParser
from GCodeListener import GCodeListener
from pymodbus.client.sync import ModbusSerialClient as ModbusClient
import serial
import time

# Função para configurar a comunicação serial
def setup_serial(port, baudrate):
    ser = serial.Serial(port, baudrate)
    return ser

# Definições de constantes Modbus
REG_PAUSE = 1

# Função para enviar mensagem Modbus para o comando Pause
def send_modbus_message_pause(client):
    # Construir mensagem Modbus conforme especificado
    message = bytearray([0x3A, 0x30, 0x31, 0x30, 0x36, 0x30, 0x31, 0x30, 0x31, 0x37, 0x37, 0x0D, 0x0A])
    client.write_registers(REG_PAUSE, message, unit=1)

class WalkListener(GCodeListener):
    def __init__(self, client, serial):
        self.client = client
        self.serial = serial

    def enterStatement(self, ctx):
        command = ctx.mfunc().getText()
        if command == 'M01':
            send_modbus_message_pause(self.client)
            # Exemplo de envio via serial (comando 'M01' envia "PAUSE" via serial)
            self.serial.write(b"PAUSE\n")
        # Adicione mais casos para outros comandos Modbus e serial conforme necessário

def main():
    # Configuração do cliente Modbus
    client = ModbusClient(method='rtu', port='/dev/ttyUSB0', baudrate=9600)
    client.connect()

    # Configuração da comunicação serial
    ser = setup_serial('/dev/ttyUSB1', 9600)  # Substitua pelo seu dispositivo serial correto

    # Processamento do arquivo GCode
    with open("caminho_para_seu_arquivo_gcode.gcode") as file:
        data = file.read()

    lexer = GCodeLexer(InputStream(data))
    stream = CommonTokenStream(lexer)
    parser = GCodeParser(stream)
    tree = parser.gcode()
    listener = WalkListener(client, ser)
    walker = ParseTreeWalker()
    walker.walk(listener, tree)

    # Fechar conexões
    client.close()
    ser.close()

if __name__ == '__main__':
    main()
