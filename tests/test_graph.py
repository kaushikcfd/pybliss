import numpy as np

import pybliss as bliss


def test_create_graph():
    g = bliss.Graph(42)
    assert g.nvertices == 42


def test_dimacs_conversion():
    pentagon = bliss.Graph(5)
    for i in range(5):
        pentagon.add_edge(i, (i + 1) % 5)

    with open("pentagon.dimacs", "w") as fp:
        pentagon.write_dimacs(fp)

    with open("pentagon.dimacs") as fp:
        assert bliss.Graph.from_dimacs(fp) == pentagon


def test_to_dot():
    pentagon = bliss.Graph(5)
    for i in range(5):
        pentagon.add_edge(i, (i + 1) % 5)

    dot_str = pentagon.to_dot()
    assert isinstance(dot_str, str)
    assert dot_str


def test_write_dot():
    pentagon = bliss.Graph(5)
    for i in range(5):
        pentagon.add_edge(i, (i + 1) % 5)

    with open("pentagon.dot", "w") as fp:
        pentagon.write_dot(fp)

    with open("pentagon.dot") as fp:
        assert fp.read() == pentagon.to_dot()


def test_pentagram_pentagon_canon():
    pentagon = bliss.Graph(5)
    for i in range(5):
        pentagon.add_edge(i, (i + 1) % 5)

    pentagram = bliss.Graph(5)
    for i in range(5):
        pentagram.add_edge(i, (i + 2) % 5)

    assert pentagon != pentagram
    s1 = bliss.Stats()
    s2 = bliss.Stats()
    pentagon_perm = pentagon.get_permutation_to_canonical_form(s1)
    pentagram_perm = pentagram.get_permutation_to_canonical_form(s2)
    pentagon_canon = pentagon.permute(pentagon_perm)
    pentagram_canon = pentagram.permute(pentagram_perm)
    assert pentagon_canon == pentagram_canon


def test_find_automorphisms_report():
    # Let's explore the automorphisms of the petersen graph
    petersen = bliss.Graph(10)

    for i in range(5):
        # Outer 5-cycle (0-1-2-3-4-0)
        petersen.add_edge(i, (i + 1) % 5)
        # Inner star-shaped connections (5-7-9-6-8-5)
        petersen.add_edge(i + 5, ((i + 2) % 5) + 5)
        # Connections between outer and inner nodes (0-5, 1-6, 2-7, 3-8, 4-9)
        petersen.add_edge(i, i + 5)

    n_generator = 0

    def report(n, aut):
        nonlocal n_generator
        n_generator += 1
        assert isinstance(aut, np.ndarray)
        assert isinstance(n, int)
        print(f"Found a new automorphism generator of length {n}, aut = {aut}.")

    s = bliss.Stats()
    petersen.find_automorphisms(s, report)

    # petersen graph has 3 automorphism generators
    assert n_generator == 3

    # petersen graph has |Aut| = 120
    assert s.group_size == 120


def test_find_automorphisms_report_terminate():
    # Let's explore the automorphisms of the petersen graph
    petersen = bliss.Graph(10)

    for i in range(5):
        # Outer 5-cycle (0-1-2-3-4-0)
        petersen.add_edge(i, (i + 1) % 5)
        # Inner star-shaped connections (5-7-9-6-8-5)
        petersen.add_edge(i + 5, ((i + 2) % 5) + 5)
        # Connections between outer and inner nodes (0-5, 1-6, 2-7, 3-8, 4-9)
        petersen.add_edge(i, i + 5)

    def report(n, aut):
        print(f"Found a new automorphism generator of length {n}, aut = {aut}.")

    isearch = 0

    def terminate():
        nonlocal isearch
        if isearch < 10:
            isearch += 1
            return False
        else:
            return True

    s = bliss.Stats()
    petersen.find_automorphisms(s, report, terminate)

    # terminate early
    assert isearch == 10


def test_change_color_get_color():
    k3 = bliss.Graph(3)
    for i in range(3):
        k3.add_edge(i, (i + 1) % 3)
        # set the color of each i-vertex as i.
        k3.change_color(i, i)

    for i in range(3):
        assert k3.get_color(i) == i
