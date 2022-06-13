import json

from os.path import isfile
from sqlite3 import connect

# Database related paths 
DB_PATH = "./db/data/iot.db"
BUILD_PATH = "./db/data/build.sql"
FIELDS_PATH = "./db/data/fields.json"

# Tables fields
parsed_json = json.load(open(FIELDS_PATH))
CONFIG_FIELDS = parsed_json["config_fields"]
ESP32_DATA_FIELDS = parsed_json["esp32_data_fields"]

conn = connect(DB_PATH)
cursor = conn.cursor()

def commit():
    conn.commit()

def close():
    conn.close()

def with_commit(fun):
    def inner(*args, **kwargs):
        ret = fun(*args, **kwargs)
        commit()
        return ret
    return inner

@with_commit
def build():
    if isfile(BUILD_PATH):
        execute_script(BUILD_PATH)

def execute(command, values):
    cursor.execute(command, dict(values))

def execute_script(path):
    with open(path, 'r', encoding="utf-8") as script:
        cursor.executescript(script.read())

def fetch_all(command, values):
    execute(command, dict(values))
    return cursor.fetchall()

def fetch_one(command, values):
    execute(command, dict(values))
    return cursor.fetchone()

@with_commit
def insert_esp32_data(**kwargs):
    values = {field: kwargs.get(field) for field in ESP32_DATA_FIELDS}
    execute("""
        INSERT INTO esp32_data VALUES(
            :id_device,:id_protocol,:batt_level,
            :datetime,:temp,:press,
            :hum,:co,:rms,
            :amp_x,:amp_y,:amp_z,
            :frec_x,:frec_y,:frec_z,
            :acc_x,:acc_y,:acc_z
            )
        """,
        values
    )

def get_esp32_data(id_device):
    return fetch_all(f"""
    SELECT * FROM esp32_data WHERE id_device=:id_device
    """,
    {"id_device": id_device}
    )

@with_commit
def set_device_config(**kwargs):
    values = {field: kwargs.get(field) for field in CONFIG_FIELDS}
    execute("""
        INSERT OR REPLACE INTO config (
            id_device,
            id_protocol,
            status,
            bmi270_sampling,
            bmi270_acc_sensibility,
            bmi270_gyro_sensibility,
            bme688_sampling,
            discontinuous_time,
            port_tcp,
            port_udp,
            host_ip_addr,
            ssid,
            passwd
        )
        VALUES (
            :id_device,
            :id_protocol,
            :status,
            :bmi270_sampling,
            :bmi270_acc_sensibility,
            :bmi270_gyro_sensibility,
            :bme688_sampling,
            :discontinuous_time,
            :port_tcp,
            :port_udp,
            :host_ip_addr,
            :ssid,
            :passwd
        )
    """,
    values
    )

def get_device_config(id_device):
    return fetch_one(f"""
    SELECT * 
    FROM config
    WHERE id_device=:id_device
    """, 
    {"id_device": id_device}
    )

def get_device_status(id_device):
    return fetch_one(f"""
    SELECT status 
    FROM config
    WHERE id_device=:id_device
    """, 
    {"id_device": id_device}
    )[0]
