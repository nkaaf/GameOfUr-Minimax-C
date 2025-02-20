from pathlib import Path
from typing import List, Dict

import matplotlib.pyplot as plt

input_with_ab = Path(f"Count_with_ab")
input_without_ab = Path(f"Count_without_ab")

depths_ab : Dict[int, List[int]] = {}
depths : Dict[int, List[int]] = {}

depth: Dict[int, int] = {}
for line in input_with_ab.read_text().split("\n"):
    if not line:
        continue

    if line.startswith("-"):
        for key in depth.keys():
            if key not in depths_ab:
                depths_ab[key] = []
            depths_ab[key].append(depth[key])
        depth.clear()
        continue

    line_ = line.strip().split(",")
    depth[int(line_[0])] = int(line_[1])

for line in input_without_ab.read_text().split("\n"):
    if not line:
        continue

    if line.startswith("-"):
        for key in depth.keys():
            if key not in depths:
                depths[key] = []
            depths[key].append(depth[key])
        depth.clear()
        continue

    line = line.strip().split(",")
    depth[int(line[0])] = int(line[1])

for key in depths:
    assert len(depths[key]) == len(depths_ab[key])

improvements_per_level = {}

for key in depths:
    improvements = []
    for index in range(len(depths[key])):
        num = depths_ab[key][index]
        denom = depths[key][index]
        improvement = num / denom
        improvements.append(improvement)
    improvements_per_level[key] = improvements

fig, ax = plt.subplots()
ax.set_title("Ratio of node count per depth with and without Alpha/Beta")
ax.set_xlabel("Depth")
ax.set_ylabel("Ratio")
ax.boxplot(improvements_per_level.values())
ax.set_xticklabels(improvements_per_level.keys())
fig.show()
