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

def decode_header(payload_dict, encoded_payload):
    payload_dict["id_device"] = int.from_bytes(encoded_payload[0:2], byteorder="little", signed=False)
    payload_dict["mac"] = ":".join([b.hex() for b in encoded_payload[2:8]])
    payload_dict["status"] = int.from_bytes(encoded_payload[8:9], byteorder="little", signed=False)
    payload_dict["id_protocol"] = int.from_bytes(encoded_payload[9:10], byteorder="little", signed=False)
    payload_dict["len_msg"] = int.from_bytes(encoded_payload[10:12], byteorder="little", signed=False)

def decode_data(payload_dict, encoded_payload, id_protocol):
    for field in DATA_FIELDS:
        payload_dict[field] = None
    
    start = 12
    payload_dict["val"] = int.from_bytes(encoded_payload[start:start+1], byteorder="little", signed=False)
    start += 1
    encoded_payload["batt_level"] = int.from_bytes(encoded_payload[start:start+1], byteorder="little", signed=False)
    start += 1
    payload_dict["timestamp"] = datetime.now()
    start += 4
    
    if id_protocol == 0:
        return
    
    payload_dict["temp"] = int.from_bytes(encoded_payload[start:start+1], byteorder="little", signed=False)
    start += 1 
    payload_dict["press"] = struct.unpack("f", encoded_payload[start:start+4])[0]
    start += 4 
    payload_dict["hum"] = int.from_bytes(encoded_payload[start:start+1], byteorder="little", signed=False)
    start += 1 
    payload_dict["co"] = struct.unpack("f", encoded_payload[start:start+4])[0]
    start += 4

    if id_protocol == 1:
        return
    
    if id_protocol == 2 or id_protocol == 3:
        payload_dict["rms"] = struct.unpack("f", encoded_payload[start:start+4])[0]
        start += 4

        if (id_protocol == 2):
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
    
    elif (id_protocol == 4):
        for i in range(2000):
            payload_dict["acc_x"][i] = struct.unpack("f", encoded_payload[start + i])[0]
            payload_dict["acc_y"][i] = struct.unpack("f", encoded_payload[start + 6400 + i])[0]
            payload_dict["acc_z"][i] = struct.unpack("f", encoded_payload[start + 2*6400 + i])[0]
        
    

def decode_payload(encoded_payload):
    payload_dict = {}
    decode_header(payload_dict, encoded_payload)
    decode_data(payload_dict, encoded_payload, payload_dict["id_protocol"])
    return payload_dict