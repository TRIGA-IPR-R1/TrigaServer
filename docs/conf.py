# Configuration file for the Sphinx documentation builder.
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import sys
from datetime import datetime

sys.path.insert(0, os.path.abspath(".."))

# -- Project information -----------------------------------------------------

project = "TrigaServer"
author = "Thalles Campagnani"
copyright = f"2023–{datetime.now().year}, {author}"
release = "0.0.2"
version = "0.0.2"

# -- General configuration ---------------------------------------------------

extensions = [
    "sphinx.ext.todo",
]

exclude_patterns = ["_build", "Thumbs.db", ".DS_Store", ".venv"]

language = "pt_BR"
root_doc = "index"
source_suffix = {".rst": "restructuredtext"}

todo_include_todos = True

# -- Options for HTML output -------------------------------------------------

html_theme = "sphinx_rtd_theme"
html_title = "TrigaServer"
html_short_title = "TrigaServer"
html_show_sourcelink = True
html_show_sphinx = True
html_theme_options = {
    "collapse_navigation": False,
    "navigation_depth": 3,
    "style_external_links": True,
}

# -- Options for LaTeX / PDF output ------------------------------------------

latex_engine = "pdflatex"
latex_elements = {
    "papersize": "a4paper",
    "pointsize": "11pt",
    "figure_align": "htbp",
    "preamble": r"""
\usepackage{indentfirst}
\setlength{\parindent}{1.25em}
""",
}
latex_documents = [
    (
        root_doc,
        "TrigaServer.tex",
        "TrigaServer — Documentação",
        author,
        "manual",
    ),
]
latex_show_urls = "footnote"

# -- Options for manual page output ------------------------------------------

man_pages = [
    (
        root_doc,
        "trigaserver",
        "servidor de dados em tempo real do reator TRIGA IPR-R1",
        [author],
        1,
    ),
]

# -- Options for EPUB output -------------------------------------------------

epub_title = project
epub_author = author
epub_publisher = author
epub_copyright = copyright
epub_exclude_files = ["search.html"]

# -- Options for text output -------------------------------------------------

text_sectionchars = '*=-~"+`'
