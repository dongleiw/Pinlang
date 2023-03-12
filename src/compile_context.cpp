#include "compile_context.h"
#include "log.h"

CompileContext::CompileContext() {
	m_context		   = nullptr;
	m_module		   = nullptr;
	m_builder		   = nullptr;
	m_cur_fn		   = nullptr;
	m_cur_fn_is_method = false;
	m_method_obj	   = nullptr;
	m_continue_block   = nullptr;
	m_break_block	   = nullptr;
}
void CompileContext::Init() {
	// Open a new context and module.
	m_context = new llvm::LLVMContext();
	m_module  = new llvm::Module("generate llvm ir", *m_context);

	// Create a new builder for the module.
	m_builder = new llvm::IRBuilder<>(*m_context);

	EnterBlock();
}
void CompileContext::AddNamedValue(std::string name, llvm::Value* value) {
	assert(!m_blocks.empty());
	Block* block = *m_blocks.rbegin();
	block->AddNamedValue(name, value);
}
void CompileContext::ReplaceNamedValue(std::string name, llvm::Value* value) {
	assert(!m_blocks.empty());
	for (auto iter = m_blocks.rbegin(); iter != m_blocks.rend(); iter++) {
		Block* block = *iter;
		if (block->HasNamedValue(name)) {
			block->ReplaceNamedValue(name, value);
			return;
		}
	}
	panicf("symbol[%s] not exists", name.c_str());
}
bool CompileContext::HasNamedValue(std::string name) const {
	assert(!m_blocks.empty());
	for (auto iter = m_blocks.rbegin(); iter != m_blocks.rend(); iter++) {
		Block* block = *iter;
		if (block->HasNamedValue(name)) {
			return true;
		}
	}
	return false;
}
llvm::Value* CompileContext::GetNamedValue(std::string name) {
	assert(!m_blocks.empty());
	for (auto iter = m_blocks.rbegin(); iter != m_blocks.rend(); iter++) {
		Block* block = *iter;
		if (block->HasNamedValue(name)) {
			return block->GetNamedValue(name);
		}
	}
	panicf("symbol[%s] not exists", name.c_str());
}
void CompileContext::EnterBlock() {
	m_blocks.push_back(new Block());
}
void CompileContext::LeaveBlock() {
	assert(!m_blocks.empty());
	Block* block = *m_blocks.rbegin();
	m_blocks.pop_back();
	delete block;
}
void CompileContext::Block::AddNamedValue(std::string name, llvm::Value* value) {
	if (m_named_values.find(name) != m_named_values.end()) {
		panicf("duplicate name[%s]", name.c_str());
	}
	m_named_values[name] = value;
}
void CompileContext::Block::ReplaceNamedValue(std::string name, llvm::Value* value) {
	auto found = m_named_values.find(name);
	if (found == m_named_values.end()) {
		panicf("name[%s] not exists", name.c_str());
	}
	assert(found->second->getType()->getPointerTo() == value->getType()); // 目前只有value替换为ptr的需求
	m_named_values[name] = value;
}
bool CompileContext::Block::HasNamedValue(std::string name) const {
	return m_named_values.find(name) != m_named_values.end();
}
llvm::Value* CompileContext::Block::GetNamedValue(std::string name) {
	assert(HasNamedValue(name));
	return m_named_values[name];
}
llvm::Value* CompileResult::GetResult() const {
	return m_compile_result;
}
llvm::Function* CompileResult::GetResultFn() const {
	assert(m_is_fn);
	return (llvm::Function*)(m_compile_result);
}
llvm::Function* CompileResult::GetResultMethod() const {
	assert(m_is_fn && m_thisobj != nullptr);
	return (llvm::Function*)(m_compile_result);
}
llvm::Value* CompileResult::GetThisObj() const {
	assert(m_is_fn && m_thisobj != nullptr);
	return m_thisobj;
}
bool CompileResult::IsMethod() const {
	return m_is_fn && m_thisobj != nullptr;
}
