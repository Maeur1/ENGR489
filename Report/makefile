all: build

clean:
	-@rm *.pdf 2>/dev/null || true

build:
	-@pdflatex proj_report_outline.tex>/dev/null || true
	-@rm *.log *.lof *.toc *.aux 2>/dev/null || true

.SILENT: clean
