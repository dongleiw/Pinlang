#include "llvm_ir.h"
#include "log.h"

void LLVMIR::Init() {
	// Open a new context and module.
	m_context = new llvm::LLVMContext();
	m_module  = new llvm::Module("generate llvm ir", *m_context);

	// Create a new builder for the module.
	m_builder = new llvm::IRBuilder<>(*m_context);

	EnterBlock();
}
void LLVMIR::AddNamedValue(std::string name, llvm::Value* value) {
	assert(!m_blocks.empty());
	Block* block = *m_blocks.rbegin();
	block->AddNamedValue(name, value);
}
bool LLVMIR::HasNamedValue(std::string name) const {
	assert(!m_blocks.empty());
	for (auto iter = m_blocks.rbegin(); iter != m_blocks.rend(); iter++) {
		Block* block = *iter;
		if (block->HasNamedValue(name)) {
			return true;
		}
	}
	return false;
}
llvm::Value* LLVMIR::GetNamedValue(std::string name) {
	assert(!m_blocks.empty());
	for (auto iter = m_blocks.rbegin(); iter != m_blocks.rend(); iter++) {
		Block* block = *iter;
		if (block->HasNamedValue(name)) {
			return block->GetNamedValue(name);
		}
	}
	panicf("symbol[%s] not exists", name.c_str());
}
void LLVMIR::EnterBlock() {
	m_blocks.push_back(new Block());
}
void LLVMIR::LeaveBlock() {
	assert(!m_blocks.empty());
	Block* block = *m_blocks.rbegin();
	delete block;
	m_blocks.pop_back();
}
void LLVMIR::Block::AddNamedValue(std::string name, llvm::Value* value) {
	if (m_named_values.find(name) != m_named_values.end()) {
		panicf("duplicate name[%s]", name.c_str());
	}
	m_named_values[name] = value;
}
bool LLVMIR::Block::HasNamedValue(std::string name) const {
	return m_named_values.find(name) != m_named_values.end();
}
llvm::Value* LLVMIR::Block::GetNamedValue(std::string name) {
	assert(HasNamedValue(name));
	return m_named_values[name];
}
