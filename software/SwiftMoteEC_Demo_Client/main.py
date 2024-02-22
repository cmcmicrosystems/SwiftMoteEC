
"""this version can smoothly receive sensor data with continuous scan, run once and stop.
 the program won't quit, it will wait for next command after a stop command is issued.
known issue: the first generated data file contains duplicated first data packet, might be a firmware issue or a python program issue"""
import asyncio
import csv
import math
import datetime
import struct
import keyboard
import bleak
from bleak import BleakClient, BleakScanner, BleakError

SENSOR_SERVICE_UUID = "350a1b80-cf4b-11e1-ac36-0002a5d5c51b"
SENSOR_DATA_UUID = "340a1b80-cf4b-11e1-ac36-0002a5d5c51b"
RUN_ONCE_UUID = "330a1b80-cf4b-11e1-ac36-0002a5d5c51b"
CONTINUOUS_RUN_UUID = "320a1b80-cf4b-11e1-ac36-0002a5d5c51b"
STOP_CMD_UUID = "310a1b80-cf4b-11e1-ac36-0002a5d5c51b"

filename1 = None
filename2 = None
total_received = 0
volt_datapoints = 0

async def connect():
    start_time = asyncio.get_event_loop().time()
    while True:
        print(f"Scanning for BlueNRG")
        devices = await BleakScanner.discover()
        for device in devices:
            if device.name == "BlueNRG":
                print("Device found")
                return device


def generate_filenames():
    global filename1  # Declare that we are using the global file_name variable
    global filename2

    f1 = "current"
    f2 = "raw"
    file_extension = "csv"

    # Get the current timestamp
    timestamp = datetime.datetime.now().strftime("%Y%m%d%H%M%S")

    # Dynamically generate a file name using the username, timestamp, and file extension
    filename1 = f"{f1}_data_{timestamp}.{file_extension}"
    filename2 = f"{f2}_data_{timestamp}.{file_extension}"
    print("filename1:", filename1)
    print("filename2:", filename2)


def set_flags(key, run_once_flag, cont_run_flag, stop_cmd_flag):
    if key.name == 'r' and key.event_type == keyboard.KEY_DOWN:
        run_once_flag[0] = True
        cont_run_flag[0] = False
        stop_cmd_flag[0] = False
        print("run_once flag is set.")

    elif key.name == 'c' and key.event_type == keyboard.KEY_DOWN:
        run_once_flag[0] = False
        cont_run_flag[0] = True
        stop_cmd_flag[0] = False
        print("continuous run flag is set.")

    elif key.name == 's' and key.event_type == keyboard.KEY_DOWN:
        run_once_flag[0] = False
        cont_run_flag[0] = False
        stop_cmd_flag[0] = True
        print("stop cont_run command flag is set.")


async def notification_handler2(sender, data):
    data_length = len(data)
    print("Number of data is:", data_length)
    number_of_datapoints = math.floor(data_length / 2)
    print(number_of_datapoints)
    data_tuple = struct.unpack('<' + 'H' * number_of_datapoints, data)
    print(data_tuple)
    # await asyncio.sleep(0.5)


async def notification_handler1(sender, data):
    global total_received
    global volt_datapoints
    """the generated sensor data is in ADC code format, need to be processed using
     the formula provided in the example code, the transformed_result is current in uA."""

    """need to check the formula used to calculate current, 
    especially it includes pga gain and rtia that are provided from user input via GUI """

    ADCRefVolt = 1.820
    kFactor = 1.835 / 1.82
    """the value of RtiaValue_Magnitude is obtained in AppSWVRtiaCal function"""
    RtiaValue_Magnitude = 121.972061
    AdcPgaGain = 1.5
    data_length = len(data)
    #print(data_length)
    number_of_datapoints = math.floor(data_length / 2)
    #print(number_of_datapoints)

    total_received = total_received + number_of_datapoints
    data_tuple = struct.unpack('<' + 'H' * number_of_datapoints, data)

    first_element = struct.unpack('B', data[0:1])[0]
    second_element = struct.unpack('BB', data[0:2])[1]

    if (first_element == 42) and (second_element == 42):
        volt_datapoints = data_tuple[1]
        generate_filenames()
    print(data_tuple)
    if volt_datapoints != 0:
        with open(filename2, 'a', newline='') as file2:
            writer = csv.writer(file2)
            for value in data_tuple:
                writer.writerow([value])
        transformed_results = []
        for value in data_tuple:
            transformed_result = ((float((int(value) - 32768))/AdcPgaGain) * ADCRefVolt/32768 * kFactor)/RtiaValue_Magnitude * 1e3
            # transformed_result = (value / 2) + 2
            transformed_results.append(transformed_result)
        with open(filename1, 'a', newline='') as file1:
            writer = csv.writer(file1)
            for element in transformed_results:
                writer.writerow([element])
        await asyncio.sleep(0.5)

    if total_received >= volt_datapoints:
        total_received = 0
        volt_datapoints = 0
    await asyncio.sleep(0.5)


async def main():
    run_once_flag = [False]
    cont_run_flag = [False]
    stop_cmd_flag = [False]

    keyboard.hook(lambda x: set_flags(x, run_once_flag, cont_run_flag, stop_cmd_flag))

    print("Connecting to the device...")

    """order of the parameter values in the list: E1, E2, Ep, Gain, Rtia, Freq, Amplitude, all in 16-bit
                         signed integer format, each signed integer is represented with 3-byte: first byte indicates the 
                         negative (represented as 1) or positive (represented as 0) sign, the rest two-byte is the data value
                          in little-endian,three commands: Run, Continuous Run, Stop"""
    """update on Jan 26"""
    #values_to_write = [100, 400, 2, 25, 50]
    values_to_write = [100, 400, 2, 1, 1, 25, 50]
    parameter_value = bytearray()
    # write_value = bytearray([0x05])
    for value in values_to_write:
        parameter_value.extend((value & 0xFF, (value >> 8) & 0xFF))

    while True:
        try:
            d = await connect()
            if not d:
                print("Device not found")
            else:
                print("BlueNRGA Address received!")
                print(f"device address:{d}")
            async with BleakClient(d) as client:
                # await asyncio.sleep(1)
                # Connect to the GATT server
                await client.start_notify(SENSOR_DATA_UUID, notification_handler1, keyword='force_indicate')
                if client.is_connected:
                    print("input command:input 'r' for run-once, 'c' for continuous run, 's' for stop ")
                while client.is_connected:
                    #print("connected")
                    #await client.start_notify(SENSOR_DATA_UUID, notification_handler1, keyword='force_indicate')
                    if run_once_flag[0]:
                        await client.write_gatt_char(RUN_ONCE_UUID, parameter_value, response=True)
                        run_once_flag[0] = False
                    #if cont_run_flag[0]:
                    if cont_run_flag[0]:
                        await client.write_gatt_char(CONTINUOUS_RUN_UUID, parameter_value, response=True)
                        cont_run_flag[0] = False
                    if stop_cmd_flag[0]:
                        await client.write_gatt_char(STOP_CMD_UUID, b'1', response=True)
                        stop_cmd_flag[0] = False
                    await asyncio.sleep(1)
                print("Connection lost. Reconnecting...")

        except Exception as e:
            print("Error occurred while connecting to GATT server:", e)
            await asyncio.sleep(2)


if __name__ == "__main__":
    asyncio.run(main())


