import serial
import time
import logging

logging.basicConfig(format='%(message)s', level=logging.WARNING)


def read_data(ser):
    pong = b""
    is_r = False
    is_n = False
    start_time = time.clock()
    while time.clock() - start_time < 0.4 or ser.in_waiting:
        one_byte = ser.read(1)
        logging.debug(one_byte)
        if one_byte == b"\n":
            if is_n:
                logging.info("pong")
                return pong
            else:
                is_r = True
        elif one_byte == b"\r":
            if is_r:
                logging.info("pong")
                return pong
            else:
                is_n = True
        else:
            pong += one_byte

        # addition = b""
        # is_slash = False
        # while True:
        #     one_byte = ser.read(1)
        #     print(one_byte)
        #     if one_byte == '/':
        #         is_slash = True
        #     elif one_byte == '>':
        #         if is_slash:
        #             break
        #     else:
        #         is_slash = False
        #         addition += one_byte


baudrate = 115200
speed = baudrate * 0.8 / 8

ser5 = serial.Serial()
ser5.port = 'COM5'
ser5.baudrate = baudrate
ser5.timeout = 0.1
ser5.setDTR(False)
ser5.setRTS(False)
ser5.open()

ser6 = serial.Serial()
ser6.port = 'COM6'
ser6.baudrate = baudrate
ser6.timeout = 0.1
ser6.setDTR(False)
ser6.setRTS(False)
ser6.open()

time.sleep(1)

fCounter = 0
lines = open("MMWMessage.h", "rb").read().split(b'\r\n')
fSize = len(lines)

f_out = open("out.h", "wb", buffering=0)

write_err_count = 0
read_err_count = 0
start_time = time.clock()

for line in lines:
    fCounter += 1
    print('%3.2f' % (fCounter / fSize * 100) + '%\t' + str(fCounter) + '/' + str(fSize) + '\r')
    if len(line) == 0:
        line = b' ' + line

    data5Out = line
    logging.info(data5Out)
    logging.info("line len: " + str(len(line)))
    eq = ser5.write(data5Out + b'\n') == len(data5Out) + 1
    if not eq:
        write_err_count += 1
        logging.warning('err write data')
    logging.info("write to COM5: " + str(eq))
    delay = len(line) / speed * 2
    logging.info("waiting transmitter " + str(int(delay * 1000)) + " ms...")
    ser5.flush()

    data5In = read_data(ser5)
    # print(data5In)
    # data5In = ser5.readline()
    if data5In is None:
        write_err_count += 1
        logging.warning("read from COM5: " + str(False))
        logging.warning('')
        continue
    data5In = data5In
    logging.info("read from COM5: " + str(data5Out == data5In))

    delay = 0.01 * len(line) / 32 + 0.1
    time.sleep(delay)
    # data6In = ser6.readline()
    data6In = read_data(ser6)
    if data6In is None:
        read_err_count += 1
        logging.warning('err read data')
        logging.warning('')
        continue

    f_out.write(data6In + b'\r\n')
    logging.info("waiting radio " + str(int(delay * 1000)) + " ms...")
    logging.info(data6In)
    eq = data6In == data5Out
    if not eq:
        read_err_count += 1
        logging.warning("read from COM6: " + str(eq))
    else:
        logging.info("read from COM6: " + str(eq))
    logging.info('')

print("================================================")
print("write_err_count: " + str(write_err_count))
print("read_err_count: " + str(read_err_count))
print("time: " + '%5.2f' % (time.clock() - start_time) + ' sec')
