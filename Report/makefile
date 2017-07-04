all: build

clean:
	-@rm *.pdf 2>/dev/null || true

build:
	@pdflatex proj_report_outline.tex
	-@rm *.log *.lof *.toc *.aux 2>/dev/null || true

.SILENT: clean
