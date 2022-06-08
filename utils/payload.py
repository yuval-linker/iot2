from base64 import encode


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
    payload_dict["id_device"] = encoded_payload[0:2]
    payload_dict["mac"] = encoded_payload[2:8]
    payload_dict["status"] = encoded_payload[8:9]
    payload_dict["id_protocol"] = encoded_payload[9:10]
    payload_dict["len_msg"] = encoded_payload[10:12]

def decode_data(payload_dict, encoded_payload, id_protocol):
    for field in DATA_FIELDS:
        payload_dict[field] = None
    
    start = 12
    payload_dict["val"] = encoded_payload[start:start+1]
    start += 1
    encoded_payload["batt_level"] = encoded_payload[start:start+1]
    start += 1
    payload_dict["timestamp"] = encoded_payload[start:start+4]
    start += 4
    
    if id_protocol == 0:
        return
    
    payload_dict["temp"] = encoded_payload[start:start+1]
    start += 1 
    payload_dict["press"] = encoded_payload[start:start+4]
    start += 4 
    payload_dict["hum"] = encoded_payload[start:start+1]
    start += 1 
    payload_dict["co"] = encoded_payload[start:start+4]
    start += 4

    if id_protocol == 1:
        return
    
    if id_protocol == 2 or id_protocol == 3:
        payload_dict["rms"] = encoded_payload[start:start+4]
        start += 4

        if (id_protocol == 2):
            return

        payload_dict["amp_x"] = encoded_payload[start:start+4]
        start += 4
        payload_dict["frec_x"] = encoded_payload[start:start+4]
        start += 4
        payload_dict["amp_y"] = encoded_payload[start:start+4]
        start += 4
        payload_dict["frec_y"] = encoded_payload[start:start+4]
        start += 4
        payload_dict["amp_z"] = encoded_payload[start:start+4]
        start += 4
        payload_dict["frec_z"] = encoded_payload[start:start+4]
        start += 4
    
    elif (id_protocol == 4):
        for i in range(2000):
            payload_dict["acc_x"][i] = encoded_payload[start + i]
            payload_dict["acc_y"][i] = encoded_payload[start + 6400 + i]
            payload_dict["acc_z"][i] = encoded_payload[start + 2*6400 + i]
        
    

def decode_payload(id_protocol, encoded_payload):
    payload_dict = {}
    decode_header(payload_dict, encoded_payload)
    decode_data(payload_dict, encoded_payload, id_protocol)
    return payload_dict