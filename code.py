from adafruit_ble import BLERadio
from adafruit_ble.advertising.standard import ProvideServicesAdvertisement
from adafruit_ble.services.nordic import UARTService
from adafruit_bluefruit_connect.packet import Packet

ble = BLERadio()
uart = UARTService()
print("scanning")
found = set()
scan_responses = set()
for advertisement in ble.start_scan():
    addr = advertisement.address
    if advertisement.scan_response and addr not in scan_responses:
        scan_responses.add(addr)
    elif not advertisement.scan_response and addr not in found:
        found.add(addr)
    else:
        continue
    try:
        if 'giviv' in advertisement.complete_name:
            connection = ble.connect(advertisement)
            print("IM CONNECTED")
            while ble.connected:
                incoming_bytes = uart.in_waiting
                print('num bytes waiting: ', uart.in_waiting)
                if incoming_bytes:
                    print('theres bytes ig')
                    try:
                        bytes_in = uart.read(incoming_bytes)
                        print('i got a packet')
                        print('Received: ', bytes_in)
                    except ValueError:
                        continue
    except:
        print('failed')
    print(addr, advertisement.complete_name)
    print("\t" + repr(advertisement))
    print()

print("scan done")
