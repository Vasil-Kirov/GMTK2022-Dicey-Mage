from PIL import Image
import numpy as np

import os
rootdir = os.getcwd()

output = bytearray()
for subdir, dirs, files in os.walk(rootdir):
    for file in files:
        full_path = os.path.join(subdir, file)
        if full_path.endswith(".png"):
            image  = Image.open(full_path)
            width  = image.width
            height = image.height
            image  = np.asarray(image)
            header = bytearray(b'ieh' + b'\0')
            header.extend(bytearray(os.path.splitext(file)[0] + '\0', 'ascii'))
            header.extend(image.size.to_bytes(4, 'little'))
            header.extend(width.to_bytes(4, 'little'))
            header.extend(height.to_bytes(4, 'little'))
            output.extend(header)
            output.extend(image)
            

with open("output.vdata", 'w+b') as file:
    file.write(output)