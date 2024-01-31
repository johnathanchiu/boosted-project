import asyncio
from bleak import BleakScanner

import hashlib


def uuid_to_mac_address(uuid):
    # Use a hash function to generate a pseudo MAC address
    # This is a simple example and may not provide a real-world mapping
    hash_object = hashlib.md5(uuid.encode())
    hashed_value = hash_object.hexdigest()
    mac_address = ":".join([hashed_value[i : i + 2] for i in range(0, 12, 2)])
    return mac_address


async def scan_devices(device_name):
    async with BleakScanner() as scanner:
        devices = await scanner.discover()

        for device in devices:
            if device.name == device_name:
                # Convert UUID to MAC address
                result_mac_address = uuid_to_mac_address(device.address)
                print(f"UUID: {device.address}")
                print(f"MAC Address: {result_mac_address}")


asyncio.run(scan_devices("BoostedRmt99AF11C6"))
