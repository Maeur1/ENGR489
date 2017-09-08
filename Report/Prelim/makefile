all: build

clean:
	-@rm *.bbl *.blg *.pdf *.log *.lof *.toc *.aux 2>/dev/null || true

build:
	-@pdflatex proj_report_outline.tex >/dev/null || true
	@bibtex proj_report_outline.aux
	-@pdflatex proj_report_outline.tex >/dev/null || true
	@pdflatex proj_report_outline.tex

.SILENT: clean
