include Config.mk

MaxFlowSolver:

watch:
	while inotifywait --event modify *.cc; do clear && make; done

clean:
	$(RM) *.o MaxFlowSolver

.PHONY: all watch clean
