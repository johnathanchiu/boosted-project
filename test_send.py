import asyncio
from bleak import BleakScanner, BleakClient


async def discover_descriptors(device_name):
    async with BleakScanner() as scanner:
        devices = await scanner.discover()

        for device in devices:
            if device.name == device_name:
                print("Device found!")
                async with BleakClient(device.address) as client:
                    desired_handle = None
                    for service in client.services:
                        if service.uuid == "afc05da0-0cd4-11e6-a148-3e1d05defe78":
                            desired_handle = service.handle

                    await client.write_gatt_char(
                        "f4c48032-0056-11e6-8d22-5e5517507c66", desired_handle
                    )


asyncio.run(discover_descriptors("BoostedRmt99BAF6E3"))
# asyncio.run(discover_descriptors("BoostedRmt99AF11C6"))
