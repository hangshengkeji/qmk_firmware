import numpy as np

N = 64
gamma = 0.6 

increasing = np.array([(i / (N - 1)) ** gamma for i in range(N)])
increasing = (increasing / max(increasing) * 255).astype(int)

decreasing = increasing[::-1]

full_array = np.concatenate([increasing, decreasing])

print("const uint8_t breathing_table[128] = {")
for i in range(0, 128, 16):
    line = ", ".join(f"{v:3}" for v in full_array[i:i+16])
    print("    " + line + ",")
print("};")