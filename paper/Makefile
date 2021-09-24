OUTDIR = out
EPS_FILES = $(wildcard img/*.eps)
SVG_FILES = $(wildcard img/*.svg)
TIFF_FILES = $(wildcard img/*.tiff)
IMAGE_FILES = $(EPS_FILES:.eps=.pdf) $(SVG_FILES:.svg=.pdf) $(TIFF_FILES:.tiff=.pdf)
PASS = pdflatex --output-directory=$(OUTDIR) main.tex

all: $(IMAGE_FILES) outdir threepass

outdir:
	@if [ -d $(OUTDIR) ]; then echo Directory $(OUTDIR) exists; else mkdir $(OUTDIR); fi

onepass: 
	$(PASS)

twopass:
	$(PASS)
	$(PASS)

threepass:
	$(PASS)
	$(PASS)
	$(PASS)

%.pdf: %.eps
	@echo Converting EPS image $< to PDF
	ps2pdf -dEPSCrop $< $@
#	epstopdf --outfile=$@ $<

%.pdf: %.svg
	@echo Converting SVG image $< to PDF
	inkscape -E $<.eps $< && ps2pdf -dEPSCrop $<.eps $@
	rm $<.eps

%.pdf: %.tiff
	@echo Converting TIFF image $< to PDF
	tiff2pdf -o $@ $<

view:
	evince $(OUTDIR)/main.pdf

xview:
	xpdf $(OUTDIR)/main.pdf

aview:
	acroread $(OUTDIR)/main.pdf

kview:
	kpdf $(OUTDIR)/main.pdf

todo:
	cat $(OUTDIR)/main.TODO

clean:
	rm -f $(OUTDIR)/*
	rm -f $(IMAGE_FILES)

rebuild: clean all
