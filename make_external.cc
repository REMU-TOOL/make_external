#include "kernel/yosys.h"
#include "kernel/utils.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

// Convert IdString to string by stripping the leading '\\'
inline std::string id2str(IdString id)
{
    return id[0] == '\\' ? id.substr(1) : id.str();
}

struct MakeExternalWorker
{
    // Yosys::dict is yosys's optimized version of std::unordered_map
    // Yosys::pool is yosys's optimized version of std::unordered_set
    // This stores make_external port names for each module
    dict<Module*, pool<IdString>> ports_per_module;

    void process_module(Module *module)
    {
        log("Processing module %s..\n", log_id(module));

        Design *design = module->design;

        ports_per_module.insert(module);

        // Find all wires with make_external attribute
        for (Wire *wire : module->wires()) {
            // IdString must start with '\\' (or '$' for yosys generated internal wires)
            std::string attr = wire->get_string_attribute("\\make_external");
            if (attr.empty())
                continue;

            log("Processing wire %s..\n", log_id(wire));

            bool is_output;
            if (attr == "input")
                is_output = false;
            else if (attr == "output")
                is_output = true;
            else
                log_error("ERROR: The value of attribute make_external must be input or output (wire %s)\n",
                    log_id(wire));

            // Make the wire an port
            if (is_output) {
                // If wire is an input port, there is no need to make it a output port
                if (!wire->port_input)
                    wire->port_output = true;
            }
            else {
                if (wire->port_output)
                    log_error("ERROR: Cannot make output port %s as an input port\n",
                        log_id(wire));
                wire->port_input = true;
            }

            ports_per_module[module].insert(wire->name);
        }

        // Create ports for make_external wires in submodules
        for (Cell *cell : module->cells()) {
            // Check if the cell type is a module in the design
            Module *submodule = design->module(cell->type);
            if (!submodule)
                continue;

            for (IdString sub_name : ports_per_module.at(submodule)) {
                log("Processing port %s (instance %s)..\n",
                    log_id(sub_name), log_id(cell));

                // Create a port for the submodule port
                Wire *sub_wire = submodule->wire(sub_name);
                IdString name = cell->name.str() + "_" + id2str(sub_name);
                // Add a wire using sub_wire as template (port_output and port_input are copied)
                Wire *wire = module->addWire(name, sub_wire);

                // If the submodule port is already connected, connect the original signal to the port wire
                if (cell->connections().count(sub_name)) {
                    // Note that in this case, this is always an output port
                    module->connect(wire, cell->getPort(sub_name));
                }

                cell->setPort(sub_name, wire);
                ports_per_module[module].insert(name);
            }
        }

        module->fixup_ports();
    }

    void run(Design *design)
    {
        // Sort all modules according to hierarchy
        // This pass processes leaf modules first and the top module last
        TopoSort<Module*> toposort;
        for (Module *module : design->modules()) {
            toposort.node(module);
            for (Cell *cell : module->cells()) {
                // Check if the cell type is a module in the design
                Module *submodule = design->module(cell->type);
                if (submodule) {
                    toposort.edge(submodule, module);
                }
            }
        }
        toposort.sort();

        for (Module *module: toposort.sorted) {
            process_module(module);
        }
    }
};

struct MakeExternalPass : public Pass
{
    MakeExternalPass() : Pass("make_external", "connect signals to top module ports") {}

    void help() override
    {
        //   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
        log("\n");
        log("    make_external\n");
        log("\n");
        log("TODO: help message\n");
        log("\n");
    }

    void execute(vector<string> /*args*/, Design* design) override
    {
        log_header(design, "Executing MAKE_EXTERNAL pass.\n");
        log_push();

        MakeExternalWorker worker;
        worker.run(design);

        log_pop();
    }

} MakeExternalPass;

PRIVATE_NAMESPACE_END
