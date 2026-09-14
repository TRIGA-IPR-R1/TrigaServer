# Makefile minimo para a documentacao Sphinx do TrigaServer.
#
# Uso (a partir desta pasta):
#   python3 -m venv .venv && .venv/bin/pip install -r requirements.txt
#   make html      gera HTML em _build/html/
#   make latexpdf  gera PDF via LaTeX em _build/latex/TrigaServer.pdf
#   make epub      gera EPUB em _build/epub/
#   make man       gera pagina de manual em _build/man/
#   make text      gera texto puro em _build/text/
#   make clean     remove _build/
#
# PDF exige uma distribuicao LaTeX (ex.: texlive-latex-recommended,
# texlive-lang-portuguese, texlive-fonts-recommended).

SPHINXOPTS    ?=
ifeq ($(wildcard .venv/bin/sphinx-build),)
SPHINXBUILD   ?= sphinx-build
else
SPHINXBUILD   ?= .venv/bin/sphinx-build
endif
SOURCEDIR     = .
BUILDDIR      = _build

.PHONY: help Makefile html latexpdf pdf epub man text dirhtml singlehtml \
        pickle json htmlhelp qthelp applehelp devhelp xml dummy linkcheck \
        doctest coverage clean livehtml

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

html latexpdf epub man text dirhtml singlehtml pickle json htmlhelp \
qthelp applehelp devhelp xml dummy linkcheck doctest coverage:
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

clean:
	rm -rf "$(BUILDDIR)"

# Recompilacao automatica ao salvar (requer sphinx-autobuild)
livehtml:
	$(if $(wildcard .venv/bin/sphinx-autobuild),.venv/bin/sphinx-autobuild,sphinx-autobuild) \
		"$(SOURCEDIR)" "$(BUILDDIR)/html" $(SPHINXOPTS) $(O)

pdf: latexpdf

# Catch-all: encaminha alvos desconhecidos ao Sphinx (-M)
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
