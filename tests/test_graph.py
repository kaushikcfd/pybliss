import pybliss as bliss


def test_create_graph():
    g = bliss.Graph(42)
    assert g.nvertices == 42
