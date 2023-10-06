#include "compile_context.h"
#include "log.h"
#include "utils.h"
#include <llvm-12/llvm/IR/Constant.h>
#include <llvm-12/llvm/IR/GlobalValue.h>
#include <llvm-12/llvm/IR/GlobalVariable.h>

CompileContext::CompileContext() {
	m_context			   = nullptr;
	m_module			   = nullptr;
	m_builder			   = nullptr;
	m_cur_fn			   = nullptr;
	m_cur_fn_is_method	   = false;
	m_method_obj		   = nullptr;
	m_continue_block	   = nullptr;
	m_break_block		   = nullptr;
	m_global_var_name_seed = 0;
}
void CompileContext::Init() {
	// Open a new context and module.
	m_context = new llvm::LLVMContext();
	m_module  = new llvm::Module("generate llvm ir", *m_context);

	// Create a new builder for the module.
	m_builder = new llvm::IRBuilder<>(*m_context);

	// 先写死datalayout
	llvm::Expected<llvm::DataLayout> r	   = llvm::DataLayout::parse("e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128");
	llvm::Error						 error = r.takeError();
	if (error) {
		std::string				 error_msg;
		llvm::raw_string_ostream os_error_msg(error_msg);
		os_error_msg << error;
		printf("failed to parse datalayout: %s\n", error_msg.c_str());
		abort();
	}
	m_module->setDataLayout(r.get());

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

	// 参数类型为数组时, 这条assert无法满足
	//assert(found->second->getType()->getPointerTo() == value->getType()); // 目前只有value替换为ptr的需求
	
	m_named_values[name] = value;
}
bool CompileContext::Block::HasNamedValue(std::string name) const {
	return m_named_values.find(name) != m_named_values.end();
}
llvm::Value* CompileContext::Block::GetNamedValue(std::string name) {
	assert(HasNamedValue(name));
	return m_named_values[name];
}
llvm::Constant* CompileContext::AddGlobalValue(std::string name_prefix, llvm::Type* type, llvm::Constant* initializer) {
	std::string name = name_prefix + to_str(m_global_var_name_seed);
	assert(nullptr == m_module->getNamedGlobal(name));
	llvm::Constant*		  v	 = m_module->getOrInsertGlobal(name, type);
	llvm::GlobalVariable* gv = m_module->getGlobalVariable(name);
	gv->setInitializer(initializer);
	m_global_var_name_seed++;
	return v;
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
