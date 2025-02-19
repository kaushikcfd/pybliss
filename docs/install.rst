.. _how_to_install:

Installing pybliss
==================

From pypi:

.. code-block::

    pip install pybliss


From source:

.. code-block::

    git clone --recursive https://github.com/kaushikcfd/pybliss
    cd pybliss
    pip install nanobind scikit-build-core[pyproject]
    pip install .

For developers (for faster builds):

.. code-block::

    git clone --recursive https://github.com/kaushikcfd/pybliss
    cd pybliss
    pip install nanobind scikit-build-core[pyproject]
    pip install -ve . --no-build-isolation  # Rerun after edits to .cc files
