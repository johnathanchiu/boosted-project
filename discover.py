import asyncio
from bleak import BleakScanner, BleakClient


async def discover_descriptors(device_name):
    async with BleakScanner() as scanner:
        devices = await scanner.discover()

        for device in devices:
            if device.name == device_name:
                print("Device found!")
                async with BleakClient(device.address) as client:
                    for service in client.services:
                        print("Service", service)
                        for characteristic in service.characteristics:
                            print(f"{' ' * 3}characteristic:", characteristic)
                            print(f"{' ' * 6}properties:", characteristic.properties)
                            if "read" in characteristic.properties:
                                print(
                                    f"{' ' * 6}value:",
                                    await client.read_gatt_char(characteristic.uuid),
                                )
                            print(f"{' ' * 6}descriptor:")
                            for descriptor in characteristic.descriptors:
                                print(f"{' ' * 9}", descriptor)
                                try:
                                    descriptor_val = await client.read_gatt_descriptor(
                                        descriptor.handle
                                    )
                                    print(f"{' ' * 9} value:", descriptor_val)
                                except:
                                    print(f"{' ' * 9} value:", "None to read")
                                    continue

                        print("\n")

                    await client.write_gatt_char(
                        "f4c48032-0056-11e6-8d22-5e5517507c66", bytes([1])
                    )


# asyncio.run(discover_descriptors("BoostedRmt99BAF6E3"))
asyncio.run(discover_descriptors("BoostedRmt99AF11C6"))
