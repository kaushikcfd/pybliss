import numpy as np

import pybliss as bliss


def test_graph_from_numpy_0():
    pentagon_0 = bliss.Graph(5)
    for i in range(5):
        pentagon_0.add_edge(i, (i + 1) % 5)

    for i in range(5):
        pentagon_0.change_color(i, i**2)

    pentagon_adj = np.array(
        [
            [0, 1, 0, 0, 1],
            [1, 0, 1, 0, 0],
            [0, 1, 0, 1, 0],
            [0, 0, 1, 0, 1],
            [1, 0, 0, 1, 0],
        ]
    )

    pentagon_1 = bliss.graph_from_numpy(
        5, np.argwhere(pentagon_adj), np.arange(5) ** 2
    )

    assert pentagon_0 == pentagon_1


def test_digraph_from_numpy_0():
    pentagon_0 = bliss.Digraph(5)
    for i in range(5):
        pentagon_0.add_edge(i, (i + 1) % 5)

    for i in range(5):
        pentagon_0.change_color(i, i**2)

    pentagon_adj = np.array(
        [
            [0, 1, 0, 0, 0],
            [0, 0, 1, 0, 0],
            [0, 0, 0, 1, 0],
            [0, 0, 0, 0, 1],
            [1, 0, 0, 0, 0],
        ]
    )

    pentagon_1 = bliss.digraph_from_numpy(
        5, np.argwhere(pentagon_adj), np.arange(5) ** 2
    )

    assert pentagon_0 == pentagon_1


def test_graph_to_numpy_0():
    pentagon_0 = bliss.Graph(5)
    for i in range(5):
        pentagon_0.add_edge(i, (i + 1) % 5)

    for i in range(5):
        pentagon_0.change_color(i, i**2)
    pentagon_1 = bliss.graph_from_numpy(5, *bliss.graph_to_numpy(pentagon_0))

    assert pentagon_0 == pentagon_1


def test_digraph_to_numpy_0():
    pentagon_0 = bliss.Digraph(5)
    for i in range(5):
        pentagon_0.add_edge(i, (i + 1) % 5)

    for i in range(5):
        pentagon_0.change_color(i, i**2)

    pentagon_1 = bliss.digraph_from_numpy(5, *bliss.graph_to_numpy(pentagon_0))

    assert pentagon_0 == pentagon_1
