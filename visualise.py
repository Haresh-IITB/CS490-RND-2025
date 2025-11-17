import os
import glob
import argparse
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.animation as animation

COLOR_MAP = {
    0: 'saddlebrown',  # village
    1: 'orange',       # tier3
    2: 'blue',         # tier2
    3: 'red'           # tier1
}

def read_frame(node_file, edge_file):
    nodes = pd.read_csv(node_file)
    edges = pd.read_csv(edge_file)
    return nodes, edges


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('snapdir', nargs='?', default='snapshots')
    parser.add_argument('--interval', type=int, default=200, help='ms between frames')
    parser.add_argument('--show-edges', type=bool, default=True)
    args = parser.parse_args()

    node_files = sorted(glob.glob(os.path.join(args.snapdir, 'snapshot_*_nodes.csv')))
    edge_files = sorted(glob.glob(os.path.join(args.snapdir, 'snapshot_*_edges.csv')))
    assert len(node_files) == len(edge_files), 'mismatched node/edge snapshots'

    frames = len(node_files)
    print(f'Found {frames} frames in {args.snapdir}')

    fig, ax = plt.subplots(figsize=(8,8))
    scat = None
    lines = []

    def update(i):
        nonlocal scat, lines
        ax.clear()
        nodes, edges = read_frame(node_files[i], edge_files[i])
        colors = [COLOR_MAP.get(int(t), 'gray') for t in nodes['tier']]
        sizes = [20 if int(t)==0 else (40 if int(t)==1 else (60 if int(t)==2 else 80)) for t in nodes['tier']]
        ax.scatter(nodes['x'], nodes['y'], c=colors, s=sizes, alpha=0.8)
        if args.show_edges:
            for _, row in edges.iterrows():
                u = int(row['u']); v = int(row['v'])
                x1 = nodes.loc[nodes['id'] == u, 'x'].values[0]
                y1 = nodes.loc[nodes['id'] == u, 'y'].values[0]
                x2 = nodes.loc[nodes['id'] == v, 'x'].values[0]
                y2 = nodes.loc[nodes['id'] == v, 'y'].values[0]
                ax.plot([x1,x2], [y1,y2], linewidth=0.5, alpha=0.4)
        ax.set_title(f'Frame {i}')
        ax.set_xlim(0,1)
        ax.set_ylim(0,1)
        ax.set_xticks([])
        ax.set_yticks([])

    ani = animation.FuncAnimation(fig, update, frames=frames, interval=args.interval)
    plt.show()

if __name__ == '__main__':
    main()