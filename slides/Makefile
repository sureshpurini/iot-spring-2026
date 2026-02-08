# Makefile for ESP32 Development Environments Slides
# IoT Course - Spring 2026

MAIN = main
LATEX = pdflatex
BIBTEX = bibtex

# Default target
all: $(MAIN).pdf

# Build PDF
$(MAIN).pdf: $(MAIN).tex
	$(LATEX) $(MAIN).tex
	$(LATEX) $(MAIN).tex  # Run twice for references

# Clean auxiliary files
clean:
	rm -f $(MAIN).aux $(MAIN).log $(MAIN).nav $(MAIN).out \
	      $(MAIN).snm $(MAIN).toc $(MAIN).vrb

# Clean everything including PDF
distclean: clean
	rm -f $(MAIN).pdf

# View PDF (macOS)
view: $(MAIN).pdf
	open $(MAIN).pdf

# Continuous build (requires latexmk)
watch:
	latexmk -pdf -pvc $(MAIN).tex

.PHONY: all clean distclean view watch
