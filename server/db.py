import sqlite3
import json
from datetime import datetime

TABLE_FIELDS = [
    "device_name",
    "id_protocol",
    "batt_level",
    "datetime",
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
    "acc_z"
]

CONFIG_FIELDS = [
    "device_name",
    "id_protocol",
    "status",
    "BMI270_sampling",
    "BMI270_acc_sens",
    "BMI270_gyro_sens",
    "BME688_sampling",
    "discontinous_time",
    "tcp_port",
    "udp_port",
    "host_ip",
    "ssid",
    "passwd",
]


def start_db():
    conn = sqlite3.connect('esp32.db')
    c = conn.cursor()

    c.execute(""" CREATE TABLE IF NOT EXISTS dataESP(
        device_name TEXT,
        id_protocol INTEGER,
        batt_level INTEGER,
        timestamp TEXT,
        temp REAL,
        press INTEGER,
        hum INTEGER,
        co INTEGER,
        rms REAL,
        amp_x REAL,
        amp_y REAL,
        amp_z REAL,
        frec_x REAL,
        frec_y REAL,
        frec_z REAL,
        acc_x TEXT,
        acc_y TEXT,
        acc_z TEXT
    )""")

    c.execute(""" CREATE TABLE IF NOT EXISTS config(
        device_name TEXT,
        id_protocol INTEGER,
        status INTEGER,
        bmi270_sampling INTEGER,
        bmi270_acc_sensibility INTEGER,
        bmi270_gyro_sensibility INTEGER,
        bme688_sampling INTEGER,
        discontinuous_time INTEGER,
        port_tcp INTEGER,
        port_udp INTEGER,
        amp_z INTEGER,
        host_ip_addr INTEGER,
        ssid INTEGER,
        pass INTEGER,
    )""")
    c.close()
    conn.close()

def delete_db():
    conn = sqlite3.connect('esp32.db')
    c = conn.cursor()

    c.execute("DELETE FROM dataESP")

    conn.commit()
    c.close()
    conn.close()


def insert_config(**kwargs):
    conn = sqlite3.connect('esp32.db')
    c = conn.cursor()

    values = {field: kwargs.get(field) for field in CONFIG_FIELDS}

    c.execute("""
        INSERT INTO config VALUES(
            :status, :id_protocol, :BMI270_sampling,
            :BMI270_acc_sens, :BMI270_gyro_sens, :BME688_sampling,
            :discontinous_time, :tcp_port, :udp_port,
            :host_ip, :ssid, :passwd,"
        )""",
        values
    )

    conn.commit()
    conn.close()
    c.close()


def insert_data(**kwargs):
    conn = sqlite3.connect('esp32.db')
    c = conn.cursor()

    values = {field: kwargs.get(field) for field in TABLE_FIELDS}
    c.execute("""
        INSERT INTO dataESP VALUES(
            :device_name,:id_protocol,:batt_level,
            :datetime,:temp,:press,
            :hum,:co,:rms,
            :amp_x,:amp_y,:amp_z,
            :frec_x,:frec_y,:frec_z,
            :acc_x,:acc_y,:acc_z
            )
        """,
        values
    )
    conn.commit()
    c.close()
    conn.close()


def extract_data(typeData):
    conn = sqlite3.connect('esp32.db')
    c = conn.cursor()

    data = []

    for row in c.execute("SELECT * FROM dataESP"):
        if(row[typeData] != None):
            data.append(row[typeData])
    
    c.close()
    conn.close()
    return data


