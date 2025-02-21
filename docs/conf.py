project = "pybliss"
autoclass_content = "class"

copybutton_prompt_text = (
    r">>> |\.\.\. |\$ |In \[\d*\]: | {2,5}\.\.\.: | {5,8}: ")
copybutton_prompt_is_regexp = True

html_theme = "insipid"
html_theme_options = {
}
copyright = "2025, pybliss Contributors"
author = "pybliss Contributors"

version = "2025.1"
release = "2025.1"

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.intersphinx",
    "sphinx.ext.doctest",
    "sphinx.ext.mathjax",
    "sphinx_copybutton",
    "nbsphinx",
]


intersphinx_mapping = {
    "IPython": ("https://ipython.readthedocs.io/en/stable/", None),
    "matplotlib": ("https://matplotlib.org/", None),
    "numpy": ("https://docs.scipy.org/doc/numpy/", None),
    "python": ("https://docs.python.org/3/", None),
    "pytools": ("https://documen.tician.de/pytools/", None),
}

nitpick_ignore_regex = [
    ["py:class", r"numpy.(u?)int[\d]+"],
    ["py:class", r"numpy.(_?)typing.(.+)"],
]
