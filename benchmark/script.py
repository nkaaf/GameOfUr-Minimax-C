from pathlib import Path
from typing import List, Dict

import matplotlib.pyplot as plt
import numpy

alpha_beta = False
depth = range(2,7+1)


ab = "with" if alpha_beta else "without"

data_with_ab : Dict[int, List[float]]= {}
data_without_ab : Dict[int, List[float]]= {}
for d in depth:
    INPUT_with_ab = Path(f"Depth{d}_with_ab")
    INPUT_without_ab = Path(f"Depth{d}_without_ab")

    games = []
    averages = []

    game = []
    for line in INPUT_with_ab.read_text().split("\n"):
        if not line:
            continue

        if line.startswith("-"):
            games.append(game.copy())
            averages.append(numpy.average(game))
            game.clear()
            continue

        line = int(line.strip())
        game.append(line)
    data_with_ab[d] = [float(average) for average in averages]
    averages.clear()

    for line in INPUT_without_ab.read_text().split("\n"):
        if not line:
            continue

        if line.startswith("-"):
            games.append(game.copy())
            averages.append(numpy.average(game))
            game.clear()
            continue

        line = int(line.strip())
        game.append(line)
    data_without_ab[d] = [float(average) for average in averages]

    #plt.boxplot(games[0])
    #plt.ylabel("ms")
    #plt.title(f"Time per Move in one Game {ab} Alpha/Beta Pruning\nDepth: {d}")
    #plt.show()

    #plt.plot(range(len(games[0])), games[0], "-o")
    #plt.ylabel("ms")
    #plt.xlabel("Move")
    #plt.title(f"Time per Move in one Game {ab} Alpha/Beta Pruning\nDepth: {d}")
    #plt.show()

    #plt.boxplot(averages)
    #plt.ylabel("ms")
    #games_count = len(games)
    #plt.title(f"Average Time per Move of {games_count} Games {ab} Alpha/Beta Pruning\nDepth: {d}")
    #plt.show()


#fig, ax = plt.subplots()
#ax.set_title(f"Average Time per Move {ab} Alpha/Beta Pruning")
#ax.set_xlabel("Depth")
#ax.set_ylabel("computation time in ms")
#ax.set_yscale("log")
#ax.boxplot(data.values())
#ax.set_xticklabels(data.keys())
#fig.show()

def boxplot(data, edge_color, fill_color):
    bp = ax.boxplot(data.values(), patch_artist=True)
    for element in ["boxes", 'whiskers', 'fliers', 'caps', 'means', 'medians']:
        plt.setp(bp[element], color=edge_color)

    for patch in bp['boxes']:
        patch.set_facecolor(fill_color)

    return bp

fig, ax = plt.subplots()

ax.set_title(f"Average Time per Move")
ax.set_xlabel("Depth Limit")
ax.set_ylabel("computation time in ns")
ax.set_yscale("log")

bp1 = boxplot(data_with_ab, 'red', 'tan')
bp2 = boxplot(data_without_ab, 'blue', 'cyan')

labels = list(data_with_ab.keys())
labels.extend(data_without_ab.keys())
ax.set_xticklabels(labels)
ax.legend([bp1['boxes'][0], bp2['boxes'][0]], ["With Alpha/Beta", "Without Alpha/Beta"])
fig.show()
