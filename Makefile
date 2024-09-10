#
# # example Makefile to build an executable named myprog from myprog.cpp
# #
SORTED=sorted_list
INTEGRAL=integral
PROG=$(INTEGRAL)

run: build
	@./bin/$(PROG)

# for i in {0..10}; \
# 	do echo Iteration $i >> output.txt; \
# 	make exec-shared-variable >> output-shared-variable.txt; \
# 	echo "\n" >> output-shared-variable.txt; \
# done \

build: $(PROG).cpp
	@g++ -std=c++17 -Wall -pthread $(PROG).cpp -o bin/$(PROG)

clean:
	$(RM) bin/$(PROG)

watch-report:report.tex
	@fswatch report.tex | xargs -n 1 -I {} make report

report:report.tex
	@pdflatex report.tex
	@pdflatex report.tex

clean-tex:
	@rm -f *.aux *.log *.toc *.blg *.out *.bbl
