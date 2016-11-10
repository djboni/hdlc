#!/usr/bin/env python3
# -*- coding: utf-8 -*-

class HDLC:

    class STAT:
        ESCAPED = -1
        RECEIVING = 0
        OK = 1
        CRCERR = 2

    def __init__(self, readByte=None, writeByte=None, CRC=None):
        if False in (readByte != None, writeByte != None, CRC != None):
            raise ValueError('Must use the three parameters function readByte, function readByte, anc CRC class')
        self.readByte = readByte
        self.writeByte = writeByte
        self.crc = CRC()
        self.txcrc = CRC()
        self.msg_queue = []
        self.init()

    def init(self):
        self.data = ''
        self.status = 0
        self.crc.init()

        self.DATAINVBIT = 0x20
        self.DATASTART = '~'
        self.DATAESCAPE = '}'
        self.DATAESCAPELIST = self.DATASTART + self.DATAESCAPE

    def transmitBlock(self, data):
        self.transmitStart()
        self.transmitBytes(data)
        self.transmitEnd()

    def transmitStart(self):
        self.writeByte(self.DATASTART)
        self.txcrc.init()

    def escapeAndWriteByte(self, data):
        if data in self.DATAESCAPELIST:
            self.writeByte(self.DATAESCAPE)
            self.writeByte(chr(ord(data) ^ self.DATAINVBIT))
        else:
            self.writeByte(data)

    def transmitByte(self, data):
        self.escapeAndWriteByte(data)
        self.txcrc.update(data)

    def transmitBytes(self, data):
        for c in data:
            self.transmitByte(c)

    def transmitEnd(self):
        self.txcrc.final()
        for i in range(0, self.txcrc.size):
            self.escapeAndWriteByte(chr(self.txcrc[i]))
        self.writeByte(self.DATASTART)

    def receive(self):
        c = self.readByte()
        if c == '':
            return 0

        if self.status >= self.STAT.OK:
            self.init()

        retv = 0

        if c == self.DATASTART:
            if self.status == self.STAT.RECEIVING and len(self.data) != 0:
                if self.crc.good():
                    self.status = self.STAT.OK
                    self.data = self.data[:-self.crc.size]
                    retv = len(self.data)
                    self.msg_queue.append(self.data)
                else:
                    self.status = self.STAT.CRCERR
            else:
                self.init()
        else:
            if self.status == self.STAT.ESCAPED:
                self.status = self.STAT.RECEIVING
                c = chr(ord(c) ^ self.DATAINVBIT)
                self.crc.update(c)
                self.data += c
            elif c != self.DATAESCAPE:
                self.crc.update(c)
                self.data += c
            else:
                self.status = self.STAT.ESCAPED

        return retv

if __name__ == '__main__':
    def f_read(x=['~\x00\x78\xF0~' + '~}^}]\xF1\xCD~']):
        if x[0] == '':
            return ''
        else:
            r = x[0][0]
            x[0] = x[0][1:]
            return r
    def f_write(data, x=['']):
        x[0] += data
        return x
    from CRC16_CCITT import CRC16_CCITT as CRC
    # from CRC32 import CRC32 as CRC

    # Init
    hdlc = HDLC(f_read, f_write, CRC)

    # TX
    hdlc.transmitBlock('\x00')
    x = f_write('')[0]
    s = '0x '
    for c in x:
        s += '%02X ' % ord(c)
    print(s)
    print(x == '~\x00\x78\xF0~')

    # TX2
    hdlc.transmitBlock('~}')
    x = f_write('')[0]
    s = '0x '
    for c in x:
        s += '%02X ' % ord(c)
    print(s)
    print(x == '~\x00\x78\xF0~' + '~}^}]\xF1\xCD~')

    # RX
    while hdlc.receive() == 0:
        pass
    print(hdlc.msg_queue)
    print(hdlc.msg_queue == ['\x00'])

    # RX 2
    while hdlc.receive() == 0:
        pass
    print(hdlc.msg_queue)
    print(hdlc.msg_queue == ['\x00', '~}'])
