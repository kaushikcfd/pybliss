import pybliss as bliss


def test_create_graph():
    g = bliss.Digraph(42)
    assert g.nvertices == 42


def test_dimacs_conversion():
    pentagon = bliss.Digraph(5)
    for i in range(5):
        pentagon.add_edge(i, (i + 1) % 5)

    with open("pentagon.dimacs", "w") as fp:
        pentagon.write_dimacs(fp)

    with open("pentagon.dimacs") as fp:
        assert bliss.Digraph.from_dimacs(fp) == pentagon


def test_to_dot():
    pentagon = bliss.Digraph(5)
    for i in range(5):
        pentagon.add_edge(i, (i + 1) % 5)

    dot_str = pentagon.to_dot()
    assert isinstance(dot_str, str)
    assert dot_str


def test_write_dot():
    pentagon = bliss.Digraph(5)
    for i in range(5):
        pentagon.add_edge(i, (i + 1) % 5)

    with open("pentagon.dot", "w") as fp:
        pentagon.write_dot(fp)

    with open("pentagon.dot") as fp:
        assert fp.read() == pentagon.to_dot()


def test_change_color_get_color():
    k3 = bliss.Digraph(3)
    for i in range(3):
        k3.add_edge(i, (i + 1) % 3)
        # set the color of each i-vertex as i.
        k3.change_color(i, i)

    for i in range(3):
        assert k3.get_color(i) == i

# TODO: Add tests for Digraph.get_caonnical_from, Digraph.find_automorphisms.
