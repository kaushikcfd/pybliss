from __future__ import annotations

from typing import TypeVar

import numpy as np

from .pybliss_ext import (
    Digraph,
    Graph,
)

__doc__ = """
.. autofunction:: graph_from_numpy
.. autofunction:: digraph_from_numpy
.. autofunction:: graph_to_numpy
.. autofunction:: digraph_to_numpy
"""


GT = TypeVar("GT", Graph, Digraph)


def _from_numpy_checks(
    N: int,
    E: np.ndarray[tuple[int, int], np.dtype[np.integer]],
    c: np.ndarray[tuple[int], np.dtype[np.integer]],
) -> None:
    assert N >= 0
    assert E.ndim == 2, ""
    assert c.ndim == 1, "'c' must be a column vector."
    assert (
        c.shape[0] == N
    ), "'c' should have a color for every vertex in the graph."
    assert E.shape[1] == 2, "'E' must have 2 columns."
    assert np.all(
        np.logical_and(E >= 0, E < N)
    ), "'E' must use 0-based labeling of vertices."


def _graph_or_digraph_from_numpy(
    G: GT,
    E: np.ndarray[tuple[int, int], np.dtype[np.integer]],
    c: np.ndarray[tuple[int], np.dtype[np.integer]],
) -> GT:
    for i, j in E:
        G.add_edge(i, j)

    for i, color in enumerate(c):
        G.change_color(i, color)

    return G


def graph_from_numpy(
    N: int,
    E: np.ndarray[tuple[int, int], np.dtype[np.integer]],
    c: np.ndarray[tuple[int], np.dtype[np.integer]],
) -> Graph:
    r"""
    Returns a :class:`pybliss.Graph`, *G*, with *N*-vertices, *E*-edges and
    colored vector *c*.

    :arg N: The number of vertices in the graph, G.
    :arg E: A 2D :class:`numpy.ndarray` of shape
        :math:`N_{\mathrm{edges}} \times 2`, where :math:`N_{\mathrm{edges}}` is
        the number of edges in G. Each row in *E* is the of the form ``[i, j]``
        to denote that there is an edge between ``i`` and ``j`` in *G*. Note
        that :math:`i, j \in \{0, 1, \ldots, N - 1\}`.
    :arg c: An *N*-long column vector such that ``c[i]`` corresponds to the
        color of i-th vertex in G.
    """
    _from_numpy_checks(N, E, c)
    G = Graph(N)
    _graph_or_digraph_from_numpy(G, E, c)
    return G


def digraph_from_numpy(
    N: int,
    E: np.ndarray[tuple[int, int], np.dtype[np.integer]],
    c: np.ndarray[tuple[int], np.dtype[np.integer]],
) -> Digraph:
    r"""
    Returns a :class:`pybliss.Digraph`, *G*, with *N*-vertices, *E*-edges and
    colored vector *c*.

    :arg N: The number of vertices in the graph, G.
    :arg E: A 2D :class:`numpy.ndarray` of shape
        :math:`N_{\mathrm{edges}} \times 2`, where :math:`N_{\mathrm{edges}}` is
        the number of edges in G. Each row in *E* is the of the form ``[i, j]``
        to denote that there is an edge from ``i`` to ``j`` in *G*. Note that
        :math:`i, j \in \{0, 1, \ldots, N - 1\}`.
    :arg c: An *N*-long column vector such that ``c[i]`` corresponds to the
        color of i-th vertex in G.
    """
    _from_numpy_checks(N, E, c)
    G = Digraph(N)
    _graph_or_digraph_from_numpy(G, E, c)
    return G


def _parse_dimacs(
    dimacs_src: str,
) -> tuple[
    np.ndarray[tuple[int, int], np.dtype[np.integer]],
    np.ndarray[tuple[int], np.dtype[np.integer]],
]:
    # See <https://users.aalto.fi/~tjunttil/bliss/fileformat.html>
    lines = dimacs_src.split("\n")
    lines = [line for line in lines if line]
    istart = 0
    while True:
        if not lines:
            break
        if not lines[istart].startswith("c"):
            break
        assert lines[istart].startswith("c")
        istart += 1

    problem_defn_line = lines[istart]
    assert problem_defn_line.startswith("p edge ")
    N, E = [
        int(val)
        for val in problem_defn_line[len("p edge ") :].strip().split(" ")
    ]
    istart += 1

    edges = np.empty((E, 2), dtype=np.uint64)
    colors = np.zeros(N, dtype=np.uint64)

    i_edge = 0

    for i in range(istart, len(lines)):
        if lines[i].startswith("n"):
            node_i, c = [int(val) for val in lines[i][2:].strip().split(" ")]
            assert c >= 0
            # :facepalm: dimacs uses 1-based system for labeling vertices.
            colors[node_i - 1] = c
        elif lines[i].startswith("e"):
            node_i, node_j = [
                int(val) for val in lines[i][2:].strip().split(" ")
            ]
            # :facepalm: dimacs uses 1-based system for labeling vertices.
            edges[i_edge, 0] = node_i - 1
            edges[i_edge, 1] = node_j - 1
            i_edge += 1
        else:
            raise AssertionError(f"Ill-formed dimacs lines: '{lines[i]}'?")

    assert i_edge == E

    return edges, colors


def graph_to_numpy(
    G: Graph,
) -> tuple[
    np.ndarray[tuple[int, int], np.dtype[np.integer]],
    np.ndarray[tuple[int], np.dtype[np.integer]],
]:
    r"""
    Returns a tuple ``(E, c)`` for an undirected graph G, where:

    - **E** is a 2D :class:`numpy.ndarray` of shape
      :math:`N_{\mathrm{edges}} \times 2`, where :math:`N_{\mathrm{edges}}` is
      the number of edges in *G*. Each row of *E* has the form ``[i, j]``,
      indicating that there is an edge between vertex ``i`` and vertex ``j`` in
      *G*. Note that :math:`i, j \in \{0, 1, \ldots, N - 1\}`.
    - **c** is a 1D :class:`numpy.ndarray` of length :math:`N` such that
      ``c[i]`` corresponds to the color of the *i*-th vertex in *G*.
    """
    return _parse_dimacs(G.to_dimacs())


def digraph_to_numpy(
    G: Digraph,
) -> tuple[
    np.ndarray[tuple[int, int], np.dtype[np.integer]],
    np.ndarray[tuple[int], np.dtype[np.integer]],
]:
    r"""
    Returns a tuple ``(E, c)`` for an undirected graph G, where:

    - **E** is a 2D :class:`numpy.ndarray` of shape
      :math:`N_{\mathrm{edges}} \times 2`, where :math:`N_{\mathrm{edges}}` is
      the number of edges in *G*. Each row of *E* has the form ``[i, j]``,
      indicating that there is an edge from vertex ``i`` to vertex ``j`` in *G*.
      Note that :math:`i, j \in \{0, 1, \ldots, N - 1\}`.
    - **c** is a 1D :class:`numpy.ndarray` of length :math:`N` such that
      ``c[i]`` corresponds to the color of the *i*-th vertex in *G*.
    """
    return _parse_dimacs(G.to_dimacs())
