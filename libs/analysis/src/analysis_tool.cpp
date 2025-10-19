#include "analysis_tool.h"
#include <stdexcept>

namespace wip {
namespace analysis {

// ==================== AnalysisToolRegistry Implementation ====================

AnalysisToolRegistry& AnalysisToolRegistry::instance() {
    static AnalysisToolRegistry instance;
    return instance;
}

void AnalysisToolRegistry::register_tool(const std::string& name, AnalysisToolFactory factory) {
    if (name.empty()) {
        throw std::invalid_argument("Tool name cannot be empty");
    }
    
    if (!factory) {
        throw std::invalid_argument("Tool factory cannot be null");
    }
    
    factories_[name] = std::move(factory);
}

std::unique_ptr<AnalysisTool> AnalysisToolRegistry::create_tool(const std::string& name) const {
    auto it = factories_.find(name);
    if (it == factories_.end()) {
        return nullptr;
    }
    
    try {
        return it->second();
    } catch (const std::exception&) {
        // Failed to create tool instance
        return nullptr;
    }
}

std::vector<std::string> AnalysisToolRegistry::get_available_tools() const {
    std::vector<std::string> tool_names;
    tool_names.reserve(factories_.size());
    
    for (const auto& [name, factory] : factories_) {
        tool_names.push_back(name);
    }
    
    return tool_names;
}

bool AnalysisToolRegistry::is_tool_available(const std::string& name) const {
    return factories_.find(name) != factories_.end();
}

void AnalysisToolRegistry::clear() {
    factories_.clear();
}

} // namespace analysis
} // namespace wip