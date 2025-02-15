## Python bindings for the BLISS toolkit.

### Install

`pip install .`

### Develop

Developers of `pybliss` can reduce build times by installing with:
```console
$ pip install nanobind scikit-build-core[pyproject]
$ pip install --no-build-isolation -ve .
```

Upon any changes to the C++ files, the command: `pip install
--no-build-isolation -ve .` must be re-run.

### LICENSE

- Certain components of this software is licensed under GPLv3.
- Specific libraries are under LGPLv3.
