# Usage

This yosys plugin is used to connect internal signals to top level I/O. Add `(* make_external = "input" *)` or `(* make_external = "output" *)` attribute to a reg/wire in the source code and run this plugin, the connections will be automatically created.

This repository can be tested with the following steps:

- Build & install yosys from source code. The package provided by Linux distributions (such as Ubuntu) may be too old to run this plugin.
- Run `make` to build the plugin.
- Run `make test` to run the test. The input file test.v is processed and the output is written to out.v.

# Reference
How to program a yosys plugin: https://yosyshq.readthedocs.io/projects/yosys/en/latest/CHAPTER_Prog.html
