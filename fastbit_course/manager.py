import serial
from serial.tools import list_ports
from enum import Enum
import argparse #cli argument parse
import zlib #CRC32 calculations

class Command(Enum):
    BL_GET_VER = 0x51
    BL_GET_HELP = 0x52
    BL_GET_CID = 0x53
    BL_GET_RDP_STATUS = 0x54
    BL_GO_TO_ADDR = 0x55
    BL_FLASH_ERASE = 0x56
    BL_MEM_WRITE = 0x57
    BL_ENDIS_RW_PROTECT = 0x58
    BL_MEM_READ = 0x59
    BL_READ_SECTOR_STATUS = 0x5A
    BL_OPT_READ = 0x5B
    BL_ACK = 0xA5
    BL_NACK = 0x7F

class Commands:
    def __init__(self, command, length):
        self.command = command
        self.length = length

#command list with payload length (without 4 bytes of CRC32)
cmd_list = []
cmd_list.append(Commands(Command.BL_GET_VER, 1))
cmd_list.append(Commands(Command.BL_GET_HELP, 1))
cmd_list.append(Commands(Command.BL_GET_CID, 1))
cmd_list.append(Commands(Command.BL_GET_RDP_STATUS, 1))
cmd_list.append(Commands(Command.BL_GO_TO_ADDR, 5))

# configure the serial connections
ser = serial.Serial(
    port = list(list_ports.grep("0483:374B"))[0][0],
    baudrate = 115200,
    parity = serial.PARITY_NONE,
    stopbits = serial.STOPBITS_ONE,
    bytesize = serial.EIGHTBITS,
    timeout = 1 #timeout seconds
)

CRC_BYTES = 4

# PROGRAM FUNCTION
def get_response(tx):
    command = tx[1]
    crc = zlib.crc32(tx).to_bytes(4, 'little')
    tx.extend(crc)
    ser.write(tx)
    ack_nack = ser.read(1)
    if(ack_nack[0] == Command.BL_ACK.value):
        follow_length = ser.read(1)
        rx = ser.read(follow_length[0])
        match command:
            case Command.BL_GET_VER.value:
                print("BL_VER: " + str(rx[0]))
            case Command.BL_GET_HELP.value:
                print("Supported commands:")
                for i in rx:
                    print(Command(i).name)
            case Command.BL_GET_CID.value:
                print("CID: " + rx[::-1].hex())
            case Command.BL_GET_RDP_STATUS.value:
                print("Data protection byte: " + rx.hex())
            case Command.BL_GO_TO_ADDR.value:
                if rx[0] == 0:
                    print("Address valid - jumping...")
                else:
                    print("Address invalid")
    elif(ack_nack[0] == Command.BL_NACK.value):
        print("NACK received")
    else:
        print("Wrong response byte")

def bl_ver():
    tx = bytearray([cmd_list[0].length + CRC_BYTES, cmd_list[0].command.value])
    get_response(tx)

def bl_help():
    tx = bytearray([cmd_list[1].length + CRC_BYTES, cmd_list[1].command.value])
    get_response(tx)

def bl_cid():
    tx = bytearray([cmd_list[2].length + CRC_BYTES, cmd_list[2].command.value])
    get_response(tx)

def bl_rdp():
    tx = bytearray([cmd_list[3].length + CRC_BYTES, cmd_list[3].command.value])
    get_response(tx)

def bl_go_addr(address_string):
    tx = bytearray([cmd_list[4].length + CRC_BYTES, cmd_list[4].command.value])
    addr = bytes.fromhex(address_string)
    tx.extend(addr[::-1])
    get_response(tx)

# PROGRAM FUNCTION


# MAIN
parser = argparse.ArgumentParser(description='STM32 bootloader client.')
parser.add_argument("-c", "--command", type=str, choices=["BL_GET_VER", "BL_GET_HELP", "BL_GET_CID", "BL_GET_RDP_STATUS", "BL_GO_TO_ADDR"],
                    help="command to send")
parser.add_argument("-a", "--address", type=str,
                    help="address to jump")

args = parser.parse_args()
#print(args)

if args.command == "BL_GET_VER":
    bl_ver()
elif args.command == "BL_GET_HELP":
    bl_help()
elif args.command == "BL_GET_CID":
    bl_cid()
elif args.command == "BL_GET_RDP_STATUS":
    bl_rdp()
elif args.command == "BL_GO_TO_ADDR":
    if args.address:
        bl_go_addr(args.address)
    else:
        print("Address not provided")
else:
    print("Unsupported command")

