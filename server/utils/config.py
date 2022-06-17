def encode_config(**kwargs):
    config_payload = b""
    for arg in kwargs:
        if arg in ["host_ip_addr", "ssid", "passwd", "id_device"]:
            continue
        
        length = 4
        if arg in ["id_protocol", "status"]:
            length = 1

        config_payload += int.to_bytes(kwargs[arg], byteorder="little", length=length)

    config_payload += from_ip_addr_to_bytes(kwargs["host_ip_addr"])
    config_payload += from_pystr_to_cstr(kwargs["ssid"])
    config_payload += from_pystr_to_cstr(kwargs["passwd"])

    assert len(config_payload) == 98
    return config_payload

def encode_status(status):
    return int.to_bytes(status, byteorder="little", length=1)


def from_ip_addr_to_bytes(ip_addr):
    ip_bytes = map(lambda n: int.to_bytes(int(n), byteorder="big", length=1), ip_addr.split("."))
    acc = b""
    for b in ip_bytes:
        acc += b
    return acc

def from_pystr_to_cstr(pystr):
    n = 32 - len(pystr)
    if n <= 0:
        s = pystr[0:31].encode() + b"\x00"
    else:
        s = pystr.encode() + n * b"\x00"
    assert(len(s) == 32)
    return s