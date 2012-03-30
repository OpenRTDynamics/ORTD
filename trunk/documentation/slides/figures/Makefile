
$XFIGPICS = $(shell ls *lfig)

allpictures:
	for pic in *.lfig  ; do \
	  echo $$pic ; \
	  fig2mpdf -H header.tex $$pic ; \
	done
