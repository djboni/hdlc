#!/usr/bin/env python3
# -*- coding: utf-8 -*-

class HDLC:
    def __init__(self, readByte=None, writeByte=None, CRC=None):
        if False in (readByte != None, writeByte != None, CRC != None):
            raise ValueError('Must use the three parameters function readByte, function readByte, anc CRC class')
        self.readByte = readByte
        self.writeByte = writeByte
        self.crc = CRC()
        self.txcrc = CRC()
        self.init()

    def init(self):
        self.data = []
        self.status = 0
        self.crc.init()

        self.msg_queue = []

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

    # TODO RX
