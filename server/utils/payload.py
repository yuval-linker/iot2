from datetime import datetime
import struct

MAX_PAYLOAD_SIZE = 19228

DATA_FIELDS = [
    "val",
    "batt_level",
    "timestamp",
    "temp",
    "press",
    "hum",
    "co",
    "rms",
    "amp_x",
    "amp_y",
    "amp_z",
    "frec_x",
    "frec_y",
    "frec_z",
    "acc_x",
    "acc_y",
    "acc_z",
]

PROTOCOL_LENGTH = [
    13,
    18,
    28,
    32,
    56,
    19228,
]

def decode_header(payload_dict, encoded_payload):
    payload_dict["id_device"] = int.from_bytes(encoded_payload[0:2], byteorder="big", signed=False)
    payload_dict["mac"] = ":".join([hex(b)[2:] for b in encoded_payload[2:8]])
    payload_dict["status"] = int.from_bytes(encoded_payload[8:9], byteorder="big", signed=False)
    payload_dict["id_protocol"] = int.from_bytes(encoded_payload[9:10], byteorder="big", signed=False)
    payload_dict["len_msg"] = int.from_bytes(encoded_payload[10:12], byteorder="big", signed=False)

def decode_data(payload_dict, encoded_payload, id_protocol):
    for field in DATA_FIELDS:
        payload_dict[field] = None
    
    start = 12
    payload_dict["val"] = int.from_bytes(encoded_payload[start:start+1], byteorder="big", signed=False)
    start += 1
    payload_dict["batt_level"] = int.from_bytes(encoded_payload[start:start+1], byteorder="big", signed=False)
    start += 1
    payload_dict["timestamp"] = datetime.now()
    start += 4
    
    if id_protocol == 1:
        return
    
    payload_dict["temp"] = int.from_bytes(encoded_payload[start:start+1], byteorder="big", signed=False)
    start += 1 
    payload_dict["press"] = struct.unpack("f", encoded_payload[start:start+4])[0]
    start += 4 
    payload_dict["hum"] = int.from_bytes(encoded_payload[start:start+1], byteorder="big", signed=False)
    start += 1 
    payload_dict["co"] = struct.unpack("f", encoded_payload[start:start+4])[0]
    start += 4

    if id_protocol == 2:
        return
    
    if id_protocol == 3 or id_protocol == 4:
        payload_dict["rms"] = struct.unpack("f", encoded_payload[start:start+4])[0]
        start += 4

        if (id_protocol == 3):
            return

        payload_dict["amp_x"] = struct.unpack("f", encoded_payload[start:start+4])[0]
        start += 4
        payload_dict["frec_x"] = struct.unpack("f", encoded_payload[start:start+4])[0]
        start += 4
        payload_dict["amp_y"] = struct.unpack("f", encoded_payload[start:start+4])[0]
        start += 4
        payload_dict["frec_y"] = struct.unpack("f", encoded_payload[start:start+4])[0]
        start += 4
        payload_dict["amp_z"] = struct.unpack("f", encoded_payload[start:start+4])[0]
        start += 4
        payload_dict["frec_z"] = struct.unpack("f", encoded_payload[start:start+4])[0]
        start += 4
    
    elif (id_protocol == 5):
        payload_dict["acc_x"] = []
        payload_dict["acc_y"] = []
        payload_dict["acc_z"] = []
        for i in range(0, 1600, 4):
            payload_dict["acc_x"].append(struct.unpack("f", encoded_payload[start + i: start + i + 4])[0])
            payload_dict["acc_y"].append(struct.unpack("f", encoded_payload[start + 6400 + i: start + 6400 + i + 4])[0])
            payload_dict["acc_z"].append(struct.unpack("f", encoded_payload[start + 2*6400 + i: start + 2*6400 + i + 4])[0])
        
    

def decode_payload(encoded_payload):
    payload_dict = {}
    decode_header(payload_dict, encoded_payload)
    decode_data(payload_dict, encoded_payload, payload_dict["id_protocol"])
    return payload_dict
