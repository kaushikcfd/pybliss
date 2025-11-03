# Configuration file for Sphinx,
# see https://www.sphinx-doc.org/en/master/usage/configuration.html

import os

# -- Recommended settings -----------------------------------------------------

html_theme = "insipid"

html_permalinks_icon = "#"

# If False, source links to Github/GitLab are shown (using html_context)
html_copy_source = False

# Update this with your own repo information:
html_context = {
    "display_github": True,
    "github_user": "kaushikcfd",
    "github_repo": "pybliss",
    "conf_py_path": "/docs/",
    "READTHEDOCS": os.environ.get("READTHEDOCS", False),
    # The 'commit' field is overwritten below (with data from Git).
    # If you prefer, you can also get it from your build environment,
    # e.g. READTHEDOCS_GIT_COMMIT_HASH.
    "commit": "main",
}

# -- Settings for source code -------------------------------------------------

# Language used for syntax highlighting (default: 'python')
# highlight_language = 'none'

# Style of syntax highlighting
# pygments_style = 'monokai'

# -- Language settings --------------------------------------------------------

# language = 'es'

# -- Theme configuration ------------------------------------------------------

html_theme_options = {
    # 'body_centered': False,
    # 'body_max_width': None,
    # 'breadcrumbs': True,
    # 'enable_search_shortcuts': False,
    # 'globaltoc_collapse': False,
    # 'globaltoc_includehidden': True,
    # 'initial_sidebar_visibility_threshold': None,
    # 'left_buttons': [
    # ],
    # 'navigation_with_keys': False,
    # 'nosidebar': True,
    # 'right_buttons': [
    #    'search-button.html',
    # ],
    # 'rightsidebar': True,
    # 'show_insipid': False,
    # 'sidebar_overlay_width': None,
    # 'sidebar_transition': '1s ease-out',
    # 'sidebarwidth': '10rem',
    # 'strip_section_numbers': False,
    # 'topbar_transition': '1.5s ease-out',
}

html_sidebars = {
   "**": [
       "home.html",
       "globaltoc.html",
       "separator.html",
       "indices.html",
   ],
   "showcase/different-sidebar": [
       "localtoc.html",
       "searchbox.html",
   ],
   "showcase/no-sidebar": [],
}

# -- Project information ------------------------------------------------------

project = "pybliss"
html_title = "PyBliss"
html_short_title = "pybliss"
version = "2025.5"


# -- Page footer --------------------------------------------------------------

html_show_copyright = False
# html_show_sphinx = False
# html_show_sourcelink = False

# Only relevant when html_copy_source is True
# html_sourcelink_suffix = ''

# -- Miscellaneous settings ---------------------------------------------------

# Numbered figures, tables and code-blocks
numfig = True

html_secnumber_suffix = "\N{FIGURE SPACE}"

# html_compact_lists = False

# smartquotes = False

# Generate alphabetic index
# html_use_index = False

# Separate page per starting letter
# html_split_index = True

# Generate domain indices, e.g. Python module index
# html_domain_indices = False

# The default in Sphinx 4+ is 'inline'
# html_codeblock_linenos_style = 'table'

# -- Sphinx extensions --------------------------------------------------------

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.intersphinx",
    "sphinx.ext.viewcode",
    "sphinx.ext.doctest",
    "sphinx.ext.mathjax",
    "sphinx_copybutton",
    "nbsphinx",
]

intersphinx_mapping = {
    "IPython": ("https://ipython.readthedocs.io/en/stable/", None),
    "matplotlib": ("https://matplotlib.org/", None),
    "numpy": ("https://numpy.org/doc/stable/", None),
    "python": ("https://docs.python.org/3/", None),
    "pytools": ("https://documen.tician.de/pytools/", None),
}
