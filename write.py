a = """4D494E000000000000000000007A1200000000000000000000000000000000000000000000000000"""

with open("speedtest.bin", "wb") as t:
    t.write(bytes.fromhex(a))
    for i in range(1, 1000000):
        p = [0,0,0,0,0,0,0,0]
        t.write(bytes(p))
    p = [0,0,0,0,0,0,0,1]
    t.write(bytes(p))