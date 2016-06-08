__author__ = 'Administrator'

class cptp_head:
    TYPE_ACK = 1
    TYPE_REQUEST = 0
    FUNC_RD = 1
    FUNC_WR = 2
    FUNC_REFRESH = 3
    HEAD_SIZE = 9
    ERRNO_SIZE = 1
    WITH_TSP = 1
    WITHOUT_TSP = 0
    def __init__(self, buff):
        self.bof = buff[0]
        self.des = buff[1] + buff[2] * 256
        self.src = buff[3] + buff[4] * 256
        self.len = buff[5]
        self.ver = buff[6] & 0x01
        self.with_tsp = (buff[6] & 0x02) >> 1
        self.kind = (buff[6] & 0x04) >> 2
        self.func = (buff[6] & 0xF8) >> 3
        self.seq = buff[7]
        self.count = buff[8] & 0x1F

class cptp:
    def __init__(self):
        self.rx = []
        self.tx = []
        self.rx_frame = []
        self.tx_frame = []

    def push_bytes(self, by):
        for b in by:
            self.rx.append(ord(b))
        while True:
            if len(self.rx) <= 9:
                return []

            if self.rx[0] != 0x68:
                self.rx = []
                return []

            need_bytes = self.rx[5] + 9 + 2
            if len(self.rx) < need_bytes:
                return []

            if self.rx[ need_bytes - 1] != 0x16:
                self.rx = self.rx[need_bytes:]
                continue

            sum = 0
            for x in range(need_bytes-2):
                sum = sum + self.rx[x]
            sum = sum & 0xFF
            if sum != self.rx[ need_bytes - 2]:
                self.rx = self.rx[need_bytes:]
                continue
            self.rx_frame.append(self.rx[:need_bytes])
            self.rx = self.rx[need_bytes:]
        return []

    def send(self, f):
        self.tx_frame.append(f)

    def _patch_head(self, buf, src, des, _type, func, seq, need_tsp):
        buf.append(0x68)
        buf.append(des&0xFF)
        buf.append((des&0xFF00) >> 8)
        buf.append(src&0xFF)
        buf.append((src&0xFF00) >> 8)
        buf.append(0)
        if need_tsp:
            need_tsp = 0x02
        if func == cptp_head.FUNC_RD:
            fun_num = 0x08
        if func == cptp_head.FUNC_WR:
            fun_num = 0x10
        if func == cptp_head.FUNC_REFRESH:
            fun_num = 0x18
        if _type == cptp_head.TYPE_ACK:
            t = 0x04
        else:
            t = 0x00
        buf.append(0x01 | need_tsp | t | fun_num)
        buf.append(seq)
        buf.append(0)
        return 9

    def patch_request_header(self, buf, src, des, func, seq, need_tsp):
        self._patch_head(buf, src, des, cptp_head.TYPE_REQUEST, func, seq, need_tsp)

    def patch_ack_header(self, buf, src, des, func, seq, errorno, need_tsp):
        self._patch_head(buf, src, des, cptp_head.TYPE_ACK, func, seq, need_tsp)
        buf.append(errorno)
        buf[5] = 1

    def patch_id(self, buf, id):
        h = cptp_head(buf)
        e = 0
        if h.kind == cptp_head.TYPE_ACK:
            e = 1
        count = cptp_head.HEAD_SIZE + e + h.count * 2
        buf.append(id & 0xFF)
        buf.append((id & 0xFF00) >> 8)
        buf[ 5 ] = buf[ 5 ] + 2  # len
        buf[ 8 ] = buf[ 8 ] + 1  # count

    def patch_point(self, buf, id, v):
        h = cptp_head(buf)
        e = 0
        if h.kind == cptp_head.TYPE_ACK:
            e = 1
        count = cptp_head.HEAD_SIZE + e + h.count * 6
        buf.append(id & 0xFF)
        buf.append((id & 0xFF00) >> 8)
        buf.append(v & 0x000000FF)
        buf.append((v & 0x0000FF00) >> 8)
        buf.append((v & 0x00FF0000) >> 16)
        buf.append((v & 0xFF000000) >> 24)
        buf[ 5 ] = buf[ 5 ] + 6
        buf[ 8 ] = buf[ 8 ] + 1

    def patch_tail(self, buf):
        h = cptp_head(buf)
        count = h.len + 9
        sum = 0
        for i in buf:
            sum = sum + i
        buf.append(sum & 0xFF)
        buf.append(0x16)

    def tohex(self, buf):
        s = []
        for b in buf:
            s.append('%02X' % b)
        return s

    def dump(self, buf):
        print self.tohex(buf)

'''
if __name__ == "__main__":
    buf=[]
    p = cptp()
    p.patch_ack_header(buf, 0x128, 0x812, cptp_head.FUNC_RD, 1, 1, 0)
    p.patch_id(buf, 0x11)
    p.patch_id(buf, 0x22)
    p.patch_id(buf, 0x33)
    p.patch_tail(buf)
    p.dump(buf)

    p.patch_ack_header(buf, 0x128, 0x812, cptp_head.FUNC_WR, 1, 1, 0)
    for i in range(35):
        p.patch_point(buf, 0x11, 33)
    p.patch_tail(buf)
    p.dump(buf)
'''
