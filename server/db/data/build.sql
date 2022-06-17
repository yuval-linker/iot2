CREATE TABLE IF NOT EXISTS config(
    id_device INTEGER PRIMARY KEY,
    status INTEGER,
    id_protocol INTEGER,
    bmi270_sampling INTEGER,
    bmi270_acc_sensibility INTEGER,
    bmi270_gyro_sensibility INTEGER,
    bme688_sampling INTEGER,
    discontinuous_time INTEGER,
    port_tcp INTEGER,
    port_udp INTEGER,
    host_ip_addr TEXT,
    ssid TEXT,
    passwd TEXT
);

CREATE TABLE IF NOT EXISTS esp32_data(
    id_device INTEGER,
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
);