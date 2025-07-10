# -*- coding: gbk -*-  # ����ʵ�ʱ����޸ģ������Ļ��� gb2312��utf-8 ��


import socket
#import requests
from threading import Thread


import mysql.connector
from mysql.connector import Error
import random
from datetime import datetime

def save_environment_data(device_code, temp_value, hum_value, alarm_flag):
    try:
        #�������ݿ�
        connection = mysql.connector.connect(
            host='xxx',
            database='xxx',
            user='xxxx',
            password='xxxx',
            port=xxxx
        )
        
        if connection.is_connected():
            cursor = connection.cursor()
            
            #�������ݵ�SQL��ѯ
            sql = """
            INSERT INTO tb_temphum (device, temp, hum, isAlarm) 
            VALUES (%s, %s, %s, %s)
            """
            values = (device_code, temp_value, hum_value, alarm_flag)
            
            #ִ�в���
            cursor.execute(sql, values)
            connection.commit()
            
            print(f"�ɹ������¼: �豸={device_code}, �¶�={temp_value}, ʪ��={hum_value}")
            print(f"����ID: {cursor.lastrowid}")  #��ȡ�Զ����ɵ�ID
            
    except Error as e:
        print(f"���ݿ����: {e}")
        
    finally:
        #�ر����ݿ�����
        if connection.is_connected():
            cursor.close()
            connection.close()
            print("���ݿ������ѹر�")
            
            
            

# ��Ե�ڵ�TCP����������
HOST = 'xxxxx'  # ������������ӿ�
PORT = xxxx   # �����˿�

# ���ݴ�������������ʵ�������д
def parse_sensor_data(raw_data):
    device_id, temperature, humidity, alarm = raw_data.split(',')
    temperature = float(temperature)
    humidity = float(humidity)
    # ʾ������������쳣�������¶ȳ���100�ȣ����򷵻�None
    if temperature < -40 or temperature > 100 or humidity < 0 or humidity > 100:
        return None
    return (device_id, temperature, humidity, alarm)

def manage_client_connection(connection, client_address):
    print(f"Connected by {client_address}")
    with connection:
        data = connection.recv(1024).decode('utf-8').strip()
        print(f"Received data: {data}")
        # ���ݴ���
        try:
            processed_data = parse_sensor_data(data)
            if processed_data is None:
                print("Invalid data, discarded.")
                return
            device_id, temperature, humidity, alarm = processed_data
            # ʾ�������뵥��ģ������
            
            save_environment_data(
              device_code=device_id,
              temp_value=temperature,
              hum_value=humidity,
              alarm_flag=alarm
    )
        except Exception as e:
            print(f"Error processing data: {str(e)}")
            return


def start_edge_server():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind((HOST, PORT))
        server_socket.listen()
        print(f"Edge server listening on {HOST}:{PORT}")
        connection_counter = 1
        while True:
            client_connection, client_address = server_socket.accept()
            # ÿ���ͻ�������ʹ�ö������̴߳���
            print(connection_counter)
            connection_counter += 1
            Thread(target=manage_client_connection, args=(client_connection, client_address)).start()

if __name__ == "__main__":
    start_edge_server()