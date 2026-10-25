# FTC (FreeTiecodec) 语言与编译器规范

project = "FTC (FreeTiecodec) 语言与编译器规范"
copyright = "2026, FreeTiecodec contributors"
author = "FreeTiecodec contributors"
release = "0.1.0"
version = "0.1"

language = "zh_CN"
root_doc = "index"

source_suffix = {".rst": "restructuredtext"}
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

extensions = []

html_theme = "sphinx_rtd_theme"
html_theme_options = {
    "collapse_navigation": False,
    "sticky_navigation": True,
    "navigation_depth": 4,
    "includehidden": True,
    "titles_only": False,
    "prev_next_buttons_location": "bottom",
    "style_external_links": False,
}
html_title = project
html_show_sphinx = False
html_show_sourcelink = True
html_static_path = ["_static"]
html_css_files = ["custom.css"]
html_search_language = "zh"
html_last_updated_fmt = "%Y-%m-%d"

highlight_language = "text"
default_role = "code"
pygments_style = "sphinx"
numfig = True
numfig_format = {
    "figure": "图 %s",
    "table": "表 %s",
    "code-block": "代码 %s",
    "section": "节 %s",
}

rst_prolog = """
.. |Tiecode| replace:: 结绳
.. |Java| replace:: Java
"""
